/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MINIDUMP.H
 *
 *  @author        CCHyper
 *
 *  @brief         Creates a mini dump for analysis.
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
#include "exceptionhandler.h"


/**
 *  Dump full memory of the process?
 */
extern bool GenerateFullCrashDump;

/**
 *  Are we currently writing a minidump by request, such as by an assert?
 */
extern bool NonFatalMinidump;


/**
 *  Should we produce a minidump with the current time and date?
 */
extern bool MinidumpUseCurrentTime;

extern char MinidumpFilename[PATH_MAX];


bool Create_Mini_Dump(struct _EXCEPTION_POINTERS *e_info, const char *app_name = nullptr, const char *path = nullptr);
