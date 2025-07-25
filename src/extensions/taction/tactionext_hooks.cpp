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
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "trigger.h"
#include "triggertype.h"
#include "taction.h"
#include "scenario.h"
#include "scenarioext.h"
#include "voc.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "house.h"
#include "housetype.h"
#include "session.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "mouse.h"
#include "rules.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TActionClass)
{
public:
    bool _Do_PLAY_SOUND_RANDOM(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);

};


/**
 *  #issue-71
 *
 *  Reimplement Do_PLAY_SOUND_RANDOM to support the new waypoint limit.
 *
 *  @author: CCHyper
 */
bool TActionClassExt::_Do_PLAY_SOUND_RANDOM(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Cell list[NEW_WAYPOINT_COUNT];
    int count = 0;

    /**
     *  Make a list of all the valid waypoints in this scenario.
     */
    for (WaypointType index = WAYPOINT_FIRST; index < NEW_WAYPOINT_COUNT; ++index) {
        if (ScenExtension->Is_Waypoint_Valid(index)) {
            list[count++] = ScenExtension->Waypoint_CellClass(index);
            if (count >= std::size(list)) break;
        }
    }

    /**
     *  Pick a random cell from the valid waypoint list and play the desired sound.
     */
    Static_Sound(Data.Sound, list[Random_Pick(0u, std::size(list) - 1)].As_Coord());
    return true;
}


/**
 *  #issue-71
 *
 *  Replace inlined instance of Do_PLAY_SOUND_RANDOM.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_TActionClass_Operator_Do_PLAY_SOUND_RANDOM_Remove_Inline_Patch)
{
    GET_REGISTER_STATIC(TActionClass *, this_ptr, esi);
    GET_REGISTER_STATIC(ObjectClass *, object, ecx);
    GET_STACK_STATIC(Cell *, cell, esp, 0x1D0);
    GET_STACK_STATIC(TriggerClass *, trigger, esp, 0x1CC);
    //GET_STACK_STATIC(ObjectClass *, object, esp, 0x1C8); // Use ECX instead.
    GET_STACK_STATIC(HouseClass *, house, esp, 0x1C4);
    static bool retval;

    retval = this_ptr->Do_PLAY_SOUND_RANDOM(house, object, trigger, *cell);

    /**
     *  Function return.
     */
return_true:
    _asm { mov al, retval }
    JMP_REG(ecx, 0x0061A9C5);
}


/**
 *  #issue-299
 * 
 *  Fixes the issue with the current difficulty not being checked
 *  when enabling triggers.
 * 
 *  @see: TriggerClass and TriggerTypeClass for the other parts of this fix.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TActionClass_Operator_Enable_Trigger_For_Difficulty_Patch)
{
    GET_REGISTER_STATIC(int, trigger_index, edi);
    static TriggerClass* trigger;

    /**
     *  This is direct port of the code from Red Alert 2, which looks to fix this issue.
     */

    /**
     *  We need to re-fetch the trigger from the vector as the
     *  register is reused by this point.
     */
    trigger = Triggers[trigger_index];
    if (trigger) {

        /**
         *  Set this trigger to be disabled if it is marked as disabled
         *  for this current mission difficulty.
         *
         *  Rampastring: Check CDifficulty instead of Difficulty.
         *  Also, in non-campaign games, consider all difficulties enabled regardless of what the trigger specifies.
         */
        if (Session.Type == GAME_NORMAL &&
            (Scen->CDifficulty == DIFF_HARD && !trigger->Class->IsEnabledEasy
            || Scen->CDifficulty == DIFF_NORMAL && !trigger->Class->IsEnabledMedium
            || Scen->CDifficulty == DIFF_EASY && !trigger->Class->IsEnabledHard)) {

            trigger->Disable();

        } else {

            trigger->Enable();
        }
    }

    JMP(0x0061A611);
}

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
    _asm { mov ecx, PlayerPtr }
    _asm { mov ecx, [ecx] }
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
    _asm { mov ecx, PlayerPtr }
    _asm { mov ecx, [ecx] }
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

    Patch_Jump(0x0061A60C, &_TActionClass_Operator_Enable_Trigger_For_Difficulty_Patch);

    /**
     *  #issue-71
     *
     *  Increases the amount of available waypoints (see ScenarioClassExtension for implementation).
     *
     *  @author: CCHyper
     */
    Patch_Jump(0x0061BF50, &TActionClassExt::_Do_PLAY_SOUND_RANDOM);
    Patch_Jump(0x00619E42, &_TActionClass_Operator_Do_PLAY_SOUND_RANDOM_Remove_Inline_Patch);

    Patch_Jump(0x00619FDB, &_TAction_Win_FlagLosersAsDefeatedInMultiplayer);
    Patch_Jump(0x0061A005, &_TAction_Lose_FlagLoserAsLostInMultiplayer);
}
