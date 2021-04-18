/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          GETREG.H
 *
 *  @author        CCHyper
 *
 *  @brief         Macros for fetching assembly register values to assist
 *                 in debug exception and stack dumping.
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


typedef uint32_t register_t;


/**
 *  Define the registers (E:R)IP, (E:R)SP, (E:R)BP.
 */
#define DEFINE_GENERAL_REGISTERS(ip, sp, bp) \
    register_t ip; \
    register_t sp; \
    register_t bp; \


/**
 *  Fetch the registers from the context.
 * 
 *  #NOTE: These will not compile under WinXP apparently...
 */
#define GET_EIP_ESP_EBP_REGISTERS(the_eip, the_esp, the_ebp) \
{ \
    static CONTEXT _ctx; \
    ZeroMemory(&_ctx, sizeof(_ctx)); \
    RtlCaptureContext(&_ctx); \
_label: /* Label to fetch EIP address. */\
    __asm __volatile { \
        mov [the_eip], offset _label \
    } \
    the_esp = _ctx.Esp; \
    the_ebp = _ctx.Ebp; \
}

#define GET_EIP_ESP_EBP_REGISTERS_LABEL(the_eip, the_esp, the_ebp, __label) \
{ \
    static CONTEXT _ctx; \
    ZeroMemory(&_ctx, sizeof(_ctx)); \
    RtlCaptureContext(&_ctx); \
    __asm __volatile { \
        mov [the_eip], offset __label \
    } \
    the_esp = _ctx.Esp; \
    the_ebp = _ctx.Ebp; \
}

#define GET_EAX_REGISTER(reg) \
{ \
    static CONTEXT _ctx; \
    ZeroMemory(&_ctx, sizeof(_ctx)); \
    RtlCaptureContext(&_ctx); \
    reg = _ctx.Eax; \
}

#define GET_ECX_REGISTER(reg) \
{ \
    static CONTEXT _ctx; \
    ZeroMemory(&_ctx, sizeof(_ctx)); \
    RtlCaptureContext(&_ctx); \
    reg = _ctx.Ecx; \
} \

#define GET_EBX_REGISTER(reg) \
{ \
    static CONTEXT _ctx; \
    ZeroMemory(&_ctx, sizeof(_ctx)); \
    RtlCaptureContext(&_ctx); \
    reg = _ctx.Ebx; \
}

#define GET_ESP_REGISTER(reg) \
{ \
    static CONTEXT _ctx; \
    ZeroMemory(&_ctx, sizeof(_ctx)); \
    RtlCaptureContext(&_ctx); \
    reg = _ctx.Esp; \
}


/**
 *  Simple helper macros.
 */
#define     GET_REGISTERS(ip, sp, bp)               GET_EIP_ESP_EBP_REGISTERS(ip, sp, bp)
#define     GET_REGISTERS_LABEL(ip, sp, bp, lbl)    GET_EIP_ESP_EBP_REGISTERS_LABEL(ip, sp, bp, lbl)
