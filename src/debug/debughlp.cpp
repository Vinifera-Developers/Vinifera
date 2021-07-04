/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DEBUGHLP.CPP
 *
 *  @author        CCHyper, OmniBlade
 *
 *  @brief         Pointers to Windows API functions to loading debug symbols.
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
#include "debughlp.h"
#include "debughandler.h"


/**
 *  The process we initialised the symbols of.
 */
extern HANDLE SymbolProcess = INVALID_HANDLE_VALUE;


BOOL(__stdcall *SymCleanupPtr)(HANDLE);
BOOL(__stdcall *SymGetSymFromAddrPtr)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
BOOL(__stdcall *SymFromAddrPtr)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL);
BOOL(__stdcall *SymInitializePtr)(HANDLE, PCSTR, BOOL);
DWORD(__stdcall *SymLoadModulePtr)(HANDLE, HANDLE, PCSTR, PCSTR, DWORD, DWORD);
DWORD(__stdcall *SymSetOptionsPtr)(DWORD);
BOOL(__stdcall *SymUnloadModulePtr)(HANDLE, DWORD);
PVOID(__stdcall *SymFunctionTableAccessPtr)(HANDLE, DWORD);
BOOL(__stdcall *SymGetLineFromAddrPtr)(HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE);
DWORD(__stdcall *SymGetModuleBasePtr)(HANDLE, DWORD);
BOOL(__stdcall *StackWalkPtr)(DWORD, HANDLE, HANDLE, LPSTACKFRAME, PVOID, PREAD_PROCESS_MEMORY_ROUTINE,
    PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
DWORD (__stdcall *UnDecorateSymbolNamePtr)(PCSTR, PSTR, DWORD, DWORD);

static const char *_sym_functions[] = {
    "SymCleanup",
    "SymGetSymFromAddr",
    "SymFromAddr",
    "SymInitialize",
    "SymLoadModule",
    "SymSetOptions",
    "SymUnloadModule",
    "SymFunctionTableAccess",
    "SymGetLineFromAddr",
    "SymGetModuleBase",
    "StackWalk",
    "UnDecorateSymbolName"
};


static FARPROC *_sym_pointers[] = {
    (FARPROC *)&SymCleanupPtr,
    (FARPROC *)&SymGetSymFromAddrPtr,
    (FARPROC *)&SymFromAddrPtr,
    (FARPROC *)&SymInitializePtr,
    (FARPROC *)&SymLoadModulePtr,
    (FARPROC *)&SymSetOptionsPtr,
    (FARPROC *)&SymUnloadModulePtr,
    (FARPROC *)&SymFunctionTableAccessPtr,
    (FARPROC *)&SymGetLineFromAddrPtr,
    (FARPROC *)&SymGetModuleBasePtr,
    (FARPROC *)&StackWalkPtr,
    (FARPROC *)&UnDecorateSymbolNamePtr,
};


/**
 *  Have the symbol pointers been initialised?
 */
bool SymbolInit = false;


static void Init_DbgHelp()
{
    static bool _initialised = false;

    if (_initialised) {
        return;
    }

    _initialised = true;

    HMODULE dll_handle = LoadLibraryA("dbghelp.dll");

    if (dll_handle != nullptr) {
        for (int i = 0; i < ARRAY_SIZE(_sym_pointers); ++i) {
            *_sym_pointers[i] = GetProcAddress(dll_handle, _sym_functions[i]);

            if (*_sym_pointers[i] == nullptr) {
                DEBUG_WARNING("Init_DbgHelp: Unable to load %s from dbghelp.dll.", _sym_functions[i]);
            }
        }
    } else {
        DEBUG_ERROR("Init_DbgHelp: Unable to load dbghelp.dll.");
    }
}


/**
 *  Cleans up the symbol info.
 */
void __cdecl Uninit_Symbol_Info()
{
    if (SymbolInit) {
        SymbolInit = false;

        if (SymCleanupPtr != nullptr) {
            SymCleanupPtr(GetCurrentProcess());
        }
    }
}


/**
 *  Initializes the symbol info.
 */
bool __cdecl Init_Symbol_Info()
{
    char drive[10];
    char pathname[PATH_MAX + 1];
    char directory[PATH_MAX + 1];

    if (SymbolInit) {
        return true;
    }

    SymbolInit = true;

    std::atexit(Uninit_Symbol_Info);

    Init_DbgHelp();

    if (SymSetOptionsPtr != nullptr) {

        // Set up the symbol options.
        // Turns on undecorated symbol names, deferred loading, line loading and find closet match symbol (if one was not found).
        // We force enable loading of source line because the symbol engine does not load source lines by default.
        DWORD dwOptions = (SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_OMAP_FIND_NEAREST);
#ifndef NDEBUG
        dwOptions |= SYMOPT_DEBUG; // Enables debugger output of PDB loading.
#endif

        /**
         *  Removes "Cannot find matching symbol files" error and prevents any validation of a .PDB file.
         * 
         *  Source:
         *  https://web.archive.org/web/20190802055718/http://ntcoder.com/bab/2012/03/06/how-to-force-symbol-loading-in-windbg/
         */
        dwOptions |= SYMOPT_LOAD_ANYTHING;

        SymSetOptionsPtr(dwOptions);

        SymbolProcess = GetCurrentProcess();

        GetModuleFileNameA(nullptr, pathname, PATH_MAX);
        _splitpath(pathname, drive, directory, 0, 0);
        std::snprintf(pathname, sizeof(pathname), "%s:\\%s", drive, directory);
        std::strcat(pathname, ";.;");

        /**
         *  Here is where things get a little interesting. We need to use the name of the DLL instead of the
         *  name of the target binary. The DLL's debug database contains all our debug info. So use the string
         *  literal of the dll name defined by the build system.
         */
        //if (SymInitializePtr != nullptr && SymInitializePtr(SymbolProcess, DLL_NAME, TRUE)) {
        if (SymInitializePtr != nullptr && SymInitializePtr(SymbolProcess, VINIFERA_DLL, TRUE)) {
            GetModuleFileNameA(nullptr, pathname, PATH_MAX);

            //if (SymLoadModulePtr != nullptr && SymLoadModulePtr(SymbolProcess, nullptr, DLL_NAME, nullptr, 0, 0)) {
            if (SymLoadModuleEx(SymbolProcess, nullptr, VINIFERA_DLL, nullptr, 0, 0, nullptr, 0)) {
                return true;
            }
        }
    }

    return false;
}
