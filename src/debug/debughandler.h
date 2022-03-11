/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DEBUGHANDLER.H
 *
 *  @author        CCHyper
 *
 *  @brief         Debug printing and output.
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


extern bool Vinifera_DeveloperMode;


enum DebugType {
    DEBUGTYPE_NORMAL,
    DEBUGTYPE_INFO,
    DEBUGTYPE_WARNING,
    DEBUGTYPE_ERROR,
    DEBUGTYPE_FATAL,
    DEBUGTYPE_TRACE,
    DEBUGTYPE_DEBUGGER,
    DEBUGTYPE_DEBUGGER_TRACE,
    DEBUGTYPE_GAME, // Special case for debug strings printed from the original binary.
    DEBUGTYPE_GAME_LINE // Special case for debug strings printed from the original binary with no carriage return.
};


/**
 *  Custom printing function.
 */
void Vinifera_Printf(DebugType type, const char *file, const char *function, int line, const char *fmt, ...);


/**
 *  Custom printing function.
 */
#ifndef NDEBUG
#define DEBUG_SAY(x, ...) Vinifera_Printf(DEBUGTYPE_NORMAL, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_INFO(x, ...) Vinifera_Printf(DEBUGTYPE_INFO, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_WARNING(x, ...) Vinifera_Printf(DEBUGTYPE_WARNING, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_ERROR(x, ...) Vinifera_Printf(DEBUGTYPE_ERROR, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_FATAL(x, ...) Vinifera_Printf(DEBUGTYPE_FATAL, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_TRACE(x, ...) Vinifera_Printf(DEBUGTYPE_TRACE, __FILE__, __FUNCTION__, __LINE__, x, ##__VA_ARGS__)
#else
#define DEBUG_SAY(x, ...) Vinifera_Printf(DEBUGTYPE_NORMAL, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_INFO(x, ...) Vinifera_Printf(DEBUGTYPE_INFO, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_WARNING(x, ...) Vinifera_Printf(DEBUGTYPE_WARNING, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_ERROR(x, ...) Vinifera_Printf(DEBUGTYPE_ERROR, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_FATAL(x, ...) Vinifera_Printf(DEBUGTYPE_FATAL, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_TRACE(x, ...) ((void)0)
#endif


/**
 *  Special case macros for debug strings printed from the original binary.
 */
#ifndef NDEBUG
#define DEBUG_GAME(x, ...) Vinifera_Printf(DEBUGTYPE_GAME, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_GAME_LINE(x, ...) Vinifera_Printf(DEBUGTYPE_GAME_LINE, __FILE__, __FUNCTION__, __LINE__, x, ##__VA_ARGS__)
#else
#define DEBUG_GAME(x, ...) Vinifera_Printf(DEBUGTYPE_GAME, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_GAME_LINE(x, ...) Vinifera_Printf(DEBUGTYPE_GAME_LINE, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#endif


/**
 *  Macros to only output to the debugger (if attached).
 */
#ifndef NDEBUG
#define DEBUG_DBG_OUTPUT(x, ...) Vinifera_Printf(DEBUGTYPE_DEBUGGER, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEBUG_DBG_OUTPUT_TRACE(x, ...) Vinifera_Printf(DEBUGTYPE_DEBUGGER_TRACE, __FILE__, __FUNCTION__, __LINE__, x, ##__VA_ARGS__)
#else
#define DEBUG_DBG_OUTPUT(x, ...) ((void)0)
#define DEBUG_DBG_OUTPUT_TRACE(x, ...) ((void)0)
#endif


/**
 *  For printing out debug info in developer mode only.
 */
#ifndef NDEBUG
#define DEV_DEBUG_SAY(x, ...) Vinifera_Printf(DEBUGTYPE_NORMAL, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEV_DEBUG_INFO(x, ...) Vinifera_Printf(DEBUGTYPE_INFO, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEV_DEBUG_WARNING(x, ...) Vinifera_Printf(DEBUGTYPE_WARNING, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEV_DEBUG_ERROR(x, ...) Vinifera_Printf(DEBUGTYPE_ERROR, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEV_DEBUG_FATAL(x, ...) Vinifera_Printf(DEBUGTYPE_FATAL, nullptr, nullptr, -1, x, ##__VA_ARGS__)
#define DEV_DEBUG_TRACE(x, ...) Vinifera_Printf(DEBUGTYPE_TRACE, __FILE__, __FUNCTION__, __LINE__, x, ##__VA_ARGS__)
#else
#define DEV_DEBUG_SAY(x, ...) if (Vinifera_DeveloperMode) { Vinifera_Printf(DEBUGTYPE_NORMAL, nullptr, nullptr, -1, x, ##__VA_ARGS__); }
#define DEV_DEBUG_INFO(x, ...) if (Vinifera_DeveloperMode) { Vinifera_Printf(DEBUGTYPE_INFO, nullptr, nullptr, -1, x, ##__VA_ARGS__); }
#define DEV_DEBUG_WARNING(x, ...) if (Vinifera_DeveloperMode) { Vinifera_Printf(DEBUGTYPE_WARNING, nullptr, nullptr, -1, x, ##__VA_ARGS__); }
#define DEV_DEBUG_ERROR(x, ...) if (Vinifera_DeveloperMode) { Vinifera_Printf(DEBUGTYPE_ERROR, nullptr, nullptr, -1, x, ##__VA_ARGS__); }
#define DEV_DEBUG_FATAL(x, ...) if (Vinifera_DeveloperMode) { Vinifera_Printf(DEBUGTYPE_FATAL, nullptr, nullptr, -1, x, ##__VA_ARGS__); }
#define DEV_DEBUG_TRACE(x, ...) ((void)0)
#endif


void __cdecl Vinifera_Debug_Handler_Startup();
void __cdecl Vinifera_Debug_Handler_Shutdown();


/**
 *  Wrapper to OutputDebugString with conditions.
 */
void Vinifera_Output_Debug_String(const char *string);


extern char CrashdumpFilename[PATH_MAX];

extern bool DisableDebuggerOutput;
