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


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class TActionClassExt : public TActionClass
{
    public:
        bool _Play_Sound_At_Random_Waypoint(HouseClass *house, ObjectClass *object, TriggerClass *trigger, Cell &cell);
};


/**
 *  #issue-71
 *
 *  Reimplement Play_Sound_At_Random_Waypoint to support the new waypoint limit.
 *
 *  @author: CCHyper
 */
bool TActionClassExt::_Play_Sound_At_Random_Waypoint(HouseClass *house, ObjectClass *object, TriggerClass *trigger, Cell &cell)
{
    Cell cell_list[NEW_WAYPOINT_COUNT];
    int cell_list_count = 0;

    /**
     *  Make a list of all the valid waypoints in this scenario.
     */
    for (WaypointType wp = WAYPOINT_FIRST; wp < NEW_WAYPOINT_COUNT; ++wp) {
        if (ScenExtension->Is_Valid_Waypoint(wp)) {
            cell_list[cell_list_count++] = ScenExtension->Get_Waypoint_Cell(wp);
            if (cell_list_count >= std::size(cell_list)) {
                break;
            }
        }
    }

    /**
     *  Pick a random cell from the valid waypoint list and play the desired sound.
     */
    Cell rnd_cell = cell_list[Random_Pick<unsigned int>(0, std::size(cell_list) - 1)];

    Static_Sound(Data.Sound, Cell_Coord(rnd_cell, true));

    return true;
}


/**
 *  #issue-71
 *
 *  Replace inlined instance of Play_Sound_At_Random_Waypoint.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_TActionClass_Operator_Play_Sound_At_Random_Waypoint_Remove_Inline_Patch)
{
    GET_REGISTER_STATIC(TActionClass *, this_ptr, esi);
    GET_REGISTER_STATIC(ObjectClass *, object, ecx);
    GET_STACK_STATIC(Cell *, cell, esp, 0x1D0);
    GET_STACK_STATIC(TriggerClass *, trigger, esp, 0x1CC);
    //GET_STACK_STATIC(ObjectClass *, object, esp, 0x1C8); // Use ECX instead.
    GET_STACK_STATIC(HouseClass *, house, esp, 0x1C4);
    static bool retval;

    retval = this_ptr->TAction_Play_Sound_At_Random_Waypoint(house, object, trigger, *cell);

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
         */
        if (Scen->Difficulty == DIFF_EASY && !trigger->Class->Easy
            || Scen->Difficulty == DIFF_NORMAL && !trigger->Class->Normal
            || Scen->Difficulty == DIFF_HARD && !trigger->Class->Hard) {

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
    Patch_Jump(0x0061BF50, &TActionClassExt::_Play_Sound_At_Random_Waypoint);
    Patch_Jump(0x00619E42, &_TActionClass_Operator_Play_Sound_At_Random_Waypoint_Remove_Inline_Patch);

    Patch_Jump(0x00619FDB, &_TAction_Win_FlagLosersAsDefeatedInMultiplayer);
    Patch_Jump(0x0061A005, &_TAction_Lose_FlagLoserAsLostInMultiplayer);
}
