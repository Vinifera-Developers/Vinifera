/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended MultiMission class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "asserthandler.h"
#include "hooker.h"


/**
 *  Main function for patching the hooks.
 */
void MultiMissionExtension_Hooks()
{
    /**
     *  #issue-8
     *  
     *  Fixes MultiMission "MaxPlayers" incorrectly loaded with "MinPlayers".
     * 
     *  @author: CCHyper
     */
    static const char *TEXT_MAXPLAYERS = "MaxPlayers";
    Patch_Dword(0x005EF124+1, (uintptr_t)TEXT_MAXPLAYERS); // +1 skips "push" opcode
    Patch_Dword(0x005EF5E4+1, (uintptr_t)TEXT_MAXPLAYERS); // +1 skips "push" opcode
}
