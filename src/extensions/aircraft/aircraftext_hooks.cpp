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
#include "aircraftext.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
#include "object.h"
#include "target.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "weapontype.h"
#include "extension.h"
#include "voc.h"
#include "mouse.h"
#include "team.h"
#include "building.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(AircraftClass)
{
public:
    bool _Unlimbo(const Coordinate& coord, Dir256 dir);
    bool _Cell_Seems_Ok(Cell& cell, bool strict) const;
};


/**
 *  Removes an aircraft from the limbo state.
 *
 *  @author: 07/26/1994 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
bool AircraftClassExt::_Unlimbo(const Coordinate& coord, Dir256 dir)
{
    Coordinate adjusted_coord = coord;

    const auto class_ext = Extension::Fetch(Class);
    const auto ext = Extension::Fetch(this);

    /**
     *  Rockets and other spawned aircraft don't have to spawn on the ground.
     */
    if (!class_ext->IsMissileSpawn && !ext->SpawnOwner)
    {
        if (IsALoaner || !Map.In_Local_Radar(coord)) {
            adjusted_coord.Z = Class->Flight_Level() + Map.Get_Height_GL(coord);
        } else {
            adjusted_coord.Z = Map.Get_Height_GL(coord);
        }
    }

    if (FootClass::Unlimbo(adjusted_coord, dir)) {

        const auto weapon = Class->Fetch_Weapon_Info(WEAPON_SLOT_PRIMARY).Weapon;
        if (!class_ext->IsSpawned && (!Class->IsSelectable || !Class->IsLandable || (weapon && weapon->IsCamera))) {
            IsALoaner = true;
        }

        /**
         *  If this aicraft has passangers, mark it accordingly.
         *  Set Ammo to the number of passangers divided by 5 rounded up for
         *  backwards compatibility with TS-Patches.
         */
        if (Cargo.Is_Something_Attached()) {
            Passenger = true;
            Ammo = (Cargo.How_Many() + 4) / 5;
        }

        /**
         *  Forces the body of the helicopter to face the correct direction.
         */
        SecondaryFacing.Set(DirType(dir));

        /**
         *  Start rotor animation.
         */
        Set_Rate(1);
        Set_Stage(0);

        /**
         *  When starting at flight level, then give it speed. When landed
         *  then it must be stationary.
         */
        if (HeightAGL == Class->Flight_Level()) {
            Set_Speed(1.0);
        }
        else {
            Set_Speed(0.0);
        }

        return true;
    }

    return false;
}


/**
 *  Checks to see if a cell is good to enter.
 *
 *  @author: 06/19/1995 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
bool AircraftClassExt::_Cell_Seems_Ok(Cell& cell, bool strict) const
{
    /**
     *  If the cell is outisde the playable area, then it is not a valid cell to enter.
     */
    if (!Map.In_Local_Radar(cell)) {
        return false;
    }

    /**
     *  Spawners and spawned objects can co-exist in cells.
     */
    if (Extension::Fetch(Class)->IsSpawned) {
        const TechnoClass* techno = Map[cell].Cell_Techno();
        if (techno) {
            if (Extension::Fetch(techno)->SpawnManager
                || Extension::Fetch(techno->TClass)->IsSpawned) {
                return true;
            }
        }
    }

    /**
     *  If we're a carryall, we can enter a potential totable unit's cell.
     */
    bool can_tote = false;
    if (Class->IsCarryall && Target_Legal(NavCom) && NavCom->RTTI == RTTI_UNIT)
        can_tote = true;

    /**
     *  Make sure that no other aircraft are heading to the selected location. If they
     *  are, then don't consider the location as valid.
     */
    AbstractClass * astarget = &Map[cell];
    for (int index = 0; index < Foots.Count(); index++) {
        const FootClass* foot = Foots[index];
        if (foot && (!can_tote || foot != NavCom) && (strict || foot != this) && !foot->IsInLimbo) {
            if (foot->IsDown && (Coord_Cell(foot->Coord) == cell || foot->NavCom == astarget)) {
                return false;
            }
        }
    }

    return true;
}


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

    class_ext = Extension::Fetch(this_ptr->Class);

    is_curley_shuffle = class_ext->IsCurleyShuffle;

    _asm { mov al, is_curley_shuffle }
    JMP_REG(edx, 0x0040BDDB);

}

DECLARE_PATCH(_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_OK_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    static AircraftTypeClassExtension * class_ext;
    static bool is_curley_shuffle;

    class_ext = Extension::Fetch(this_ptr->Class);

    is_curley_shuffle = class_ext->IsCurleyShuffle;

    _asm { mov cl, is_curley_shuffle }
    JMP_REG(edx, 0x0040BFA8);
}

DECLARE_PATCH(_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_FACING_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    static AircraftTypeClassExtension * class_ext;
    static bool is_curley_shuffle;

    class_ext = Extension::Fetch(this_ptr->Class);

    is_curley_shuffle = class_ext->IsCurleyShuffle;

    _asm { mov dl, is_curley_shuffle }
    JMP_REG(edx, 0x0040C060);
}

DECLARE_PATCH(_AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_DEFAULT_Patch)
{
    GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
    static AircraftTypeClassExtension *class_ext;
    static bool is_curley_shuffle;

    class_ext = Extension::Fetch(this_ptr->Class);

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
        technotypeext = Extension::Fetch(this_ptr->TClass);
        if (technotypeext->LeaveTransportSound != VOC_NONE) {
            Static_Sound(technotypeext->LeaveTransportSound, this_ptr->Coord);
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
        if (target->RTTI == RTTI_UNIT) {

            target_unit = reinterpret_cast<UnitClass *>(target);

            /**
             *  Fetch the extension instance.
             */
            unittypeext = Extension::Fetch(target_unit->Class);

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


DECLARE_PATCH(_AircraftClass_Enter_Idle_Mode_Spawner_Patch)
{
    GET_REGISTER_STATIC(AircraftClass*, this_ptr, esi);
    GET_REGISTER_STATIC(int, layer, eax);
    GET_REGISTER_STATIC(int, landingaltitude, ebp);
    static AircraftTypeClassExtension* aircrafttypeext;

    aircrafttypeext = Extension::Fetch(this_ptr->Class);

    if (layer != LAYER_GROUND && this_ptr->HeightAGL > landingaltitude && !aircrafttypeext->IsMissileSpawn)
    {
        JMP(0x0040B3C1);
    }
    else
    {
        JMP(0x0040B5DC);
    }
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

    /**
     *  #issue-1091
     *
     *  Fix bug where aircraft are unable to attack shrouded targets in campaign games and instead get stuck in mid-air.
     *
     *  Author: Rampastring
     */
    Patch_Jump(0x0040D0C5, (uintptr_t)0x0040D0EA);

    Patch_Jump(0x00408940, &AircraftClassExt::_Unlimbo);
    Patch_Jump(0x0040D260, &AircraftClassExt::_Cell_Seems_Ok);
    Patch_Jump(0x0040B3A6, &_AircraftClass_Enter_Idle_Mode_Spawner_Patch);
}
