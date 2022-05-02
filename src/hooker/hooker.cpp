/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOOKER.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Provides methods for accessing data and functions in an
 *                 existing binary and replacing functions with new 
 *                 implementations from an injected DLL.
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
#include "hooker.h"
#include "mapview.h"
#include "asserthandler.h"


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
        if (VirtualProtectEx(process, info.BaseOfData, info.SizeOfData, PAGE_READWRITE, &OriginalDataProtect) == FALSE) {
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
