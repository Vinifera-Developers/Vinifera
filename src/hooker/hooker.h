/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOOKER.H
 *
 *  @author        CCHyper, OmniBlade
 *
 *  @brief         Provides methods for accessing data and functions in an
 *                 existing binary and replacing functions with new 
 *                 implementations from an injected DLL.
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


/**
 *  These are the functions that must be called at both DLL attach and detach
 *  to handle changing of the code segments read only protection to read|write.
 */
__declspec(dllexport) void StartHooking();
__declspec(dllexport) void StopHooking();


/**
 *  Simple structs to pack the assembly in for jumping into replacement code.
 *  So long as the calling conventions and arguments for the replaced and
 *  replacement functions are the same, everything should just work.
 */
#pragma pack(1)
struct /*alignas(1)*/ jump_opcode
{
    const uint8_t cmd = 0xE9; // Relative JMP instruction.
    uintptr_t addr = 0x0;
};
#pragma pack()

#pragma pack(1)
struct /*alignas(1)*/ call_opcode
{
    const uint8_t cmd = 0xE8; // CALL instruction.
    uintptr_t addr = 0x0;
};
#pragma pack()


/**
 *  Patch a call hook to the input address/function.
 */
template<typename T>
void Patch_Call(uintptr_t address, T new_address)
{
    static_assert(sizeof(call_opcode) == 5, "Call struct not expected size!");

    SIZE_T bytes_written;

    call_opcode cmd;
    cmd.addr = reinterpret_cast<uintptr_t>((void*&)new_address) - address - sizeof(call_opcode);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, &cmd, sizeof(call_opcode), &bytes_written);
    //assert(bytes_written == 5);
}


/**
 *  Patch a jump hook to the input address/function.
 */
template<typename T>
void Patch_Jump(uintptr_t address, T new_address)
{
    static_assert(sizeof(jump_opcode) == 5, "Jump struct not expected size!");

    SIZE_T bytes_written;

    jump_opcode cmd;
    cmd.addr = reinterpret_cast<uintptr_t>((void*&)new_address) - address - sizeof(jump_opcode);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, &cmd, sizeof(jump_opcode), &bytes_written);
    //assert(bytes_written == 5);
}


/**
 *  Patch byte/word/dword at input address.
 */
inline void Patch_Byte(uintptr_t in, uint8_t byte)
{
    SIZE_T bytes_written;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)in, &byte, sizeof(uint8_t), &bytes_written);
    //assert(bytes_written == 1);
}

inline void Patch_Word(uintptr_t in, uint16_t word)
{
    SIZE_T bytes_written;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)in, &word, sizeof(uint16_t), &bytes_written);
    //assert(bytes_written == 2);
}

inline void Patch_Dword(uintptr_t in, uint32_t dword)
{
    SIZE_T bytes_written;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)in, &dword, sizeof(uint32_t), &bytes_written);
    //assert(bytes_written == 4);
}


/**
 *  Get the memory address of a function.
 */
template<typename T>
uintptr_t Get_Func_Address(T func)
{
    return reinterpret_cast<uintptr_t>((void*&)func);
}


/**
 *  Adjust a pointer value by the input amount.
 * 
 *  When writing hook wrappers for stdcall interface methods, MSVC offsets
 *  "this" by 4 bytes to try and will try to call from the wrong interface/class
 *  table (due to dual inheritance and adjuster thunks).
 *
 *  So use these helpers to commit treason and adjust the pointer... I'm sorry...
 */
template<class T>
__forceinline T Adjust_Ptr(T ptr, int amount)
{
    uintptr_t rawptr = reinterpret_cast<uintptr_t>(ptr);
    return reinterpret_cast<T>(rawptr + amount);
}


/**
 *  Change the address of a call, jump, or reference.
 */
__forceinline void Change_Address(uintptr_t addr, uintptr_t newaddr)
{
    Patch_Dword(addr, _byteswap_ulong(newaddr));
}


/**
 *  Change the address virtual table entry.
 */
__forceinline void Change_Virtual_Address(uintptr_t addr, uintptr_t newaddr)
{
    Patch_Dword(addr, newaddr);
}


/**
 *  Hook regular functions and static methods. This is templated
 *  to allow hooking without casting.
 */
template<typename T>
__forceinline void Hook_Function(uintptr_t address, T new_address)
{
    Patch_Jump(address, reinterpret_cast<uintptr_t>((void *&)new_address));
}


/**
 *  Hook regular functions and static methods. This is only used for
 *  the ASM macro below, which allows us to bypass some of the compiler
 *  limitations and rules.
 *
 *  #WARNING: This MUST be cdecl, as the macro is written with this calling
 *            convention expected!
 */
__forceinline void __cdecl Hook_Func(uintptr_t address, uintptr_t new_address)
{
    Patch_Jump(address, reinterpret_cast<uintptr_t>((void *&)new_address));
}


/**
 *  Virtual functions are even trickier, using Hook_Function on these will produce
 *  a thunk which is a simple virtual call, but we need to reference the function 
 *  implementation directly, so we need to resort to inline asm for those.
 * 
 *  #NOTE: Does not support input function pointers from static casts or
 *         functions/classes using templates. Also, where as "Hook_Function" requires
 *         you to dereference the function name to fetch the address, this macro does
 *         note require you to do so.
 */
#define ASM_Hook_Function(address, new_address) \
    { \
        _asm { push new_address } \
        _asm { push address } \
        _asm { call Hook_Func } \
        _asm { add esp, 8 } \
    }

/**
 *  Simple define to redirect to the ASM macro for easy reading of hooks.
 */
#define Hook_Virtual(x, y) ASM_Hook_Function(x, y)
