/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended EnvironmentClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "environmentext_hooks.h"
#include "environmentext.h"

#include "debughandler.h"
#include "environment.h"
#include "hooker.h"


/**
 *  Main function for patching the hooks.
 */
void EnvironmentExtension_Hooks()
{
    Patch_Jump(0x004938A0, &ExtEnvironmentClass::Snapshot_Game_State);
    Patch_Jump(0x00493920, &ExtEnvironmentClass::Apply_To_Game_State);
    Patch_Jump(0x00493A30, &ExtEnvironmentClass::Load);
    Patch_Jump(0x00493A50, &ExtEnvironmentClass::Save);
    Patch_Jump(0x00493860, &ExtEnvironmentClass::Hook_Ctor);
    Patch_Jump(0x00493890, &ExtEnvironmentClass::Hook_Dtor);
    Patch_Jump(0x00493810, &ExtEnvironmentClass::Static_Init);
    Patch_Jump(0x00493850, &ExtEnvironmentClass::Static_Deinit);
}
