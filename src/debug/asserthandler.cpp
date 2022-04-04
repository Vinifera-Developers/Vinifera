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
#include "critsection.h"
#include "debughandler.h"
#include "stackdump.h"
#include "textfile.h"
#include "fixedstring.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "tspp_assert.h"


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

/**
 *  Working stack dump buffer.
 */
static FixedString<65536> StackBuffer;

/**
 *  File instance for the stack file.
 */
static TextFileClass StackFile;


extern int Execute_Day;
extern int Execute_Month;
extern int Execute_Year;
extern int Execute_Hour;
extern int Execute_Min;
extern int Execute_Sec;


static wchar_t *to_utf16(const char *str)
{
    if (str == nullptr) {
        return nullptr;
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);

    if (!len) {
        return nullptr;
    }

    wchar_t *ret = (wchar_t *)malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, ret, len);

    return ret;
}


void Vinifera_Assert(AssertType type, const char *expr, const char *file, int line, const char *function, volatile bool *ignore, volatile bool *allow_break, volatile bool *exit, const char *msg, ...)
{
    static const char *_assert_names[] = {
        "NORMAL", "FATAL"
    };

    static SimpleCriticalSectionClass AssertMutex;
    ScopedCriticalSectionClass mutex(&AssertMutex);

    char buffer[4096];
    char msgbuff[1024];

    ++TotalAssertions;

    if (SilentAsserts || (*ignore)) {
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

    if (StackBuffer.Get_Length() > 0) {
        std::snprintf(buffer,
            sizeof(buffer),
            "Assertion failed!\n\n"
            "File: %s\nFunction: %s\nLine: %d\n\nExpression: %s\n\nMessage: %s\n\n%s\n\n"
            "For more information on how your program can cause an assertion"
            " failure, see the C++ documentation on asserts.\n\n"
            "(Press Abort to exit the application)\n"
            "(Press Retry to debug the application - JIT must be enabled)\n"
            "(Press Ignore to ignore this assertion for this session)",
            file,
            function,
            line,
            expr,
            msgbuff,
            StackBuffer.Peek_Buffer());
    } else {
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
    }

   //WWMouseClass::System_Show_Mouse();
    ShowCursor(TRUE);

    /**
     *  Display the assertion dialog.
     */
    int result = MessageBoxA(nullptr, buffer, "Assertion", MB_APPLMODAL | MB_ICONERROR| MB_ABORTRETRYIGNORE);

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
            result = MessageBoxA(nullptr, "Ignore further occurrences of this assert?", "Assertion", MB_ICONERROR | MB_YESNO | MB_DEFBUTTON1);
            if (result == IDNO) {
                *ignore = false;
                break;
            }
            *ignore = true;
            ++GlobalIgnoreCount;
            break;
    }

    //WWMouseClass::System_Hide_Mouse();
    ShowCursor(FALSE);

    /**
     *  Output the assertion to the debugger (if it is attached).
     */
    if (IsDebuggerPresent()) {
        std::snprintf(buffer, sizeof(buffer),
            "[%s] %s:%d %s Expr: \"%s\"\n", _assert_names[type], file, line, function, expr);
        Vinifera_Output_Debug_String(buffer);
    }

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

    /**
     *  Clear the previous callstack buffer.
     */
    StackBuffer.Clear();
}


/**
 *  Callback for the stack dumping routine.
 */
static void __cdecl Vinifera_Assert_StackCallback(const char *buffer)
{
    StackBuffer += buffer;
}


/**
 *  Populates the callstack buffer.
 */
void Vinifera_Assert_StackDump()
{
    /**
     *  First things we should do is dump the stack and memory.
     *  
     *  The default stack walker skip frames is 2, but we need to include
     *  the call to us also here, so make that 3.
     */
    Stack_Dump(Vinifera_Assert_StackCallback, 1);

    /**
     *  Create a unique filename for the stack dump based on the time of execution.
     */
    char filename_buffer[512];
    std::snprintf(filename_buffer, sizeof(filename_buffer), "%s\\STACK_%02u-%02u-%04u_%02u-%02u-%02u.LOG",
        Vinifera_DebugDirectory,
        Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);
        
    StackFile.Set_Name(filename_buffer);

    /**
     *  Write the buffer to the file.
     */
    StackFile.Write(StackBuffer.Peek_Buffer(), StackBuffer.Get_Length());

    DEBUG_ERROR("\n");
    DEBUG_ERROR("***** Dumping stack! *****\n");

    /**
     *  Output the stack info to the debugger.
     */
    DEBUG_ERROR("See call stack in debugger for more information.\n");
    DEBUG_ERROR("\n");
    if (!StackBuffer.Empty()) {
        DEBUG_ERROR(StackBuffer.Peek_Buffer());
        DEBUG_ERROR("\n");
    }

    static char buffer[4096];
    std::snprintf(buffer, sizeof(buffer),
        "Memory alloc error!\n\n"
        "See STACK_<date-time>.TXT in the application directory for more details.\n\n"
        "%s", StackBuffer.Peek_Buffer());
}
