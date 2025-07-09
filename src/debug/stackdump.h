/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          STACKDUMP.H
 *
 *  @author        OmniBlade, CCHyper
 *
 *  @brief         Functions for dumping the call stack.
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
#include "getreg.h"

/**
 *  Default stack walk depth.
 *  Value of 2 skips Do_Stack_Dump and Make_Stack_Trace int the call stack.
 */
#define STACK_WALK_SKIP 2


typedef void(__cdecl *stackcallback_ptr_t)(const char *buffer);


void Stack_Dump_From_Context(register_t myeip, register_t myesp, register_t myebp, stackcallback_ptr_t callback, int skipframes = STACK_WALK_SKIP);
void Stack_Dump(stackcallback_ptr_t callback, int skipframes = STACK_WALK_SKIP);
