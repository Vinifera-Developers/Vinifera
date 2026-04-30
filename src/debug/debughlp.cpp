/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Pointers to Windows API functions to loading debug symbols.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "debughlp.h"

#include "debughandler.h"

#include <cstdlib>
#include <iterator>

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
        for (int i = 0; i < std::size(_sym_pointers); ++i) {
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
        _splitpath(pathname, drive, directory, nullptr, nullptr);
        _makepath(pathname, drive, directory, ";", ";");

        /**
         *  Here is where things get a little interesting. We need to use the name of the DLL instead of the
         *  name of the target binary. The DLL's debug database contains all our debug info. So use the string
         *  literal of the dll name defined by the build system.
         */
        if (SymInitializePtr != nullptr && SymInitializePtr(SymbolProcess, VINIFERA_DLL, TRUE)) {
            return true;
        }
    }

    return false;
}
