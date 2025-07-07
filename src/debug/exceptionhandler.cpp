/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXCEPTIONHANDLER.CPP
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         Custom exception handler.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "exceptionhandler.h"
#include "stackdump.h"
#include "minidump.h"
#include "cpudetect.h"
#include "buildnum.h"
#include "winutil.h"
#include "miscutil.h"
#include "crc32.h"
#include "cdctrl.h"
#include "version.h"
#include "dsurface.h"
#include "textfile.h"
#include "debughlp.h"
#include "wwmouse.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "resource.h"
#include "fetchres.h"
#include "windialog.h"
#include "tspp_gitinfo.h"
#include "vinifera_gitinfo.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "tibsun_globals.h"
#include "vinifera_newdel.h"
#include <Windows.h>
#include <dbghelp.h>
#include <eh.h>
#include <string>


extern HMODULE DLLInstance;

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

bool AlreadyExiting = false;
bool ExitAfterException = false;
bool ReturnedAfterException = false;
bool ShowExceptionWindow = true;
bool ExceptionDumpFinished = false;

int RecursionCount = -1;

FixedString<65536> ExceptionBuffer;

static TextFileClass ExceptionFile;

static FixedString<1024> ExceptionInfoDescription;
static bool ExceptionInfoCanContinue = false;
static bool ExceptionInfoIgnore = false;


/**
 *  The installable exception intercept function pointer.
 */
LONG (* __stdcall Exception_Intercept_Func_Ptr)(unsigned int code, EXCEPTION_POINTERS *e_info);


bool Any_Surface_Locked()
{
    return (PrimarySurface && PrimarySurface->Is_Locked())
        || (HiddenSurface && HiddenSurface->Is_Locked())
        || (CompositeSurface && CompositeSurface->Is_Locked())
        || (TileSurface && TileSurface->Is_Locked())
        || (SidebarSurface && SidebarSurface->Is_Locked())
        || (AlternateSurface && AlternateSurface->Is_Locked());
}


void Clear_All_Surfaces()
{
    if (PrimarySurface) PrimarySurface->Clear();
    if (HiddenSurface) HiddenSurface->Clear();
    if (CompositeSurface) CompositeSurface->Clear();
    if (TileSurface) TileSurface->Clear();
    if (SidebarSurface) SidebarSurface->Clear();
    if (AlternateSurface) AlternateSurface->Clear();
}


/**
 *  Finds the first instance of the address in the loaded database and extracts its info.
 */
static bool Exception_Find_Datbase_Entry(uintptr_t address, bool &can_continue, bool &ignore, FixedString<1024> &desc)
{
    for (int i = 0; i < ExceptionInfoDatabase.Count(); ++i) {
        if (ExceptionInfoDatabase[i].Address == address) {
            can_continue = ExceptionInfoDatabase[i].CanContinue;
            ignore = ExceptionInfoDatabase[i].Ignore;
            desc = ExceptionInfoDatabase[i].Description;
            return true;
        }
    }
    return false;
}


/**
 *  Write a mini dump file for analysis.
 */
static bool Exception_Generate_Mini_Dump()
{
    int retval = MessageBox(MainWindow,
        "Would you like to generate a crash dump that can be sent to the\n"
        "developers for further analysis?\n\n"
        "Please note: This may take some time depending on the options\n"
        "set by the crash dump generator, please be patient and allow\n"
        "this process to finish. You will be notified when it is complete.\n\n",
        "Generate Crash dump?", 
        MB_YESNO|MB_ICONQUESTION);

    if (retval == IDYES) {

        GenerateFullCrashDump = false; // We don't need a full memory dump.
        bool res = Create_Mini_Dump(ExceptionInfo, Get_Module_File_Name());

        if (res) {
            char buffer[512];
            std::snprintf(buffer, sizeof(buffer),
                "Crash dump file generated successfully.\n\n"
                "Please make sure you package EXCEPT_<date-time>.TXT\n"
                "and DEBUG_<date-time>.LOG along with this crash dump file!\n\n"
                "Filename:\n\"%s\" \n", MinidumpFilename);
            MessageBox(MainWindow, buffer, "Crash dump", MB_OK);
        } else {
            MessageBox(MainWindow, "Failed to create crash dump!\n\n", "Crash dump", MB_OK|MB_ICONASTERISK);
        }

        return true;
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
    "Error code: CONTROL_C_EXIT\r\r\nDescription: The application terminated as a result of a CTRL+C."
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


static void Dump_Exception_Info(unsigned int e_code, struct _EXCEPTION_POINTERS *e_info)
{
    static char scratch[1024]; // Scratch buffer, to use for anything.
    ZeroMemory(scratch, sizeof(scratch));

    /**
     *  Clear the buffer just in case we did a previous dump (like in a recursive situation).
     */
    ExceptionBuffer.Clear();

    Init_Symbol_Info();

    EXCEPTION_RECORD *record = e_info->ExceptionRecord;
    CONTEXT *context = e_info->ContextRecord;

    if (e_code == EXCEPTION_ACCESS_VIOLATION) {
        //Exception_Printf("Exception is access violation\r\n");
        DEBUG_WARNING("Exception is access violation\n");
    } else {
        //Exception_Printf("Exception code is %x\r\n", e_code);
        DEBUG_WARNING("Exception code is %x\n", e_code);
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

    DEBUG_WARNING("%s\n", the_exception_desc);

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
    };

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
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " was written to.\r\n", record->ExceptionInformation[1]);
                break;
            case 8: // User-mode data execution prevention (DEP).
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " DEP violation.\r\n", record->ExceptionInformation[1]);
                break;
            default: // Unknown
                Exception_Printf("Access address: 0x%" PRIPTRSIZE PRIXPTR " Unknown violation.\r\n", record->ExceptionInformation[1]);
                break;
        };
    }

    Exception_Printf("Exception occurred at 0x%" PRIPTRSIZE PRIXPTR "\r\n", context->Eip);

    Exception_Printf("\r\n");

    /**
     *  Has additional info for this EIP been loaded from the exception database?
     */
    if (ExceptionInfoDescription.Peek_Buffer()[0] != '\0') {
        Exception_Printf("Additional Information:\r\n");
        DEBUG_WARNING("\r\nAdditional Information:\n");
        Exception_Printf("  %s\r\n", ExceptionInfoDescription.Peek_Buffer());
        DEBUG_WARNING("  %s\n\n", ExceptionInfoDescription.Peek_Buffer());
        Exception_Printf("\r\n");
    }

    DEBUG_WARNING("Call dump...\n");
    //Exception_Printf("Call stack:\r\n");

    int stack_skip_frames = 1; // #TODO: This needs checking. Value of 1 skips the EIP address, which seems ideal.

    Stack_Dump_From_Context(context->Eip, context->Esp, context->Ebp, Exception_Stack_Dump_Handler, stack_skip_frames);

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

        Exception_Printf("   %+#.17e\r\n", *reinterpret_cast<double *>(&context->FloatSave.RegisterArea[i * 10]));
    }

    /**
     *  MMX Registers.
     */
    if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE)) {
        Exception_Printf("MMX0:%016llX\tMMX1:%016llX\tMMX2:%016llX\tMMX3:%016llX\r\n", context->ExtendedRegisters[0], context->ExtendedRegisters[1], context->ExtendedRegisters[2], context->ExtendedRegisters[3]);
        Exception_Printf("MMX4:%016llX\tMMX5:%016llX\tMMX6:%016llX\tMMX7:%016llX\r\n", context->ExtendedRegisters[4], context->ExtendedRegisters[5], context->ExtendedRegisters[6], context->ExtendedRegisters[7]);
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

    uintptr_t *address = reinterpret_cast<uintptr_t *>(context->Esp);

    char symbol_buffer[sizeof(IMAGEHLP_SYMBOL64)+EXCEPTION_STACK_SYMNAME_MAX];
    IMAGEHLP_SYMBOL64 *const symbol_bufferp = reinterpret_cast<IMAGEHLP_SYMBOL64 *>(symbol_buffer);

    /**
     *  Dump the contents of the stack as defined by the maximum depth.
     */
    for (int frame = 0; frame < EXCEPTION_STACK_DEPTH_MAX; ++frame) {

        //DEBUG_WARNING("Frame %d\n", frame);

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
         *  Removed, no longer needed as we ship a fake PDB for GAME.EXE that contains
         *  most of the addresses for the original game.
         */
#if 0
        /**
         *  Super kludge for catching target binary addresses!
         */
        if ((uintptr_t)*address >= (uintptr_t)0x00401000 && (uintptr_t)*address < (uintptr_t)0x006CA000) {
            Exception_Printf("%" PRIPTRSIZE PRIXPTR ": %" PRIPTRSIZE PRIXPTR " - (Game code, function-name not available)\r\n", (uintptr_t)address, *address);
            ++address;
            continue;
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
    CurrentExceptionCRC = CRC32_Memory(ExceptionBuffer.Peek_Buffer(), ExceptionBuffer.Get_Length());

    DEBUG_WARNING("****************************** END EXEPTION DUMP ******************************!\n");
}


static INT_PTR CALLBACK Exception_Dialog_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    // FALSE == We are not interested in the message. Let dialog manager do any default processing.
    INT_PTR result = FALSE;

#if defined(TS_CLIENT)
    /**
     *  Disable the Main Menu button.
     */
    EnableWindow(GetDlgItem(hDlg, IDC_EXCEPTION_MAINMENU), FALSE); // Main Menu button
#endif

    /**
     *  Disable/Enable the Continue button.
     */
    EnableWindow(GetDlgItem(hDlg, IDC_EXCEPTION_CONTINUE), ExceptionInfoCanContinue ? TRUE : FALSE); // Continue button

    switch (uMsg) {
        case WM_MOVING:
            result = WinDialogClass::Dialog_Move(hDlg, wParam, lParam, uMsg);
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDC_EXCEPTION_SAVE: // Emergency save button
                    if (Debug_Map) {
                        EndDialog(hDlg, IDC_EXCEPTION_SAVE);
                        result = TRUE;
                    } else {
                        MessageBox(hDlg,
                            "Sorry, you can only attempt an emergency save if you were in the map editor when the exception occurred.",
                            "Bummer",  MB_OK|MB_ICONEXCLAMATION);
                        result = FALSE;
                    }
                    break;

                case IDC_EXCEPTION_QUIT: // Quit button
                    EndDialog(hDlg, IDC_EXCEPTION_QUIT);
                    result = TRUE;
                    break;

                case IDC_EXCEPTION_DEBUG: // Debug button
                    EndDialog(hDlg, IDC_EXCEPTION_DEBUG);
                    result = TRUE;
                    break;

                case IDC_EXCEPTION_MAINMENU: // Main menu button
                    EndDialog(hDlg, IDC_EXCEPTION_MAINMENU);
                    result = TRUE;
                    break;

                case IDC_EXCEPTION_CONTINUE: // Continue button
                    EndDialog(hDlg, IDC_EXCEPTION_CONTINUE);
                    result = TRUE;
                    break;

                default:
                    result = FALSE;
                    break;
            };
            break;

        case WM_CLOSE:
            EndDialog(hDlg, IDC_EXCEPTION_QUIT);
            result = FALSE;
            break;

        case WM_INITDIALOG:
            /**
             *  Send the exception buffer to the dialog.
             */
            if (ExceptionDumpFinished) {
                SetDlgItemTextA(hDlg, IDC_EXCEPTION_LOG, ExceptionBuffer.Peek_Buffer()); // Debug edit box.
            }

            SetFocus(hDlg);

            if (MainWindow != nullptr) {
                WinDialogClass::Center_Window_Within(hDlg, MainWindow);
            }

            ShowWindow(hDlg, SW_SHOWNORMAL);
            result = FALSE;
            break;

        default:
            result = FALSE;
            break;
    };

    return result;
}


static INT_PTR Exception_Dialog()
{
    switch (RecursionCount) {
        case 1:
            CDControl.Unlock_All_CD_Drives();
            DEBUG_ERROR("Recursive exception detected!\n");
            MessageBox(nullptr, "Recursive exception detected!\n", "Error!", MB_OK|MB_ICONEXCLAMATION);
            Sleep(1000); // was 4000
            return IDC_EXCEPTION_QUIT; // Quit button

        case 2:
            CDControl.Unlock_All_CD_Drives();
            return IDC_EXCEPTION_QUIT; // Quit button

        case 3:
            CDControl.Unlock_All_CD_Drives();
            ExitProcess(EXIT_SUCCESS);
            break; //return IDC_EXCEPTION_QUIT; // Quit button

        default:
            break;
    };

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
        retval = DialogBoxIndirectParam(ProgramInstance, (LPDLGTEMPLATE)hGlobalDlg, MainWindow, (DLGPROC)Exception_Dialog_Proc, (LPARAM)0);
    } else {
        DEBUG_ERROR("Unable to find the exception dialog resource!\n");
    }
    
    CDControl.Unlock_All_CD_Drives();

    return retval;
}


LONG Vinifera_Exception_Handler(unsigned int e_code, struct _EXCEPTION_POINTERS *e_info)
{
    DEBUG_WARNING("Exception!\n");

    /**
     *  Clear previous exception info.
     */
    ExceptionInfoCanContinue = false;
    ExceptionInfoIgnore = false;
    ExceptionInfoDescription.Clear();

    /**
     *  
     */
    if (Exception_Intercept_Func_Ptr) {
        LONG code = Exception_Intercept_Func_Ptr(e_code, e_info);
        if (code == EXCEPTION_CONTINUE_EXECUTION) {

            /**
             *  Flag that we returned to the application after an exception occurred.
             */
            ReturnedAfterException = true;

            /**
             *  Clear the recursion flag.
             */
            RecursionCount = -1;

            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }

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
        return EXCEPTION_CONTINUE_SEARCH;
    }

    /**
     *  Are we already trying to exit from an existing exception?
     */
    if (AlreadyExiting || RecursionCount == 3) {
        ExitProcess(ERROR_SUCCESS);
    }

    if (++RecursionCount > 2) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (e_code == EXCEPTION_BREAKPOINT) {
        --RecursionCount;
        return EXCEPTION_CONTINUE_SEARCH; // The system continues to search for a handler.
    }

    if (e_code == MS_VC_EXCEPTION || e_code == MS_UNHANDLED_CPP_EXCEPTION) { // Exception thrown and not caught.
        --RecursionCount;
        return EXCEPTION_CONTINUE_SEARCH; // The system continues to search for a handler.
    }

    /**
     *  Search for additional info for this EIP in the exception database.
     */
    Exception_Find_Datbase_Entry(e_info->ContextRecord->Eip, ExceptionInfoCanContinue, ExceptionInfoIgnore, ExceptionInfoDescription);

    /**
     *  Should we ignore this exception?
     */
    if (ExceptionInfoIgnore) {
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    /**
     *  It most cases, this is our first attempt at an exception crash dump.
     */
    if (RecursionCount < 2) {

        ExceptionBuffer.Clear();

        DEBUG_WARNING("About to call Dump_Exception_Info()\n");
        Dump_Exception_Info(e_code, e_info);

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
        ExceptionFile.Write(ExceptionBuffer.Peek_Buffer(), ExceptionBuffer.Get_Length());

        if (LastExceptionCRC && CurrentExceptionCRC == LastExceptionCRC) {
            DEBUG_WARNING("Exception dump is identical to the previous exception!\n");
        }

        LastExceptionCRC = CurrentExceptionCRC;

        /**
         *  If OS is Windows 9x only.
         */
        if (ShowExceptionWindow && Any_Surface_Locked() && CPUDetectClass::Get_OS_Version_Platform_Id() == 1) {
            DEBUG_WARNING("Can't bring up exception dialog due to Win16 mutex issues!\n");
            return EXCEPTION_CONTINUE_SEARCH;
        }

        //WWMouseClass::System_Show_Mouse();
        ShowCursor(TRUE);

        /**
         *  Show the exception error message to the user?
         */
        if (ShowExceptionWindow) {

            // https://docs.microsoft.com/en-us/windows/win32/debug/exception-handler-syntax
            DEBUG_WARNING("About to call Exception_Dialog()\n");
            DWORD retval = Exception_Dialog();
            switch (retval) {

                /**
                 *  Emergency save (scenario editor only).
                 */
                case IDC_EXCEPTION_SAVE: // Emergency save button
                    DEBUG_WARNING("Emergency save button pressed!\n");

                    /**
                     *  Ask the user if the wish to produce a minidump.
                     */
                    Exception_Generate_Mini_Dump();
                    Vinifera_Collect_Debug_Files();

                    ExitAfterException = true; // #TEMP!
                    break;

                /**
                 *  Trigger the debugger to break at the exception address.
                 */
                case IDC_EXCEPTION_DEBUG: // Debug button
                    DEBUG_WARNING("Break debugger button pressed!\n");

                    if (!IsDebuggerPresent()) {
                        /**
                         *  Ask the user if the wish to produce a minidump.
                         */
                        Exception_Generate_Mini_Dump();
                    }

                    Vinifera_Collect_Debug_Files();

                    __debugbreak();

                    return EXCEPTION_EXECUTE_HANDLER;

                /**
                 *  This is a real hack case for continuing the game after an exception has
                 *  occurred. We force the EIP to that of "Select_Game" and return "CONTINUE_EXECUTION"
                 *  which will tell the handler to continue execution from this address.
                 */
                case IDC_EXCEPTION_MAINMENU: // Main menu button
                {
                    DEBUG_WARNING("Main menu button pressed!\n");

                    /**
                     *  Ask the user if the wish to produce a minidump.
                     */
                    Exception_Generate_Mini_Dump();

                    if (!ExceptionReturnAddress) {
                        /**
                         *  EIP was invalid, just trigger the handler.
                         */
                        return EXCEPTION_EXECUTE_HANDLER;
                    }
                    
                    Vinifera_Collect_Debug_Files();

                    /**
                     *  #BUGFIX:
                     *  Clear all surfaces to remove any blitting artifacts.
                     */
                    Clear_All_Surfaces();

                    //WWMouseClass::System_Hide_Mouse();
                    ShowCursor(FALSE);

                    /**
                     *  Flag that we returned to the application after an exception occurred.
                     */
                    ReturnedAfterException = true;
                    
                    /**
                     *  Clear the recursion flag.
                     */
                    RecursionCount = -1;

                    /**
                     *  Now we pray...
                     */
                    DWORD *ebp = &(e_info->ContextRecord->Ebp);
                    DWORD *esp = &(e_info->ContextRecord->Esp);
                    DWORD *eip = &(e_info->ContextRecord->Eip);
                    *ebp = ExceptionReturnBase;
                    *esp = ExceptionReturnStack;
                    *eip = ExceptionReturnAddress;

                    return EXCEPTION_CONTINUE_EXECUTION;
                }

                /**
                 *  Similar to the "return to the main menu" button hack above, this
                 *  will tell the process to continue after the exception. We do
                 *  this by returning "CONTINUE_EXECUTION" which will tell the
                 *  handler to continue execution from this address.
                 */
                case IDC_EXCEPTION_CONTINUE: // Continue button
                    DEBUG_WARNING("Continue button pressed!\n");

                    /**
                     *  Ask the user if the wish to produce a minidump.
                     */
                    Exception_Generate_Mini_Dump();

                    Vinifera_Collect_Debug_Files();
                    
                    //WWMouseClass::System_Hide_Mouse();
                    ShowCursor(FALSE);

                    /**
                     *  Flag that we returned to the application after an exception occurred.
                     */
                    ReturnedAfterException = true;
                    
                    /**
                     *  Clear the recursion flag.
                     */
                    RecursionCount = -1;

                    ExitAfterException = false;

                    return EXCEPTION_CONTINUE_EXECUTION;

                default:
                case IDC_EXCEPTION_QUIT: // Quit button
                    DEBUG_WARNING("Quit button pressed!\n");
                    
                    /**
                     *  Ask the user if the wish to produce a minidump.
                     */
                    Exception_Generate_Mini_Dump();

                    Vinifera_Collect_Debug_Files();

                    ExitAfterException = true;
                    break;
            };
        }

        //WWMouseClass::System_Hide_Mouse();
        ShowCursor(FALSE);
    }

    if (RecursionCount == 2) {
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

    if (ExitAfterException) {
        AlreadyExiting = true;
        return EXCEPTION_EXECUTE_HANDLER;
    }

    //--RecursionCount;

    if (WinDialogClass::CurrentWindowHandle) {
        WinDialogClass::End_Dialog(WinDialogClass::CurrentWindowHandle);
    }
    
    CDControl.Unlock_All_CD_Drives();

    Vinifera_Collect_Debug_Files();

    /**
     *  The system transfers control to the exception handler, and execution continues
     *  in the stack frame in which the handler is found.
     */
    return EXCEPTION_EXECUTE_HANDLER;
}


void Vinifera_Install_Exception_Handler_Intercept(LONG (* __stdcall func_ptr)(unsigned int, EXCEPTION_POINTERS *))
{
    Exception_Intercept_Func_Ptr = func_ptr;
}
