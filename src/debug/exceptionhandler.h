/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Custom exception handler.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "getreg.h"
#include "stringid.h"

#include <windows.h>


typedef LONG(*exceptionhandler_ptr_t)(unsigned int, struct _EXCEPTION_POINTERS *);
typedef void(*exceptioncallback_ptr_t)();

LONG Vinifera_Exception_Handler(unsigned int e_code, struct _EXCEPTION_POINTERS *e_info);


extern register_t LastExceptionEIP;
extern uint32_t LastExceptionCRC;

extern _EXCEPTION_POINTERS *ExceptionInfo;

extern exceptioncallback_ptr_t ExceptionHandlerPtr;

extern bool AlreadyExiting;
extern bool ExitAfterException;
extern bool ReturnedAfterException;
extern bool ShowExceptionWindow;
extern bool ExceptionDumpFinished;

extern int RecursionCount;

/**
 *  The buffer which holds the exception log info.
 */
extern FixedString<65536> ExceptionBuffer;

/**
 *  Installs a custom exception intercept handler.
 */
void Vinifera_Install_Exception_Handler_Intercept(LONG (* __stdcall func_ptr)(unsigned int, _EXCEPTION_POINTERS *));
