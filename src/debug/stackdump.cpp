/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          STACKDUMP.CPP
 *
 *  @author        OmniBlade, CCHyper
 *
 *  @brief         Functions for dumping the call stack.
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
#include "stackdump.h"
#include "debughlp.h"
#include <Windows.h>
#include <eh.h>
 

#define PRIPTRSIZE "08"


/**
 *  Options for the stack walker.
 */
#define STACK_SYMNAME_MAX 512
#define STACK_DEPTH_MAX 30


/**
 *  Strip the full path from the source name in the debug info?
 */
static bool StripFilenamePaths = true;


static void Get_Function_Details(void *pointer, char *funcname, char *filename, unsigned *linenumber, uintptr_t *address)
{
    char symbol_buffer[sizeof(IMAGEHLP_SYMBOL64) + STACK_SYMNAME_MAX];
    IMAGEHLP_SYMBOL64 *const symbol_bufferp = reinterpret_cast<IMAGEHLP_SYMBOL64 *>(symbol_buffer);
    //IMAGEHLP_SYMBOL64 symbol_buffer;
    ZeroMemory(&symbol_buffer, sizeof(symbol_buffer));
    symbol_bufferp->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64)+STACK_SYMNAME_MAX;
    symbol_bufferp->MaxNameLength = STACK_SYMNAME_MAX-1;

    //SYMBOL_INFO symbol_info;
    //ZeroMemory(&symbol_info, sizeof(symbol_info));
	//symbol_info.SizeOfStruct = sizeof(SYMBOL_INFO)+STACK_SYMNAME_MAX;
	//symbol_info.MaxNameLen = STACK_SYMNAME_MAX-1;

    /**
     *  Set fallbacks for the output values.  
     */
    if (funcname != nullptr) {
        std::strcpy(funcname, "<Unknown>");
    }

    if (filename != nullptr) {
        std::strcpy(filename, "<Unknown>");
    }

    if (linenumber != nullptr) {
        *linenumber = (unsigned)-1;
    }

    if (address != nullptr) {
        *address = (uintptr_t)-1;
    }

    /**
     *  Reset last error code.
     */
    SetLastError(0);

    DWORD64 displacement = 0;
    HANDLE process = SymbolProcess; //GetCurrentProcess();

    //if (SymGetSymFromAddrPtr != nullptr) {

        BOOL got_it = SymGetSymFromAddr64(process, reinterpret_cast<DWORD64>(pointer), &displacement, symbol_bufferp);
        //BOOL got_it = SymFromAddr(process, reinterpret_cast<DWORD64>(pointer), reinterpret_cast<PDWORD64>(&displacement), &symbol_info);
        if (got_it) {

            if (funcname != nullptr) {
                std::strcpy(funcname, symbol_bufferp->Name);
                //std::strcpy(funcname, symbol_info.Name);
                std::strcat(funcname, "();");
            }

            //if (SymGetLineFromAddrPtr != nullptr) {
                IMAGEHLP_LINE64 line;
                line.Key = 0;
                line.LineNumber = 0;
                line.SizeOfStruct = sizeof(line);
                line.FileName = 0;
                line.Address = 0;

                // Reset last error code.
                SetLastError(0);

                if (SymGetLineFromAddr64(process, reinterpret_cast<DWORD_PTR>(pointer), reinterpret_cast<PDWORD>(&displacement), &line)) {
                    if (filename != nullptr) {

                        if (StripFilenamePaths) {
                            char path[_MAX_PATH];
                            char fname[_MAX_FNAME];
                            char fext[_MAX_EXT];

                            /**
                             *  Strip the drive and path (if present) off of the filename.
                             */
                            _splitpath(line.FileName, nullptr, nullptr, fname, fext);
                            _makepath(path, nullptr, nullptr, fname, fext);

                            std::strcpy(filename, path);

                        } else {
                            std::strcpy(filename, line.FileName);
                        }
                    }

                    if (linenumber != nullptr) {
                        *linenumber = line.LineNumber;
                    }

                    if (address != nullptr) {
                        *address = line.Address;
                    }
                }
            //}
        } else {
            //DEBUG_INFO("Get_Function_Details() - SymFromAddr failed: %d\n", GetLastError());
        }
    //}
}


static void Write_Stack_Line(void *address, stackcallback_ptr_t callback)
{
    static char filename[STACK_SYMNAME_MAX];
    static char funcname[PATH_MAX];
    static char dest[PATH_MAX];

    uintptr_t addr;
    unsigned line;

    char pathname[PATH_MAX+1];
    char path[PATH_MAX];
    char name[_MAX_FNAME];
    char ext[_MAX_EXT];

    /**
     *  Get the module name, removing the full path.
     */
    //GetModuleFileNameA(nullptr, pathname, PATH_MAX);
    //_splitpath(pathname, nullptr, nullptr, name, ext);
    //_makepath(path, nullptr, nullptr, name, ext);

    Get_Function_Details(address, funcname, filename, &line, &addr);

    //std::snprintf(dest, sizeof(dest), "  [%s] %s(%d) : %s 0x%" PRIPTRSIZE PRIXPTR "\n", path, filename, line, funcname, (uintptr_t)address);
    std::snprintf(dest, sizeof(dest), "  %s(%d) : %s 0x%" PRIPTRSIZE PRIXPTR "\r\n", filename, line, funcname, (uintptr_t)address);

    if (callback != nullptr) {
        callback(dest);
    }
}


void Make_Stack_Trace(register_t instructionptr, register_t stackptr, register_t frameptr, int skip_frames, stackcallback_ptr_t callback)
{
    BOOL carry_on = true;
    //DWORD error = 0;

    HANDLE thread = GetCurrentThread();
    HANDLE process = SymbolProcess; //GetCurrentProcess();

    /**
     *  Initialize the STACKFRAME structure for the first call. This is only
     *  necessary for Intel CPUs, and isn't mentioned in the documentation.
     * 
     *  #NOTE: Stack frame must be set based on architecture.
     */
    STACKFRAME64 stack_frame;
    ZeroMemory(&stack_frame, sizeof(stack_frame));
    stack_frame.AddrPC.Mode = AddrModeFlat;
    stack_frame.AddrPC.Offset = (DWORD64)instructionptr;
    stack_frame.AddrStack.Mode = AddrModeFlat;
    stack_frame.AddrStack.Offset = (DWORD64)stackptr;
    stack_frame.AddrFrame.Mode = AddrModeFlat;
    stack_frame.AddrFrame.Offset = (DWORD64)frameptr;

    /**
     *  Get the context for the current thread.
     */
    CONTEXT ctx;
    ZeroMemory(&ctx, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_FULL;
    //GetThreadContext(thread, &ctx);

    /**
     *  Make a copy here because StackWalk can modify the one we give it.
     */
    CONTEXT ctx_cpy;
    CopyMemory(&ctx_cpy, &ctx, sizeof(CONTEXT));

    /**
     *  Context record parameter is required only when the machine type
     *  parameter is not IMAGE_FILE_MACHINE_I386.
     */
    void *ctx_r = nullptr;

    /**
     *  Get the architecture type of the computer for which the stack trace is generated.
     */
    DWORD machine_type = IMAGE_FILE_MACHINE_I386;

    if (callback != nullptr) {
        callback("Call Stack:\r\n");
    }

    /**
     *  Obtain a call stack trace.
     */
    //if (StackWalkPtr != nullptr) {

        /**
         *  The top stack frame is the call to this routine itself, the next is the
         *  routine that called us. All calls to this function should be using skipframes
         *  of at least 2, anything higher depends on the call chain to get here.
         */
        while (skip_frames-- > 0) {

            // Reset last error code.
            SetLastError(0);

            /**
             *  Trace to the next frame.
             */
            //carry_on = StackWalkPtr(machine_type,
            carry_on = StackWalk64(machine_type,
                process,
                thread,
                &stack_frame,
                ctx_r,
                nullptr,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                nullptr);

            if (!carry_on) {
			    break;
            }
        }

        /**
         *  We skipped the specified stack frames, now we fetch the important information
         *  from the call stack at the depth specified by STACK_DEPTH_MAX.
         */
        if (carry_on) {
            for (int i = STACK_DEPTH_MAX; i > 0; --i) {

                // Reset last error code.
                SetLastError(0);

                //carry_on = StackWalkPtr(machine_type,
                carry_on = StackWalk64(machine_type,
                    process,
                    thread,
                    &stack_frame,
                    ctx_r,
                    nullptr,
                    SymFunctionTableAccess64,
                    SymGetModuleBase64,
                    nullptr);

                /**
                 *  Basic sanity check to make sure the frame is OK. Bail if not.
                 */
                if (stack_frame.AddrFrame.Offset == 0) {
				    continue; //break;
                }

                if (carry_on) {
                    Write_Stack_Line((void *)stack_frame.AddrPC.Offset, callback);
                }
            }
        }
    //}
}


void Stack_Dump_From_Context(register_t myeip, register_t myesp, register_t myebp, stackcallback_ptr_t callback, int skipframes)
{
    Init_Symbol_Info();

    Make_Stack_Trace(myeip, myesp, myebp, skipframes, callback);
}


void Stack_Dump(stackcallback_ptr_t callback, int skipframes)
{
    /**
     *  Define EIP/RIP, ESP/RSP, EBP/RBP registers.
     */
    DEFINE_GENERAL_REGISTERS(instructionptr, stackptr, frameptr);
    GET_REGISTERS(instructionptr, stackptr, frameptr);

    Init_Symbol_Info();

    Make_Stack_Trace(instructionptr, stackptr, frameptr, skipframes, callback);
}
