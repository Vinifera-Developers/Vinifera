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


/**
 *  We need to use a static object so any global instances are initialised
 *  before we hook anything into the binary. Place any priority classes here.
 */
class StaticInitObject
{
	public:
		StaticInitObject() {}
		~StaticInitObject() {}
};
StaticInitObject InitHooks;


/**
 *  Checks if a file exists in the directory.
 */
static bool File_Exists(const char *file)
{
    WIN32_FIND_DATA fileinfo;
    HANDLE handle = FindFirstFile(file, &fileinfo) ;
    bool found = (handle != INVALID_HANDLE_VALUE);
    if (found) {
        FindClose(handle);
    }
    return found;
}


/**
 *  Use DLLMain to Set up our hooks when the DLL loads. The launcher should stall
 *  the main thread at the entry point so hooked code called after that should
 *  be our code.
 */
BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            std::printf("Attaching DLL to process.\n");
            StartHooking();

            /**
             *  Get the timestamp of execution. Used for generating debug log filenames.
             */
            Get_Full_Time(Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

            /**
             *  Setup hooks and any other systems here.
             */
            Setup_Hooks();
			
            DLLInstance = hModule;
            break;

        case DLL_PROCESS_DETACH:
            std::printf("Detaching DLL from process.\n");
            StopHooking();
			
            /**
             *  Shutdown systems here.
             */

            
            DLLInstance = nullptr;
            break;
            
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;
    };

    return TRUE;
}
