/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ASSERTHANDLER.CPP
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
#include "asserthandler.h"


#ifndef NDEBUG

#include "critsection.h"
#include "debughandler.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <cassert>


/**
 *  Ignore all assertions, even under DEBUG builds.
 */
bool IgnoreAllAsserts = false;

/**
 *  Should asserts fire but not report?
 */
bool SilentAsserts = false;

/**
 *  The number of assertions to ignore on a global basis.
 */
int GlobalIgnoreCount = 0;

/**
 *  The total number of assertions.
 */
int TotalAssertions = 0;

/**
 *  Force exit application any assertion?
 */
bool ExitOnAssert = false;


static wchar_t *to_utf16(const char *str)
{
    int len;
    wchar_t *ret;

    if (str == NULL) {
        return NULL;
    }

    len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

    if (len == 0) {
        return NULL;
    }

    ret = (wchar_t *)malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, ret, len);

    return ret;
}


void Vinifera_Assert(AssertType type, const char *expr, const char *file, int line, const char *function, volatile bool *ignore, volatile bool *allow_break, volatile bool *exit, const char *msg, ...)
{
    static const char *_assert_names[] = {
        "NORMAL", "PRINT", "FATAL"
    };

    static SimpleCriticalSectionClass AssertMutex;
    ScopedCriticalSectionClass mutex(&AssertMutex);

    char buffer[4096];
    char msgbuff[1024];

    ++TotalAssertions;

    if (SilentAsserts || *ignore) {
        return;
    }

    if (msg == nullptr) {
        std::strcpy(msgbuff, "No additional information.");
    } else {
        va_list args;
        va_start(args, msg);
        std::vsnprintf(msgbuff, sizeof(msgbuff), msg, args);
        va_end(args);
    }

    /**
     *  Strip path from "file".
     */
    file = (std::strrchr(file, '\\') ? std::strrchr(file, '\\') + 1 : file);

    std::snprintf(buffer,
        sizeof(buffer),
        "Assertion failed!\n\n"
        "File: %s\nFunction: %s\nLine: %d\n\nExpression: %s\n\nMessage: %s\n\n"
        "For more information on how your program can cause an assertion"
        " failure, see the C++ documentation on asserts.\n\n"
        "(Press Abort to exit the application)\n"
        "(Press Retry to debug the application - JIT must be enabled)\n"
        "(Press Ignore to ignore this assertion for this session)",
        file,
        function,
        line,
        expr,
        msgbuff);

    /**
     *  Display the assertion dialog.
     */
    wchar_t *utf16_cap = to_utf16("Assertion");
    wchar_t *utf16_msg = to_utf16(buffer);
    int result = MessageBoxW(nullptr, utf16_msg, utf16_cap, MB_APPLMODAL | MB_ICONERROR| MB_ABORTRETRYIGNORE);
    std::free(utf16_msg);

    /**
     *  Handle the user choice.
     */
    switch (result) {
        case IDABORT:
            *allow_break = true;
            *exit = true;
            break;
        case IDRETRY:
            *allow_break = true;
        case IDIGNORE:
            utf16_cap = to_utf16("Assertion");
            utf16_msg = to_utf16("Ignore further occurrences of this assert?");
            result = MessageBoxW(nullptr, utf16_msg, utf16_cap, MB_ICONERROR | MB_YESNO | MB_DEFBUTTON1);
            if (result == IDNO) {
                *ignore = false;
                break;
            }
            *ignore = true;
            ++GlobalIgnoreCount;
            break;
    }

#ifndef NDEBUG
    /**
     *  Output the assertion to the debugger (if it is attached).
     */
    if (type == ASSERT_NORMAL) {
        std::snprintf(buffer, sizeof(buffer),
            "[ASSERT] %s:%d %s Expr: \"%s\"\n", file, line, function, expr);
    } else {
        std::snprintf(buffer, sizeof(buffer),
            "[%s] %s:%d %s Expr: \"%s\"\n", _assert_names[type], file, line, function, expr);
    }
    Vinifera_Output_Debug_String(buffer);
#endif

    /**
     *  If we are in the debugger, we want to force break.
     */
    if (!(*ignore) && IsDebuggerPresent()) {
        *allow_break = true;
    }

    /**
     *  If the assert system has been flagged to exit on all asserts, force it.
     */
    if (ExitOnAssert) {
        *exit = true;
    }
}

#endif
