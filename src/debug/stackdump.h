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
 *  Default stack walk depth.
 *  Value of 2 skips Do_Stack_Dump and Make_Stack_Trace int the call stack.
 */
#define STACK_WALK_SKIP 2


typedef void(__cdecl *stackcallback_ptr_t)(const char *buffer);


void Stack_Dump_From_Context(register_t myeip, register_t myesp, register_t myebp, stackcallback_ptr_t callback, int skipframes = STACK_WALK_SKIP);
void Stack_Dump(stackcallback_ptr_t callback, int skipframes = STACK_WALK_SKIP);
void Get_Function_Details(void* pointer, char* funcname, char* filename, unsigned* linenumber, uintptr_t* address);
