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
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class AircraftClassExt final : public AircraftClass
{
public:
    bool _Unlimbo(Coordinate& coord, DirType dir);
    bool _Enter_Idle_Mode(bool initial, bool a2);
    bool _Cell_Seems_Ok(Cell& cell, bool strict) const;
};


/**
 *  Removes an aircraft from the limbo state.
 *
 *  @author: 07/26/1994 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
bool AircraftClassExt::_Unlimbo(Coordinate& coord, DirType dir)
{
    Coordinate adjusted_coord = coord;

    const auto class_ext = Extension::Fetch<AircraftTypeClassExtension>(Class);
    const auto ext = Extension::Fetch<AircraftClassExtension>(this);

    /**
     *  Rockets and other spawned aircraft don't have to spawn on the ground.
     */
    if (!class_ext->IsMissileSpawn && !ext->SpawnOwner)
    {
        if (IsALoaner || !Map.In_Local_Radar(coord)) {
            adjusted_coord.Z = Class->Flight_Level() + Map.Get_Cell_Height(coord);
        } else {
            adjusted_coord.Z = Map.Get_Cell_Height(coord);
        }
    }

    if (FootClass::Unlimbo(adjusted_coord, dir)) {

        const auto weapon = Class->Fetch_Weapon_Info(WEAPON_SLOT_PRIMARY).Weapon;
        if (!Class->IsSelectable || !Class->IsLandable || (weapon && weapon->IsCamera)) {
            IsALoaner = true;
        }

        /**
         *  If this aicraft has passangers, mark it accordingly.
         */
        if (Cargo.Is_Something_Attached()) {
            Passenger = true;
        }

        /**
         *  Forces the body of the helicopter to face the correct direction.
         */
        SecondaryFacing.Set(DirStruct(dir));

        /**
         *  Start rotor animation.
         */
        Set_Rate(1);
        Set_Stage(0);

        /**
         *  When starting at flight level, then give it speed. When landed
         *  then it must be stationary.
         */
        if (Get_Height() == Class->Flight_Level()) {
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
 *  Gives the aircraft an appropriate mission.
 *
 *  @author: 06/05/1995 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
bool AircraftClassExt::_Enter_Idle_Mode(bool initial, bool a2)
{
    if (Has_Suspended_Mission()) {

        Restore_Mission();
        if (Mission == MISSION_PATROL) {
            Status = 0;
            IsLocked = false;
        }

        return false;
    }

    const bool result = FootClass::Enter_Idle_Mode(initial, a2);

    MissionType mission = House->Is_Human_Control() || Team || !Is_Weapon_Equipped() ? MISSION_GUARD : MISSION_GUARD_AREA;

    if (In_Which_Layer() != LAYER_GROUND && Get_Height() > Landing_Altitude() && !Extension::Fetch<AircraftTypeClassExtension>(Class)->IsMissileSpawn) {

        if (Cargo.Is_Something_Attached()) {
            if (IsALoaner) {
                if (Team) {
                    mission = MISSION_GUARD;
                }
                else {
                    mission = MISSION_UNLOAD;
                    Assign_Destination(Good_LZ());
                }
            }
            else {
                Assign_Destination(Good_LZ());
                mission = MISSION_MOVE;
            }
        }
        else {

            /**
             *  If this transport is a loaner and part of a team, then remove it from
             *  the team it is attached to.
             */
            if ((IsALoaner && House->Is_Human_Control()) || (!House->Is_Human_Control() && !Class->MaxAmmo)) {
                if (Team && Team->Has_Entered_Map()) {
                    Team->Remove(this);
                }
            }

            if (Get_Weapon()->Weapon) {

                /**
                 *  Weapon equipped helicopters that run out of ammo and were
                 *  brought in as reinforcements will leave the map.
                 */
                if (IsALoaner) {

                    /**
                     *  If it has no ammo, then break off of the team and leave the map.
                     *  If it can fight, then give it fighting orders.
                     */
                    if (Ammo == 0) {
                        if (Team) Team->Remove(this);
                        mission = MISSION_RETREAT;
                    }
                    else {
                        if (!Team) {
                            mission = MISSION_HUNT;
                        }
                    }

                }
                else if (Ammo && Target_Legal(TarCom) && Get_Mission() == MISSION_ATTACK || MissionQueue == MISSION_ATTACK) {
                    mission = MISSION_ATTACK;
                }
                else if (In_Air()) {
                    if (!Target_Legal(NavCom) || (Mission != MISSION_MOVE && Mission != MISSION_ENTER)) {
                        if (Class->Dock.Count() > 0 && (IsLocked || !Team)) {

                            /**
                             *  Normal aircraft try to find a good landing spot to rest.
                             */
                            BuildingClass* building = nullptr;
                            for (int i = 0; i < Class->Dock.Count(); i++) {
                                building = Find_Docking_Bay(Class->Dock[i], false);
                                if (building) break;
                            }

                            Assign_Destination(nullptr);
                            if (building && Transmit_Message(RADIO_HELLO, building) == RADIO_ROGER) {
                                Assign_Destination(building);
                                mission = MISSION_ENTER;
                            }
                        }
                    }
                }
            }
            else {
                if (Team) return false;

                Assign_Destination(Good_LZ());
                mission = MISSION_MOVE;
            }
        }
    }
    else {
        if (IsALoaner) {
            if (Cargo.Is_Something_Attached()) {

                /**
                 *  In the case of a computer controlled helicopter that hold passengers,
                 *  don't unload when landing. Wait for specific instructions from the
                 *  controlling team.
                 */
                if (Team) {
                    mission = MISSION_GUARD;
                }
                else {
                    mission = MISSION_UNLOAD;
                }
            }
            else if (!Team) {
                mission = MISSION_RETREAT;
            }
        }
        else {
            Assign_Destination(nullptr);
            Assign_Target(nullptr);

            if (!House->Is_Human_Control() && !Team && Is_Weapon_Equipped()) {
                mission = MISSION_GUARD_AREA;
            } else {
                mission = MISSION_GUARD;
            }
        }
    } 

    Assign_Mission(mission);
    if (Ready_To_Commence()) {
        Commence();
    }

    return result;
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
    if (Extension::Fetch<AircraftTypeClassExtension>(Class)->IsSpawned) {
        const TechnoClass* techno = Map[cell].Cell_Techno();
        if (techno) {
            if (Extension::Fetch<TechnoClassExtension>(techno)->SpawnManager
                || Extension::Fetch<TechnoTypeClassExtension>(techno->Techno_Type_Class())->IsSpawned) {
                return true;
            }
        }
    }

    /**
     *  If we're a carryall, we can enter a potential totable unit's cell.
     */
    bool can_tote = false;
    if (Class->IsCarryall && Target_Legal(NavCom) && NavCom->What_Am_I() == RTTI_UNIT)
        can_tote = true;

    /**
     *  Make sure that no other aircraft are heading to the selected location. If they
     *  are, then don't consider the location as valid.
     */
    TARGET astarget = &Map[cell];
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

    /**
     *  #issue-1091
     *
     *  Fix bug where aircraft are unable to attack shrouded targets in campaign games and instead get stuck in mid-air.
     *
     *  Author: Rampastring
     */
    Patch_Jump(0x0040D0C5, (uintptr_t)0x0040D0EA);

    Patch_Jump(0x00408940, &AircraftClassExt::_Unlimbo);
    Patch_Jump(0x0040B310, &AircraftClassExt::_Enter_Idle_Mode);
    Patch_Jump(0x0040D260, &AircraftClassExt::_Cell_Seems_Ok);
}
