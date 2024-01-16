/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SCORECLASSEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended ScoreClass.
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
#include "multiscoreext_hooks.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "tibsun_globals.h"
#include "house.h"
#include "vector.h";
#include "scenarioext.h"

#include "hooker.h"
#include "hooker_macros.h"

static char r;
static char g;
static char b;

/**
 *  Macro for applying color to the score bar.
 *  Saves and restores ecx to avoid the compiler trashing it
 *  since in every case it is used after reading in the color.
 */
#define APPLY_SCORE_BAR_COLOR(funcname, jumpaddr) \
_asm { push ecx } \
funcname(); \
_asm { mov al, byte ptr ds:r } \
_asm { mov [esp+0x1C], al } \
_asm { mov al, byte ptr ds:g } \
_asm { mov [esp+0x1C+1], al } \
_asm { mov al, byte ptr ds:b } \
_asm { mov [esp+0x1C+2], al } \
_asm { pop ecx } \
JMP(jumpaddr)


void Fetch_Player_Score_Color_From_ScenExtension()
{
    r = ScenExtension->ScorePlayerColor.R;
    g = ScenExtension->ScorePlayerColor.G;
    b = ScenExtension->ScorePlayerColor.B;
}


void Fetch_Enemy_Score_Color_From_ScenExtension()
{
    r = ScenExtension->ScoreEnemyColor.R;
    g = ScenExtension->ScoreEnemyColor.G;
    b = ScenExtension->ScoreEnemyColor.B;
}


/**
 *  #issue-242
 *
 *  Allows customizing the colors of the singleplayer score screen.
 *
 *  Author: Rampastring
 */
DECLARE_PATCH(_ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_1)
{
    APPLY_SCORE_BAR_COLOR(Fetch_Player_Score_Color_From_ScenExtension, 0x005E5338);
}

DECLARE_PATCH(_ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_2)
{
    APPLY_SCORE_BAR_COLOR(Fetch_Player_Score_Color_From_ScenExtension, 0x005E5379);
}

DECLARE_PATCH(_ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_3)
{
    APPLY_SCORE_BAR_COLOR(Fetch_Player_Score_Color_From_ScenExtension, 0x005E53BA);
}

DECLARE_PATCH(_ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_1)
{
    APPLY_SCORE_BAR_COLOR(Fetch_Enemy_Score_Color_From_ScenExtension, 0x005E5405);
}

DECLARE_PATCH(_ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_2)
{
    APPLY_SCORE_BAR_COLOR(Fetch_Enemy_Score_Color_From_ScenExtension, 0x005E5448);
}

DECLARE_PATCH(_ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_3)
{
    APPLY_SCORE_BAR_COLOR(Fetch_Enemy_Score_Color_From_ScenExtension, 0x005E548B);
}

/**
 *  Main function for patching the hooks.
 */
void ScoreClassExtension_Hooks()
{
    Patch_Jump(0x005E532A, &_ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_1);
    Patch_Jump(0x005E536B, &_ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_2);
    Patch_Jump(0x005E53AC, &_ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_3);
    Patch_Jump(0x005E53F8, &_ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_1);
    Patch_Jump(0x005E543B, &_ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_2);
    Patch_Jump(0x005E547E, &_ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_3);
}
