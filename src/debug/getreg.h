/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Macros for fetching assembly register values to assist in debug
 *          exception and stack dumping.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


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
