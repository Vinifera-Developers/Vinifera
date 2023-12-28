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
 *  @brief         Contains the hooks for the extended TActionClass.
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
#include "tactionext.h"
#include "taction.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "vinifera_defines.h"
#include "house.h"
#include "housetype.h"
#include "object.h"
#include "objecttype.h"
#include "trigger.h"
#include "triggertype.h"
#include "tag.h"
#include "tagtype.h"
#include "scenario.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


static const char *Get_Object_Coord_String(ObjectClass *this_ptr) { return this_ptr->Get_Coord().As_String(); }
static const char *Get_TAction_Waypoint_String(TActionClass *this_ptr)
{
    static char _buffer[3+3];
    Cell cell = Scen->Get_Waypoint_Location(this_ptr->Location);
    return cell.As_String();
}


/**
 *  This patch extends the TActionClass operator.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TActionClass_Operator_Extend_Switch_Patch)
{
    GET_REGISTER_STATIC(bool, success, al);                   // Was this action able to perform what it needed to do?
    GET_REGISTER_STATIC(TActionClass *, this_ptr, esi);

    /**
     *  Function arguments
     */
    GET_STACK_STATIC(HouseClass *, house, esp, 0x1C4);        // The owner of this action.
    GET_STACK_STATIC(ObjectClass *, object, esp, 0x1C8);      // Pointer to the object that the springing trigger was attached to (if attached).
    GET_STACK_STATIC(TriggerClass *, trigger, esp, 0x1CC);    // Trigger instance (only if forced).
    GET_STACK_STATIC(Cell *, cell, esp, 0x1D0);               // The cell this trigger is attached to (if any).

//#ifndef NDEBUG
    /**
     *  Helper info for debugging when adding new actions.
     */
    DEV_DEBUG_INFO("TActionClass:\n");
    DEV_DEBUG_INFO("  Executing: \"%s\"\n", TActionClassExtension::Action_Name(this_ptr->Action)); // Use extension Action_Name so new actions are referenced correctly.
    DEV_DEBUG_INFO("  ID: %d\n", this_ptr->ID);
    if (this_ptr->Next) DEV_DEBUG_INFO("  Next: \"%s\"\n", TActionClassExtension::Action_Name(this_ptr->Next->Action)); // Use extension Action_Name so new actions are referenced correctly.
    if (house) {
        DEV_DEBUG_INFO("  House: \"%s\"\n", house->Class->Name());
        if (house->Is_Player()) {
            DEV_DEBUG_INFO("    IsPlayer: true\n");
        } else {
            DEV_DEBUG_INFO("    IsPlayer: false\n");
        }
        if (house->IsHuman) {
            DEV_DEBUG_INFO("    IsHuman: true\n");
        } else {
            DEV_DEBUG_INFO("    IsHuman: false\n");
        }
        if (house->IsPlayerControl) {
            DEV_DEBUG_INFO("    IsPlayerControl: true\n");
        } else {
            DEV_DEBUG_INFO("    IsPlayerControl: false\n");
        }
    }
    if (this_ptr->Location != WAYPOINT_NONE) DEV_DEBUG_INFO("  Location: %s\n", Get_TAction_Waypoint_String(this_ptr));
    if (object) {
        DEV_DEBUG_INFO("  Object: \"%s\"\n", object->Name());
        DEV_DEBUG_INFO("    Coord: %s\n", Get_Object_Coord_String(object));
    }
    if (this_ptr->Tag) DEV_DEBUG_INFO("  Tag: \"%s\"\n", this_ptr->Tag->Name());
    if (trigger) DEV_DEBUG_INFO("  Trigger: \"%s\"\n", trigger->Class_Of()->Name());
    if (cell && *cell) DEV_DEBUG_INFO("  Cell: %d,%d\n", cell->X, cell->Y);
//#endif

    /**
     *  Skip null actions.
     */
    if (this_ptr->Action == TACTION_NONE) {
        success = false;
        goto return_success;
    }

    /**
     *  Handle the original TActionTypes.
     */
    if (this_ptr->Action < TACTION_COUNT) {
        goto taction_switch;
    }

    /**
     *  Execute the new trigger action. The map ini format must be greater than
     *  or equal to the value of 5. This allows us to make sure new actions are
     *  not used by mistake!
     */
    if (NewINIFormat >= 5) {
        if (this_ptr->Action < EXT_TACTION_COUNT) {
            success = TActionClassExtension::Execute(this_ptr, house, object, trigger, cell);
            goto return_success;
        }
    }

    /**
     *  The default case, return success.
     */
return_success:
    _asm { mov al, success }
    JMP(0x0061A9C5);

    /**
     *  The switch case for the original TActionTypes
     */
taction_switch:
    _asm { mov esi, this_ptr }
    _asm { mov edx, [esi+0x1C] } // this->Action
    _asm { dec edx } 
    JMP_REG(eax, 0x00619141);
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
    static TriggerClass *trigger;

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
    Patch_Jump(0x00619134, &_TActionClass_Operator_Extend_Switch_Patch);
}
