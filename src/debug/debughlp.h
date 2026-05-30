/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Pointers to Windows API functions to loading debug symbols.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <mutex>
#include <windows.h>
#include <dbghelp.h>


bool __cdecl Init_Symbol_Info();
void __cdecl Uninit_Symbol_Info();


extern HANDLE SymbolProcess;
extern bool SymbolInit;


/**
 *  DbgHelp is documented as not thread-safe; every Sym*, StackWalk, and
 *  MiniDumpWriteDump call across the codebase must hold this mutex.
 *
 *  Recursive because Make_Stack_Trace locks once around the whole walk,
 *  but the per-frame callback re-enters via Get_Function_Details which
 *  needs the same protection when called on its own from other paths.
 *
 *  Lock ordering: ExceptionCriticalSection (exception entry gate) ->
 *  MiniDumpCriticalSection -> DbgHelpMutex. Never reverse.
 */
extern std::recursive_mutex DbgHelpMutex;


extern BOOL(__stdcall *SymCleanupPtr)(HANDLE);
extern BOOL(__stdcall *SymGetSymFromAddrPtr)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
extern BOOL(__stdcall *SymFromAddrPtr)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL);
extern BOOL(__stdcall *SymInitializePtr)(HANDLE, PCSTR, BOOL);
extern DWORD(__stdcall *SymLoadModulePtr)(HANDLE, HANDLE, PCSTR, PCSTR, DWORD, DWORD);
extern DWORD(__stdcall *SymSetOptionsPtr)(DWORD);
extern BOOL(__stdcall *SymUnloadModulePtr)(HANDLE, DWORD);
extern PVOID(__stdcall *SymFunctionTableAccessPtr)(HANDLE, DWORD);
extern BOOL(__stdcall *SymGetLineFromAddrPtr)(HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE);
extern DWORD(__stdcall *SymGetModuleBasePtr)(HANDLE, DWORD);
extern BOOL(__stdcall *StackWalkPtr)(DWORD, HANDLE, HANDLE, LPSTACKFRAME, PVOID, PREAD_PROCESS_MEMORY_ROUTINE,
    PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
extern DWORD (__stdcall *UnDecorateSymbolNamePtr)(PCSTR, PSTR, DWORD, DWORD);
