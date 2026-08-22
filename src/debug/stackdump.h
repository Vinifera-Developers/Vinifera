/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Functions for dumping the call stack.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "getreg.h"

/**
 *  Default frames to skip in Stack_Dump/Stack_Dump_From_Context.
 *  PC is captured inside Stack_Dump, so the topmost frame is Stack_Dump
 *  itself; subsequent frames are its callers up the chain.
 *    0 - report from the captured PC outward
 *    1 - skip the helper that called Stack_Dump
 *    2 - skip an additional layer (the default; suits one-level-deep helpers)
 */
#define STACK_WALK_SKIP 2


typedef void(__cdecl *stackcallback_ptr_t)(const char *buffer);


void Stack_Dump_From_Context(register_t myeip, register_t myesp, register_t myebp, stackcallback_ptr_t callback, int skipframes = STACK_WALK_SKIP);
void Stack_Dump(stackcallback_ptr_t callback, int skipframes = STACK_WALK_SKIP);
void Get_Function_Details(void* pointer, char* funcname, char* filename, unsigned* linenumber, uintptr_t* address);
