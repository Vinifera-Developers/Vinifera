/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DEBUGHLP.H
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
#pragma once

#include "always.h"
#include <Windows.h>
#include <dbghelp.h>


bool __cdecl Init_Symbol_Info();
void __cdecl Uninit_Symbol_Info();


extern HANDLE SymbolProcess;
extern bool SymbolInit;


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
