/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended CD class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "cdext_hooks.h"

#include "cd.h"
#include "hooker.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class CDExt : CD
{
public:
    bool _ForceAvailable(DiskID disk);
};


/**
 *  #issue-513
 *
 *  Patch to add check for CD::IsOverrideSwap() in CD::Is_Available
 *
 *  @author: CCHyper
 */
bool CDExt::_ForceAvailable(DiskID disk)
{
    /**
     *  If the CD system has been flagged that the files are local, then
     *  return true as they are always available.
     */
    if (IsOverrideSwap()) {
        return true;
    }

    if (disk == DISK_LOCAL) {
        return true;
    }
    ThemePlaying = THEME_NONE;

    return DiskSwap::ForceAvailable(disk);
}


/**
 *  Main function for patching the hooks.
 */
void CDExtension_Hooks()
{
    Patch_Jump(0x0044E7A0, &CDExt::_ForceAvailable);
}
