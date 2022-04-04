/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ASSERTHANDLER.H
 *
 *  @author        CCHyper
 *
 *  @brief         Basic debug assertion implementation.
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
#include "fatal.h"
#include <intrinsics.h>
#include <cstdlib>


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

void Vinifera_Assert(AssertType type, const char *expr, const char *file, int line, const char *function, volatile bool *ignore, volatile bool *allow_break, volatile bool *exit, const char *msg, ...);

#define ASSERT(exp) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, nullptr); \
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

#define ASSERT_PRINT(exp, msg, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, msg, ##__VA_ARGS__); \
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

#define ASSERT_FATAL(exp, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    Vinifera_Assert(ASSERT_FATAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, nullptr, ##__VA_ARGS__); \
                    if (_break) { \
                        __debugbreak(); \
                    } \
                    Emergency_Exit(EXIT_FAILURE); \
                } \
            } \
        } \
    } while (false)

#define ASSERT_FATAL_PRINT(exp, msg, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    Vinifera_Assert(ASSERT_FATAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, msg, ##__VA_ARGS__); \
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
                    Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, nullptr); \
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

#define ASSERT_STACKDUMP_PRINT(exp, msg, ...) \
    do { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        static volatile bool _exit = false; \
        if (!IgnoreAllAsserts) { \
            if (!_ignore_assert) { \
                if (!(exp)) { \
                    Vinifera_Assert_StackDump(); \
                    Vinifera_Assert(ASSERT_NORMAL, #exp, __FILE__, __LINE__, __FUNCTION__, &_ignore_assert, &_break, &_exit, msg, ##__VA_ARGS__); \
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