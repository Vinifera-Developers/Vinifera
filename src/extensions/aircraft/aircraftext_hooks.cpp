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
#include "rules.h"


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
    bool _Unlimbo(const Coord& coord, Dir256 dir);
    bool _Cell_Seems_Ok(Cell& cell, bool strict) const;
    ActionType _What_Action(ObjectClass const* target, bool disallow_force);
    LONG STDMETHODCALLTYPE _Landing_Altitude();
    LONG STDMETHODCALLTYPE _Landing_Altitude_Thunk();
    RadioMessageType _Receive_Message(RadioClass * from, RadioMessageType message, long& param);
};


/**
 *  Removes an aircraft from the limbo state.
 *
 *  @author: 07/26/1994 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
bool AircraftClassExt::_Unlimbo(const Coord& coord, Dir256 dir)
{
    Coord adjusted_coord = coord;

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
            if (foot->IsDown && (foot->Position.As_Cell() == cell || foot->NavCom == astarget)) {
                return false;
            }
        }
    }

    return true;
}


/**
 *  Determines what action to perform.
 *
 *  @author: 06/19/1995 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
ActionType AircraftClassExt::_What_Action(ObjectClass const* target, bool disallow_force)
{
    ActionType action = FootClass::What_Action(target, disallow_force);

    /**
     *  If this is a carryall, we might be able to tote a unit the mouse is over.
     */
    if (Class->IsCarryall && House->Is_Player_Control()) {
        if (action == ACTION_SELECT || action == ACTION_NONE) {
            if (House->Is_Ally(target)) {
                if (!target->Is_Techno() || target->Owner_HouseClass()->Is_Ally(this)) {

                    /**
                     *  #issue-208
                     *
                     *  Check if the target unit is "totable" before we attempt to pick it up.
                     *
                     *  @author: CCHyper
                     */
                    if (!Cargo.Is_Something_Attached() && target->RTTI == RTTI_UNIT && Extension::Fetch(static_cast<const UnitClass*>(target)->Class)->IsTotable) {
                        action = ACTION_TOTE;
                    }
                }
            }
        }
    }

    /**
     *  Check if we can do something to the cell under the target.
     */
    if (action == ACTION_NONE) {
        action = What_Action(target->Center_Coord().As_Cell(), false, disallow_force);
    }

    if (action == ACTION_SELF) {

        /**
         *  Can't unload the passengers if there are none.
         */
        if (!Cargo.How_Many()) {

            /**
             *  If this is also a normal transport, show the "can't deploy" cursor, like for APCs,
             *  otherwise just show the normal cursor.
             */
            if (Class->Max_Passengers() > 0) {
                action = ACTION_NO_DEPLOY;
            } else {
                action = ACTION_NONE;
            }
        }

        /**
         *  #FIX: Check if there's a building under the aircraft, and if so
         *  don't allow unloading to prevent units stuck in limbo.
         */
        else {
            BuildingClass* building = Map[PositionCell].Cell_Building();
            if (building != nullptr) {
                action = ACTION_NO_DEPLOY;
            }
        }
        
    }

    /**
     *  Can't fire a weapon if there is none.
     */
    if (action == ACTION_ATTACK && PrimaryWeapon == nullptr) {
        action = ACTION_NONE;
    }

    /**
     *  Special return to friendly repair factory action.
     *
     *  #FIX: Check for ACTION_MOVE, because for allied buildings we get that sometimes, not ACTION_SELECT.
     */
    if (House->Is_Player_Control() && (action == ACTION_SELECT || action == ACTION_MOVE) && target->RTTI == RTTI_BUILDING) {
        BuildingClass* building = (BuildingClass*)target;

        /**
         *  #FIX: Allow any repair depot to repair aircraft, not just Rule->RepairBay
         */
        if ((building->Class->IsCanUnitRepair || building->Class->IsHelipad) && !building->In_Radio_Contact() && !building->Cargo.Is_Something_Attached()) {
            if (Transmit_Message(RADIO_CAN_LOAD, building) == RADIO_ROGER) {
                action = ACTION_ENTER;
            }
        }
    }

    /**
     *  #FIX: If we're carrying a unit, only allow dropping it off on a repair depot,
     *  not on a helipad or anything else.
     */
    if (Class->IsCarryall && action == ACTION_ENTER && Cargo.Is_Something_Attached(RTTI_UNIT)) {
        BuildingClass* building = (BuildingClass*)target;
        if (!building->Class->IsCanUnitRepair) {
            action = ACTION_NO_ENTER;
        }
    }

    /**
     *  Make sure we can't tote things out of the weapons factory.
     */
    if (Class->IsCarryall && action == ACTION_TOTE) {
        Cell cell = target->PositionCell;
        if (cell != CELL_NONE) {
            BuildingClass* building = Map[cell].Cell_Building();
            if (building != nullptr && building->Class->IsWeaponsFactory) {
                action = ACTION_NONE;
            }
        }
    }

    return action;
}


/**
 *  AircraftClass::Receive_Message replacement.
 *
 *  @author: ZivDero
 */
RadioMessageType AircraftClassExt::_Receive_Message(RadioClass* from, RadioMessageType message, long& param)
{
    AbstractClass* target;

    switch (message) {

    case RADIO_RELOAD:
        if (Ammo >= Class->MaxAmmo / 2 && TarCom != nullptr) {
            return RADIO_ROGER;
        }
        return FootClass::Receive_Message(from, message, param);

    case RADIO_PREPARED:
        if (TarCom != nullptr) return RADIO_NEGATIVE;
        if ((HeightAGL == 0 && Ammo == Class->MaxAmmo) || (HeightAGL > 0 && Ammo > 0)) return RADIO_ROGER;
        return RADIO_NEGATIVE;

    case RADIO_ALL_DONE:
        if (Ammo == Class->MaxAmmo) {
            return RADIO_ROGER;
        }
        return RADIO_NEGATIVE;

    /*
    **  Something disastrous has happened to the object in contact with. Fall back
    **  and regroup. This means that any landing process is immediately aborted.
    */
    case RADIO_RUN_AWAY:
        Scatter(COORD_NONE, true);
        break;

    /*
    **  The ground control requests that this specified landing spot be used.
    */
    case RADIO_MOVE_HERE:
        FootClass::Receive_Message(from, message, param);
        target = reinterpret_cast<AbstractClass*>(param);
        if (dynamic_cast<BuildingClass*>(target) != nullptr) {
            if (Transmit_Message(RADIO_CAN_LOAD, ::As_Techno(target)) != RADIO_ROGER) {
                return RADIO_NEGATIVE;
            }
            Assign_Mission(MISSION_ENTER);
            Assign_Destination(target);
        } else {
            Assign_Mission(MISSION_MOVE);
            Assign_Destination(target);
        }
        Commence();
        return RADIO_ROGER;

    /*
    **  Ground control is requesting if the aircraft requires navigation direction.
    */
    case RADIO_NEED_TO_MOVE:
        FootClass::Receive_Message(from, message, param);
        if (!Locomotion->Is_Moving() || NavCom == nullptr) {
            return RADIO_ROGER;
        }
        return RADIO_NEGATIVE;

    /*
    **  This message is sent by the passenger when it determines that it has
    **  entered the transport.
    */
    case RADIO_IM_IN:
        if (Cargo.How_Many() == Class->Max_Passengers()) {
            Door.Close_Door(Class->DeployTime);
        }

        /*
        **  If a civilian has entered the transport, then the transport will immediately
        **  fly off the map.
        */
        if (Counts_As_Civ_Evac(from)) {
            Assign_Mission(MISSION_RETREAT);
        }
        return RADIO_ATTACH;

    /*
    **  Docking maintenance message received. Check to see if new orders should be given
    **  to the impatient unit.
    */
    case RADIO_DOCKING:
        if (Class->Max_Passengers() > 0 && Cargo.How_Many() < Class->Max_Passengers()) {
            FootClass::Receive_Message(from, message, param);

            if (!Locomotion->Is_Moving()) {

                Door.Open_Door(Class->DeployTime);

                /*
                **  If the potential passenger needs someplace to go, then figure out a good
                **  spot and tell it to go.
                */
                if (Transmit_Message(RADIO_NEED_TO_MOVE, from) == RADIO_ROGER) {

                    /*
                    **  Tell the potential passenger where it should go. If the passenger is
                    **  already at the staging location, then tell it to move onto the transport
                    **  directly.
                    */
                    param = reinterpret_cast<long>(this);
                    if (Transmit_Message(RADIO_MOVE_HERE, param, from) != RADIO_ROGER) {
                        Transmit_Message(RADIO_OVER_OUT, from);
                    } else {
                        Contact_With_Whom()->Unselect();
                    }
                }
            }
            return RADIO_ROGER;
        }
        break;

    /*
    **  Asks if the passenger can load on this transport.
    */
    case RADIO_CAN_LOAD:
        if (Class->Max_Passengers() == 0 || from == nullptr || !House->Is_Ally(from)) return RADIO_STATIC;

        /*
        **  Don't allow boarding if we're docked.
        */
        if (In_Radio_Contact() && Contact_With_Whom()->RTTI == RTTI_BUILDING) return RADIO_NEGATIVE;

        /*
        **  Carryalls can only carry one vehicle, and only by itself.
        */
        if (Class->IsCarryall && Cargo.Is_Something_Attached(RTTI_UNIT)) return RADIO_NEGATIVE;

        if (Cargo.How_Many() < Class->Max_Passengers()) {
            return RADIO_ROGER;
        }
        return RADIO_NEGATIVE;

    case RADIO_UNLOADED:
        if (Class->IsCarryall && Mission == MISSION_MOVE && IsTethered) {
            if ((Cargo.Is_Something_Attached() && Cargo.Attached_Object() == from) || NavCom == from) {
                return RADIO_NEGATIVE;
            }
        }
        break;

    default:
        break;
    }

    /*
    **	Let the base class take over processing this message.
    */
    return FootClass::Receive_Message(from, message, param);
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
            Static_Sound(technotypeext->LeaveTransportSound, this_ptr->Position);
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
 *  The below patches make the carryall only unload vehicles by dropping them off,
 *  leaving infantry to be manually unloaded.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_AircraftClass_Do_MISSION_UNLOAD_Carryall_Drop_Off_Patch)
{
    GET_REGISTER_STATIC(AircraftClass*, this_ptr, esi);

    if (this_ptr->Class->IsCarryall && this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT)) {
        JMP(0x0040980F);
    }

    JMP(0x00409833);
}

DECLARE_PATCH(_AircraftClass_Do_MISSION_MOVE_CARRYALL_Drop_Off_Patch)
{
    GET_REGISTER_STATIC(AircraftClass*, this_ptr, esi);

    _asm add esp, 4

    if (this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT)) {
        JMP(0x0040AD82);
    }

    JMP(0x0040ADD0);
}

DECLARE_PATCH(_AircraftClass_Do_MISSION_ENTER_Drop_Off_Patch)
{
    GET_REGISTER_STATIC(AircraftClass*, this_ptr, esi);

    if (this_ptr->Class->IsCarryall && this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT)) {
        JMP(0x0040D62F);
    }

    JMP(0x0040D6D3);
}


/**
 *  Patches AircraftClass::Draw_It to only draw vehicle passengers' shadows
 *  for carryalls.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_AircraftClass_Draw_It_Carry_All_Patch)
{
    GET_REGISTER_STATIC(AircraftClass*, this_ptr, ebp);
    GET_STACK_STATIC(Rect*, cliprect, esp, 0xD0);
    LEA_STACK_STATIC(Point2D*, drawpoint, esp, 0x10);

    if (this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT) && this_ptr->Class->IsCarryall) {
        this_ptr->Cargo.Attached_Object(RTTI_UNIT)->Draw_It(*drawpoint, *cliprect);
    }

    JMP(0x00408C27);
}


/**
 *  Replacement for AircraftClass::Landing_Altitude.
 *  Fixes a problem where a carryall would land too high with any cargo,
 *  not just units.
 *
 *  @author: ZivDero
 */
LONG AircraftClassExt::_Landing_Altitude()
{
    /**
     *  If this is a carryall, if it's landing by itself on a helipad or a service depot,
     *  it should land at a normal height.
     */
    if (Class->IsCarryall && !Cargo.Is_Something_Attached()) {
        TechnoClass* tptr = Contact_With_Whom();
        if (tptr != nullptr && Mission == MISSION_ENTER) {
            BuildingClass* bptr = dynamic_cast<BuildingClass*>(tptr);
            if (bptr != nullptr) {
                if (bptr->Class->IsCanUnitRepair || bptr->Class->IsHelipad) {
                    return 0;
                }
            }
        }
    }

    if (Class->IsCarryall) {

        /**
         *  We're picking something up.
         *  Check for RTTI is new to prevent landing too high on buildings we're in contact with.
         */
        if (In_Radio_Contact() && Contact_With_Whom()->RTTI == RTTI_UNIT) {
            return 100;
        }

        /**
         *  Something is attached below us, account for that.
         */
        if (Cargo.Is_Something_Attached(RTTI_UNIT)) {
            return 100;
        }
    }

    return 0;
}

/**
 *  AircraftClass::Landing_Altitude is an interface method so we need to make
 *  a thunk to properly patch it.
 */
LONG AircraftClassExt::_Landing_Altitude_Thunk()
{
    return static_cast<AircraftClassExt*>(reinterpret_cast<IFlyControl*>(this))->_Landing_Altitude();
}


/**
 *  Fix a bug where carryalls assign their ROT (via the FacingClass assignment) to the unit they're carrying.
 *
 *  Author: ZivDero
 */
DECLARE_PATCH(_AircraftClass_AI_Carryall_Facing_Patch)
{
    GET_REGISTER_STATIC(AircraftClass*, this_ptr, ebp);

    if (this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT) && this_ptr->Class->IsCarryall) {
        this_ptr->Cargo.Attached_Object()->PrimaryFacing.Set(this_ptr->SecondaryFacing.Current());
        this_ptr->Cargo.Attached_Object()->SecondaryFacing.Set(this_ptr->SecondaryFacing.Current());
        this_ptr->Cargo.Attached_Object()->PositionCoord = this_ptr->PositionCoord;
    }

    JMP(0x004093DE);
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
    Patch_Jump(0x0040B7E0, &AircraftClassExt::_What_Action);

    Patch_Jump(0x004097FF, &_AircraftClass_Do_MISSION_UNLOAD_Carryall_Drop_Off_Patch);
    Patch_Jump(0x0040AD7B, &_AircraftClass_Do_MISSION_MOVE_CARRYALL_Drop_Off_Patch);
    Patch_Jump(0x0040D60D, &_AircraftClass_Do_MISSION_ENTER_Drop_Off_Patch);
    Patch_Jump(0x00408BF3, &_AircraftClass_Draw_It_Carry_All_Patch);
    Patch_Jump(0x0040EDD0, &AircraftClassExt::_Landing_Altitude_Thunk);
    Patch_Jump(0x0040C8A0, &AircraftClassExt::_Receive_Message);

    Patch_Jump(0x00409366, &_AircraftClass_AI_Carryall_Facing_Patch);
}
