/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended AircraftClass.
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
#include "aircraftext_hooks.h"
#include "aircraftext_init.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
#include "object.h"
#include "target.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "extension.h"
#include "voc.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-996
 * 
 *  Implements IsCurleyShuffle for AircraftTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET0_Can_Fire_FIRE_FACING_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    static AircraftTypeClassExtension *class_ext;
    static bool is_curley_shuffle;

    class_ext = Extension::Fetch<AircraftTypeClassExtension>(this_ptr->Class);

    is_curley_shuffle = class_ext->IsCurleyShuffle;

    _asm { mov al, is_curley_shuffle }
    JMP_REG(edx, 0x0040BDDB);

}

DECLARE_PATCH(_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_OK_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    static AircraftTypeClassExtension * class_ext;
    static bool is_curley_shuffle;

    class_ext = Extension::Fetch<AircraftTypeClassExtension>(this_ptr->Class);

    is_curley_shuffle = class_ext->IsCurleyShuffle;

    _asm { mov cl, is_curley_shuffle }
    JMP_REG(edx, 0x0040BFA8);
}

DECLARE_PATCH(_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_FACING_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    static AircraftTypeClassExtension * class_ext;
    static bool is_curley_shuffle;

    class_ext = Extension::Fetch<AircraftTypeClassExtension>(this_ptr->Class);

    is_curley_shuffle = class_ext->IsCurleyShuffle;

    _asm { mov dl, is_curley_shuffle }
    JMP_REG(edx, 0x0040C060);
}

DECLARE_PATCH(_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_DEFAULT_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    static AircraftTypeClassExtension *class_ext;
    static bool is_curley_shuffle;

    class_ext = Extension::Fetch<AircraftTypeClassExtension>(this_ptr->Class);

    is_curley_shuffle = class_ext->IsCurleyShuffle;

    _asm { mov al, is_curley_shuffle }
    JMP_REG(edx, 0x0040C0B8);
}


/**
 *  #issue-264
 * 
 *  Implements LeaveTransportSound for this aircraft is unloading its passengers.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftClass_Mission_Unload_Transport_Detach_Sound_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    GET_REGISTER_STATIC(FootClass *, passenger, edi);
    static TechnoTypeClassExtension *technotypeext;

    /**
     *  Don't play the passenger leave sound for carryalls.
     */
    if (!this_ptr->Class->IsCarryall) {

        /**
         *  Do we have a sound to play when passengers leave us? If so, play it now.
         */
        technotypeext = Extension::Fetch<TechnoTypeClassExtension>(this_ptr->Techno_Type_Class());
        if (technotypeext->LeaveTransportSound != VOC_NONE) {
            Sound_Effect(technotypeext->LeaveTransportSound, this_ptr->Coord);
        }

    }

    /**
     *  Stolen bytes/code.
     * 
     *  Carryalls do not add their cargo to its team, so skip them.
     */
    if (!this_ptr->Class->IsCarryall) {

        /**
         *  Are we a part of a team? If so, make any passengers we unload part of it too.
         */
        if (this_ptr->Team) {
            goto add_to_team;
        }
    }

    /**
     *  Finished unloading passengers.
     */
finish_up:
    JMP(0x004098AC);

    /**
     *  Add this passenger to my team.
     */
add_to_team:
    _asm { mov edi, passenger }     // Restore EBP pointer.
    JMP(0x004098A0);
}


/**
 *  #issue-604
 * 
 *  Fixes a bug where air-transports are unable to land when given a move order.
 * 
 *  This is a well known side-effect of a official bug-fix from Patch 1.13. The
 *  fix below is a back-port of a change in Red Alert 2 which fixes the issue.
 * 
 *  @author: tomsons26, CCHyper
 */
static bool Locomotion_Is_Moving(AircraftClass *this_ptr) { return this_ptr->Locomotion->Is_Moving(); }
DECLARE_PATCH(_AircraftClass_Mission_Move_LAND_Is_Moving_Check_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    
    /**
     *  If the aircraft is not currently moving, enter idle mode.
     */
    if (!Locomotion_Is_Moving(this_ptr)) {
        this_ptr->Enter_Idle_Mode(false, true);
    }

    /**
     *  Function return with "1".
     */
return_one:
    JMP(0x0040A421);
}


/**
 *  #issue-208
 * 
 *  Check if the target unit is "totable" before we attempt to pick it up.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftClass_What_Action_Is_Totable_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    GET_REGISTER_STATIC(ObjectClass *, target, edi);
    GET_REGISTER_STATIC(ActionType, action, ebx);
    static UnitClass *target_unit;
    static UnitTypeClassExtension *unittypeext;

    /**
     *  Code before this patch checks for if this aircraft
     *  is a carryall and if it is owned by a player (non-AI).
     */

    /**
     *  Make sure the mouse is over something.
     */
    if (action != ACTION_NONE) {

        /**
         *  Target is a unit?
         */
        if (target->What_Am_I() == RTTI_UNIT) {

            target_unit = reinterpret_cast<UnitClass *>(target);

            /**
             *  Fetch the extension instance.
             */
            unittypeext = Extension::Fetch<UnitTypeClassExtension>(target_unit->Class);

            /**
             *  Can this unit be toted/picked up by us?
             */
            if (!unittypeext->IsTotable) {

                /**
                 *  If not, then show the "no move" mouse.
                 */
                action = ACTION_NOMOVE;

                goto failed_tote_check;

            }
        }
    }

    /**
     *  Stolen code.
     */
    if (action != ACTION_NONE && action != ACTION_SELECT) {
        goto action_self_check;
    }

    /**
     *  Passes our tote check, continue original carryall checks.
     */
passes_tote_check:
    _asm { mov ebx, action }
    _asm { mov edi, target }
    JMP_REG(ecx, 0x0040B826);

    /**
     *  Undeploy/unload check.
     */
action_self_check:
    _asm { mov ebx, action }
    _asm { mov edi, target }
    JMP(0x0040B8C2);

    /**
     *  We cant pick this unit up, so continue to evaluate the target.
     */
failed_tote_check:
    _asm { mov ebx, action }
    _asm { mov edi, target }
    JMP(0x0040B871);
}


/**
 *  #issue-469
 * 
 *  Fixes a bug where IsCloakable has no effect on Aircrafts. This was
 *  because the TechnoType value was not copied to the Aircraft instance
 *  when it is created.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftClass_Init_IsCloakable_BugFix_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    GET_REGISTER_STATIC(AircraftTypeClass *, aircrafttype, eax);

    /**
     *  Stolen bytes/code.
     */
    this_ptr->Strength = aircrafttype->MaxStrength;
    this_ptr->Ammo = aircrafttype->MaxAmmo;

    /**
     *  This is the line that was missing (maybe it was by design?).
     */
    this_ptr->IsCloakable = aircrafttype->IsCloakable;

    JMP_REG(ecx, 0x004088AA);
}


/**
 *  Main function for patching the hooks.
 */
void AircraftClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    AircraftClassExtension_Init();

    Patch_Jump(0x00408898, &_AircraftClass_Init_IsCloakable_BugFix_Patch);
    Patch_Jump(0x0040B819, &_AircraftClass_What_Action_Is_Totable_Patch);
    Patch_Jump(0x0040A413, &_AircraftClass_Mission_Move_LAND_Is_Moving_Check_Patch);
    Patch_Jump(0x0040988C, &_AircraftClass_Mission_Unload_Transport_Detach_Sound_Patch);
    Patch_Jump(0x0040BDCF, &_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET0_Can_Fire_FIRE_FACING_Patch);
    Patch_Jump(0x0040C054, &_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_OK_Patch);
    Patch_Jump(0x0040BF9D, &_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_FACING_Patch);
    Patch_Jump(0x0040C0AC, &_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_DEFAULT_Patch);
}
