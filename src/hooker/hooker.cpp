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

#include <cstring>

struct ProtectedSectionInfo
{
    LPVOID Base;
    SIZE_T Size;
    DWORD OriginalProtect;
    char Name[IMAGE_SIZEOF_SHORT_NAME + 1];
};

static ProtectedSectionInfo ProtectedSections[MAX_MODULE_SECTIONS];
static int ProtectedSectionCount = 0;

static bool HookingFlag = false;


static bool RestoreProtectedSections()
{
    bool success = true;

    for (int index = ProtectedSectionCount - 1; index >= 0; --index) {
        DWORD old_protect;
        ProtectedSectionInfo &section = ProtectedSections[index];

        if (VirtualProtect(section.Base, section.Size, section.OriginalProtect, &old_protect) == FALSE) {
            DWORD error = GetLastError();
            success = false;
            ASSERT_FATAL_PRINT(success == true, "Failed to restore permissions for section {} at 0x{:X} (size 0x{:X}, error {})!",
                section.Name, reinterpret_cast<uintptr_t>(section.Base), section.Size, error);
            break;
        }
    }

    if (success) {
        ProtectedSectionCount = 0;
    }

    return success;
}


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
        ProtectedSectionCount = 0;

        for (int index = 0; index < info.SectionCount; ++index) {
            DWORD original_protect;
            const ImageSectionRange &section = info.Sections[index];

            if (VirtualProtect(section.Base, section.Size, PAGE_EXECUTE_READWRITE, &original_protect) == FALSE) {
                DWORD error = GetLastError();
                success = false;
                RestoreProtectedSections();
                ASSERT_FATAL_PRINT(success == true, "Failed to change permissions for section {} at 0x{:X} (size 0x{:X}, error {})!",
                    section.Name, reinterpret_cast<uintptr_t>(section.Base), section.Size, error);
                break;
            }

            ProtectedSectionInfo &protected_section = ProtectedSections[ProtectedSectionCount++];
            protected_section.Base = section.Base;
            protected_section.Size = section.Size;
            protected_section.OriginalProtect = original_protect;
            std::memcpy(protected_section.Name, section.Name, sizeof(protected_section.Name));
        }
    } else {
        ASSERT_FATAL_PRINT(success == true, "Failed to get module section info!");
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

    if (!HookingFlag) {
        return true;
    }

    bool success = RestoreProtectedSections();
    
    ASSERT_FATAL(success == true);

    HookingFlag = false;

    return success;
}
