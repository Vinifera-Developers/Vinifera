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
#include "tactionext.h"
#include "taction.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "house.h"
#include "housetype.h"
#include "object.h"
#include "objecttype.h"
#include "trigger.h"
#include "triggertype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  This patch extends the TActionClass operator.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TAction_Operator_Extend_Switch_Patch)
{
    GET_REGISTER_STATIC(bool, success, al);
    GET_REGISTER_STATIC(TActionClass *, this_ptr, esi);

    /**
     *  Function arguments
     */
    GET_STACK_STATIC(HouseClass *, house, esp, 0x1C4);
    GET_STACK_STATIC(ObjectClass *, object, esp, 0x1C8);
    GET_STACK_STATIC(TriggerClass *, trigger, esp, 0x1CC);
    GET_STACK_STATIC(Cell *, cell, esp, 0x1D0);

#ifndef NDEBUG
    /**
     *  Helper info for debugging when adding new actions.
     */
    DEV_DEBUG_INFO("TActionClass::operator()\n");
    if (house) DEV_DEBUG_INFO("  House: \"%s\"\n", house->Class->Name());
    if (object) DEV_DEBUG_INFO("  Object: \"%s\"\n", object->Name());
    if (trigger) DEV_DEBUG_INFO("  Trigger: \"%s\"\n", trigger->Class_Of()->Name());
    if (cell && *cell) DEV_DEBUG_INFO("  Cell: %d,%d\n", cell->X, cell->Y);
#endif

    /**
     *  Skip null actions.
     */
    if (this_ptr->Action == TACTION_NONE) {
        goto do_nothing;
    }

    /**
     *  Handle the original TActionTypes.
     */
    if (this_ptr->Action < TACTION_COUNT) {
#ifndef NDEBUG
        DEV_DEBUG_INFO("Executing action: \"%s\"\n", TActionClass::Action_Name(this_ptr->Action));
#endif
        goto taction_switch;
    }

    /**
     *  New TActionType switch.
     */
    if (this_ptr->Action < NEW_TACTION_COUNT) {
        DEV_DEBUG_INFO("Executing new action: \"%s\"\n", TActionClassExtension::Action_Name(this_ptr->Action));
    }

    /**
     *  Execute the new trigger action.
     */
    TActionClassExtension::Execute(this_ptr, house, object, trigger, cell);

    /**
     *  The default case, return doing nothing.
     */
do_nothing:
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

    Patch_Jump(0x00619134, &_TAction_Operator_Extend_Switch_Patch);
}
