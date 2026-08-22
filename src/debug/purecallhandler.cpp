/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Custom _purecall virtual handler.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "purecallhandler.h"

#include "debughandler.h"
#include "fatal.h"
#include "stackdump.h"
#include "stringid.h"
#include "textfile.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"

#include <windows.h>


extern int Execute_Day;
extern int Execute_Month;
extern int Execute_Year;
extern int Execute_Hour;
extern int Execute_Min;
extern int Execute_Sec;


/**
 *  These replace the CRT definitions of _purecall().
 */
extern "C" int __cdecl _purecall()
{
    Vinifera_PureCall_Handler();
    return EXIT_FAILURE;
}

extern "C" void __cxa_pure_virtual()
{
    Vinifera_PureCall_Handler();
}


/**
 *  Working buffer if the file has not been opened.
 */
static FixedString<65536> StackBuffer;

/**
 *  File instance for the stack file.
 */
static TextFileClass StackFile;

/**
 *  Was the stack file opened by us?
 */
static bool StackFileOpen = false;


/**
 *  Callback for the stack dumping routine.
 */
static void __cdecl Vinifera_PureCall_StackCallback(const char *buffer)
{
    StackBuffer += buffer;
}


extern "C" void __cdecl Vinifera_PureCall_Handler()
{
    /**
     *  First things we should do is dump the stack and memory.
     *
     *  Skip 3 frames: Stack_Dump, Vinifera_PureCall_Handler, and the CRT
     *  _purecall stub. The first reported frame is then the pure-virtual
     *  call site.
     */
    Stack_Dump(Vinifera_PureCall_StackCallback, 3);

    /**
     *  Create a unique filename for the stack dump based on the time of execution.
     */
    char filename_buffer[512];
    std::snprintf(filename_buffer, sizeof(filename_buffer), "%s\\STACK_%02u-%02u-%04u_%02u-%02u-%02u.LOG",
        Vinifera_DebugDirectory.c_str(),
        Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);
        
    StackFile.Set_Name(filename_buffer);

    /**
     *  Write the buffer to the file.
     */
    StackFile.Write(StackBuffer.c_str(), StackBuffer.size());

    DEBUG_ERROR("\n");
    DEBUG_ERROR("***** Pure virtual function called! *****\n");

    /**
     *  Output the stack info to the debugger.
     */
    DEBUG_ERROR("See call stack in debugger for more information.\n");
    DEBUG_ERROR("\n");
    if (!StackBuffer.empty()) {
        DEBUG_ERROR("{}", StackBuffer);
        DEBUG_ERROR("\n");
    }

    static char buffer[4096];
    std::snprintf(buffer, sizeof(buffer),
        "Pure virtual function called!\n\n"
        "See STACK_<date-time>.TXT in the application directory for more details.\n\n"
        "%s", StackBuffer.c_str());

    MessageBoxA(
        MainWindow,
        buffer,
        "Runtime Error!",
        MB_OK|MB_ICONEXCLAMATION
    );

    /**
     *  Trigger a break so the debugger can catch it if one is attached.
     */
    if (IsDebuggerPresent()) {
        __debugbreak();
    }
    
    Vinifera_Collect_Debug_Files();

    /**
     *  Exit gracefully.
     */
    Emergency_Exit(EXIT_FAILURE);
}
