/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended ScoreClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "hooker.h"
#include "scenarioext.h"
#include "syringe.h"
#include "tibsun_globals.h"


/**
 *  #issue-242
 *
 *  Allows customizing the colors of the singleplayer score screen.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x005E532A, _ScoreClass_Draw_Dual_Bars_Player_RGB_Patch, 0)
{
    R->Stack<unsigned char>(0x18, ScenExtension->ScorePlayerColor.R);
    R->Stack<unsigned char>(0x18 + 1, ScenExtension->ScorePlayerColor.G);
    R->Stack<unsigned char>(0x18 + 2, ScenExtension->ScorePlayerColor.B);
    return R->Origin() + 0xE;
}
DEFINE_HOOK_AGAIN(0x005E536B, _ScoreClass_Draw_Dual_Bars_Player_RGB_Patch, 0);
DEFINE_HOOK_AGAIN(0x005E53AC, _ScoreClass_Draw_Dual_Bars_Player_RGB_Patch, 0);

DEFINE_HOOK(0x005E53F8, _ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch, 0)
{
    R->Stack<unsigned char>(0x18, ScenExtension->ScoreEnemyColor.R);
    R->Stack<unsigned char>(0x18 + 1, ScenExtension->ScoreEnemyColor.G);
    R->Stack<unsigned char>(0x18 + 2, ScenExtension->ScoreEnemyColor.B);
    return R->Origin() + 0xD;
}
DEFINE_HOOK_AGAIN(0x005E543B, _ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch, 0);
DEFINE_HOOK_AGAIN(0x005E547E, _ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch, 0);

/**
 *  Main function for patching the hooks.
 */
void ScoreClassExtension_Hooks()
{

}
