/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PURECALLHANDLER.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Custom _purecall virtual handler.
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
#include "purecallhandler.h"
#include "stackdump.h"
#include "textfile.h"
#include "fatal.h"
#include "fixedstring.h"
#include "debughandler.h"
#include "tibsun_globals.h"
#include <Windows.h>
#include <string>


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
     *  The default stack walker skip frames is 2, but we need to include
     *  the call to us also here, so make that 3.
     */
    Stack_Dump(Vinifera_PureCall_StackCallback, 1);

    /**
     *  Create a unique filename for the stack dump based on the time of execution.
     */
    char filename_buffer[512];
    std::snprintf(filename_buffer, sizeof(filename_buffer), "STACK_%02u-%02u-%04u_%02u-%02u-%02u.TXT",
        Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);
        
    StackFile.Set_Name(filename_buffer);

    /**
     *  Write the buffer to the file.
     */
    StackFile.Write(StackBuffer.Peek_Buffer(), StackBuffer.Get_Length());

    DEBUG_ERROR("\n");
    DEBUG_ERROR("***** Pure virtual function called! *****\n");

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
        "Pure virtual function called!\n\n"
        "See STACK_<date-time>.TXT in the application directory for more details.\n\n"
        "%s", StackBuffer.Peek_Buffer());

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

    /**
     *  Exit gracefully.
     */
    Emergency_Exit(EXIT_FAILURE);
}
