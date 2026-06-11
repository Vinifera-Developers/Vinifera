/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Wrapper that installs Vinifera's exception machinery on a worker
 *          thread so any crash there reaches Vinifera_Exception_Handler and
 *          produces full artifacts before the process exits.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "exceptionhandler.h"

#include <eh.h>
#include <exception>
#include <utility>
#include <windows.h>


/**
 *  SEH and C++ object unwinding can't share the same function, so the body
 *  runs inside this no-inline stub which only the __try frame sees.
 */
template<class F>
__declspec(noinline) static void Vinifera_Thread_Stub(F& body)
{
    body();
}


/**
 *  Drop this at the very top of any std::thread / _beginthreadex entry that
 *  Vinifera owns. It:
 *    - installs _set_se_translator (per-thread) so C++ catches map SEH
 *    - installs std::set_terminate (per-thread) so escaped C++ throws route
 *      through Vinifera_Exception_Handler instead of going to abort()
 *    - wraps the body in __try/__except so any SEH that escapes runs the
 *      same handler the main thread uses, then ExitProcess
 *
 *  Any uncaught exception on any thread that uses this wrapper kills the
 *  process with full crash artifacts.
 */
template<class F>
inline void Vinifera_Run_Thread(F&& body)
{
    Vinifera_Reserve_Exception_Stack();
    _set_se_translator((_se_translator_function)&_Structured_Exception_Translator);
    std::set_terminate(&Vinifera_Terminate_Handler);

    auto local = std::forward<F>(body);

    __try {
        Vinifera_Thread_Stub(local);
    } __except (Vinifera_Exception_Handler(GetExceptionCode(), GetExceptionInformation())) {
        ExitProcess(GetExceptionCode());
    }
}
