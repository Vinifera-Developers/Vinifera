/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Debug printing and output.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <format>
#include <string>
#include <string_view>
#include <utility>


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
    DEBUGTYPE_GAME,      // Reserved for vanilla logger intercepts in debug_hooks.cpp.
    DEBUGTYPE_GAME_LINE, // Reserved for vanilla logger intercepts; no trailing newline expected.
};


/**
 *  Core dispatch. Takes an already-formatted message and routes it through
 *  the console / debugger / log-file pipeline. All other entry points
 *  ultimately call this.
 */
void Vinifera_Log_Raw(DebugType type, const char *file, const char *function, int line, std::string_view message);


/**
 *  Compile-time-checked formatting entry point. The format string is validated
 *  by std::format_string<Args...> at compile time.
 */
template <class... Args>
inline void Vinifera_Log(DebugType type, const char *file, const char *function, int line,
                         std::format_string<Args...> fmt, Args &&... args)
{
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    Vinifera_Log_Raw(type, file, function, line, formatted);
}


/**
 *  Runtime-format-string entry point. Use when the format string is computed
 *  at runtime (e.g. read from config). std::vformat throws std::format_error
 *  if the format string is malformed.
 */
template <class... Args>
inline void Vinifera_Log_VFormat(DebugType type, const char *file, const char *function, int line,
                                 std::string_view fmt, Args &&... args)
{
    std::string formatted = std::vformat(fmt, std::make_format_args(args...));
    Vinifera_Log_Raw(type, file, function, line, formatted);
}


/**
 *  Standard logging macros. The format string is compile-time validated
 *  against the argument types by std::format_string<Args...>.
 */
#ifndef NDEBUG
#define DEBUG_SAY(fmt, ...)     ::Vinifera_Log(DEBUGTYPE_NORMAL,  nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_INFO(fmt, ...)    ::Vinifera_Log(DEBUGTYPE_INFO,    nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_WARNING(fmt, ...) ::Vinifera_Log(DEBUGTYPE_WARNING, nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_ERROR(fmt, ...)   ::Vinifera_Log(DEBUGTYPE_ERROR,   nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_FATAL(fmt, ...)   ::Vinifera_Log(DEBUGTYPE_FATAL,   nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_TRACE(fmt, ...)   ::Vinifera_Log(DEBUGTYPE_TRACE,   __FILE__, __FUNCTION__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define DEBUG_SAY(fmt, ...)     ::Vinifera_Log(DEBUGTYPE_NORMAL,  nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_INFO(fmt, ...)    ::Vinifera_Log(DEBUGTYPE_INFO,    nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_WARNING(fmt, ...) ::Vinifera_Log(DEBUGTYPE_WARNING, nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_ERROR(fmt, ...)   ::Vinifera_Log(DEBUGTYPE_ERROR,   nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_FATAL(fmt, ...)   ::Vinifera_Log(DEBUGTYPE_FATAL,   nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_TRACE(fmt, ...)   ((void)0)
#endif


/**
 *  Runtime-format-string variants (std::vformat). Use when the format string
 *  is not known at compile time. Throws std::format_error on malformed input.
 */
#define DEBUG_SAY_VFMT(fmt, ...)     ::Vinifera_Log_VFormat(DEBUGTYPE_NORMAL,  nullptr, nullptr, -1, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_INFO_VFMT(fmt, ...)    ::Vinifera_Log_VFormat(DEBUGTYPE_INFO,    nullptr, nullptr, -1, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_WARNING_VFMT(fmt, ...) ::Vinifera_Log_VFormat(DEBUGTYPE_WARNING, nullptr, nullptr, -1, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_ERROR_VFMT(fmt, ...)   ::Vinifera_Log_VFormat(DEBUGTYPE_ERROR,   nullptr, nullptr, -1, (fmt) __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_FATAL_VFMT(fmt, ...)   ::Vinifera_Log_VFormat(DEBUGTYPE_FATAL,   nullptr, nullptr, -1, (fmt) __VA_OPT__(,) __VA_ARGS__)


/**
 *  Macros to only output to the debugger (if attached).
 */
#ifndef NDEBUG
#define DEBUG_DBG_OUTPUT(fmt, ...)       ::Vinifera_Log(DEBUGTYPE_DEBUGGER,       nullptr, nullptr, -1, fmt __VA_OPT__(,) __VA_ARGS__)
#define DEBUG_DBG_OUTPUT_TRACE(fmt, ...) ::Vinifera_Log(DEBUGTYPE_DEBUGGER_TRACE, __FILE__, __FUNCTION__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define DEBUG_DBG_OUTPUT(fmt, ...)       ((void)0)
#define DEBUG_DBG_OUTPUT_TRACE(fmt, ...) ((void)0)
#endif


/**
 *  For printing out debug info in developer mode only.
 */
#ifndef NDEBUG
#define DEV_DEBUG_SAY     DEBUG_SAY
#define DEV_DEBUG_INFO    DEBUG_INFO
#define DEV_DEBUG_WARNING DEBUG_WARNING
#define DEV_DEBUG_ERROR   DEBUG_ERROR
#define DEV_DEBUG_FATAL   DEBUG_FATAL
#define DEV_DEBUG_TRACE   DEBUG_TRACE
#else
#define DEV_DEBUG_SAY(fmt, ...)     do { if (Vinifera_DeveloperMode) { DEBUG_SAY    (fmt __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define DEV_DEBUG_INFO(fmt, ...)    do { if (Vinifera_DeveloperMode) { DEBUG_INFO   (fmt __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define DEV_DEBUG_WARNING(fmt, ...) do { if (Vinifera_DeveloperMode) { DEBUG_WARNING(fmt __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define DEV_DEBUG_ERROR(fmt, ...)   do { if (Vinifera_DeveloperMode) { DEBUG_ERROR  (fmt __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define DEV_DEBUG_FATAL(fmt, ...)   do { if (Vinifera_DeveloperMode) { DEBUG_FATAL  (fmt __VA_OPT__(,) __VA_ARGS__); } } while (false)
#define DEV_DEBUG_TRACE(fmt, ...)   ((void)0)
#endif


void __cdecl Vinifera_Debug_Handler_Startup();
void __cdecl Vinifera_Debug_Handler_Shutdown();


/**
 *  Wrapper to OutputDebugString with conditions.
 */
void Vinifera_Output_Debug_String(const char *string);


extern char CrashdumpFilename[/*PATH_MAX*/260];

extern bool DisableDebuggerOutput;
