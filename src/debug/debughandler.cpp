/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DEBUGHANDLER.CPP
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
#include "debughandler.h"
#include "critsection.h"
#include "cpudetect.h"
#include "rawfile.h"
#include "tibsun_globals.h"
#include "tspp_gitinfo.h"
#include "vinifera_gitinfo.h"
#include "vinifera_globals.h"
#include <cstdio>
#include <cstring>
#include <conio.h>
#include <iostream>
#include <wincon.h>
#include <Windows.h> // For OutputDebugString().
#include <fstream>


/**
 *  Constants and formats for the debug console output.
 */

// The escape character that begins all VT sequences
#define AICLI_VT_ESCAPE     "\x1b"

// The beginning of a Control Sequence Introducer
#define AICLI_VT_CSI        AICLI_VT_ESCAPE "["

// The beginning of an Operating system command
#define AICLI_VT_OSC        AICLI_VT_ESCAPE "]"

// Define a text formatting sequence with an integer id
#define AICLI_VT_TEXTFORMAT(_id_)       AICLI_VT_CSI #_id_ "m"

// Color and format defines.
// https://superuser.com/questions/413073/windows-console-with-ansi-colors-handling
#define AICLI_STRONG_RED        AICLI_VT_TEXTFORMAT(91)
#define AICLI_STRONG_GREEN      AICLI_VT_TEXTFORMAT(92)
#define AICLI_STRONG_YELLOW     AICLI_VT_TEXTFORMAT(93)
#define AICLI_STRONG_BLUE       AICLI_VT_TEXTFORMAT(94)
#define AICLI_STRONG_MAGENTA    AICLI_VT_TEXTFORMAT(95)
#define AICLI_STRONG_CYAN       AICLI_VT_TEXTFORMAT(96)
#define AICLI_STRONG_WHITE      AICLI_VT_TEXTFORMAT(97)

#define AICLI_BOLD              AICLI_VT_TEXTFORMAT(1)
#define AICLI_UNDERLINE         AICLI_VT_TEXTFORMAT(4)
#define AICLI_INVERSE           AICLI_VT_TEXTFORMAT(7)


extern int Execute_Day;
extern int Execute_Month;
extern int Execute_Year;
extern int Execute_Hour;
extern int Execute_Min;
extern int Execute_Sec;


/**
 *  Debug log filename
 */
char DebugLogFilename[PATH_MAX] = { '\0' };

/**
 *  File instance for the debug log.
 */
static std::ofstream DebugLogFile;

/**
 *  Was the log file opened by us?
 */
static bool DebugLogFileOpen = false;

/**
 *  Is the debug console active and available to output to?
 */
static bool DebugConsoleActive = false;

static HANDLE DebugConsoleHandle = nullptr;

static bool DisableDebuggerOutput = false;

static bool DebugHandler_DeveloperMode = false;


void Vinifera_Output_Debug_String(const char *string)
{
    if (DisableDebuggerOutput || !IsDebuggerPresent()) return;
    return OutputDebugString(string);
}


static void Output_To_Console(/*const char *cmd, */const char *string)
{
    if (!DebugConsoleActive) return;

    std::cout << string;

    //std::fprintf(stderr, string);
    //std::fflush(stderr);
}


static BOOL WINAPI SetConsoleIcon(HICON hIcon)
{
    typedef BOOL (WINAPI *SetConsoleIconFunc)(HICON);

    static SetConsoleIconFunc pSetConsoleIcon = NULL;
    if (pSetConsoleIcon == nullptr) {
        pSetConsoleIcon = (SetConsoleIconFunc)GetProcAddress(GetModuleHandle("kernel32"), "SetConsoleIcon");
    }
    if (pSetConsoleIcon == nullptr) {
        return FALSE;
    }
    return pSetConsoleIcon(hIcon);
}


static bool Set_Console_Properties(HANDLE handle, int x, int y, int w, int h)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD coordMax;

    coordMax = GetLargestConsoleWindowSize(handle);

    if (h > coordMax.Y) {
        h = coordMax.Y;
    }

    if (w > coordMax.X) {
        w = coordMax.X;
    }

    if (!GetConsoleScreenBufferInfo(handle, &info)) {
        return false;
    }

    // Height
    info.srWindow.Left = 0;
    info.srWindow.Right = info.dwSize.X - 1;
    info.srWindow.Top = 0;
    info.srWindow.Bottom = h - 1;

    if (h < info.dwSize.Y) {
        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }

        info.dwSize.Y = h;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }
    } else if (h > info.dwSize.Y) {
        info.dwSize.Y = h;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }

        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }
    }

    if (!GetConsoleScreenBufferInfo(handle, &info)) {
        return false;
    }

    // Width
    info.srWindow.Left = 0;
    info.srWindow.Right = w - 1;
    info.srWindow.Top = 0;
    info.srWindow.Bottom = info.dwSize.Y - 1;

    if (w < info.dwSize.X) {
        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }

        info.dwSize.X = w;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }
    } else if (w > info.dwSize.X) {
        info.dwSize.X = w;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }

        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }
    }

    SetWindowPos(GetConsoleWindow(), 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    return true;
}


static void Debug_Console_Wait_For_Input()
{
    if (!DebugConsoleActive) return;

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), AICLI_STRONG_MAGENTA "*** WAITING FOR USER INPUT - PRESS ANY KEY TO CONTINUE ***\n");
    Output_To_Console(buffer);

    while (!_getch()) {
        Sleep(100);
    }
}


static void __cdecl Debug_Console_Shutdown()
{
    DebugConsoleActive = false;
}


static void Debug_Console_Init()
{
    static bool _onetime = false;
    if (!_onetime) {
        std::atexit(Debug_Console_Shutdown);
        _onetime = true;
    }

    /**
     *  Attach to the console that started us if any.
     */
    //AttachConsole(ATTACH_PARENT_PROCESS);
    AllocConsole();

    /**
     *  Redirect stderr/stdout/stdin to new console.
     */

    /**
     *  Redirect the CRT standard input, output, and error handles to the console.
     */
    std::freopen("CONIN$", "r", stdin);
    std::freopen("CONOUT$", "w", stdout);
    std::freopen("CONOUT$", "w", stderr);

    /**
     *  Note that there is no CONERR$ file
     */
    HANDLE hStdout = CreateFile("CONOUT$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hStdin = CreateFile("CONIN$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
    SetStdHandle(STD_ERROR_HANDLE, hStdout);
    SetStdHandle(STD_INPUT_HANDLE, hStdin);

    DebugConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!DebugConsoleHandle || DebugConsoleHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    /**
     *  Set console size.
     */
    Set_Console_Properties(DebugConsoleHandle, 0, 0, 82, 60);

    COORD buffsize = { 82, 4096 };
    SetConsoleScreenBufferSize(DebugConsoleHandle, buffsize);

    /**
     *  Set console to allow ANSI colours.
     */
    DWORD dwMode = 0;
    if (!GetConsoleMode(DebugConsoleHandle, &dwMode)) {
        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(DebugConsoleHandle, dwMode)) {
        /**
         *  Fallback to set all text colour to green.
         */
        SetConsoleTextAttribute(DebugConsoleHandle, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
    }

    SetConsoleTitle("Vinifera Debug Console");

    DebugConsoleActive = true;
}


static void Debug_Announce()
{
    /**
     *  MSVC does not need to know this information.
     */
    DisableDebuggerOutput = true;

    DEBUG_INFO("--------------------------------------------------------------------------------\n");
    DEBUG_INFO("--------------------  V I N I F E R A   D E B U G   L O G  ---------------------\n"); 
    DEBUG_INFO("--------------------------------------------------------------------------------\n");
    DEBUG_INFO("\n");
    DEBUG_INFO("TS++ author: %s\n", TSPP_Git_Author());
    DEBUG_INFO("TS++ date: %s\n", TSPP_Git_DateTime());
    DEBUG_INFO("TS++ branch: %s\n", TSPP_Git_Branch());
    DEBUG_INFO("TS++ commit: %s\n", TSPP_Git_Hash_Short());
    DEBUG_INFO("TS++ local changes: %s\n", TSPP_Git_Uncommitted_Changes() ? "YES" : "NO");
    DEBUG_INFO("Vinifera author: %s\n", Vinifera_Git_Author());
    DEBUG_INFO("Vinifera date: %s\n", Vinifera_Git_DateTime());
    DEBUG_INFO("Vinifera branch: %s\n", Vinifera_Git_Branch());
    DEBUG_INFO("Vinifera commit: %s\n", Vinifera_Git_Hash_Short());
    DEBUG_INFO("Vinifera local changes: %s\n", Vinifera_Git_Uncommitted_Changes() ? "YES" : "NO");
    DEBUG_INFO("\n");
    DEBUG_INFO(CPUDetectClass::Get_Processor_Log());
    //DEBUG_INFO("\n"); // Get_Processor_Log writes a new line for us.
    DEBUG_INFO("--------------------------------------------------------------------------------\n");
    DEBUG_INFO("\n");

    DisableDebuggerOutput = false;
}


void __cdecl Vinifera_Debug_Handler_Startup()
{
    static bool _onetime = false;
    if (_onetime) return;
    _onetime = true;

    std::atexit(Vinifera_Debug_Handler_Shutdown);

    /**
     *  Workaround for enabling the debug console before the command lines are process by the game.
     */
    const char *cmdline = GetCommandLineA();
    DebugHandler_DeveloperMode = Vinifera_DeveloperMode || (std::strstr(cmdline, "-DEVELOPER") != nullptr);
    bool enable_console = Vinifera_DeveloperMode || (std::strstr(cmdline, "-CONSOLE") != nullptr);

#ifdef NDEBUG
    if (DebugHandler_DeveloperMode) {
#endif
    if (enable_console || IsDebuggerPresent() || (MessageBox(nullptr, "Enable debug output console?", "Debug Console", MB_YESNO) == IDYES)) {
        Debug_Console_Init();
    }

//#ifndef NDEBUG
//    /**
//     *  Halt the program until the user is ready to continue.  
//     */
//    if (!IsDebuggerPresent()) {
//        Debug_Console_Wait_For_Input();
//    }
//#endif

#ifdef NDEBUG
    } // DebugHandler_DeveloperMode
#endif

    /**
     *  Create a unique filename for the debug log based on the current time.
     */
    std::snprintf((char *)DebugLogFilename, sizeof(DebugLogFilename), "%s\\DEBUG_%02u-%02u-%04u_%02u-%02u-%02u.LOG",
        Vinifera_DebugDirectory,
        Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

    /**
     *  Make sure the first thing in the log file is the header.
     */
    Debug_Announce();
}


void __cdecl Vinifera_Debug_Handler_Shutdown()
{
    if (DebugLogFileOpen) {
        DebugLogFile.flush();
        DebugLogFile.close();
    }
}


void Vinifera_Printf(DebugType type, const char *file, const char *function, int line, const char *fmt, ...)
{
    static SimpleCriticalSectionClass DebugMutex;
    ScopedCriticalSectionClass mutex(&DebugMutex);
    
    char buffer[4096];
    char tmpbuff[4096];
    char filebuff[4096];
    bool write_to_file = false;

    va_list args;
    va_start(args, fmt);

    /**
     *  Fill the buffer with the arguments.
     */
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);

    /**
     *  Strip path from "file".
     */
    if (file != nullptr) {
        file = (std::strrchr(file, '\\') ? std::strrchr(file, '\\') + 1 : file);
    }

    switch (type) {

        default:
        case DEBUGTYPE_GAME:
        {
#ifdef NDEBUG
            if (DebugHandler_DeveloperMode || Vinifera_DeveloperMode) {
#endif
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_BLUE "%s", buffer);
            Output_To_Console(tmpbuff);
#ifdef NDEBUG
            } // DebugHandler_DeveloperMode
#endif

            Vinifera_Output_Debug_String(buffer);

            std::snprintf(filebuff, sizeof(filebuff), "%s", buffer);

            write_to_file = true;

            break;
        }

        case DEBUGTYPE_GAME_LINE:
        {
#ifdef NDEBUG
            if (DebugHandler_DeveloperMode || Vinifera_DeveloperMode) {
#endif
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_CYAN "%s", buffer);
            Output_To_Console(tmpbuff);
#ifdef NDEBUG
            } // DebugHandler_DeveloperMode
#endif

            Vinifera_Output_Debug_String(buffer);

            std::snprintf(filebuff, sizeof(filebuff), "%s", buffer);

            write_to_file = true;

            break;
        }

        case DEBUGTYPE_NORMAL:
        case DEBUGTYPE_INFO:
        {
#ifdef NDEBUG
            if (DebugHandler_DeveloperMode || Vinifera_DeveloperMode) {
#endif
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_GREEN "%s", buffer);
            Output_To_Console(tmpbuff);
#ifdef NDEBUG
            } // DebugHandler_DeveloperMode
#endif

            Vinifera_Output_Debug_String(buffer);

            std::snprintf(filebuff, sizeof(filebuff), "%s", buffer);

            write_to_file = true;

            break;
        }

        case DEBUGTYPE_WARNING:
        {
#ifdef NDEBUG
            if (DebugHandler_DeveloperMode || Vinifera_DeveloperMode) {
#endif
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_YELLOW "%s", buffer);
            Output_To_Console(tmpbuff);
#ifdef NDEBUG
            } // DebugHandler_DeveloperMode
#endif

            Vinifera_Output_Debug_String(buffer);

            std::snprintf(filebuff, sizeof(filebuff), "[WARNING] %s", buffer);

            write_to_file = true;

            break;
        }

        case DEBUGTYPE_ERROR:
        {
#ifdef NDEBUG
            if (DebugHandler_DeveloperMode || Vinifera_DeveloperMode) {
#endif
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_RED "%s", buffer);
            Output_To_Console(tmpbuff);
#ifdef NDEBUG
            } // DebugHandler_DeveloperMode
#endif

            Vinifera_Output_Debug_String(buffer);

            std::snprintf(filebuff, sizeof(filebuff), "[ERROR] %s", buffer);

            write_to_file = true;

            break;
        }

        case DEBUGTYPE_FATAL:
        {
#ifdef NDEBUG
            if (DebugHandler_DeveloperMode || Vinifera_DeveloperMode) {
#endif
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_RED "%s", buffer);
            Output_To_Console(tmpbuff);
#ifdef NDEBUG
            } // DebugHandler_DeveloperMode
#endif

            Vinifera_Output_Debug_String(buffer);

            std::snprintf(filebuff, sizeof(filebuff), "[FATAL] %s", buffer);

            write_to_file = true;

            break;
        }

#ifndef NDEBUG
        case DEBUGTYPE_TRACE:
        {
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_WHITE "%s", "TRACE!\n");
            Output_To_Console(tmpbuff);

            char tracebuff[4096];
            std::snprintf(tracebuff,
                sizeof(tracebuff),
                "  File: %s\n"
                "  Func: %s\n"
                "  Line: %d\n"
                "  Msg:  %s"
                "\n"
                ,
                file, function, line, buffer);

            Vinifera_Output_Debug_String(tracebuff);
            
            std::snprintf(tmpbuff, sizeof(tmpbuff), AICLI_STRONG_WHITE "%s", tracebuff);
            Output_To_Console(tmpbuff);

            Debug_Console_Wait_For_Input();

            break;
        }
#endif

#ifndef NDEBUG
        /**
         *  Output to the debugger only.
         */
        case DEBUGTYPE_DEBUGGER:
        {
            std::snprintf(tmpbuff,
                sizeof(tmpbuff),
                "%s",
                buffer);
            Vinifera_Output_Debug_String(tmpbuff);
            break;
        }
#endif

#ifndef NDEBUG
        /**
         *  Output to the debugger only with trace information.
         */
        case DEBUGTYPE_DEBUGGER_TRACE:
        {
            std::snprintf(tmpbuff,
                sizeof(tmpbuff),
                "%s %s %d: %s\n",
                file, function, line, buffer);
            Vinifera_Output_Debug_String(tmpbuff);
            break;
        }
#endif

    };

    /**
     *  Write the log file if flagged to do so.
     */
    if (write_to_file) {

        if (!DebugLogFileOpen) {
            DebugLogFile.open(DebugLogFilename, std::ios::app|std::ios::binary);
            DebugLogFileOpen = true;
        }

        /**
         *  Write the buffer to the log file.
         */
        DebugLogFile << filebuff;

        if (DebugLogFileOpen) {
            DebugLogFile.close();
            DebugLogFileOpen = false;
        }
    }

    va_end(args);
}
