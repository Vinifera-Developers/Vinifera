/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended AircraftClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "aircraftext_hooks.h"

#include "aircraft.h"
#include "aircraftext.h"
#include "aircraftext_init.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
#include "asserthandler.h"
#include "building.h"
#include "extension.h"
#include "hooker.h"
#include "house.h"
#include "mouse.h"
#include "object.h"
#include "rules.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "team.h"
#include "technotype.h"
#include "technotypeext.h"
#include "unit.h"
#include "unitext.h"
#include "unittype.h"
#include "unittypeext.h"
#include "voc.h"
#include "weapontype.h"


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
    bool _Do_MISSION_MOVE_Apply_QMove();
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
            if (Extension::Fetch(techno)->SpawnManager || Extension::Fetch(techno->TClass)->IsSpawned) {
                return true;
            }
        }
    }

    /**
     *  If we're a carryall, we can enter a potential totable unit's cell.
     */
    bool can_tote = false;
    if (Class->IsCarryall && NavCom != nullptr && NavCom->RTTI == RTTI_UNIT && Extension::Fetch(static_cast<UnitClass*>(NavCom)->Class)->IsTotable) {
        can_tote = true;
    }

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
        if ((building->Class->IsCanUnitRepair || building->Class->IsHelipad) && !building->Cargo.Is_Something_Attached()) {
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
DEFINE_HOOK(0x0040BDCF, _AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET0_Can_Fire_FIRE_FACING_Patch, 0)
{
    GET(AircraftClass *, this_ptr, ESI);

    AircraftTypeClassExtension* class_ext = Extension::Fetch(this_ptr->Class);
    bool is_curley_shuffle = class_ext->Get_IsCurleyShuffle();
    R->AL(is_curley_shuffle);

    return 0x0040BDDB;

}


DEFINE_HOOK(0x0040C054, _AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_OK_Patch, 0)
{
    GET(AircraftClass *, this_ptr, ESI);

    AircraftTypeClassExtension* class_ext = Extension::Fetch(this_ptr->Class);
    bool is_curley_shuffle = class_ext->Get_IsCurleyShuffle();
    R->CL(is_curley_shuffle);

    return 0x0040BFA8;
}


DEFINE_HOOK(0x0040BF9D, _AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_FIRE_FACING_Patch, 0)
{
    GET(AircraftClass *, this_ptr, ESI);

    AircraftTypeClassExtension* class_ext = Extension::Fetch(this_ptr->Class);
    bool is_curley_shuffle = class_ext->Get_IsCurleyShuffle();
    R->DL(is_curley_shuffle);

    return 0x0040C060;
}


DEFINE_HOOK(0x0040C0AC, _AircraftClass_Mission_Attack_IsCurleyShuffle_FIRE_AT_TARGET2_Can_Fire_DEFAULT_Patch, 0)
{
    GET(AircraftClass *, this_ptr, ESI);

    AircraftTypeClassExtension* class_ext = Extension::Fetch(this_ptr->Class);
    bool is_curley_shuffle = class_ext->Get_IsCurleyShuffle();
    R->AL(is_curley_shuffle);

    return 0x0040C0B8;
}


/**
 *  #issue-264
 * 
 *  Implements LeaveTransportSound for this aircraft is unloading its passengers.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0040988C, _AircraftClass_Mission_Unload_Transport_Detach_Sound_Patch, 0)
{
    GET(AircraftClass *, this_ptr, ESI);

    /**
     *  Don't play the passenger leave sound for carryalls.
     */
    if (!this_ptr->Class->IsCarryall) {

        /**
         *  Do we have a sound to play when passengers leave us? If so, play it now.
         */
        TechnoTypeClassExtension* technotypeext = Extension::Fetch(this_ptr->TClass);
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

            /**
             *  Add this passenger to my team.
             */
            return 0x004098A0;
        }
    }

    /**
     *  Finished unloading passengers.
     */
    return 0x004098AC;
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
DEFINE_HOOK(0x0040A413, _AircraftClass_Mission_Move_LAND_Is_Moving_Check_Patch, 0)
{
    GET(AircraftClass *, this_ptr, ESI);
    
    /**
     *  If the aircraft is not currently moving, enter idle mode.
     */
    if (!this_ptr->Locomotion->Is_Moving()) {
        this_ptr->Enter_Idle_Mode(false, true);
    }

    /**
     *  Function return with "1".
     */
    return 0x0040A421;
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
DEFINE_HOOK(0x00408898, _AircraftClass_Init_IsCloakable_BugFix_Patch, 0)
{
    GET(AircraftClass *, this_ptr, ESI);
    GET(AircraftTypeClass *, aircrafttype, EAX);

    /**
     *  Stolen bytes/code.
     */
    this_ptr->Strength = aircrafttype->MaxStrength;
    this_ptr->Ammo = aircrafttype->MaxAmmo;

    /**
     *  This is the line that was missing (maybe it was by design?).
     */
    this_ptr->IsCloakable = aircrafttype->IsCloakable;

    return 0x004088AA;
}


DEFINE_HOOK(0x0040B3A6, _AircraftClass_Enter_Idle_Mode_Spawner_Patch, 0)
{
    GET(AircraftClass*, this_ptr, ESI);
    GET(int, layer, EAX);
    GET(int, landingaltitude, EBP);

    AircraftTypeClassExtension* aircrafttypeext = Extension::Fetch(this_ptr->Class);

    if (layer != LAYER_GROUND && this_ptr->HeightAGL > landingaltitude && !aircrafttypeext->IsMissileSpawn) {
        return 0x0040B3C1;
    } else {
        return 0x0040B5DC;
    }
}


/**
 *  The below patches make the carryall only unload vehicles by dropping them off,
 *  leaving infantry to be manually unloaded.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004097FF, _AircraftClass_Do_MISSION_UNLOAD_Carryall_Drop_Off_Patch, 0)
{
    GET(AircraftClass*, this_ptr, ESI);

    if (this_ptr->Class->IsCarryall && this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT)) {
        return 0x0040980F;
    }

    return 0x00409833;
}


DEFINE_HOOK(0x0040AD82, _AircraftClass_Do_MISSION_MOVE_CARRYALL_Drop_Off_Patch, 6)
{
    GET(AircraftClass*, this_ptr, ESI);

    if (this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT)) {
        return 0;
    }

    return 0x0040ADD0;
}


DEFINE_HOOK(0x0040D60D, _AircraftClass_Do_MISSION_ENTER_Drop_Off_Patch, 0)
{
    GET(AircraftClass*, this_ptr, ESI);

    if (this_ptr->Class->IsCarryall && this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT)) {
        return 0x0040D62F;
    }

    return 0x0040D6D3;
}


/**
 *  Patches AircraftClass::Draw_It to only draw vehicle passengers' shadows
 *  for carryalls.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00408BF3, _AircraftClass_Draw_It_Carry_All_Patch, 0)
{
    GET(AircraftClass*, this_ptr, EBP);
    GET_STACK(Rect*, cliprect, 0xD0);
    LEA_STACK(Point2D*, drawpoint, 0x10);

    if (this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT) && this_ptr->Class->IsCarryall) {
        this_ptr->Cargo.Attached_Object(RTTI_UNIT)->Draw_It(*drawpoint, *cliprect);
    }

    return 0x00408C27;
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
         *  Added check for RTTI to prevent landing too high on buildings we're in contact with.
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
DEFINE_HOOK(0x00409366, _AircraftClass_AI_Carryall_Facing_Patch, 0)
{
    GET(AircraftClass*, this_ptr, EBP);

    if (this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT) && this_ptr->Class->IsCarryall) {
        this_ptr->Cargo.Attached_Object()->PrimaryFacing.Set(this_ptr->SecondaryFacing.Current());
        this_ptr->Cargo.Attached_Object()->SecondaryFacing.Set(this_ptr->SecondaryFacing.Current());
        this_ptr->Cargo.Attached_Object()->PositionCoord = this_ptr->PositionCoord;
    }

    return 0x004093DE;
}


/**
 *  Patch to prevent spawned aircraft from revealing terrain when they fire.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x0040A195, _AircraftClass_Fire_At_No_Reveal_On_Fire_For_Spawned_Aircraft_Patch, 0)
{
    GET(AircraftClass*, this_ptr, EDI);

    if (Extension::Fetch(this_ptr)->SpawnOwner == nullptr) {
        if (Rule->AttackingAircraftSightRange > 0) {

            // Don't reveal if attacking aircraft sight range has been specified as 0 in Rules.
            // The original game did not have this check (though maybe it has one in MapClass::Sight_From).
            Map.Sight_From(this_ptr->PositionCoord, Rule->AttackingAircraftSightRange, this_ptr->House);
        }
    }

    return 0x0040A1C8;
}


/**
 *  Patches AircraftClass::Assign_Mission to allow Q-Move missions to be registered.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0040B78E, _AircraftClass_Assign_Mission_QMove_Patch, 8)
{
    GET(MissionType, mission, EDI);

    if (mission == MISSION_MOVE && (Keyboard->Down(Options.KeyQueueMove1) || Keyboard->Down(Options.KeyQueueMove2))) {
        mission = MISSION_QMOVE;        
    }

    R->EDI(mission);

    return 0;
}


/**
 *  Patches AircraftClass::Enter_Idle_Mode to add Q-Move processing, similarly to ground classes.
 *  When entering this function, we have no NavCom. When we enter Handle_Navigation_List, 
 *  it will pop out the next Q-Move registered in the queue, if any, and will assign it to the NavCom as the next destination.
 *  We then assign the move mission so it will go out of being idle and continue processing the next move.
 *  Note that in most cases, this is not actually used by aircraft as typically players will move aircraft around with a move order followed by
 *  additional queued moves, so this is a default in case we entered idle mode for some reason (such as carryalls just releasing a unit it was carrying)
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0040B35A, _AircraftClass_Enter_Idle_Mode_QMove_Patch, 6)
{
    GET(AircraftClass*, this_ptr, ESI);

    this_ptr->Handle_Navigation_List();
    if (this_ptr->NavCom != nullptr) {
        this_ptr->Assign_Mission(MISSION_MOVE);
        return 0x0040B340;
    }

    return 0;
}


/**
 *  Determines if the aircraft applies their next QMove while moving.
 *  Returns whether the aircraft has been assigned to the next QMove target
 *
 *  @author: JoyfulShush
 */
bool AircraftClassExt::_Do_MISSION_MOVE_Apply_QMove()
{
    if (NavQueue.Count() <= 0) {
        return false;
    }

    if (NavCom == nullptr) {
        return false;
    }

    // For carryalls, if the current navcom is a unit, then we want to stop by and pick it up.
    // Defer to pick up patch to handle Q-Move instead
    if (Class->IsCarryall && NavCom->Fetch_RTTI() == RTTI_UNIT) {
        return false;
    }

    Coord dest_coords = NavCom->Center_Coord();
    Coord current_coords = Get_Coord();

    dest_coords.Z = 0;
    current_coords.Z = 0;

    int distance = dest_coords.Distance_To(current_coords);

    if (distance < CELL_LEPTON) {
        if (NavQueue.Count() > 0) {
            NavCom = nullptr;

            Handle_Navigation_List();
            if (NavCom != nullptr) {
                return true;
            }
        }
    }

    return false;
}


/**
 *  Patches AircraftClass::Do_MISSION_MOVE_ to support Q-Moving during movement in order to allow aircraft to stay in the air.
 *  Without this, they will land after each position, enter idle mode, trigger Q-Move processing, and move to the next position.
 *  During flight, if we have additional movements queued up, we trigger Q-Move processing once the aircraft is close enough to its target position. 
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0040A655, _AircraftClass_Do_MISSION_MOVE__QMove_Patch, 6)
{    
    GET(AircraftClassExt*, this_ptr, ESI);

    bool qmove_applied = this_ptr->_Do_MISSION_MOVE_Apply_QMove();

    if (qmove_applied) {
        return 0x0040A711; // jump to statement resetting status to 0 and returning 1
    }
    
    return 0;
}


/**
 *  Patches AircraftClass::Do_MISSION_MOVE_Carryall to support Q-Moving during movement in order to allow aircraft to stay in the air.
 *  Without this, they will land after each position, enter idle mode, trigger Q-Move processing, and move to the next position.
 *  During flight, if we have additional movements queued up, we trigger Q-Move processing once the aircraft is close enough to its target position.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0040AD38, _AircraftClass_Do_MISSION_MOVE_Carryall_QMove_Patch, 6)
{
    GET(AircraftClassExt*, this_ptr, ESI);

    bool qmove_applied = this_ptr->_Do_MISSION_MOVE_Apply_QMove();

    if (qmove_applied) {
        return 0x0040ACFC; // jump to statement resetting status to 0 and returning 1
    }

    return 0;
}


/**
 *  Patches AircraftClass::Do_MISSION_MOVE_Carryall after possibly attaching a unit
 *  Reimplmements the parts between having a cargo and skipping over entering idle mode
 *  This allows a carryall to keep moving while holding its cargo until completing all queued movements orders.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0040AE92, _AircraftClass_Do_MISSION_MOVE_Carryall_QMove_Attach_Cargo_Patch, 8)
{
    GET(AircraftClass*, this_ptr, ESI);

    if (!this_ptr->Cargo.Is_Something_Attached(RTTI_UNIT))
        return 0;

    if (this_ptr->NavQueue.Count() > 0) {
        this_ptr->NavCom = nullptr;
        
        this_ptr->Handle_Navigation_List();
        if (this_ptr->NavCom != nullptr) {
            this_ptr->Status = 0;
            this_ptr->Transmit_Message(RADIO_OVER_OUT);
            this_ptr->Mark(MARK_DOWN);
            this_ptr->field_370 = true; // seems to tell a service depot that it is carrying cargo that can be dropped into it for repairs
            return 0x0040AEC4; // jump to statement cleaning up and returning 1
        }
    }

    return 0;
}


/**
 *  Patches AircraftClass::Do_MISSION_MOVE_Carryall to fix a bug where carryall can land on a unit without picking it up.
 *  This can happen if the player clicks on a Service Depot while the carryall is landing to pick up a unit
 *  which seems to break radio contact with the unit in order to talk with the Service Depot instead (RADIO_CAN_LOAD)
 *  Instead, this forces the carryall to go into a nearby valid cell.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0040ABEB, _Aircraft_Class_Do_MISSION_MOVE_Carryall_Lost_Contact, 6)
{
    GET(AircraftClass*, this_ptr, ESI);    
    
    if (this_ptr->Radio) {
        this_ptr->NavCom = nullptr;
        this_ptr->Enter_Idle_Mode();
    }

    return 0;
}


/*
 *  Patches AircraftClass::Look, replacing the value of Sight to take into account all sight range modifications for this techno.
 *  Particularly relevant to veterancy granting sight range bonuses.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0040E565, _Aircraft_Class_Look_Sight_Range_Patch, 6)
{
    GET(AircraftClass*, this_ptr, ESI);
    
    auto techno_ext = Extension::Fetch(this_ptr);

    R->EDI(techno_ext->Get_Sight_Range());

    return 0;
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
    Patch_Jump(0x0040B7E0, &AircraftClassExt::_What_Action);
    Patch_Jump(0x0040EDD0, &AircraftClassExt::_Landing_Altitude_Thunk);
    Patch_Jump(0x0040C8A0, &AircraftClassExt::_Receive_Message);
}
