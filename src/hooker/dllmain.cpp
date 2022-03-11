/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DLLMAIN.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Defines the entry point for the DLL.
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
#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <ctime>

#include "hooker.h"
#include "setup_hooks.h"

#include "miscutil.h"
#include "vinifera_util.h"


/**
 *  DLL module instance for fetching resources from ourself.
 */
HMODULE DLLInstance = nullptr;


/**
 *  Timestamp of execution.
 */
int Execute_Day = 0;
int Execute_Month = 0;
int Execute_Year = 0;
int Execute_Hour = 0;
int Execute_Min = 0;
int Execute_Sec = 0;
char Execute_Time_Buffer[256];


/**
 *  Use DLLMain to Set up our hooks when the DLL loads. The launcher should stall
 *  the main thread at the entry point so hooked code called after that should
 *  be our code.
 */
BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:

            if (lpReserved) {
                OutputDebugString(VINIFERA_DLL " is being loaded statically.\n");
            } else {
                OutputDebugString(VINIFERA_DLL " is being loaded dynamicly.\n");
            }

            OutputDebugString(VINIFERA_DLL " attached to " VINIFERA_TARGET_EXE ".\n");

            OutputDebugString("About to call StartHooking()...\n\n");

            if (!StartHooking()) {
                return FALSE;
            }

            /**
             *  Get the timestamp of execution. Used for generating debug log filenames.
             */
            Get_Full_Time(Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);
            std::snprintf(Execute_Time_Buffer, sizeof(Execute_Time_Buffer), "%02u-%02u-%04u_%02u-%02u-%02u",
                              Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

            /**
             *  Setup hooks and any other systems here.
             */
            Setup_Hooks();

            OutputDebugString("\n\nSetup_Hooks() done!\n\n");

            DLLInstance = hModule;

            return TRUE;

        case DLL_PROCESS_DETACH:

            OutputDebugString("\n\nAbout to call StopHooking()...\n\n");

            if (!StopHooking()) {
                return FALSE;
            }

            /**
             *  Collect the debug files from this session.
             */
            Vinifera_Collect_Debug_Files();
            
            DLLInstance = nullptr;

            OutputDebugString(VINIFERA_DLL " detached from " VINIFERA_TARGET_EXE ".\n");

            return TRUE;
            
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            OutputDebugString(VINIFERA_DLL " is not allowed to be loaded within a thread!\n");
            return FALSE;

        default:
            return FALSE;
    };
}
