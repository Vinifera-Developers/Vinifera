/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOOKER_MACROS.H
 *
 *  @author        CCHyper
 *
 *  @brief         This file contains macros for interacting with pure assembly.
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


#include <cstdio>


/**
 *  MSVC++ Stack Frames
 */
#define PROLOG_THISCALL \
    _asm { push ecx } \
    _asm { push esp, ebp } \
    _asm { push ebp }

#define PROLOG_STDCALL \
    _asm { mov esp, ebp } \
    _asm { push ebp }
 
#define EPILOG_THISCALL \
    _asm { pop ecx } \
    _asm { mov esp, ebp } \
    _asm { pop ebp }

#define EPILOG_STDCALL \
    _asm { mov esp, ebp } \
    _asm { pop ebp }


/**
 *  Jumps
 */
#define JMP(address) \
    _asm { mov eax, address } \
    _asm { jmp eax }

#define JMP_REG(reg, address) \
    _asm { mov reg, address } \
    _asm { jmp reg }

#define JMP_THIS(address) \
    { \
        EPILOG_THISCALL; \
        JMP(address); \
    }

#define JMP_STD(address) \
    { \
        EPILOG_STDCALL; \
        JMP(address); \
    }


/**
 *  Calls
 */
#define CALL(p_func) \
    _asm { mov eax, p_func }\
    _asm { call eax }

#define CALL_VT(vt_offs) \
    _asm { mov eax, [ecx] }\
    _asm { call dword ptr [eax+vt_offs] }

#define THISCALL(p_func) \
    _asm { mov ecx, this }\
    _asm { mov eax, p_func }\
    _asm { call eax }

#define THISCALL_EX(p_this, p_func) \
    _asm { mov ecx, p_this }\
    _asm { mov eax, p_func }\
    _asm { call eax }

#define THISCALL_VT(vt_offs) \
    _asm { mov ecx, this }\
    _asm { mov eax, [ecx] }\
    _asm { call dword ptr [eax+vt_offs] }

#define THISCALL_EX_VT(p_this, vt_offs) \
    _asm { mov ecx, p_this }\
    _asm { mov edx, [ecx] }\
    _asm { call dword ptr [edx+vt_offs] }


/**
 *  Get/Set register to a variable
 */
#define GET_REGISTER(type, dst, reg) type dst; _asm { mov dst, reg }
#define GET_REGISTER_STATIC(type, dst, reg) static type dst; _asm { mov dst, reg }
#define SET_REGISTER(reg, src) _asm { mov reg, src }

/**
 *  Get stack value to a variable
 */
#define GET_STACK(type, dst, reg, off) type dst; _asm { mov eax, [reg+off] } _asm { mov dst, eax }
#define GET_STACK_STATIC(type, dst, reg, off) static type dst; _asm { mov eax, [reg+off] } _asm { mov dst, eax }
#define LEA_STACK_STATIC(type, dst, reg, off) static type dst; _asm { lea eax, [reg+off] } _asm { mov dst, eax }
#define GET_STACK_STATIC8(type, dst, reg, off) static type dst; _asm { mov al, byte ptr [reg+off] } _asm { mov dst, al }
#define GET_STACK_STATIC16(type, dst, reg, off) static type dst; _asm { mov ax, word ptr [reg+off] } _asm { mov dst, ax }


/**
 *   
 */
#define ZERO_REG(reg) _asm { xor reg, reg }


/**
 *  Basic stack pointer operations.
 */
#define ADD_ESP(i) _asm { add esp, i }
#define SUB_ESP(i) _asm { sub esp, i }


/**
 *  Pushes
 */
// Push immediate value
#define PUSH_IMM(i) _asm { push i }

// Push / Pop a register
#define PUSH_REG(r) _asm { push r }
#define POP_REG(r) _asm { pop r }

// Push a pointer to a variable
#define PUSH_PTR(var) \
    _asm { lea eax, var }\
    _asm { push eax }

// Push 8bit variable
#define PUSH_VAR8(var) \
    _asm { movzx eax, var }\
    _asm { push eax }

// Push 16bit variable
#define PUSH_VAR16(var) \
    _asm { movzx eax, var }\
    _asm { push eax }
	
// Push 32bit variable
#define PUSH_VAR32(var) \
    _asm { mov eax, var }\
    _asm { push eax }

// Push 64bit variable
#define PUSH_VAR64(var) \
    _asm { mov eax, var }\
    _asm { mov ecx, [eax+4] }\
    _asm { mov eax, [eax] }\
    _asm { push ecx }\
    _asm { push eax }


/**
 *  Read and Write to memory.
 */
#define MEM_READ8(dst, mem) \
    _asm { mov dl, byte ptr ds:mem }\
    _asm { mov dst, dl }

#define MEM_WRITE8(mem, src) \
    _asm { mov dl, src }\
    _asm { mov byte ptr ds:mem, dl }

#define MEM_READ16(dst, mem) \
    _asm { mov dx, word ptr ds:mem }\
    _asm { mov dst, dx }

#define MEM_WRITE16(mem, src) \
    _asm { mov dx, src }\
    _asm { mov word ptr ds:mem, dx }

#define MEM_READ32(dst, mem) \
    _asm { mov edx, dword ptr ds:mem }\
    _asm { mov dst, edx }

#define MEM_WRITE32(mem, src) \
    _asm { mov edx, src }\
    _asm { mov dword ptr ds:mem, edx }

#define MEM_WRITEIMM8(mem, imm)	 \
    _asm { mov byte ptr ds:mem, imm }

#define MEM_WRITEIMM16(mem, imm)	 \
    _asm { mov word ptr ds:mem, imm }

#define MEM_WRITEIMM32(mem, imm)	 \
    _asm { mov dword ptr ds:mem, imm }


/**
 *  Fetch the value of a register into a C variable.
 */
#define VAR32_REG(type, name, reg) \
    type name; _asm { mov name, reg }

#define VAR8_REG(type, name, reg) \
    type name; _asm { mov name, reg }


/**
 *  Declare a patch hook. This function will not produce any
 *  prolog or epilog, ideal for jumping into and back out of.
 */
#define DECLARE_PATCH(name) \
    [[ noreturn ]] static __declspec(noinline) __declspec(naked) void name() noexcept
