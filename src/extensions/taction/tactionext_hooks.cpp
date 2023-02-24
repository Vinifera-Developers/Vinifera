/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TriggerClass.
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
#include "tactionext_hooks.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "house.h"
#include "housetype.h"
#include "session.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Helper function. Flags all houses aside from the winner
 *  as defeated.
 *
 *  @author: Rampastring
 */
void TAction_Win_Flag_Houses(HousesType winner)
{
    /**
     *  Flag the player as won or lost, like in the original code.
     */
    if (PlayerPtr->Class->House == winner) {
        PlayerPtr->Flag_To_Win(false);
    } else {
        PlayerPtr->Flag_To_Lose(false);
    }

    /**
     *  Mark all other houses than the winner as defeated.
     */
    for (int i = 0; i < Houses.Count(); i++) {
        HouseClass *house = Houses[i];

        if (house->Class->House != winner) {
            house->IsDefeated = true;
        }
    }
}


/**
 *  #issue-965
 *
 *  Makes the "Winner is" trigger action set the IsDefeated flag on losing
 *  houses in multiplayer.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_TAction_Win_FlagLosersAsDefeatedInMultiplayer)
{
    GET_STACK_STATIC(HousesType, housestype, esi, 0x40);

    if (Session.Type == GAME_NORMAL) {
        goto original_code;
    }

    TAction_Win_Flag_Houses(housestype);

    /**
     *  The action has done its job, return true.
     */
return_true:
    JMP_REG(ebx, 0x00619FF6);

original_code:
    /**
     *  Stolen bytes / code.
     */
    _asm { mov ecx, [PlayerPtr] }
    _asm { mov ecx, [ecx]}
    JMP(0x00619FE1);
}


/**
 *  Helper function. Flags the losers as defeated.
 *
 *  @author: Rampastring
 */
void TAction_Lose_Flag_Houses(HousesType loser)
{
    /**
     *  Flag the player as won or lost, like in the original code.
     */
    if (PlayerPtr->Class->House == loser) {
        PlayerPtr->Flag_To_Lose(false);
    } else {
        PlayerPtr->Flag_To_Win(false);
    }

    /**
     *  Mark all losers as defeated.
     */
    for (int i = 0; i < Houses.Count(); i++) {
        HouseClass* house = Houses[i];

        if (house->Class->House == loser) {
            house->IsDefeated = true;
        }
    }
}


/**
 *  #issue-965
 *
 *  Makes the "Loser is" trigger action set the IsDefeated flag on the
 *  losing house in multiplayer.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_TAction_Lose_FlagLoserAsLostInMultiplayer)
{
    GET_STACK_STATIC(HousesType, housestype, esi, 0x40);

    if (Session.Type == GAME_NORMAL) {
        goto original_code;
    }

    TAction_Lose_Flag_Houses(housestype);

    /**
     *  The action has done its job, return true.
     */
return_true:
    JMP_REG(ebx, 0x0061A020);

original_code:
    /**
     *  Stolen bytes / code.
     */
    _asm { mov ecx, [PlayerPtr] }
    _asm { mov ecx, [ecx]}
    JMP(0x0061A00B);
}


/**
 *  Main function for patching the hooks.
 */
void TActionClassExtension_Hooks()
{
    /**
     *  #issue-674
     * 
     *  Fixes a bug where the game would crash when TACTION_WAKEUP_GROUP was
     *  executed but the game was not able to match the Group to the triggers
     *  group. This was because the game was searching the Foots vector with
     *  the count of the Technos vector, and in cases where the Group did
     *  not match, the game would crash trying to search out of bounds.
     * 
     *  @author: CCHyper
     */
    Patch_Dword(0x00619552+2, (0x007E4820+4)); // Foot vector to Technos vector.

    Patch_Jump(0x00619FDB, &_TAction_Win_FlagLosersAsDefeatedInMultiplayer);
    Patch_Jump(0x0061A005, &_TAction_Lose_FlagLoserAsLostInMultiplayer);
}
