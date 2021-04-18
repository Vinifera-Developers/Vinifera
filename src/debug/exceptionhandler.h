/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXCEPTIONHANDLER.H
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         Custom exception handler.
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
#include "fixedstring.h"
#include <Windows.h>


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
