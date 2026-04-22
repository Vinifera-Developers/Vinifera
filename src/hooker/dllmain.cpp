/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Defines the entry point for the DLL.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "hooker.h"
#include "miscutil.h"
#include "setup_hooks.h"
#include "vinifera_util.h"

#include <cstdio>
#include <windows.h>


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
        {
            if (lpReserved) {
                OutputDebugString(VINIFERA_DLL " is being loaded statically.\n");
            } else {
                OutputDebugString(VINIFERA_DLL " is being loaded dynamically.\n");
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
        }

        case DLL_PROCESS_DETACH:
        {
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
        }
            
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            OutputDebugString(VINIFERA_DLL " is not allowed to be loaded within a thread!\n");
            return FALSE;

        default:
            return FALSE;
    };
}
