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
