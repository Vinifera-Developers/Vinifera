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
#include "vector.h"
#include "scenarioext.h"

#include "hooker.h"
#include "syringe.h"


/**
 *  Macro for applying color to the score bar.
 *  Saves and restores ecx to avoid the compiler trashing it
 *  since in every case it is used after reading in the color.
 */
#define APPLY_SCORE_BAR_COLOR(color, jumpaddr) \
R->Stack<unsigned char>(0x18, (color).R); \
R->Stack<unsigned char>(0x18 + 1, (color).G); \
R->Stack<unsigned char>(0x18 + 2, (color).B); \
return (jumpaddr)


/**
 *  #issue-242
 *
 *  Allows customizing the colors of the singleplayer score screen.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x005E532A, _ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_1, 0)
{
    APPLY_SCORE_BAR_COLOR(ScenExtension->ScorePlayerColor, 0x005E5338);
}

DEFINE_HOOK(0x005E536B, _ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_2, 0)
{
    APPLY_SCORE_BAR_COLOR(ScenExtension->ScorePlayerColor, 0x005E5379);
}

DEFINE_HOOK(0x005E53AC, _ScoreClass_Draw_Dual_Bars_Player_RGB_Patch_3, 0)
{
    APPLY_SCORE_BAR_COLOR(ScenExtension->ScorePlayerColor, 0x005E53BA);
}

DEFINE_HOOK(0x005E53F8, _ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_1, 0)
{
    APPLY_SCORE_BAR_COLOR(ScenExtension->ScoreEnemyColor, 0x005E5405);
}

DEFINE_HOOK(0x005E543B, _ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_2, 0)
{
    APPLY_SCORE_BAR_COLOR(ScenExtension->ScoreEnemyColor, 0x005E5448);
}

DEFINE_HOOK(0x005E547E, _ScoreClass_Draw_Dual_Bars_Enemy_RGB_Patch_3, 0)
{
    APPLY_SCORE_BAR_COLOR(ScenExtension->ScoreEnemyColor, 0x005E548B);
}

/**
 *  Main function for patching the hooks.
 */
void ScoreClassExtension_Hooks()
{

}

