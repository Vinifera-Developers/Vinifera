/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Provides methods for accessing data and functions in an existing
 *          binary and replacing functions with new implementations from an
 *          injected DLL.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "hooker.h"

#include "asserthandler.h"
#include "mapview.h"


static DWORD OriginalCodeProtect = 0;
static DWORD OriginalDataProtect = 0;

static bool HookingFlag = false;


/**
 *  Unprotects the binary, run before patches are applied.
 */
bool StartHooking()
{
    if (HookingFlag) {
        return true;
    }

    OutputDebugString("StartHooking()...\n\n");

    bool success = false;
    ImageSectionInfo info;

    if (GetModuleSectionInfo(info)) {
        success = true;
        HANDLE process = GetCurrentProcess();
        if (VirtualProtectEx(process, info.BaseOfCode, info.SizeOfCode, PAGE_EXECUTE_READWRITE, &OriginalCodeProtect) == FALSE) {
            success = false;
            ASSERT_FATAL_PRINT(success == true, "Failed to change code section permissions!");
        }
        if (VirtualProtectEx(process, info.BaseOfData, info.SizeOfData, PAGE_EXECUTE_READWRITE, &OriginalDataProtect) == FALSE) {
            success = false;
            ASSERT_FATAL_PRINT(success == true, "Failed to change data section permissions!");
        }
    }

    ASSERT_FATAL(success == true);

    HookingFlag = success;

    return success;
}


/**
 *  Restores protection on the binary, run once patches have been applied.
 */
bool StopHooking()
{
    OutputDebugString("StopHooking()...\n\n");

    bool success = false;
    DWORD old_protect;
    ImageSectionInfo info;

    if (GetModuleSectionInfo(info)) {
        success = true;
        HANDLE process = GetCurrentProcess();
        if (VirtualProtectEx(process, info.BaseOfCode, info.SizeOfCode, OriginalCodeProtect, &old_protect) == FALSE) {
            success = false;
            ASSERT_FATAL_PRINT(success == true, "Failed to change code section permissions!");
        }
        if (VirtualProtectEx(process, info.BaseOfData, info.SizeOfData, OriginalDataProtect, &old_protect) == FALSE) {
            success = false;
            ASSERT_FATAL_PRINT(success == true, "Failed to change data section permissions!");
        }
    }
    
    ASSERT_FATAL(success == true);

    HookingFlag = false;

    return success;
}
