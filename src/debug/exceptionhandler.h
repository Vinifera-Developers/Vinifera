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

#include <atomic>
#include <windows.h>


typedef LONG(*exceptionhandler_ptr_t)(unsigned int, struct _EXCEPTION_POINTERS *);
typedef void(*exceptioncallback_ptr_t)();

LONG Vinifera_Exception_Handler(unsigned int e_code, struct _EXCEPTION_POINTERS *e_info);

/**
 *  Top-level SEH filter and C++ exception translator. Both forward to
 *  Vinifera_Exception_Handler. Exposed in this header so per-thread guards
 *  (see vinifera_thread.h) can install the translator on worker threads.
 */
LONG __stdcall _Top_Level_Exception_Filter(EXCEPTION_POINTERS *e_info);
void __cdecl _Structured_Exception_Translator(unsigned int code, EXCEPTION_POINTERS *e_info);

/**
 *  std::set_terminate target. Calls Vinifera_Exception_Handler with a
 *  synthetic record then ExitProcess. Per-thread; install in DllMain and in
 *  each worker thread wrapper.
 */
[[noreturn]] void __cdecl Vinifera_Terminate_Handler();

/**
 *  Lifecycle. Called once from DllMain DLL_PROCESS_ATTACH (before any hook
 *  can fire) and DLL_PROCESS_DETACH respectively.
 */
void Init_Exception_Handler();
void Shutdown_Exception_Handler();

/**
 *  Reserve a guaranteed stack region (via SetThreadStackGuarantee) for the SEH
 *  filter on the calling thread, so a stack overflow can still reach the crash
 *  dumper. Per-thread; call as early as possible on every thread.
 */
void Vinifera_Reserve_Exception_Stack();

/**
 *  Spawn the dedicated crash-dump thread. Call once from a post-init, main-
 *  thread context (not DllMain - loader lock). Idempotent; until it runs,
 *  crashes use the inline dump path.
 */
void Start_Dumper_Thread();


extern register_t LastExceptionEIP;
extern uint32_t LastExceptionCRC;

extern _EXCEPTION_POINTERS *ExceptionInfo;

extern exceptioncallback_ptr_t ExceptionHandlerPtr;

extern std::atomic<bool> AlreadyExiting;
extern std::atomic<bool> ShowExceptionWindow;
extern std::atomic<bool> ExceptionDumpFinished;

extern std::atomic<int> RecursionCount;

/**
 *  The buffer which holds the exception log info.
 */
extern FixedString<131072> ExceptionBuffer;
