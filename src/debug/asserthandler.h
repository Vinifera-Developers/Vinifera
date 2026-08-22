/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Basic debug assertion implementation.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <cstdlib>
#include <format>
#include <intrinsics.h>
#include <string>
#include <string_view>


extern bool IgnoreAllAsserts;
extern bool SilentAsserts;

extern int GlobalIgnoreCount;
extern int TotalAssertions;

extern bool ExitOnAssert;

extern void Vinifera_Assert_StackDump();

enum AssertType {
    ASSERT_NORMAL,
    ASSERT_FATAL,
};

/**
 *  Non-variadic assertion entry point. The std::format-based ASSERT_*_PRINT
 *  macros format their message at the call site and then call this with the
 *  result. The variadic Vinifera_Assert_Handler in debug_hooks.cpp (installed
 *  as the TSpp assertion callback) also lands here after running vsnprintf
 *  locally on the printf-style format strings that TSpp expects.
 */
void Vinifera_Assert(AssertType type, const char *expr, const char *file, int line, const char *function, volatile bool *ignore, volatile bool *allow_break, volatile bool *exit, std::string_view msg);

#define ASSERT(exp) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, std::string_view{}); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    if (_exit || ExitOnAssert) { \
                        Emergency_Exit(EXIT_FAILURE); \
                    } \
                } \
            } \
        } \
    } while (false)

#define ASSERT_PRINT(exp, fmt, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    std::string _assert_msg = std::format(fmt __VA_OPT__(,) __VA_ARGS__); \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, _assert_msg); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    if (_exit || ExitOnAssert) { \
                        Emergency_Exit(EXIT_FAILURE); \
                    } \
                } \
            } \
        } \
    } while (false)

#define ASSERT_FATAL(exp) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    ::Vinifera_Assert(ASSERT_FATAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, std::string_view{}); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    Emergency_Exit(EXIT_FAILURE); \
                } \
            } \
        } \
    } while (false)

#define ASSERT_FATAL_PRINT(exp, fmt, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    std::string _assert_msg = std::format(fmt __VA_OPT__(,) __VA_ARGS__); \
                    ::Vinifera_Assert(ASSERT_FATAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, _assert_msg); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    Emergency_Exit(EXIT_FAILURE); \
                } \
            } \
        } \
    } while (false)

#define ASSERT_STACKDUMP(exp) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    Vinifera_Assert_StackDump(); \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, std::string_view{}); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    if (_exit || ExitOnAssert) { \
                        Emergency_Exit(EXIT_FAILURE); \
                    } \
                } \
            } \
        } \
    } while (false)

#define ASSERT_STACKDUMP_PRINT(exp, fmt, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    Vinifera_Assert_StackDump(); \
                    std::string _assert_msg = std::format(fmt __VA_OPT__(,) __VA_ARGS__); \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, _assert_msg); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    if (_exit || ExitOnAssert) { \
                        Emergency_Exit(EXIT_FAILURE); \
                    } \
                } \
            } \
        } \
    } while (false)


/**
 *  Assertion macros for overriding source information.
 */
#define ASSERT_CUSTOM(exp, file, line, func) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, file, line, func, &_ignore_assert, &_break, &_exit, std::string_view{}); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    if (_exit || ExitOnAssert) { \
                        Emergency_Exit(EXIT_FAILURE); \
                    } \
                } \
            } \
        } \
    } while (false)

#define ASSERT_CUSTOM_FATAL(exp, file, line, func) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, file, line, func, &_ignore_assert, &_break, &_exit, std::string_view{}); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    if (_exit || ExitOnAssert) { \
                        Emergency_Exit(EXIT_FAILURE); \
                    } \
                } \
            } \
        } \
    } while (false)

#define ASSERT_CUSTOM_PRINT(exp, file, line, func, fmt, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    std::string _assert_msg = std::format(fmt __VA_OPT__(,) __VA_ARGS__); \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, file, line, func, &_ignore_assert, &_break, &_exit, _assert_msg); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    if (_exit || ExitOnAssert) { \
                        Emergency_Exit(EXIT_FAILURE); \
                    } \
                } \
            } \
        } \
    } while (false)

#define ASSERT_CUSTOM_PRINT_FATAL(exp, file, line, func, fmt, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    std::string _assert_msg = std::format(fmt __VA_OPT__(,) __VA_ARGS__); \
                    ::Vinifera_Assert(ASSERT_NORMAL, #exp, file, line, func, &_ignore_assert, &_break, &_exit, _assert_msg); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    Emergency_Exit(EXIT_FAILURE); \
                } \
            } \
        } \
    } while (false)
