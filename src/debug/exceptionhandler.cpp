/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Custom exception handler.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "exceptionhandler.h"

#include "cdctrl.h"
#include "cpudetect.h"
#include "crc32.h"
#include "debughandler.h"
#include "debughlp.h"
#include "dsurface.h"
#include "minidump.h"
#include "miscutil.h"
#include "resource.h"
#include "stackdump.h"
#include "stringid.h"
#include "syringe.h"
#include "textfile.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tspp_gitinfo.h"
#include "vinifera_gitinfo.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "windialog.h"
#include "winutil.h"

#include <atomic>
#include <cmath>
#include <dbghelp.h>
#include <eh.h>
#include <exception>
#include <mutex>
#include <thread>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>


/**
 *  Declare a side-by-side dependency on Common Controls v6 so the exception
 *  dialog can be rendered with modern visual styles. The linker aggregates
 *  this into the auto-generated manifest (resource ID 2). The dialog binds
 *  against comctl32 v6 via CreateActCtx/ActivateActCtx around the modal
 *  call in Exception_Dialog.
 */
#pragma comment(linker, \
    "\"/manifestdependency:type='win32' " \
    "name='Microsoft.Windows.Common-Controls' " \
    "version='6.0.0.0' " \
    "processorArchitecture='*' " \
    "publicKeyToken='6595b64144ccf1df' " \
    "language='*'\"")


extern HMODULE DLLInstance;
extern DWORD Vinifera_MainThreadId;

extern int Execute_Day;
extern int Execute_Month;
extern int Execute_Year;
extern int Execute_Hour;
extern int Execute_Min;
extern int Execute_Sec;


register_t LastExceptionEIP = 0x0;
uint32_t CurrentExceptionCRC = 0;
uint32_t LastExceptionCRC = 0;

_EXCEPTION_POINTERS *ExceptionInfo = nullptr;

exceptioncallback_ptr_t ExceptionHandlerPtr = nullptr;

std::atomic<bool> AlreadyExiting{false};
std::atomic<bool> ShowExceptionWindow{true};
std::atomic<bool> ExceptionDumpFinished{false};

std::atomic<int> RecursionCount{-1};

/**
 *  Thread currently running the inline (in-filter) dump. A nested fault there
 *  is uncatchable (see DumperRequest), so the handler bails on re-entry from
 *  this thread rather than recursing to the old ExitProcess CTD.
 */
static std::atomic<DWORD> DumpingThreadId{0};

FixedString<65536> ExceptionBuffer;

static TextFileClass ExceptionFile;

static FixedString<1024> ExceptionInfoDescription;


/**
 *  Thread synchronization for the exception handler.
 *
 *  Lock ordering (acquire top→bottom; never reverse):
 *    1. ExceptionCriticalSection (entry gate, recursive)
 *    2. MiniDumpCriticalSection  (inside Create_Mini_Dump)
 *    3. DbgHelp mutex            (inside Sym*, StackWalk64, MiniDumpWriteDump)
 *
 *  The exception path MUST NEVER acquire a game-subsystem lock (audio
 *  manager mutexes, scan-thread mutexes, WinDialog internals, etc.) because
 *  sibling threads are SUSPENDED holding them. Touching one of those locks
 *  deadlocks the dumper.
 */
static CRITICAL_SECTION ExceptionCriticalSection;
static bool ExceptionCriticalSectionReady = false;
static std::atomic<DWORD> FirstCrashThreadId{0};


/**
 *  Dedicated crash-dump thread. The dump reads the crashing thread's corrupt
 *  stack and calls dbghelp, which can fault - and on the crashing thread the
 *  dump runs inside an exception *filter*, where that fault is uncatchable (a
 *  nested exception's search skips frames newer than the active filter).
 *  Running it here gives normal context (so the per-section __try guards work)
 *  and a fresh stack (survives stack overflow).
 *
 *  Protocol: the crasher fills TheDumperRequest, signals DumperRequestEvent,
 *  and waits on DumperDoneEvent; the dumper runs Write_Crash_Artifacts and
 *  signals back. Created once at Vinifera_Post_Init_Game and detached.
 */
struct DumperRequest
{
    CONTEXT             Context;          // copy of *e_info->ContextRecord
    EXCEPTION_RECORD    Record;           // copy of *e_info->ExceptionRecord
    EXCEPTION_POINTERS  Pointers;         // { &Record, &Context }
    unsigned int        ECode;
    DWORD               CrashedThreadId;
    std::atomic<bool>   DumpOK;
};
static DumperRequest TheDumperRequest;

static std::atomic<DWORD> DumperThreadId{0};   // 0 == dumper not available yet
static HANDLE DumperRequestEvent = nullptr;
static HANDLE DumperDoneEvent = nullptr;
static std::atomic<bool> DumperStarted{false};

#define DUMPER_TIMEOUT_MS 60000


/**
 *  Stack reserved by SetThreadStackGuarantee so the SEH filter has room to run
 *  after EXCEPTION_STACK_OVERFLOW (which fires with only ~1 page of stack left).
 *  64 KB comfortably covers the gate + Suspend_Other_Threads + dumper handoff on
 *  the crashing thread; the heavy dump itself runs on the dumper's full stack.
 *  Costs nothing unless used; trims ~64 KB off a 1 MB stack, which is negligible.
 */
static constexpr ULONG VINIFERA_EXCEPTION_STACK_GUARANTEE = 64 * 1024;


/**
 *  Reserve a guaranteed stack region for exception handling on the calling
 *  thread. Must be called per-thread (the guarantee only ever increases). Call
 *  as early as possible on every thread so a stack overflow can
 *  still reach the crash dumper instead of instantly killing the process.
 */
void Vinifera_Reserve_Exception_Stack()
{
    ULONG guarantee = VINIFERA_EXCEPTION_STACK_GUARANTEE;
    SetThreadStackGuarantee(&guarantee); // Ignore failure - best effort.
}


bool Any_Surface_Locked()
{
    return (VisibleSurface && VisibleSurface->Is_Locked())
        || (HiddenSurface && HiddenSurface->Is_Locked())
        || (CompositeSurface && CompositeSurface->Is_Locked())
        || (TileSurface && TileSurface->Is_Locked())
        || (SidebarSurface && SidebarSurface->Is_Locked())
        || (AlternateSurface && AlternateSurface->Is_Locked());
}


void Clear_All_Surfaces()
{
    if (VisibleSurface) VisibleSurface->Clear();
    if (HiddenSurface) HiddenSurface->Clear();
    if (CompositeSurface) CompositeSurface->Clear();
    if (TileSurface) TileSurface->Clear();
    if (SidebarSurface) SidebarSurface->Clear();
    if (AlternateSurface) AlternateSurface->Clear();
}


/**
 *  Suspend every other thread in this process and record their TIDs so a
 *  later Resume_Other_Threads can wake them. Used to capture a frozen
 *  address space for the dump + minidump only; workers are resumed before
 *  the exception dialog runs (otherwise the dialog deadlocks on OS locks
 *  the workers may have been holding).
 *
 *  Issues a GetThreadContext after each SuspendThread as a kernel barrier:
 *  on x86 SuspendThread returns before the target is fully out of user
 *  mode, but GetThreadContext only returns once the thread is parked, so
 *  the dump observes the thread's true register state.
 */
static void Suspend_Other_Threads(std::vector<DWORD>& out_suspended_tids)
{
    const DWORD self_tid = GetCurrentThreadId();
    const DWORD self_pid = GetCurrentProcessId();

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return;
    }

    THREADENTRY32 te{};
    te.dwSize = sizeof(te);

    if (Thread32First(snap, &te)) {
        do {
            if (te.dwSize < FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
                continue;
            }
            /**
             *  Never suspend ourselves or the dedicated dumper thread - a
             *  suspended dumper would deadlock the handoff wait.
             */
            const DWORD dumper_tid = DumperThreadId.load();
            if (te.th32OwnerProcessID != self_pid
                || te.th32ThreadID == self_tid
                || (dumper_tid != 0 && te.th32ThreadID == dumper_tid)) {
                continue;
            }
            HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, te.th32ThreadID);
            if (th == nullptr) {
                continue;
            }
            if (SuspendThread(th) == DWORD(-1)) {
                CloseHandle(th);
                continue;
            }
            CONTEXT ctx{};
            ctx.ContextFlags = CONTEXT_CONTROL;
            GetThreadContext(th, &ctx);
            out_suspended_tids.push_back(te.th32ThreadID);
            CloseHandle(th);
        } while (Thread32Next(snap, &te));
    }

    CloseHandle(snap);
}


/**
 *  Resume every thread we suspended in Suspend_Other_Threads. Tolerates
 *  TIDs that no longer exist (a worker may have re-crashed during the
 *  dialog and parked itself in the FirstCrashThreadId gate).
 */
static void Resume_Other_Threads(const std::vector<DWORD>& suspended_tids)
{
    for (DWORD tid : suspended_tids) {
        HANDLE th = OpenThread(THREAD_SUSPEND_RESUME, FALSE, tid);
        if (th == nullptr) {
            continue;
        }
        ResumeThread(th);
        CloseHandle(th);
    }
}


/**
 *  Single centralized process-kill primitive used by every dialog exit
 *  path. TerminateProcess bypasses DLL_DETACH and per-thread cleanup,
 *  neither of which can run safely after a crash with the game in an
 *  inconsistent state.
 */
[[noreturn]] static void Terminate_Now()
{
    TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
    __assume(0);
}


/**
 *  Top-level SEH filter. Process-wide and per-thread entry points both
 *  forward here.
 */
LONG __stdcall _Top_Level_Exception_Filter(EXCEPTION_POINTERS *e_info)
{
    return Vinifera_Exception_Handler(e_info->ExceptionRecord->ExceptionCode, e_info);
}


/**
 *  C++ -> SEH translator installed by _set_se_translator. Per-thread; see
 *  vinifera_thread.h for worker installation.
 */
void __cdecl _Structured_Exception_Translator(unsigned int code, EXCEPTION_POINTERS *e_info)
{
    Vinifera_Exception_Handler(code, e_info);
}


/**
 *  Terminate handler for escaped C++ exceptions. Re-raises as a structured
 *  exception so the SEH machinery delivers proper EXCEPTION_POINTERS to
 *  _Top_Level_Exception_Filter via SetUnhandledExceptionFilter.
 */
[[noreturn]] void __cdecl Vinifera_Terminate_Handler()
{
    RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, nullptr);
    ExitProcess(EXIT_FAILURE);
}


/**
 *  Initialize and tear down the entry-gate critical section. Called from
 *  DllMain before any hook can fire / after all hooks are removed.
 */
void Init_Exception_Handler()
{
    if (!ExceptionCriticalSectionReady) {
        InitializeCriticalSection(&ExceptionCriticalSection);
        ExceptionCriticalSectionReady = true;
    }
}


void Shutdown_Exception_Handler()
{
    if (ExceptionCriticalSectionReady) {
        DeleteCriticalSection(&ExceptionCriticalSection);
        ExceptionCriticalSectionReady = false;
    }
}


/**
 *  Finds the first instance of the address in the loaded database and extracts its description.
 *  CanContinue and Ignore on the struct are part of the EDB binary format and are not consulted.
 */
static bool Exception_Find_Datbase_Entry(uintptr_t address, FixedString<1024> &desc)
{
    for (int i = 0; i < ExceptionInfoDatabase.Count(); ++i) {
        if (ExceptionInfoDatabase[i].Address == address) {
            desc = ExceptionInfoDatabase[i].Description;
            return true;
        }
    }
    return false;
}


/**
 *  Number of code bytes to record from EIP register.
 */
#define NUM_CODE_BYTES 32


/**
 *  Macros for printing address formats.
 */
#define PRIPTRSIZE "08"
#define UNKNOWN_MEMORY_AREA "????????"


/**
 *  Constants that control the level of info to output.
 */
#define EXCEPTION_STACK_SYMNAME_MAX 128
#define EXCEPTION_STACK_DEPTH_MAX 1024
#define EXCEPTION_STACK_COLUMNS 8 // Number of columns in stack dump.


/**
 *  Exception codes not defined in the Windows headers.
 */
#define MS_VC_EXCEPTION 0x406D1388 // Visual Studio debugger code.
#define MS_UNHANDLED_CPP_EXCEPTION 0xE06D7363 // Visual Studio unhandled exception.


static const char *ExceptionText[] = {
    "Error code: EXCEPTION_ACCESS_VIOLATION\r\r\nDescription: The thread tried to read from or write to a virtual address for which it does not have the appropriate access.",
    "Error code: EXCEPTION_DATATYPE_MISALIGNMENT\r\r\nDescription: The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.",
    "Error code: EXCEPTION_BREAKPOINT\r\r\nDescription: A breakpoint was encountered.",
    "Error code: EXCEPTION_SINGLE_STEP\r\r\nDescription: A trace trap or other single-instruction mechanism signaled that one instruction has been executed.",
    "Error code: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\r\r\nDescription: The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.",
    "Error code: EXCEPTION_FLT_DENORMAL_OPERAND\r\r\nDescription: One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.",
    "Error code: EXCEPTION_FLT_DIVIDE_BY_ZERO\r\r\nDescription: The thread tried to divide a floating-point value by a floating-point divisor of zero.",
    "Error code: EXCEPTION_FLT_INEXACT_RESULT\r\r\nDescription: The result of a floating-point operation cannot be represented exactly as a decimal fraction.",
    "Error code: EXCEPTION_FLT_INVALID_OPERATION\r\r\nDescription: Some strange unknown floating point operation was attempted.",
    "Error code: EXCEPTION_FLT_OVERFLOW\r\r\nDescription: The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.",
    "Error code: EXCEPTION_FLT_STACK_CHECK\r\r\nDescription: The stack overflowed or underflowed as the result of a floating-point operation.",
    "Error code: EXCEPTION_FLT_UNDERFLOW\r\r\nDescription:\tThe exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.",
    "Error code: EXCEPTION_INT_DIVIDE_BY_ZERO\r\r\nDescription: The thread tried to divide an integer value by an integer divisor of zero.",
    "Error code: EXCEPTION_INT_OVERFLOW\r\r\nDescription: The result of an integer operation caused a carry out of the most significant bit of the result.",
    "Error code: EXCEPTION_PRIV_INSTRUCTION\r\r\nDescription: The thread tried to execute an instruction whose operation is not allowed in the current machine mode.",
    "Error code: EXCEPTION_IN_PAGE_ERROR\r\r\nDescription: The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.",
    "Error code: EXCEPTION_ILLEGAL_INSTRUCTION\r\r\nDescription:\tThe thread tried to execute an invalid instruction.",
    "Error code: EXCEPTION_NONCONTINUABLE_EXCEPTION\r\r\nDescription: The thread tried to continue execution after a non-continuable exception occurred.",
    "Error code: EXCEPTION_STACK_OVERFLOW\r\r\nDescription: The thread used up its stack.",
    "Error code: EXCEPTION_INVALID_DISPOSITION\r\r\nDescription: An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.",
    "Error code: EXCEPTION_GUARD_PAGE\r\r\nDescription: The thread accessed memory allocated with the PAGE_GUARD modifier.",
    "Error code: EXCEPTION_INVALID_HANDLE\r\r\nDescription: The thread used a handle to a kernel object that was invalid (probably because it had been closed.)",
#if defined(EXCEPTION_POSSIBLE_DEADLOCK) && defined(STATUS_POSSIBLE_DEADLOCK) // This type seems to be non-existent in practice.
    "Error code: EXCEPTION_POSSIBLE_DEADLOCK\r\r\nDescription: The wait operation on the critical section timed out.",
#endif
    "Error code: CONTROL_C_EXIT\r\r\nDescription: The application terminated as a result of a CTRL+C.",
    "Error code: " UNKNOWN_MEMORY_AREA "\r\r\nDescription: Unknown exception."
};

static uint32_t ExceptionCodes[] = {
    EXCEPTION_ACCESS_VIOLATION,
    EXCEPTION_DATATYPE_MISALIGNMENT,
    EXCEPTION_BREAKPOINT,
    EXCEPTION_SINGLE_STEP,
    EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
    EXCEPTION_FLT_DENORMAL_OPERAND,
    EXCEPTION_FLT_DIVIDE_BY_ZERO,
    EXCEPTION_FLT_INEXACT_RESULT,
    EXCEPTION_FLT_INVALID_OPERATION,
    EXCEPTION_FLT_OVERFLOW,
    EXCEPTION_FLT_STACK_CHECK,
    EXCEPTION_FLT_UNDERFLOW,
    EXCEPTION_INT_DIVIDE_BY_ZERO,
    EXCEPTION_INT_OVERFLOW,
    EXCEPTION_PRIV_INSTRUCTION,
    EXCEPTION_IN_PAGE_ERROR,
    EXCEPTION_ILLEGAL_INSTRUCTION,
    EXCEPTION_NONCONTINUABLE_EXCEPTION,
    EXCEPTION_STACK_OVERFLOW,
    EXCEPTION_INVALID_DISPOSITION,
    EXCEPTION_GUARD_PAGE,
    EXCEPTION_INVALID_HANDLE,
#if defined(EXCEPTION_POSSIBLE_DEADLOCK) && defined(STATUS_POSSIBLE_DEADLOCK) // This type seems to be non-existant in practice.
    EXCEPTION_POSSIBLE_DEADLOCK,
#endif
    CONTROL_C_EXIT,
    uint32_t(-1)
};


/**
 *  Append line to the exception print buffer.
 */
static void Exception_Printf(const char *buffer, ...)
{
    char scratch[2048];
    ZeroMemory(scratch, sizeof(scratch));

    va_list args;
    va_start(args, buffer);

    std::vsnprintf(scratch, sizeof(scratch), buffer, args);
    ExceptionBuffer += scratch;

    va_end(args);
}


/**
 *  Callback for the stack walker, appends line to the exception print buffer.
 */
static void __cdecl Exception_Stack_Dump_Handler(const char *buffer)
{
    Exception_Printf(buffer);
}


/**
 *  Interpret an 80-bit x87 register (10 bytes, little-endian) as a double.
 *  FloatSave.RegisterArea holds 80-bit extended-precision values (64-bit
 *  mantissa, 15-bit exponent, sign - no implicit bit), so they must be
 *  decoded rather than reinterpreted as a 64-bit IEEE-754 double.
 */
static double Read_x87_Register(const uint8_t *bytes)
{
    const uint64_t mantissa = *reinterpret_cast<const uint64_t *>(bytes);
    const uint16_t sign_exp = *reinterpret_cast<const uint16_t *>(bytes + 8);
    const double sign = (sign_exp & 0x8000) ? -1.0 : 1.0;
    const int exponent = sign_exp & 0x7FFF;

    if (exponent == 0 && mantissa == 0) {
        return 0.0;
    }

    if (exponent == 0x7FFF) {
        return sign * HUGE_VAL; // Infinity or NaN.
    }

    return sign * std::ldexp(static_cast<double>(mantissa), exponent - 16383 - 63);
}


/**
 *  Each dbghelp-touching section of the dump runs in its own lock-free __try
 *  helper, so a fault in one is contained and the rest still writes. (These
 *  only catch off-filter, on the dumper thread; inline, a fault hits the
 *  DumpingThreadId latch instead.) Per C2712 a __try function can't own
 *  unwinding objects, so they use C-style locals and raw DbgHelpMutex
 *  lock()/unlock(), not std::scoped_lock.
 */

/**
 *  Resolve and print the crashing instruction's function/file/line.
 */
static void Guarded_Crash_Site(CONTEXT *context)
{
    __try {
        static char filename[512];
        static char funcname[PATH_MAX];

        uintptr_t addr;
        unsigned line;

        Get_Function_Details(reinterpret_cast<void*>(context->Eip), funcname, filename, &line, &addr);

        if (addr != -1) {
            addr = context->Eip - addr;
        }

        Exception_Printf("Exception occurred at 0x%" PRIPTRSIZE PRIXPTR " (%s +0x%" PRIXPTR ") [%s:%d]\r\n", context->Eip, funcname, addr, filename, line);

        if (SyringeData::LastHookOrigin != nullptr) {
            Exception_Printf("Last entered hook at address: 0x%" PRIPTRSIZE PRIXPTR "\r\n", reinterpret_cast<register_t>(SyringeData::LastHookOrigin));
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Exception_Printf("Exception occurred at 0x%" PRIPTRSIZE PRIXPTR " <symbol lookup faulted>\r\n", context->Eip);
    }
}


/**
 *  Symbol-free backtrace by walking the EBP frame chain, each frame validated
 *  before deref. Needs no dbghelp, so it yields a usable return-address list
 *  even when the dbghelp walk below dies entirely.
 */
static void Guarded_Manual_Backtrace(CONTEXT *context)
{
    __try {
        Exception_Printf("Raw EBP-chain backtrace:\r\n");

        uintptr_t *frame = reinterpret_cast<uintptr_t *>(context->Ebp);
        uintptr_t prev = 0;

        for (int i = 0; i < EXCEPTION_STACK_DEPTH_MAX; ++i) {

            /**
             *  A valid frame must be readable as a [saved-EBP, return-address]
             *  pair and strictly increasing (the x86 stack grows downward, so
             *  each saved EBP is at a higher address than the last).
             */
            if (frame == nullptr
                || reinterpret_cast<uintptr_t>(frame) <= prev
                || IsBadReadPtr(frame, 2 * sizeof(uintptr_t))) {
                break;
            }

            Exception_Printf("  0x%" PRIPTRSIZE PRIXPTR "\r\n", frame[1]);

            prev = reinterpret_cast<uintptr_t>(frame);
            frame = reinterpret_cast<uintptr_t *>(frame[0]);
        }

        Exception_Printf("\r\n");
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Exception_Printf("  <manual backtrace faulted>\r\n");
    }
}


/**
 *  Full dbghelp call-stack walk (StackWalk64 + symbol resolution). The #1
 *  faulter on a corrupt stack. Lock-free here - Make_Stack_Trace locks
 *  DbgHelpMutex internally.
 */
static void Guarded_Call_Stack(CONTEXT *context)
{
    __try {
        /**
         *  The EIP/ESP/EBP from EXCEPTION_POINTERS point at the crashing
         *  instruction itself - the first reported frame should be that
         *  instruction, so no frames are skipped.
         */
        Stack_Dump_From_Context(context->Eip, context->Esp, context->Ebp, Exception_Stack_Dump_Handler, 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Exception_Printf("  <call-stack walk faulted (corrupt stack or symbols); skipped>\r\n");
    }
}


/**
 *  Raw scan of stack memory from ESP: print each slot and, when it looks like
 *  a code pointer, resolve a symbol. SymGetSymFromAddr64 can fault inside
 *  dbghelp even when IsBadCodePtr passes, so the whole scan is guarded.
 */
static void Guarded_Raw_Stack_Scan(CONTEXT *context)
{
    /**
     *  Lock once for the whole scan. Raw lock/unlock (not scoped_lock) so this
     *  function can own the __try (C2712); recursive_mutex makes a missed
     *  unlock on a caught fault harmless.
     */
    DbgHelpMutex.lock();

    __try {

        uintptr_t *address = reinterpret_cast<uintptr_t *>(context->Esp);

        char symbol_buffer[sizeof(IMAGEHLP_SYMBOL64)+EXCEPTION_STACK_SYMNAME_MAX];
        IMAGEHLP_SYMBOL64 *const symbol_bufferp = reinterpret_cast<IMAGEHLP_SYMBOL64 *>(symbol_buffer);

        for (int frame = 0; frame < EXCEPTION_STACK_DEPTH_MAX; ++frame) {

            /**
             *  If we can't read the address, then we don't know where we are.
             */
            if (IsBadReadPtr(address, sizeof(uintptr_t))) {
                Exception_Printf("%" PRIPTRSIZE PRIXPTR ": " UNKNOWN_MEMORY_AREA "\r\n", (uintptr_t)address);
                ++address;
                continue;
            }

            /**
             *  If we aren't in code, then we don't know where we are.
             *
             *  #WARNING: From Microsoft Docs.
             *  If the application is compiled as a debugging version, and the process does not
             *  have read access to the specified memory location, the function causes an assertion
             *  and breaks into the debugger. Leaving the debugger, the function continues as
             *  usual, and returns a nonzero value. This behavior is by design, as a debugging aid.
             */
#ifndef NDEBUG
            if (!IsDebuggerPresent()) {
#endif
            if (IsBadCodePtr(reinterpret_cast<FARPROC>(*address))) {
                Exception_Printf("%" PRIPTRSIZE PRIXPTR ": %" PRIPTRSIZE PRIXPTR " DATA_PTR\r\n", (uintptr_t)address, *address);
                ++address;
                continue;
            }
#ifndef NDEBUG
            }
#endif

            /**
             *  Looks like a good address, try and find the debug symbol name for it.
             */
            Exception_Printf("%" PRIPTRSIZE PRIXPTR ": %" PRIPTRSIZE PRIXPTR "", (uintptr_t)address, *address);

            if (SymbolInit) {

                ZeroMemory(symbol_buffer, sizeof(symbol_buffer));
                symbol_bufferp->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64)+EXCEPTION_STACK_SYMNAME_MAX;
                symbol_bufferp->MaxNameLength = EXCEPTION_STACK_SYMNAME_MAX-1;
                symbol_bufferp->Address = *address; // Fetch address (to code) located at ESP address.
                symbol_bufferp->Size = 0;

                DWORD64 displacement = 0;
                HANDLE process = SymbolProcess; //GetCurrentProcess();

                /**
                 *  Translate the current address into a symbol and byte offset (displacement) from the symbol.
                 */
                BOOL got_it = SymGetSymFromAddr64(process, symbol_bufferp->Address, &displacement, symbol_bufferp);
                if (got_it) {
                    Exception_Printf(" - %s(); + %" PRIPTRSIZE PRIXPTR "", symbol_bufferp->Name, displacement);
                }

            } else {

                /**
                 *  Debug symbols not available.
                 */
                Exception_Printf(" *");
            }

            Exception_Printf("\r\n");

            ++address;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Exception_Printf("  <raw stack scan faulted; truncated>\r\n");
    }

    DbgHelpMutex.unlock();
}


static void Dump_Exception_Info(unsigned int e_code, struct _EXCEPTION_POINTERS *e_info)
{
    static char scratch[1024]; // Scratch buffer, to use for anything.
    ZeroMemory(scratch, sizeof(scratch));

    /**
     *  Clear the buffer just in case we did a previous dump (like in a recursive situation).
     */
    ExceptionBuffer.clear();

    Init_Symbol_Info();

    EXCEPTION_RECORD *record = e_info->ExceptionRecord;
    CONTEXT *context = e_info->ContextRecord;

    if (e_code == EXCEPTION_ACCESS_VIOLATION) {
        //Exception_Printf("Exception is access violation\r\n");
        DEBUG_WARNING("Exception is access violation\n");
    } else {
        //Exception_Printf("Exception code is %x\r\n", e_code);
        DEBUG_WARNING("Exception code is {:x}\n", e_code);
    }

    const char *the_exception_desc = "UNKNOWN EXCEPTION";
    uint32_t the_exception_code = uint32_t(-1);

    for (int i = 0; i < std::size(ExceptionText); ++i) {
        if (e_info->ExceptionRecord->ExceptionCode == ExceptionCodes[i]) {
            DEBUG_WARNING("Found exception description.\n");
            the_exception_desc = ExceptionText[i];
            the_exception_code = ExceptionCodes[i];
            break;
        }
    }

    //WWASSERT(the_exception_desc != nullptr);

    DEBUG_WARNING("{}\n", the_exception_desc);

    DEBUG_WARNING("Dump exception info...\n");

    DEBUG_WARNING("**************************** START EXCEPTION DUMP *****************************!\n");

    Exception_Printf("%s\r\n", the_exception_desc);

    switch (e_code) {

        case EXCEPTION_STACK_OVERFLOW:
            DEBUG_WARNING("Exception is stack overflow!\n");
            break;

        case EXCEPTION_ACCESS_VIOLATION:
            DEBUG_WARNING("Exception is access violation\n");
            break;

        case EXCEPTION_IN_PAGE_ERROR:
            DEBUG_WARNING("Exception is page fault\n");
            break;

        default:
            DEBUG_WARNING("Exception code is 0x%" PRIPTRSIZE PRIXPTR "\n", e_code);
            break;
    }

    if (e_code == EXCEPTION_ACCESS_VIOLATION) {

        /**
         *  This checks the kind of access violation.
         */
        switch (record->ExceptionInformation[0]) {
            case 0: // Read violation
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " was read from.\r\n", record->ExceptionInformation[1]);
                break;
            case 1: // Write violation
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " was written to.\r\n", record->ExceptionInformation[1]);
                break;
            case 2: // Execute violation
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " was executed.\r\n", record->ExceptionInformation[1]);
                break;
            case 8: // User-mode data execution prevention (DEP).
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " DEP violation.\r\n", record->ExceptionInformation[1]);
                break;
            default: // Unknown
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " Unknown violation.\r\n", record->ExceptionInformation[1]);
                break;
        }
    }

    Guarded_Crash_Site(context);

    Exception_Printf("\r\n");

    /**
     *  Has additional info for this EIP been loaded from the exception database?
     */
    if (!ExceptionInfoDescription.empty()) {
        Exception_Printf("Additional Information:\r\n");
        DEBUG_WARNING("\r\nAdditional Information:\n");
        Exception_Printf("  %s\r\n", ExceptionInfoDescription.c_str());
        DEBUG_WARNING("  {}\n\n", ExceptionInfoDescription);
        Exception_Printf("\r\n");
    }

    DEBUG_WARNING("Call dump...\n");

    /**
     *  A symbol-free EBP-chain backtrace first (always works, even if dbghelp
     *  dies), then the full dbghelp call-stack walk. Both are individually
     *  guarded so a fault in either doesn't cost us the rest of the dump.
     */
    Guarded_Manual_Backtrace(context);
    Guarded_Call_Stack(context);

    Exception_Printf("\r\n");

    Exception_Printf("Time Stamp : %s\r\n", Get_Date_Time_String());
    Exception_Printf("Module Name : %s\r\n", Get_Module_File_Name_Ext());
    
    Exception_Printf("\r\n");

    Exception_Printf("Project information:\r\n");
    if (Vinifera_ProjectName[0] != '\0') {
        Exception_Printf("Title: %s\r\n", Vinifera_ProjectName);
        Exception_Printf("Version: %s\r\n", Vinifera_ProjectVersion);
        Exception_Printf("\r\n");
    }

    Exception_Printf("Application : %s (%s)\r\n", VINIFERA_PROJECT_NAME, VINIFERA_DLL);
    //Exception_Printf("Version : %s\r\n", VerNum.Version_Name());

    Exception_Printf("Build Type : %s\r\n", Vinifera_Build_Type_String());

    Exception_Printf("TS++ commit author: %s\r\n", TSPP_Git_Author());
    Exception_Printf("TS++ commit date: %s\r\n", TSPP_Git_DateTime());
    Exception_Printf("TS++ commit branch: %s\r\n", "master"); // TSPP_Git_Branch());
    Exception_Printf("TS++ commit hash: %s\r\n", TSPP_Git_Hash_Short());
    Exception_Printf("TS++ local changes: %s\r\n", TSPP_Git_Uncommitted_Changes() ? "YES" : "NO");

    Exception_Printf("Vinifera commit author: %s\r\n", Vinifera_Git_Author());
    Exception_Printf("Vinifera commit date: %s\r\n", Vinifera_Git_DateTime());
    Exception_Printf("Vinifera commit branch: %s\r\n", Vinifera_Git_Branch());
    Exception_Printf("Vinifera commit hash: %s\r\n", Vinifera_Git_Hash_Short());
    Exception_Printf("Vinifera local changes: %s\r\n", Vinifera_Git_Uncommitted_Changes() ? "YES" : "NO");

    Exception_Printf("\r\n");

    //Exception_Printf("New Count: %s\r\n", Vinifera_New_Count);
    //Exception_Printf("Delete Count: %s\r\n", Vinifera_Delete_Count);

    //Exception_Printf("\r\n");

    /**
     *  Log System information.
     */   
    Exception_Printf("System information:\r\n");
    Exception_Printf(CPUDetectClass::Get_Processor_Log());
    //Exception_Printf("\r\n"); // Get_Processor_Log writes a new line for us.

    DEBUG_WARNING("Register dump...\n");
    Exception_Printf("Details:\r\n");

    Exception_Printf("Eip:%" PRIPTRSIZE PRIXPTR "\tEsp:%" PRIPTRSIZE PRIXPTR "\tEbp:%" PRIPTRSIZE PRIXPTR "\r\n",
        context->Eip,
        context->Esp,
        context->Ebp);

    Exception_Printf("Eax:%" PRIPTRSIZE PRIXPTR "\tEbx:%" PRIPTRSIZE PRIXPTR "\tEcx:%" PRIPTRSIZE PRIXPTR "\r\n",
        context->Eax,
        context->Ebx,
        context->Ecx);

    Exception_Printf("Edx:%" PRIPTRSIZE PRIXPTR "\tEsi:%" PRIPTRSIZE PRIXPTR "\tEdi:%" PRIPTRSIZE PRIXPTR "\r\n",
        context->Edx,
        context->Esi,
        context->Edi);

    Exception_Printf("EFlags:%08X \r\n", context->EFlags);

    Exception_Printf("CS:%04x  SS:%04x  DS:%04x  ES:%04x  FS:%04x  GS:%04x\r\n",
        context->SegCs,
        context->SegSs,
        context->SegDs,
        context->SegEs,
        context->SegFs,
        context->SegGs);

    Exception_Printf("\r\nFloating point status:\r\n");

    Exception_Printf("     Control word: %08x\r\n", context->FloatSave.ControlWord);
    Exception_Printf("      Status word: %08x\r\n", context->FloatSave.StatusWord);
    Exception_Printf("         Tag word: %08x\r\n", context->FloatSave.TagWord);
    Exception_Printf("     Error Offset: %08x\r\n", context->FloatSave.ErrorOffset);
    Exception_Printf("   Error Selector: %08x\r\n", context->FloatSave.ErrorSelector);
    Exception_Printf("      Data Offset: %08x\r\n", context->FloatSave.DataOffset);
    Exception_Printf("    Data Selector: %08x\r\n", context->FloatSave.DataSelector);
    Exception_Printf("      Cr0NpxState: %08x\r\n", context->FloatSave.Spare0);
    //Exception_Printf("      NpxSavedCpu: %08x\r\n", context->FloatSave.NpxSavedCpu);

    /**
     *  128-bit SSE Registers (64-bit x86 Only).
     */
    if (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)) {
        //Exception_Printf("Xmm0:%016llX\tXmm1:%016llX\tXmm2:%016llX\tXmm3:%016llX\r\n", uint64_t(context->Xmm0), uint64_t(context->Xmm1), uint64_t(context->Xmm2), uint64_t(context->Xmm3));
        //Exception_Printf("Xmm4:%016llX\tXmm5:%016llX\tXmm6:%016llX\tXmm7:%016llX\r\n", uint64_t(context->Xmm4), uint64_t(context->Xmm5), uint64_t(context->Xmm6), uint64_t(context->Xmm7));
        //Exception_Printf("Xmm8:%016llX\tXmm9:%016llX\tXmm10:%016llX\tXmm11:%016llX\r\n", uint64_t(context->Xmm8), uint64_t(context->Xmm9), uint64_t(context->Xmm10), uint64_t(context->Xmm11));
        //Exception_Printf("Xmm12:%016llX\tXmm13:%016llX\tXmm14:%016llX\tXmm15:%016llX\r\n", uint64_t(context->Xmm12), uint64_t(context->Xmm13), uint64_t(context->Xmm14), uint64_t(context->Xmm15));
    }

    for (int i = 0; i < EXCEPTION_STACK_COLUMNS; ++i) {
        Exception_Printf("ST%d : ", i);

        for (int j = 0; j < 10; ++j) {
            Exception_Printf("%02X", context->FloatSave.RegisterArea[i * 10 + j]);
        }

        Exception_Printf("   %+#.17e\r\n", Read_x87_Register(&context->FloatSave.RegisterArea[i * 10]));
    }

    /**
     *  MMX Registers.
     */
    if ((context->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS
        && IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE)) {

        /**
         *  MM0-MM7 alias the ST register mantissas and live in the FXSAVE area
         *  (ExtendedRegisters) at offset 32, 16 bytes apart - not in the first
         *  bytes of the area, which hold the control/status words.
         */
        const uint8_t *mmx = &context->ExtendedRegisters[32];
        Exception_Printf("MMX0:%016llX\tMMX1:%016llX\tMMX2:%016llX\tMMX3:%016llX\r\n",
            *reinterpret_cast<const uint64_t *>(mmx + 0 * 16), *reinterpret_cast<const uint64_t *>(mmx + 1 * 16),
            *reinterpret_cast<const uint64_t *>(mmx + 2 * 16), *reinterpret_cast<const uint64_t *>(mmx + 3 * 16));
        Exception_Printf("MMX4:%016llX\tMMX5:%016llX\tMMX6:%016llX\tMMX7:%016llX\r\n",
            *reinterpret_cast<const uint64_t *>(mmx + 4 * 16), *reinterpret_cast<const uint64_t *>(mmx + 5 * 16),
            *reinterpret_cast<const uint64_t *>(mmx + 6 * 16), *reinterpret_cast<const uint64_t *>(mmx + 7 * 16));
    }

    /**
     *  Debug Registers.
     */
    Exception_Printf("Dr0:%016llX\tDr1:%016llX\tDr2:%016llX\tDr3:%016llX\r\n", context->Dr0, context->Dr1, context->Dr2, context->Dr3);

    /**
     *  DR4 and DR5 are reserved and are obsolete synonyms for DR6 and DR7, see
     *  https://en.wikipedia.org/wiki/X86_debug_register.
     *  But we will log them anyhow.
     * 
     *  #NOTE: x86 context does not contain DR4 and DR5, so we just print them as OBSOLETE.
     */
    Exception_Printf("Dr4:    OBSOLETE    \tDr5:    OBSOLETE    \tDr6:%08X\t\t\tDr7:%08X\r\n", /*context->Dr4, context->Dr5,*/ context->Dr6, context->Dr7);

    /**
     *  Dump bytes at the instruction pointer.
     */
    DEBUG_WARNING("EIP bytes dump...\n");
    Exception_Printf("\r\nBytes at CS:EIP (%" PRIPTRSIZE PRIXPTR ")  : ", context->Eip);
    uint8_t *eip_pointer = reinterpret_cast<uint8_t *>(context->Eip);

    /**
     *  Print out the bytes of code at the instruction pointer. Since the
     *  crash may have been caused by an instruction pointer that was bad,
     *  this code needs to be wrapped in an exception handler, in case there
     *  is no memory to read. If the dereferencing of code[] fails, the
     *  exception handler will print '??'.
     */
    for (int i = NUM_CODE_BYTES; i > 0; --i) {
        if (IsBadReadPtr(eip_pointer, sizeof(uint8_t))) {
            Exception_Printf("?? ");
        } else {
            Exception_Printf("%02X ", (uintptr_t)*eip_pointer);
        }
        ++eip_pointer;
    }

    Exception_Printf("\r\n");
    
    DEBUG_WARNING("Stack dump...\n");

    if (SymbolInit) {
        Exception_Printf("\r\nStack dump (* indicates possible code address) :\r\n");
    } else {
        Exception_Printf("\r\nStack dump :\r\n");
    }

    Guarded_Raw_Stack_Scan(context);

    /**
     *  Flag that we have finished so functions who use the buffer outside
     *  of this function know it is safe to do so.
     */
    ExceptionDumpFinished = true;

    /**
     *  Store the EIP value for checking recursive exceptions.
     */
    LastExceptionEIP = static_cast<register_t>(context->Eip);

    /**
     *  Calculate unique crc for the exception data (used for checking recursive exceptions).
     */
    CurrentExceptionCRC = CRC32_Memory(ExceptionBuffer.data(), ExceptionBuffer.size());

    DEBUG_WARNING("****************************** END EXEPTION DUMP ******************************!\n");
}


static INT_PTR CALLBACK Exception_Dialog_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // FALSE == We are not interested in the message. Let dialog manager do any default processing.
    INT_PTR result = FALSE;

    switch (uMsg) {
        case WM_MOVING:
            result = On_WM_MOVING(hDlg, wParam, lParam);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_EXCEPTION_QUIT: // Quit button
                    EndDialog(hDlg, IDC_EXCEPTION_QUIT);
                    result = TRUE;
                    break;

                case IDC_EXCEPTION_DEBUG: // Debug button
                    EndDialog(hDlg, IDC_EXCEPTION_DEBUG);
                    result = TRUE;
                    break;

                default:
                    result = FALSE;
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, IDC_EXCEPTION_QUIT);
            result = TRUE;
            break;

        case WM_INITDIALOG: {

            /**
             *  Render the log edit box in a monospaced font so stack
             *  addresses and register columns line up. Font handle is
             *  intentionally leaked — the process terminates right after
             *  this dialog so cleanup is unnecessary.
             */
            HDC hdc = GetDC(hDlg);
            const int log_font_height = -MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);
            ReleaseDC(hDlg, hdc);
            HFONT log_font = CreateFontA(
                log_font_height, 0, 0, 0,
                FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                FIXED_PITCH | FF_MODERN,
                "Consolas");
            if (log_font != nullptr) {
                SendDlgItemMessageA(hDlg, IDC_EXCEPTION_LOG, WM_SETFONT, (WPARAM)log_font, MAKELPARAM(FALSE, 0));
            }

            /**
             *  Send the exception buffer to the dialog.
             */
            if (ExceptionDumpFinished) {
                SetDlgItemTextA(hDlg, IDC_EXCEPTION_LOG, ExceptionBuffer.c_str()); // Debug edit box.
            }

            if (MainWindow != nullptr) {
                WinDialogClass::Center_Window_Within(hDlg, MainWindow);
            }

            ShowWindow(hDlg, SW_SHOWNORMAL);

            /**
             *  Focus the Quit button so Enter takes the safe action and we
             *  tell the dialog manager we set focus ourselves (return FALSE).
             */
            SetFocus(GetDlgItem(hDlg, IDC_EXCEPTION_QUIT));
            result = FALSE;
            break;
        }

        default:
            result = FALSE;
            break;
    }

    return result;
}


static INT_PTR Exception_Dialog(HWND parent)
{
    switch (RecursionCount.load()) {
        case 1:
            CDControl.Unlock_All_CD_Trays();
            DEBUG_ERROR("Recursive exception detected!\n");
            MessageBox(nullptr, "Recursive exception detected!\n", "Error!", MB_OK|MB_ICONEXCLAMATION);
            Sleep(1000); // was 4000
            return IDC_EXCEPTION_QUIT; // Quit button

        case 2:
            CDControl.Unlock_All_CD_Trays();
            return IDC_EXCEPTION_QUIT; // Quit button

        case 3:
            CDControl.Unlock_All_CD_Trays();
            ExitProcess(EXIT_SUCCESS);
            break; //return IDC_EXCEPTION_QUIT; // Quit button

        default:
            break;
    }

    HMODULE hResHandle = DLLInstance;
    const char *resId = MAKEINTRESOURCE(IDD_EXCEPTION);

    /**
     *  In rare cases, the DLL can be detached before the exception handler
     *  has time to process the exception. In the event of this, use the original
     *  exception dialog.
     */
    if (!hResHandle) {
        hResHandle = ProgramInstance;
        resId = MAKEINTRESOURCE(222);
    }

    DWORD retval = 0;
    //HGLOBAL hGlobalDlg = Fetch_Resource(MAKEINTRESOURCE(IDD_EXCEPTION), RT_DIALOG);
    HGLOBAL hGlobalDlg = FETCH_RESOURCE(hResHandle, resId, RT_DIALOG);
    if (hGlobalDlg != nullptr) {

        /**
         *  Activate the side-by-side manifest embedded in our DLL (resource
         *  ID 2 / RT_MANIFEST) so the dialog binds against comctl32 v6 and
         *  draws with modern visual styles. GAME.EXE has no manifest of its
         *  own, so without this activation the dialog would use classic
         *  Win9x-style chrome.
         */
        char module_path[MAX_PATH] = {};
        GetModuleFileNameA(DLLInstance, module_path, MAX_PATH);

        ACTCTXA actctx{};
        actctx.cbSize = sizeof(actctx);
        actctx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
        actctx.lpSource = module_path;
        actctx.lpResourceName = MAKEINTRESOURCEA(2); // ISOLATIONAWARE_MANIFEST_RESOURCE_ID

        HANDLE hctx = CreateActCtxA(&actctx);
        ULONG_PTR cookie = 0;
        bool activated = false;
        if (hctx != INVALID_HANDLE_VALUE) {
            activated = ActivateActCtx(hctx, &cookie) != FALSE;
        }

        retval = DialogBoxIndirectParam(ProgramInstance, (LPDLGTEMPLATE)hGlobalDlg, parent, (DLGPROC)Exception_Dialog_Proc, (LPARAM)0);

        if (activated) {
            DeactivateActCtx(0, cookie);
        }
        if (hctx != INVALID_HANDLE_VALUE) {
            ReleaseActCtx(hctx);
        }
    } else {
        DEBUG_ERROR("Unable to find the exception dialog resource!\n");
    }

    CDControl.Unlock_All_CD_Trays();

    return retval;
}


/**
 *  Write the on-disk artifacts: minidump first (independent of the fragile
 *  walk), then the text dump, then the EXCEPT_*.TXT. Shared by the dumper
 *  thread and the inline fallback. crashed_tid lets the minidump record the
 *  faulting thread even when this runs on the dumper.
 */
static void Write_Crash_Artifacts(struct _EXCEPTION_POINTERS *ptrs, unsigned int e_code, DWORD crashed_tid)
{
    /**
     *  Minidump first - anything after can crash, but it's already on disk.
     */
    GenerateFullCrashDump = false;
    Create_Mini_Dump(ptrs, Get_Module_File_Name(), nullptr, crashed_tid);

    ExceptionBuffer.clear();

    DEBUG_WARNING("About to call Dump_Exception_Info()\n");
    Dump_Exception_Info(e_code, ptrs);

    /**
     *  Create a unique filename for the crash dump based on the time of execution.
     */
    char filename_buffer[512];
    std::snprintf(filename_buffer, sizeof(filename_buffer), "%s\\EXCEPT_%02u-%02u-%04u_%02u-%02u-%02u.TXT",
        Vinifera_DebugDirectory,
        Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

    ExceptionFile.Set_Name(filename_buffer);

    /**
     *  Write the exception log buffer to the file.
     */
    ExceptionFile.Write(ExceptionBuffer.c_str(), ExceptionBuffer.size());

    if (LastExceptionCRC && CurrentExceptionCRC == LastExceptionCRC) {
        DEBUG_WARNING("Exception dump is identical to the previous exception!\n");
    }

    LastExceptionCRC = CurrentExceptionCRC;
}


/**
 *  Dumper thread body: park on DumperRequestEvent, run the dump (normal
 *  context, so its __try guards catch faults), signal DumperDoneEvent.
 */
static void Dumper_Thread_Proc()
{
    /**
     *  Publish our TID first: the handler keys "dumper available" off it, and
     *  Suspend_Other_Threads must exclude us.
     */
    DumperThreadId.store(GetCurrentThreadId());

    /**
     *  Give the dumper's own __try/__except fault catcher guaranteed stack
     *  headroom too, in case the heavy dump path overflows.
     */
    Vinifera_Reserve_Exception_Stack();

    /**
     *  The __except below is the catch-all for a dump fault - and must never
     *  forward to Vinifera_Exception_Handler, or the dumper waits on itself.
     */
    for (;;) {
        if (WaitForSingleObject(DumperRequestEvent, INFINITE) != WAIT_OBJECT_0) {
            continue;
        }

        TheDumperRequest.DumpOK.store(false);

        __try {
            Write_Crash_Artifacts(&TheDumperRequest.Pointers, TheDumperRequest.ECode, TheDumperRequest.CrashedThreadId);
            TheDumperRequest.DumpOK.store(true);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            /**
             *  Fault escaped the per-section guards; the minidump was written
             *  first, so the key artifact survived.
             */
            DEBUG_WARNING("Dumper thread faulted writing crash artifacts; report is partial.\n");
        }

        SetEvent(DumperDoneEvent);
    }
}


void Start_Dumper_Thread()
{
    /**
     *  Idempotent - only ever one dumper.
     */
    bool expected = false;
    if (!DumperStarted.compare_exchange_strong(expected, true)) {
        return;
    }

    /**
     *  Create the events before spawning, so DumperThreadId != 0 (the
     *  availability signal) implies both are valid. Auto-reset = one-shot.
     */
    DumperRequestEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    DumperDoneEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (DumperRequestEvent == nullptr || DumperDoneEvent == nullptr) {
        DEBUG_WARNING("Could not create dumper events; crash dumps will use the inline path.\n");
        return;
    }

    /**
     *  Detached daemon, never joined (a join from Shutdown_Exception_Handler
     *  would run under the DllMain loader lock).
     */
    std::thread(&Dumper_Thread_Proc).detach();
}


LONG Vinifera_Exception_Handler(unsigned int e_code, struct _EXCEPTION_POINTERS *e_info)
{
    /**
     *  First-thread-wins entry gate. A second thread that crashes while the
     *  winner is dumping parks forever; the winner owns the exit. Same-thread
     *  re-entry falls through, bounded by the DumpingThreadId latch and
     *  recursion counter below.
     */
    const DWORD self_tid = GetCurrentThreadId();
    DWORD expected = 0;
    if (!FirstCrashThreadId.compare_exchange_strong(expected, self_tid)
        && expected != self_tid) {
        // Lost the race - park this thread forever; winner owns the exit.
        SuspendThread(GetCurrentThread());
        ExitProcess(EXIT_FAILURE);
    }

    /**
     *  Same-thread re-entry mid-(inline-)dump means the dump faulted - which
     *  can't be caught in filter context (see DumperRequest). Re-running it
     *  would recurse to the old ExitProcess CTD; terminate here instead (the
     *  minidump is already on disk).
     */
    if (DumpingThreadId.load() == self_tid) {
        DEBUG_WARNING("Nested fault inside the exception dump - terminating (minidump already written).\n");
        Terminate_Now();
    }

    if (ExceptionCriticalSectionReady) {
        EnterCriticalSection(&ExceptionCriticalSection); // recursive on Windows
    }

    const bool on_main_thread = (self_tid == Vinifera_MainThreadId);

    DEBUG_WARNING("Exception!\n");

    /**
     *  Clear previous exception info.
     */
    ExceptionInfoDescription.clear();

    /**
     *  Store this exceptions info for use in other functions that
     *  so not take the section info struct.
     */
    ExceptionInfo = e_info;

    /**
     *  The original games value is actually "disable exceptions", but
     *  we have chosen a different approach, based on the later implementations.
     */
    bool DisableExceptions = CatchExceptions;
    if (DisableExceptions /*|| IsDebuggerPresent()*/) {
        if (ExceptionCriticalSectionReady) {
            LeaveCriticalSection(&ExceptionCriticalSection);
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    /**
     *  Are we already trying to exit from an existing exception?
     */
    if (AlreadyExiting.load() || RecursionCount.load() == 3) {
        ExitProcess(ERROR_SUCCESS);
    }

    if (RecursionCount.fetch_add(1) + 1 > 2) {
        if (ExceptionCriticalSectionReady) {
            LeaveCriticalSection(&ExceptionCriticalSection);
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (e_code == EXCEPTION_BREAKPOINT) {
        RecursionCount.fetch_sub(1);
        if (ExceptionCriticalSectionReady) {
            LeaveCriticalSection(&ExceptionCriticalSection);
        }
        return EXCEPTION_CONTINUE_SEARCH; // The system continues to search for a handler.
    }

    if (e_code == MS_VC_EXCEPTION || e_code == MS_UNHANDLED_CPP_EXCEPTION) { // Exception thrown and not caught.
        RecursionCount.fetch_sub(1);
        if (ExceptionCriticalSectionReady) {
            LeaveCriticalSection(&ExceptionCriticalSection);
        }
        return EXCEPTION_CONTINUE_SEARCH; // The system continues to search for a handler.
    }

    /**
     *  Look up an informational description for this EIP in the exception database.
     */
    if (e_info != nullptr) {
        Exception_Find_Datbase_Entry(e_info->ContextRecord->Eip, ExceptionInfoDescription);
    }

    /**
     *  Phase A — Snapshot. Suspend every other thread so the dump and
     *  minidump observe a frozen address space, then resume them before
     *  the dialog runs in Phase B. Suspending workers across the dialog
     *  causes the dialog to deadlock on OS locks (heap CS, USER32) held
     *  by a worker at its arbitrary suspended instruction.
     *
     *  Must happen AFTER the entry gate (so we won the race). dbghelp.dll
     *  must already be initialized at this point (Debug_Hooks eagerly
     *  calls Init_Symbol_Info), or we risk suspending a thread that holds
     *  the loader lock and deadlocking SymInitialize.
     */
    std::vector<DWORD> suspended_tids;
    suspended_tids.reserve(32);
    Suspend_Other_Threads(suspended_tids);

    if (RecursionCount.load() < 2) {

        /**
         *  Prefer the dumper thread: off-filter, so a walk/dbghelp fault is
         *  caught by the per-section guards instead of recursing. Only the
         *  first crash on a non-dumper thread with a usable context delegates.
         */
        const DWORD dumper_tid = DumperThreadId.load();
        const bool dumper_available =
            dumper_tid != 0
            && self_tid != dumper_tid
            && RecursionCount.load() == 0
            && e_info != nullptr
            && e_info->ContextRecord != nullptr
            && e_info->ExceptionRecord != nullptr;

        bool delegated = false;

        if (dumper_available) {

            /**
             *  Copy the context into stable storage (don't hand the dumper a
             *  pointer into this thread's live stack), then hand off and park.
             */
            TheDumperRequest.Context = *e_info->ContextRecord;
            TheDumperRequest.Record = *e_info->ExceptionRecord;
            TheDumperRequest.Pointers.ContextRecord = &TheDumperRequest.Context;
            TheDumperRequest.Pointers.ExceptionRecord = &TheDumperRequest.Record;
            TheDumperRequest.ECode = e_code;
            TheDumperRequest.CrashedThreadId = self_tid;

            DEBUG_WARNING("Handing crash dump to the dumper thread...\n");
            SetEvent(DumperRequestEvent);
            delegated = (WaitForSingleObject(DumperDoneEvent, DUMPER_TIMEOUT_MS) == WAIT_OBJECT_0);

            if (!delegated) {
                /**
                 *  Dumper wedged. Don't re-run inline (the faulting path we're
                 *  avoiding) - minidump-first means the key artifact likely
                 *  already landed.
                 */
                DEBUG_WARNING("Dumper thread timed out; continuing with whatever artifacts exist.\n");
            }
        }

        if (!delegated) {

            /**
             *  Inline fallback: dumper not up yet, crash is on the dumper, or a
             *  recursive entry. Filter context (guards inert) - the
             *  DumpingThreadId latch is what stops a nested fault recursing.
             */
            DumpingThreadId.store(self_tid);
            Write_Crash_Artifacts(e_info, e_code, self_tid);
            DumpingThreadId.store(0);
        }
    }

    /**
     *  End of Phase A — resume workers before any UI runs. A worker that
     *  re-crashes after this lands in the FirstCrashThreadId gate and is
     *  parked harmlessly; the dump is already on disk.
     */
    Resume_Other_Threads(suspended_tids);
    suspended_tids.clear();

    if (RecursionCount.load() < 2) {

        /**
         *  If OS is Windows 9x only. Surfaces are owned by the main thread,
         *  so don't query their lock state from a worker (and Win9x is no
         *  longer a supported runtime anyway).
         */
        if (on_main_thread && ShowExceptionWindow.load() && Any_Surface_Locked() && CPUDetectClass::Get_OS_Version_Platform_Id() == 1) {
            DEBUG_WARNING("Can't bring up exception dialog due to Win16 mutex issues!\n");
            return EXCEPTION_CONTINUE_SEARCH;
        }

        //WWMouseClass::System_Show_Mouse();
        ShowCursor(TRUE);

        /**
         *  Phase B — UI. Workers are running again so the dialog's allocations
         *  and cross-thread message sends don't deadlock. Worker-thread crashes
         *  parent the dialog to nullptr; main-thread crashes parent to
         *  MainWindow.
         */
        if (ShowExceptionWindow.load()) {

            // https://docs.microsoft.com/en-us/windows/win32/debug/exception-handler-syntax
            DEBUG_WARNING("About to call Exception_Dialog()\n");
            DWORD retval = Exception_Dialog(on_main_thread ? MainWindow : nullptr);

            /**
             *  Phase C — Terminate. Every dialog exit path goes through
             *  Terminate_Now (TerminateProcess). Don't return through the
             *  vanilla game's __try/__except chain or the OS unhandled
             *  filter default — both can stall while running cleanup that
             *  depends on subsystems whose state is now suspect.
             */
            switch (retval) {

                case IDC_EXCEPTION_DEBUG: // Debug button
                    DEBUG_WARNING("Break debugger button pressed!\n");
                    __debugbreak();
                    Terminate_Now();

                default:
                case IDC_EXCEPTION_QUIT: // Quit button
                    DEBUG_WARNING("Quit button pressed!\n");
                    Terminate_Now();
            }
        }
    }

    if (RecursionCount.load() == 2) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    /**
     *  If there has been an additional exception handler defined, this will be called 'before'
     *  the main exception handler. Excepted use for this would be to print info with a custom
     *  debug handler or sending the message across a network to a listening server.
     */
    if (ExceptionHandlerPtr != nullptr) {
        ExceptionHandlerPtr();
        ExceptionHandlerPtr = nullptr; // Reset the pointer after use, so it can be defined again per exception.
    }

    CDControl.Unlock_All_CD_Trays();

    /**
     *  Silent-mode fallthrough (ShowExceptionWindow == false). The dialog
     *  branches above all call Terminate_Now, so this only fires when
     *  testing without UI — still terminate deterministically rather than
     *  returning to the broken caller.
     */
    Terminate_Now();
}
