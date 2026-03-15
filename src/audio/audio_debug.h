/*******************************************************************************
/*                  O P E N  S O U R C E -- V I N I F E R A                   **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_DEBUG.H
 *
 *  @author        CCHyper
 *
 *  @brief         Audio engine debug logging macros and utilities.
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
#include "debughandler.h"
#include "vinifera_globals.h"
#include <cstdarg>
#include <cstdio>
#include <string>


typedef enum AudioDebugLogLevel {
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR
} AudioDebugLogLevel;


typedef enum AudioDebugLogType {
    TYPE_MANAGER,
    TYPE_INSTANCE,
    TYPE_SAMPLE,
    TYPE_AMBIENT,
    TYPE_THREAD,
    TYPE_DECODER,
    TYPE_IO,
    TYPE_VOC,
    TYPE_VOX,
    TYPE_THEME,

    // Special for anything else
    TYPE_HOOKS
} AudioDebugLogType;


/**
 *  Macro alias for the audio debug logging function.
 */
#define AUDIO_DEBUG_MSG Audio_Debug_Log
void __cdecl Audio_Debug_Log(AudioDebugLogLevel level, AudioDebugLogType type, const char * message, ...);
