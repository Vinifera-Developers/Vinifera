/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended BuildingClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "buildingext_hooks.h"

#include "aircraft.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
#include "anim.h"
#include "animext.h"
#include "asserthandler.h"
#include "audio_vox.h"
#include "bsurface.h"
#include "building.h"
#include "buildingext.h"
#include "buildingext_init.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "bullettype.h"
#include "cell.h"
#include "convert.h"
#include "debughandler.h"
#include "drawshape.h"
#include "event.h"
#include "extension.h"
#include "factory.h"
#include "hooker.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "infantrytype.h"
#include "jumpjetlocomotion.h"
#include "map.h"
#include "mouse.h"
#include "rules.h"
#include "rulesext.h"
#include "session.h"
#include "sideext.h"
#include "spritecollection.h"
#include "super.h"
#include "supertypeext.h"
#include "syringe.h"
#include "tactical.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_globals.h"
#include "unit.h"
#include "unitext.h"
#include "unittype.h"
#include "vinifera_saveload.h"
#include "voc.h"
#include "vox.h"
#include "weapontype.h"

#include <algorithm>


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(BuildingClass)
{
public:
    bool _Can_Have_Rally_Point();
    void _Update_Buildables();
    const InfantryTypeClass* _Crew_Type() const;
    int _How_Many_Survivors() const;
    int _Shape_Number() const;
    void _Detach_Anim(AnimClass* anim);
    void _Draw_It(Point2D const& xdrawpoint, Rect const& xcliprect);
    void _Detach_All(bool all);
    bool _Toggle_Primary();
    void _Assign_Rally_Point(const Cell& cell);
    ActionType _What_Action(ObjectClass const* object, bool disallow_force);
    ActionType _What_Action(const Cell& cell, bool check_fog, bool disallow_force) const;
    void _Factory_AI();
    SuperWeaponType _Fetch_Super_Weapon() const;
    SuperWeaponType _Fetch_Super_Weapon2() const;
    void _Swizzle_Light_Source();
    RadioMessageType _Receive_Message(RadioClass * from, RadioMessageType message, long& param);
};


bool BuildingClassExt::_Can_Have_Rally_Point()
{
    RTTIType tobuild = this->Class->ToBuild;
    if (tobuild == RTTI_UNITTYPE || tobuild == RTTI_INFANTRYTYPE || tobuild == RTTI_AIRCRAFTTYPE)
        return true;

    /**
     *  #issue-966
     *
     *  Makes it possible to give rally points to Service Depots.
     *
     *  @author: Rampastring
     */
    if (this->Class->IsCanUnitRepair)
        return true;

    /**     
     *  Makes it possible to give rally points to Hospitals and Armories
     *
     *  @author: JoyfulShush
     */
    if (this->Class->IsArmory || this->Class->IsHospital) 
        return true;

    return false;
}


/**
 *  Makes the game check whether you can actually build the object before adding it to the sidebar,
 *  preventing grayed out cameos (except for build limited types)
 *
 *  This reimplements the entire BuildingClass::Update_Buildables() function
 *
 *  @author: ZivDero
 */
void BuildingClassExt::_Update_Buildables()
{
    if (House == PlayerPtr && !IsInLimbo && IsDiscoveredByPlayer && IsOn) {
        switch (Class->ToBuild) {
        case RTTI_AIRCRAFTTYPE:
            for (int i = 0; i < AircraftTypes.Count(); i++) {
                int can_build = PlayerPtr->Can_Build(AircraftTypes[i], false, true); // 0 = not allowed, 1 = allowed, -1 = build limit
                if (can_build && (can_build == -1 || AircraftTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)) {
                    Map.Add(RTTI_AIRCRAFTTYPE, i);
                }
            }
            break;

        case RTTI_BUILDINGTYPE:
            for (int i = 0; i < BuildingTypes.Count(); i++) {
                int can_build = PlayerPtr->Can_Build(BuildingTypes[i], false, true);
                if (can_build && (can_build == -1 || BuildingTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)) {
                    Map.Add(RTTI_BUILDINGTYPE, i);
                }
            }
            break;

        case RTTI_INFANTRYTYPE:
            for (int i = 0; i < InfantryTypes.Count(); i++) {
                int can_build = PlayerPtr->Can_Build(InfantryTypes[i], false, true);
                if (can_build && (can_build == -1 || InfantryTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)) {
                    Map.Add(RTTI_INFANTRYTYPE, i);
                }
            }
            break;

        case RTTI_UNITTYPE:
            for (int i = 0; i < UnitTypes.Count(); i++) {
                int can_build = PlayerPtr->Can_Build(UnitTypes[i], false, true);
                if (can_build && (can_build == -1 || UnitTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)) {
                    Map.Add(RTTI_UNITTYPE, i);
                }
            }
            break;

        default:
            break;
        }
    }
}


/**
 *  Fetches the kind of crew this object contains.
 *
 *  @author: ZivDero
 */
const InfantryTypeClass* BuildingClassExt::_Crew_Type() const
{
    /**
     *  Construction yards can sometimes have an engineer exit them.
     */
    const int engineer_chance = Extension::Fetch(Class)->EngineerChance;
    if (!IsCaptured && Percent_Chance(engineer_chance))
        return SideClassExtension::Get_Engineer(House);

    return TechnoClass::Crew_Type();
}


/**
 *  This determines the maximum number of survivors.
 *
 *  @author: 08/04/1996 JLB - Created
 *           ZivDero - Adjustments for Tiberian Sun
 */
int BuildingClassExt::_How_Many_Survivors() const
{
    if (IsSurvivorless || !Class->IsCrew)
        return 0;

    int divisor = SideClassExtension::Get_Survivor_Divisor(House);
    if (divisor == 0)
        return 0;

    if (IsCaptured)
        divisor *= 2;

    const int count = Class->Cost_Of(House) * Rule->SurvivorFraction / divisor;
    return std::clamp(count, 1, 5);
}


/**
 *  Fetch the shape number for this building.
 *
 *  @author: 07/29/1996 JLB - Created
 *           ZivDero - Adjustments for Tiberian Sun
 */
int BuildingClassExt::_Shape_Number() const
{
    int shapenum = Fetch_Stage();

    if (Class->IsLaserFence) {
        return LaserFenceFrame;
    }

    if (Class->IsFirestormWall) {
        return FirestormWallFrame;
    }

    /**
     *  The shape file to use for rendering depends on whether the building
     *  is undergoing construction or not.
     */
    if (BState == BSTATE_CONSTRUCTION) {

        if (Class->IsGate) {
            shapenum = Class->Anims[BSTATE_CONSTRUCTION].Start + Class->Anims[BSTATE_CONSTRUCTION].Count - 1 - shapenum;
        }

        /**
         *  If the building is deconstructing, then the display frame progresses
         *  from the end to the beginning. Reverse the shape number accordingly.
         */
        if (Mission == MISSION_DECONSTRUCTION) {
            shapenum = Class->Anims[BState].Start + Class->Anims[BState].Count - 1 - shapenum;
        }

    } else if (Class->IsGate) {

        if (HealthRatio <= Rule->ConditionYellow) {
            return Class->GateStages + 1;
        } else {
            return 0;
        }

    } else {

        /**
         *  If below half strenth, then show the damage frames of the
         *  building.
         */
        if (HealthRatio <= Rule->ConditionYellow) {
            if (BState == BSTATE_IDLE) {
                shapenum++;
            } else {
                int last1 = Class->Anims[BSTATE_IDLE].Start + Class->Anims[BSTATE_IDLE].Count;
                int last2 = Class->Anims[BSTATE_ACTIVE].Start + Class->Anims[BSTATE_ACTIVE].Count;
                int largest = std::max(last1, last2);
                last2 = Class->Anims[BSTATE_AUX1].Start + Class->Anims[BSTATE_AUX1].Count;
                largest = std::max(largest, last2);
                last2 = Class->Anims[BSTATE_AUX2].Start + Class->Anims[BSTATE_AUX2].Count;
                largest = std::max(largest, last2);
                shapenum += largest;
            }
        }
    }
    return shapenum;
}


/**
 *  Detaches the animation from the building, and also
 *  creates a "sequel" animation in some cases.
 *
 *  @author: ZivDero
 */
void BuildingClassExt::_Detach_Anim(AnimClass* anim)
{
    if (IsActive) {
        for (int i = 0; i < BANIM_COUNT; i++) {
            if (Anims[i] == anim) {
                Anims[i] = nullptr;
                switch (i) {
                case BANIM_SPECIAL_ONE:
                    if (Class->IsCanUnitRepair) {
                        if (In_Radio_Contact() && Get_Mission() == MISSION_REPAIR) {
                            Begin_Anim(BANIM_SPECIAL_TWO, Get_Health_Ratio() <= Rule->ConditionYellow, 0);
                        }
                    }
                    else if (Class->IsNukeSilo) {
                        if (Get_Mission() == MISSION_MISSILE) {
                            Begin_Anim(BANIM_SPECIAL_TWO, Get_Health_Ratio() <= Rule->ConditionYellow, 0);
                        }
                    }
                    IsToDisplay = true;
                    break;

                case BANIM_SPECIAL_TWO:
                    if (Class->IsNukeSilo) {
                        if (Get_Mission() == MISSION_MISSILE) {
                            Begin_Anim(BANIM_SPECIAL_THREE, Get_Health_Ratio() <= Rule->ConditionYellow, 0);
                        }
                    }
                    IsToDisplay = true;
                    break;

                case BANIM_SPECIAL_THREE:
                    IsToDisplay = true;
                    break;

                default:
                    break;
                }
            }
        }
    }
}


/**
 *  Reimplementation of BuildingClass::Draw_It.
 *
 *  @author: ZivDero
 */
void BuildingClassExt::_Draw_It(Point2D const& xdrawpoint, Rect const& xcliprect)
{
    Cell cell = Get_Cell();

    /*
    **  The shape file to use for rendering depends on whether the building
    **  is undergoing construction or not.
    */
    ShapeSet const* shapefile = Get_Image_Data();
    if (shapefile == nullptr) return;

    if (Class->IsInvisibleInGame) return;

    const auto type_ext = Extension::Fetch(Class);
    if (type_ext->IsHideDuringSpecialAnim && (Anims[BANIM_SPECIAL_ONE] || Anims[BANIM_SPECIAL_TWO] || Anims[BANIM_SPECIAL_THREE])) return;

    bool open_roof = false;
    if (Get_Mission() == MISSION_UNLOAD) {
        TechnoClass* radio = Contact_With_Whom();
        if (radio != nullptr && radio->TClass->Locomotor == __uuidof(JumpjetLocomotionClass)) {
            open_roof = true;
        }
    }

    Point2D zdrawpoint(144, 172);
    int zadjust = Class->NormalZAdjust;

    if (Get_Mission() == MISSION_OPEN && !Door.Func1()) {

        int shapenum = static_cast<int>(Door.Get_Percent_Complete() * Class->GateStages);
        if (Door.Is_Door_Closing()) {
            shapenum = Class->GateStages - shapenum;
        }
        if (Door.Is_Door_Closed()) {
            shapenum = 0;
        }
        if (Door.Is_Door_Open()) {
            shapenum = Class->GateStages - 1;
        }
        shapenum = std::min(shapenum, Class->GateStages - 1);
        shapenum = std::max(shapenum, 0);

        shapefile = Get_Image_Data();

        ZGradientType zgrad = ZGRAD_GROUND;
        if (shapenum < Class->GateStages / 2 || type_ext->IsBarGate) {
            zgrad = ZGRAD_90DEG;
        }

        shapenum += HealthRatio <= Rule->ConditionYellow ? Class->GateStages + 1 : 0;
        Techno_Draw_Object(shapefile, shapenum, xdrawpoint, xcliprect, DIR_N, 256, zadjust - TacticalMap->Z_Lepton_To_Pixel(Height), zgrad, true, Map[cell].Brightness);

        return;
    }

    if (Get_Mission() == MISSION_UNLOAD) {
        if (open_roof) {
            if (type_ext->RoofDeployingAnim != nullptr) {
                shapefile = type_ext->RoofDeployingAnim;
                zadjust = 0;
            }
        }
        else {
            if (Class->DeployingAnim != nullptr) {
                shapefile = Class->DeployingAnim;
                zadjust = 0;
            }
        }
    }

    Point2D drawpoint = xdrawpoint;
    int height = drawpoint.Y + shapefile->Get_Height() / 2;

    Rect cliprect = xcliprect;
    cliprect.Height = std::min(cliprect.Height, height);

    zdrawpoint += Class->ZShapePointMove;
    Point2D zsizeoffset(Class->Width() * CELL_LEPTON_W - CELL_LEPTON_W, Class->Height() * CELL_LEPTON_H - CELL_LEPTON_H);
    zdrawpoint -= TacticalMap->func_60F270(zsizeoffset);

    ShapeSet const* zshapefile = BuildingTypeClass::BuildingZShape;
    if (Class->Width() >= 6) {
        zshapefile = nullptr;
    }

    if (cliprect.Height > 0) {

        /*
        **  Actually draw the building shape.
        */
        if ((Class->IsLaserFence && (LaserFenceFrame == 12 || LaserFenceFrame == 8)) || Class->IsFirestormWall) {
            Techno_Draw_Object(shapefile, Shape_Number(), drawpoint, cliprect, DIR_N, 256, -1 - TacticalMap->Z_Lepton_To_Pixel(Height), ZGRAD_GROUND, true, Map[cell].Brightness + Class->ExtraLight);
        }
        else {
            Techno_Draw_Object(shapefile, Shape_Number() < shapefile->Get_Count() / 2 ? Shape_Number() : shapefile->Get_Count() / 2, drawpoint, cliprect, DIR_N, 256, zadjust - TacticalMap->Z_Lepton_To_Pixel(Height), ZGRAD_90DEG, true, Map[cell].Brightness + Class->ExtraLight, zshapefile, 0, zdrawpoint);
        }
    }

    /*
    **  Patch for adding overlay onto weapon factory.  Only add the overlay if
    **  the building has more than 1 hp.  Also, if the building's in radio
    **  contact, he must be unloading a constructed vehicle, so draw that
    **  vehicle before drawing the overlay.
    */
    if (Class->BibShape && BState != BSTATE_CONSTRUCTION) {
        Techno_Draw_Object(Class->BibShape, Shape_Number(), xdrawpoint, xcliprect, DIR_N, 256, -1 - TacticalMap->Z_Lepton_To_Pixel(Height), ZGRAD_GROUND, true, Map[cell].Brightness + Class->ExtraLight);
    }

    /*
    **  Draw the weapon factory custom overlay graphic.
    */
    if (Get_Mission() == MISSION_UNLOAD) {
        ShapeSet const* under_door_anim;
        if (open_roof) {
            under_door_anim = type_ext->UnderRoofDoorAnim;
        } else {
            under_door_anim = Class->UnderDoorAnim;
        }
        if (under_door_anim != nullptr) {
            Techno_Draw_Object(under_door_anim, HealthRatio <= Rule->ConditionYellow ? 1 : 0, xdrawpoint, xcliprect, DIR_N, 256, -TacticalMap->Z_Lepton_To_Pixel(Height), ZGRAD_GROUND, true, Map[cell].Brightness + Class->ExtraLight);
        }
    }
}


/**
 *  Reimplementation of BuildingClass::Detach_All.
 *
 *  @author: ZivDero
 */
void BuildingClassExt::_Detach_All(bool all)
{
    if (all) {
        /*
        **  If it is producing something, then it must be abandoned.
        */
        if (Factory) {
            Factory->Abandon();
            delete Factory;
            Factory = nullptr;
        }

        /*
        **  If the owner HouseClass is building something, and this building can
        **  build that thing, we may be the last building for that house that can
        **  build that thing; if so, abandon production of it.
        */
        if (House) {
            auto type_ext = Extension::Fetch(Class);
            ProductionFlags prodflags = PRODFLAG_NONE;
            if (type_ext->IsNaval) {
                prodflags = PRODFLAG_NAVAL;
            }

            FactoryClass* factory = Extension::Fetch(House)->Fetch_Factory(Class->ToBuild, prodflags);

            /*
            **  If a factory was found, then temporarily disable this building and then
            **  determine if any object that is being produced can still be produced. If
            **  not, then the object being produced must be abandoned.
            */
            if (factory) {
                TechnoClass* object = factory->Get_Object();
                bool limbo = IsInLimbo;
                IsInLimbo = true;
                if (object && !object->TClass->Who_Can_Build_Me(true, false, false, House)) {
                    Extension::Fetch(House)->Abandon_Production(Class->ToBuild, -1, prodflags);
                }
                IsInLimbo = limbo;
            }
        }
    }

    if (!all) {
        if (In_Radio_Contact() && !House->Is_Ally(Contact_With_Whom())) {
            Transmit_Message(RADIO_OVER_OUT);
        }
    } else {
        Transmit_Message(RADIO_OVER_OUT);
    }

    TechnoClass::Detach_All(all);
}


/**
 *  Reimplementation of BuildingClass::Toggle_Primary.
 *
 *  @author: ZivDero
 */
bool BuildingClassExt::_Toggle_Primary()
{
    if (Class->ToBuild == RTTI_NONE) {
        return IsLeader;
    }

    if (IsLeader) {
        IsLeader = false;
    } else {
        for (int index = 0; index < Buildings.Count(); index++) {
            BuildingClass* building = Buildings[index];

            if (!building->IsInLimbo && building->House == House && building->Class->ToBuild == Class->ToBuild &&
                Extension::Fetch(building->Class)->IsNaval == Extension::Fetch(Class)->IsNaval) {
                building->IsLeader = false;
            }
        }
        IsLeader = true;
        if (House->Is_Player_Control()) {
            Speak(VOX_PRIMARY_SELECTED);
        }
    }
    Mark(MARK_CHANGE);
    return IsLeader;
}


/**
 *  Reimplementation of BuildingClass::Assign_Rally_Point.
 *
 *  @author: ZivDero
 */
void BuildingClassExt::_Assign_Rally_Point(Cell const& cell)
{
    SpeedType speed = SPEED_FOOT;
    MZoneType mzone = MZONE_NORMAL;

    bool underbridge = Map[cell].IsUnderBridge;

    if (Class->ToBuild == RTTI_AIRCRAFTTYPE) {
        speed = SPEED_WINGED;
        mzone = MZONE_FLYER;
    } else {

        /**
         *  If this is a factory that produces units, and is flagged as a shipyard (Naval=yes), then
         *  change the zone flags to scan for water regions only.
         *
         *  @author: CCHyper, modified by Rampastring
         */
        if (Class->ToBuild == RTTI_UNITTYPE && Extension::Fetch(Class)->IsNaval) {
            speed = SPEED_AMPHIBIOUS;
            mzone = MZONE_AMPHIBIOUS_CRUSHER;
        }
    }

    int zone = Map.Get_Cell_Zone(Get_Coord().As_Cell(), mzone, underbridge);

    Cell nearbyloc = Map.Nearby_Location(cell, speed, zone, mzone, underbridge);

    if (nearbyloc != CELL_NONE) {
        OutList.Add(EventClass(Owner(), EVENT_ARCHIVE, TargetClass(this), TargetClass(&Map[nearbyloc])));
    } else {
        if (Class->IsConstructionYard && House->Is_Human_Player() && Session.Type != GAME_NORMAL && Session.Options.RedeployMCV) {
            OutList.Add(EventClass(Owner(), EVENT_ARCHIVE, TargetClass(this), TargetClass(&Map[Center_Coord()])));
        }
    }
}


/**
 *  Reimplementation of BuildingClass::What_Action.
 *
 *  @author: ZivDero
 */
ActionType BuildingClassExt::_What_Action(ObjectClass const* object, bool disallow_force)
{
    if (Class->IsInvisibleInGame) {
        return ACTION_NONE;
    }

    if (object->RTTI == RTTI_BUILDING && ((BuildingClass*)object)->Class->IsInvisibleInGame) {
        return ACTION_NONE;
    }

    ActionType action = TechnoClass::What_Action(object, disallow_force);

    if (action == ACTION_SELF) {
        int index; 
        if (EMPFramesRemaining == 0 && Class->ToBuild != RTTI_NONE && PlayerPtr == House &&
            Extension::Fetch(House)->Factory_Count(Class->ToBuild, Extension::Fetch(Class)->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE) > 1) {

            switch (Class->ToBuild) {
            case RTTI_INFANTRYTYPE:
            case RTTI_INFANTRY:
                action = ACTION_NONE;
                for (index = 0; index < Buildings.Count(); index++) {
                    BuildingClass* bldg = Buildings[index];
                    if (bldg != this && bldg->House == House && bldg->Class->ToBuild == RTTI_INFANTRYTYPE) {
                        action = ACTION_SELF;
                        break;
                    }
                }
                break;

            case RTTI_NONE:
                action = ACTION_NONE;
                break;

            default:
                break;
            }

        } else {
            action = ACTION_NONE;
        }
    }

    /*
    **  Don't allow targeting with SAM sites, even if the CTRL key
    **  is held down. Also don't allow targeting if the object is too
    **  far away.
    */
    if (action == ACTION_ATTACK && PrimaryWeapon != nullptr) {
        if (!In_Range_Of(const_cast<ObjectClass*>(object))/* || !PrimaryWeapon->Bullet->IsAntiGround*/) {
            action = ACTION_NONE;
        } else if (Class->IsEMPulseCannon || Class->IsLimpetMine) {
            action = ACTION_NONE;
        }
        if (CurrentMission == MISSION_DECONSTRUCTION) {
            action = ACTION_NONE;
        }
    }

    if (action == ACTION_MOVE || action == ACTION_NOMOVE) {
        if (!Can_Player_Move()) {
            action = ACTION_SELECT;
        } else if (Class->ToBuild == RTTI_INFANTRYTYPE || Class->ToBuild == RTTI_UNITTYPE || Class->ToBuild == RTTI_AIRCRAFTTYPE) {
            bool altdown = Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2);
            if (!altdown) {
                action = ACTION_SELECT;
            } else {
                Cell cell = object->Center_Coord().As_Cell();
                if (Class->ToBuild != RTTI_AIRCRAFTTYPE) {
                    if (!Is_In_Same_Zone(cell.As_Coord())) {
                        action = ACTION_NOMOVE;
                    }
                    if (!Map[cell].IsUnderBridge) {

                        /**
                         *  This patch allows naval yards to display the "place rally point" cursor action
                         *  on water cells.
                         *
                         *  @author: Rampastring
                         */
                        if (Map[cell].Passability != PASSABLE_LAND && !(Extension::Fetch(Class)->IsNaval && Map[cell].Passability == PASSABLE_WATER)) {
                            action = ACTION_NOMOVE;
                        }
                    }
                }
            }
        }
    }

    return action;
}


/**
 *  Reimplementation of BuildingClass::What_Action.
 *
 *  @author: ZivDero
 */
ActionType BuildingClassExt::_What_Action(const Cell& cell, bool check_fog, bool disallow_force) const
{
    if (Class->IsInvisibleInGame) {
        return ACTION_NONE;
    }

    ActionType action;

    if (Class->UndeploysInto != nullptr && check_fog) {
        action = TechnoClass::What_Action(cell, false, disallow_force);
    } else {
        action = TechnoClass::What_Action(cell, check_fog, disallow_force);
    }


    if (action == ACTION_RALLY_TO_POINT) {
        if (Class->ToBuild != RTTI_AIRCRAFTTYPE) {
            if (!Is_In_Same_Zone(cell.As_Coord())) {
                action = ACTION_NOMOVE;
            }
            if (!Map[cell].IsUnderBridge) {

                /**
                 *  This patch allows naval yards to display the "place rally point" cursor action
                 *  on water cells.
                 *
                 *  @author: Rampastring
                 */
                if (Map[cell].Passability != PASSABLE_LAND && !(Extension::Fetch(Class)->IsNaval && Map[cell].Passability == PASSABLE_WATER)) {
                    action = ACTION_NOMOVE;
                }
                
            }
        }
    }

    /*
    **  Don't allow targeting of SAM sites, even if the CTRL key
    **  is held down.
    */
    if (action == ACTION_ATTACK && PrimaryWeapon != nullptr) {
        if (!PrimaryWeapon->Bullet->IsAntiGround) {
            action = ACTION_NONE;
        } else if (Class->IsEMPulseCannon || Class->IsLimpetMine) {
            action = ACTION_NONE;
        } if (CurrentMission == MISSION_DECONSTRUCTION) {
            action = ACTION_NONE;
        }
    }

    return action;
}


/**
 *  Reimplementation of BuildingClass::Factory_AI.
 *
 *  @author: ZivDero
 */
void BuildingClassExt::_Factory_AI()
{
    /*
    **  Handle any production tied to this building. Only computer controlled buildings have
    **  production attached to the building itself. The player uses the sidebar interface for
    **  all production control.
    */
    if (Factory != nullptr && Factory->Has_Completed() && PlacementDelay == 0) {
        TechnoClass* product = Factory->Get_Object();

        switch (Exit_Object(product)) {

            /*
            **  If the object could not leave the factory, then either request
            **  a transport, place the (what must be a) building using another method, or
            **  abort the production and refund money.
            */
        case 0:
            Factory->Abandon();
            delete Factory;
            Factory = nullptr;
            break;

            /*
            **  Exiting this building is prevented by some temporary blockage. Wait
            **  a bit before trying again.
            */
        case 1:
            PlacementDelay = static_cast<int>(Rule->PlacementDelay * TICKS_PER_MINUTE);
            break;

            /*
            **  The object was successfully sent from this factory. Inform the house
            **  tracking logic that the requested object has been produced.
            */
        case 2:
            House->Just_Built(product);
            Factory->Completed();
            delete Factory;
            Factory = nullptr;
            break;

        default:
            break;
        }
    }

    /*
    **  Pick something to create for this factory.
    */
    if (House->IsStarted && Mission != MISSION_CONSTRUCTION && Mission != MISSION_DECONSTRUCTION) {

        /*
        **  Buildings that produce other objects have special factory logic handled here.
        */
        if (Class->ToBuild != RTTI_NONE) {
            if (Factory != nullptr) {

                /*
                **  If production has halted, then just abort production and make the
                **  funds available for something else.
                */
                if (PlacementDelay == 0 && !Factory->Is_Building()) {
                    Factory->Abandon();
                    delete Factory;
                    Factory = nullptr;
                }

            } else {

                /*
                **  Only look to start production if there is at least a small amount of
                **  money available. In cases where there is no practical money left, then
                **  production can never complete -- don't bother starting it.
                */
                if (House->IsStarted && House->Available_Money() > 10) {
                    auto btype_ext = Extension::Fetch(Class);
                    TechnoTypeClass const* ttype = Extension::Fetch(House)->Suggest_New_Object(Class->ToBuild, btype_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);

                    /*
                    **  If a suitable object type was selected for production, then start
                    **  producing it now.
                    */
                    if (ttype != nullptr) {

                        /*
                        **  But first, verify if this building is a valid factory for this object.
                        */
                        bool allowed_factory = true;
                        auto ttype_ext = Extension::Fetch(ttype);
                        if (ttype->RTTI == RTTI_UNITTYPE) {
                            if (btype_ext->IsNaval != ttype_ext->IsNaval) {
                                allowed_factory = false;
                            }
                        }

                        /*
                        ** There may be limitations on whether this specific factory can build this object.
                        */
                        if (allowed_factory && !ttype_ext->BuiltAt.Is_Present(Class)) {

                            /*
                            **  This object doesn't allow this factory to produce it.
                            */
                            if (ttype_ext->BuiltAt.Count() != 0) {
                                allowed_factory = false;
                            }

                            /*
                            **  This factory can't produce this kind of object.
                            */
                            else if (btype_ext->IsExclusiveFactory) {
                                allowed_factory = false;
                            }
                        }

                        /*
                        **  If everything is okay, create the factory.
                        */
                        if (allowed_factory) {
                            Factory = new FactoryClass;
                            if (Factory != nullptr) {
                                if (!Factory->Set(*ttype, *House, false)) {
                                    delete Factory;
                                    Factory = nullptr;
                                } else {
                                    House->Production_Begun(Factory->Get_Object());
                                    Factory->Start(false);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


/**
 *  Reimplementation of BuildingClass::Fetch_Super_Weapon.
 *
 *  @author: ZivDero
 */
SuperWeaponType BuildingClassExt::_Fetch_Super_Weapon() const
{
    if (Class->SuperWeapon != SUPER_NONE) {
        BuildingTypeClass const* aux = Supers[Class->SuperWeapon]->Class->AuxBuilding;

        /**
         *  Fix: use the prerequisite check to allow building upgrades to be AuxBulding.
         */
        if (aux != nullptr && !Extension::Fetch(House)->Has_Prerequisite(aux->HeapID)) {
            return SUPER_NONE;
        }
    }
    return Class->SuperWeapon;
}


/**
 *  Reimplementation of BuildingClass::Fetch_Super_Weapon2.
 *
 *  @author: ZivDero
 */
SuperWeaponType BuildingClassExt::_Fetch_Super_Weapon2() const
{
    if (Class->SuperWeapon2 != SUPER_NONE) {

        /**
         *  Fix: use the prerequisite check to allow building upgrades to be AuxBulding.
         */
        BuildingTypeClass const* aux = Supers[Class->SuperWeapon2]->Class->AuxBuilding;
        if (aux != nullptr && !Extension::Fetch(House)->Has_Prerequisite(aux->HeapID)) {
            return SUPER_NONE;
        }
    }
    return Class->SuperWeapon2;
}


/**
 *  This patch fetches the correct factory when displaying a cameo on a spied factory.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00428AA4, _BuildingClass_Draw_Overlays_Fetch_Factory_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    BuildingTypeClassExtension const* type_ext = Extension::Fetch(this_ptr->Class);
    FactoryClass* factory = Extension::Fetch(this_ptr->House)->Fetch_Factory(
        this_ptr->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);
    R->EAX(factory);

    return 0x00428AC4;
}


/**
 *  Replaces an inlined call of Detach_Anim with a direct call.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00433F1D, _BuildingClass_Detach_Detach_Anim_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    GET(AnimClass*, anim, ECX);

    this_ptr->Detach_Anim(anim);

    return 0x00433F84;
}


/**
 *  #issue-204
 * 
 *  Implements ReloadRate for AircraftTypes, allowing each aircraft to have
 *  its own independent ammo reloading rate when docked with a helipad.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0043266C, _BuildingClass_Mission_Repair_ReloadRate_Patch, 0)
{
    GET(BuildingClass*, this_ptr, EBP);

    AircraftClass* radio = reinterpret_cast<AircraftClass*>(this_ptr->Contact_With_Whom());
    AircraftTypeClassExtension* radio_class_ext = Extension::Fetch(radio->Class);
    int time = radio_class_ext->Get_ReloadRate() * TICKS_PER_MINUTE;
    R->EAX(time);

    return 0x0043260F;
}


/**
 *  #issue-966
 *
 *  Assigns destination to a unit when it's leaving a service depot.
 *
 *  @author: Rampastring
 */
bool _BuildingClass_Mission_Repair_Assign_Unit_Destination(BuildingClass *building, TechnoClass *techno, bool clear_archive)
{
    AbstractClass * target = nullptr;

    if (building->ArchiveTarget != nullptr) {
        if (building->ArchiveTarget != nullptr) target = building->ArchiveTarget;
    }

    if (target == nullptr) {

        /**
         *  Stolen bytes/code.
         *  Reimplements original game behaviour.
         */
        Cell exitcell = building->Find_Exit_Cell(reinterpret_cast<TechnoClass*>(building->Radio));

        if (exitcell.X == 0 && exitcell.Y == 0) {

            /**
             *  Failed to find valid exit cell.
             */
            return false;
        }

        CellClass* cellptr = &Map[exitcell];
        target = As_Target(cellptr);
    }

    techno->Assign_Mission(MISSION_MOVE);
    techno->Assign_Destination(target);

    if (clear_archive) {
        /**
         *  Clear the archive target in cases where the original game did so as well.
         */
        techno->ArchiveTarget = nullptr;
    }

    building->Transmit_Message(RADIO_OVER_OUT);
    techno->field_20C = nullptr;
    return true;
}


/**
 *  #issue-966
 *
 *  Makes Service Depots assign their archive target as the destination to units
 *  that didn't need any repair when entering the depot.
 *
 *  This reimplements the whole 0x00432184 - 0x00432202 range of the original game's code.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00432184, _BuildingClass_Mission_Repair_Assign_Rally_Destination_When_No_Repair_Needed, 0)
{
    GET(BuildingClass*, building, EBP);
    GET(TechnoClass*, techno, ESI);

    _BuildingClass_Mission_Repair_Assign_Unit_Destination(building, techno, true);

    /**
     *  Set mission delay and return, regardless of whether the
     *  destination assignment succeeded.
     */
    return 0x004324DF;
}


/**
 *  #issue-966
 *
 *  Makes Service Depots assign their archive target as the destination to the
 *  unit that they've finished repairing.
 *
 *  This reimplements the whole 0x00431DAB - 0x00431E27 range of the original game's code.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00431DAB, _BuildingClass_Mission_Repair_Assign_Rally_Destination_After_Repair_Complete, 0)
{
    GET(BuildingClass *, building, EBP);
    GET(TechnoClass *, techno, ESI);

    if (_BuildingClass_Mission_Repair_Assign_Unit_Destination(building, techno, false)) {
        goto success;
    } else {
        goto fail_return;
    }

    /**
     *  The unit destination was applied successfully.
     */
success:
    R->EAX(true);
    return 0x00431E27;

    /**
     *  Return from the function without assigning any location for the unit to move into.
     */
fail_return:
    return 0x00431E90;
}


/**
 *  #issue-26
 * 
 *  Adds functionality for the produce cash per-frame logic.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00429A96, _BuildingClass_AI_ProduceCash_Patch, 0)
{
    GET(BuildingClass *, this_ptr, ESI);

    /**
     *  Fetch the extension instance.
     */
    Extension::Fetch(this_ptr)->Produce_Cash_AI();

    /**
     *  Stolen bytes/code here.
     */
original_code:

    /**
     *  Animation per frame update.
     */
    this_ptr->Animation_AI();

    return 0x00429A9D;
}


/**
 *  #issue-26
 * 
 *  Grants cash bonus and starts the cash timer on building capture.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0042F67D, _BuildingClass_Captured_ProduceCash_Patch, 0)
{
    GET(BuildingClass *, this_ptr, ESI);
    GET_STACK(HouseClass *, newowner, 0x58);

    /**
     *  Fetch the extension instances.
     */
    BuildingClassExtension* ext_ptr = Extension::Fetch(this_ptr);
    BuildingTypeClassExtension* exttype_ptr = Extension::Fetch(this_ptr->Class);

    /**
     *  Is the owner a passive/neutral house? Only they can provide the capture bonus.
     */
    if (this_ptr->House->Class->IsMultiplayPassive) {

        /**
         *  Should this building produce a cash bonus on capture?
         */
        if (exttype_ptr->ProduceCashStartup > 0) {

            /**
             *  Grant the bonus to the new owner, making sure this
             *  building has not already done so if flagged
             *  as a one time bonus.
             */
            if (!ext_ptr->IsCaptureOneTimeCashGiven) {
                newowner->Refund_Money(exttype_ptr->ProduceCashStartup);
            }

            /**
             *  Is a one time bonus?
             */
            if (exttype_ptr->IsStartupCashOneTime) {
                ext_ptr->IsCaptureOneTimeCashGiven = true;
            }

            /**
             *  Start the cycle timer.
             */
            ext_ptr->ProduceCashTimer = exttype_ptr->ProduceCashDelay;
            ext_ptr->ProduceCashTimer.Start();
        }

        /**
         *  Should we reset the available budget?
         */
        if (exttype_ptr->IsResetBudgetOnCapture) {
            if (exttype_ptr->ProduceCashBudget > 0) {
                ext_ptr->CurrentProduceCashBudget = exttype_ptr->ProduceCashBudget;
            }
        }
    }

    /**
     *  Stolen bytes/code here.
     */
original_code:
    if (this_ptr->Class->IsCloakGenerator) {
        newowner->HasCloakGenerator = true;
    }

    return 0x0042F68E;
}


/**
 *  #issue-26
 * 
 *  Starts the cash timer on building placement complete (the "grand opening").
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0042E179, _BuildingClass_Grand_Opening_ProduceCash_Patch, 0)
{
    GET_STACK(bool, captured, 0x40);
    GET(BuildingClass *, this_ptr, ESI);
    BuildingClassExtension *ext_ptr;
    BuildingTypeClassExtension *exttype_ptr;

    /**
     *  Stolen bytes/code here.
     */
    if (this_ptr->HasOpened) {
        if (!captured) {
            goto function_return;
        }
        goto has_opened_else;
    }

    /**
     *  Fetch the extension instances.
     */
    ext_ptr = Extension::Fetch(this_ptr);
    exttype_ptr = Extension::Fetch(this_ptr->Class);

    /**
     *  Start the cash delay timer.
     */
    if (exttype_ptr->ProduceCashAmount != 0) {

        ext_ptr->ProduceCashTimer = exttype_ptr->ProduceCashDelay;
        ext_ptr->ProduceCashTimer.Start();

        if (exttype_ptr->ProduceCashBudget > 0) {
            ext_ptr->CurrentProduceCashBudget = exttype_ptr->ProduceCashBudget;
        }
    }

    /**
     *  Continue function flow (HasOpened == false).
     */
continue_function:
    return 0x0042E197;

    /**
     *  Function return.
     */
function_return:
    return 0x0042E9DF;

    /**
     *  Else case from "HasOpened" check.
     */
has_opened_else:
    return 0x0042E4C7;
}


/**
 *  #issue-65
 * 
 *  Gate lowering and rising sound overrides for buildings.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00433BB5, _BuildingClass_Mission_Open_Gate_Open_Sound_Patch, 0)
{
    GET(Coord *, coord, EAX);
    GET(BuildingClass *, this_ptr, ESI);

    BuildingTypeClass* buildingtype = this_ptr->Class;

    /**
     *  Fetch the default gate lowering sound.
     */
    VocType voc = Rule->GateDownSound;

    /**
     *  Fetch the extension instance.
     */
    BuildingTypeClassExtension* buildingtypeext = Extension::Fetch(buildingtype);

    /**
     *  Does this building have a custom gate lowering sound? If so, use it.
     */
    if (buildingtypeext->GateDownSound != VOC_NONE) {
        voc = buildingtypeext->GateDownSound;
    }

    /**
     *  Play the sound effect at the buildings location.
     */
    Static_Sound(voc, *coord);

    return 0x00433BC8;
}


DEFINE_HOOK(0x00433C6F, _BuildingClass_Mission_Open_Gate_Close_Sound_Patch, 0)
{
    GET(Coord *, coord, EAX);
    GET(BuildingClass *, this_ptr, ESI);

    BuildingTypeClass* buildingtype = this_ptr->Class;

    /**
     *  Fetch the default gate rising sound.
     */
    VocType voc = Rule->GateUpSound;

    /**
     *  Fetch the extension instance.
     */
    BuildingTypeClassExtension* buildingtypeext = Extension::Fetch(buildingtype);

    /**
     *  Does this building have a custom gate rising sound? If so, use it.
     */
    if (buildingtypeext->GateUpSound != VOC_NONE) {
        voc = buildingtypeext->GateUpSound;
    }

    /**
     *  Play the sound effect at the buildings location.
     */
    Static_Sound(voc, *coord);

    /**
     *  Function return (0).
     */
    return 0x00433C81;
}


/**
 *  #issue-333
 * 
 *  Fixes a division by zero crash when Rule->ShakeScreen is zero
 *  and a building dies/explodes.
 * 
 *  @author: CCHyper
 */
static void BuildingClass_Shake_Screen(BuildingClass *building)
{
    /**
     *  Fetch the extension instance.
     */
    BuildingTypeClassExtension* buildingtypeext = Extension::Fetch(
        static_cast<const BuildingTypeClass*>(building->TClass));

    /**
     *  #issue-414
     * 
     *  Can this unit shake the screen when it is destroyed?
     * 
     *  @author: CCHyper
     */
    if (buildingtypeext->IsShakeScreen) {

        /**
         *  If this building has screen shake values defined, then set the blitter
         *  offset values. GScreenClass::Blit will handle the rest for us.
         */
        if (buildingtypeext->ShakePixelXLo > 0 || buildingtypeext->ShakePixelXHi > 0
         || buildingtypeext->ShakePixelYLo > 0 || buildingtypeext->ShakePixelYHi > 0) {

            if (buildingtypeext->ShakePixelXLo > 0 || buildingtypeext->ShakePixelXHi > 0) {
                Map.ScreenX = Sim_Random_Pick(buildingtypeext->ShakePixelXLo, buildingtypeext->ShakePixelXHi);
            }
            if (buildingtypeext->ShakePixelYLo > 0 || buildingtypeext->ShakePixelYHi > 0) {
                Map.ScreenY = Sim_Random_Pick(buildingtypeext->ShakePixelYLo, buildingtypeext->ShakePixelYHi);
            }

        } else {

            /**
             *  Make sure both the screen shake factor and the buildings cost
             *  are valid before performing the division.
             */
            if (Rule->ShakeScreen > 0 && building->Class->Cost_Of() > 0) {

                int shakes = std::min(building->Class->Cost_Of() / Rule->ShakeScreen, 6);
                //int shakes = building->Class->Cost_Of() / Rule->ShakeScreen;
                if (shakes > 0) {

                    /**
                     *  #issue-414
                     * 
                     *  Restores the vertical screen shake when a strong building is destroyed.
                     * 
                     *  @author: CCHyper
                     */
                    Map.ScreenY = shakes;

                    //Shake_The_Screen(shakes);
                }

            }

        }

    }
}

DEFINE_HOOK(0x0042B250, _BuildingClass_Explode_ShakeScreen_Division_BugFix_Patch, 0)
{
    GET(BuildingClass *, this_ptr, ESI);

    BuildingClass_Shake_Screen(this_ptr);

    /**
     *  Continue execution of function.
     */
continue_function:

    /**
     *  #issue-502
     * 
     *  Fixes the bug where buildings randomly respawn in a "limbo" state
     *  when destroyed. The EDI register was used to set Strength to 0 further
     *  down in the function after we return back.
     * 
     *  @author: CCHyper
     */
    R->EDI(0);

    return 0x0042B27F;
}


/**
 *  #issue-72
 * 
 *  Fixes the bug where the wrong palette used to draw the cameo of the object
 *  being produced above a enemy spied factory building.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00428AD3, _BuildingClass_Draw_Spied_Cameo_Palette_Patch, 0)
{
    GET(TechnoClass *, factory_obj, EAX);
    GET(Point2D *, pos_xy, EDI);
    GET(Rect *, window_rect, EBP);

    const TechnoTypeClass* technotype = factory_obj->TClass;

    /**
     *  #issue-487
     * 
     *  Adds support for PCX/PNG cameo icons.
     * 
     *  @author: CCHyper
     */
    TechnoTypeClassExtension* technotypeext = Extension::Fetch(technotype);
    if (technotypeext->CameoImageSurface) {

        /**
         *  Draw the cameo pcx image.
         */
        Rect pcxrect;
        pcxrect.X = window_rect->X + pos_xy->X;
        pcxrect.Y = window_rect->Y + pos_xy->Y;
        pcxrect.Width = technotypeext->CameoImageSurface->Get_Width();
        pcxrect.Height = technotypeext->CameoImageSurface->Get_Height();

        SpriteCollection.Draw(pcxrect, *LogicalSurface, *technotypeext->CameoImageSurface);

    } else {

        const ShapeSet* cameo_shape = technotype->Get_Cameo_Data();

        /**
         *  Draw the cameo shape.
         * 
         *  Original code used NormalDrawer, which is the old Red Alert shape
         *  drawer, so we need to use CameoDrawer here for the correct palette.
         */
        Draw_Shape(*LogicalSurface, *CameoDrawer, cameo_shape, 0, *pos_xy, *window_rect, SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_NORMAL);
    }

    return 0x00428B13;
}


/**
 *  #issue-1049
 *
 *  The AI undeploys deployed Tick Tanks, Artillery and Juggernauts that get attacked by something
 *  that is out of their range. This is done by assigning MISSION_DECONSTRUCTION, which is used for both
 *  undeploying and selling.
 *
 *  The AI does not check whether the building actually has UndeploysInto= specified as something
 *  non-null, meaning if the building has UndeploysInto as null, the AI ends up selling the
 *  buildings.
 *
 *  This patch fixes the bug by denying the AI from assigning MISSION_DECONSTRUCTION
 *  when the building has UndeploysInto as null.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x0042C624, _BuildingClass_Assign_Target_No_Deconstruction_With_Null_UndeploysInto, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    if (this_ptr->Class->UndeploysInto == nullptr) {

        /**
         *  This building cannot undeploy. Exit the function.
         */
        return 0x0042C58C;
    }

    /**
     *  Stolen bytes / code.
     *  Assign MISSION_DECONSTRUCTION and exit.
     */
    this_ptr->Assign_Mission(MISSION_DECONSTRUCTION);
    this_ptr->Commence();
    return 0x0042C63A;
}


bool Is_Allowed_Harvester(BuildingClass* building, UnitClass* harvester)
{
    int dockcount = harvester->Class->Dock.Count();

    for (int i = 0; i < dockcount; i++) {
        if (harvester->Class->Dock[i] == building->Class) {
            return true;
        }
    }

    return false;
}


/**
 *  #issue-445
 *
 *  Fixes a bug where crew wouldn't come out of sold/destroyed construction yards
 *  (or buildings that undeploy, to be more specific).
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00430CC2, _BuildingClass_Mission_Deconstruction_ConYard_Survivors_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    // Unfortunately, it seems like Mission_Deconstruction does not know if the building was sold or is undeploying
    // So we're going to
    // a) check if this building doesn't ever undeploy
    // b) if it does undeploy, check that it has no archive target (the place you order them to move is set as the archive target, although rally points are also set as archive targets, so it may have side-effects)
    // c) ensure that this isn't artillery/icbm/etc.
    if ((this_ptr->ArchiveTarget == nullptr || this_ptr->Class->UndeploysInto == nullptr) && !this_ptr->Class->Is_Deployable())
    {
        // Process crew
        return 0x00430CE4;
    }

    // Don't process crew
    return 0x00430EEA;
}


/**
 *  Fixes a bug where if when undeploying a construction yard an
 *  MCV couldn't be placed, it would stay limboed.
 *
 *  @author: ZivDero
 */
static bool Unlimbo_Helper(UnitClass* unit, Coord const& coord, Dir256 dir)
{
    ScenarioInit++;
    bool result = unit->Unlimbo(coord, dir);
    ScenarioInit--;

    if (!result) {
        delete unit;
    }

    return result;
}

DEFINE_HOOK(0x00430A01, _BuildingClass_Mission_Deconstruction_ConYard_Unlimbo_Patch, 0)
{
    GET(UnitClass*, mcv, EBP);
    GET(Dir256, dir, EAX);
    LEA_STACK(Coord const*, coord, 0x40);

    if (Unlimbo_Helper(mcv, *coord, dir)) {
        return 0x00430A1A;
    } else {
        return 0x00430B37;
    }
}


/**
 *  Fixes a bug where you could receive double the amount of survivors
 *  if a building that was being sold got destroyed,
 *  or free survivors by undeploying a building that was being sold.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00430F2B, _BuildingClass_Mission_Deconstruction_Double_Survivors_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    // We've already ejected the survivors, don't eject them any more.
    this_ptr->IsSurvivorless = true;

    // Stolen instructions
    this_ptr->Status = 2;
    this_ptr->Begin_Mode(BSTATE_CONSTRUCTION);

    return 0x00430F3B;
}


/**
 *  Patch to not assign archive targets to buildings currently being sold.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0049436A, _EventClass_Execute_Archive_Selling_Patch, 0)
{
    GET(TechnoClass*, techno, EDI);
    GET(AbstractClass *, target, EAX);

    // Don't assign an archive target if currently selling
    if (techno->Mission != MISSION_DECONSTRUCTION) {
        techno->Assign_Archive_Target(target);
    }

    return 0x00494372;
}


/**
 *  Patch in BuildingClass::Captured to not count captured DontScore buildings.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0042F799, _BuildingClass_Captured_DontScore_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    auto ext = Extension::Fetch(this_ptr->Class);
    if ((Session.Type == GAME_INTERNET || Session.Type == GAME_IPX) && !ext->IsDontScore) {
        return 0x0042F7A3;
    }

    return 0x0042F7BB;
}

/**
 *  #issue-203
 *
 *  Assigns the last docked building of a spawned free unit on
 *  building placement complete (the "grand opening").
 *  This allows harvesters to know which refinery they spawned from.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x0042E5F5, _BuildingClass_Grand_Opening_Assign_FreeUnit_LastDockedBuilding_Patch, 6)
{
    GET(BuildingClass*, this_ptr, ESI);
    GET(UnitClass*, unit, EDI);

    UnitClassExtension* unitext = Extension::Fetch(unit);
    unitext->LastDockedBuilding = this_ptr;

    /**
     *  Continue the FreeUnit down-placing process.
     */
    return 0;
}


/**
 *  An enum for BuildingClass::Mission_Missile missile states
 */
enum {
    INITIAL,
    DOOR_OPENING,
    LAUNCH_UP,
    LAUNCH_DOWN,
    DONE_LAUNCH
};


/**
 *  Play SpecialAnim(Two, Three) as the MultiMissile/ChemMissile
 *  Nuke open/wait/close animations.
 *
 *  @author: ZivDero
 */
int _BuildingClass_Mission_Missile_INITIAL(BuildingClass * this_ptr)
{
    /**
     *  Play the silo opening animation.
     */
    this_ptr->Begin_Anim(BANIM_SPECIAL_ONE, this_ptr->HealthRatio <= Rule->ConditionYellow, 0);
    this_ptr->Status = DOOR_OPENING;
    return 1;
}


static bool Is_Anim_Present(BuildingClass * building, BAnimType anim)
{
    const char* name = building->HealthRatio <= Rule->ConditionYellow ? building->Class->field_580[anim].AnimDamaged : building->Class->field_580[anim].Anim;
    return std::strlen(name) != 0;
}


int _BuildingClass_Mission_Missile_DOOR_OPENING(BuildingClass* this_ptr)
{
    /**
     *  Check if the silo opening animation has finished.
     */
    if (this_ptr->Anims[BANIM_SPECIAL_TWO] != nullptr || !Is_Anim_Present(this_ptr, BANIM_SPECIAL_TWO)) {

        /**
         *  If so, signal that we're ready to fire and play the "holding open" animation.
         */
        this_ptr->Status = LAUNCH_UP;
    }
    return 1;
}


int _BuildingClass_Mission_Missile_LAUNCH_DOWN(BuildingClass* this_ptr)
{
    /**
     *  Check if the silo open animation has finished.
     */
    if (this_ptr->Anims[BANIM_SPECIAL_THREE] != nullptr || !Is_Anim_Present(this_ptr, BANIM_SPECIAL_THREE)) {

        /**
         *  If so, play the closing animation.
         */
        this_ptr->Status = DONE_LAUNCH;
    }

    return 1;
}


DEFINE_HOOK(0x00432709, _BuildingClass_Mission_Missile_INITIAL_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    int delay = _BuildingClass_Mission_Missile_INITIAL(this_ptr);
    this_ptr->IsToDisplay = true;
    R->EAX(delay);

    return 0x00432721;
}



DEFINE_HOOK(0x00432729, _BuildingClass_Mission_Missile_DOOR_OPENING_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    int delay;
    
    delay = _BuildingClass_Mission_Missile_DOOR_OPENING(this_ptr);
    R->EAX(delay);

    return 0x0043274C;
}


DEFINE_HOOK(0x00432957, _BuildingClass_Mission_Missile_LAUNCH_DOWN_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    int delay = _BuildingClass_Mission_Missile_LAUNCH_DOWN(this_ptr);
    R->EAX(delay);

    return 0x0043296C;
}


/**
 *  Implements `MissileLaunchedVoice` for missile SWs.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00432937, _BuildingClass_Mission_Missile_LAUNCH_DOWN_Voice_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);

    SuperWeaponTypeClassExtension* super_ext = Extension::Fetch(SuperWeaponTypes[this_ptr->field_298]);
    if (super_ext->VoxMissileLaunched != VOX_NONE) {
        AudioVoxClass::Speak(super_ext->VoxMissileLaunched);
    }

    return 0x00432943;
}


/**
 *  Should the factory open the roof as opposed to the door?
 *
 *  @author: ZivDero
 */
bool Should_Open_Roof(BuildingClass* building)
{
    if (building->Get_Mission() == MISSION_UNLOAD) {
        TechnoClass* radio = building->Contact_With_Whom();
        if (radio != nullptr && radio->TClass->Locomotor == __uuidof(JumpjetLocomotionClass)) {
            return true;
        }
    }
    return false;
}


/**
 *  Patches to make the factory show the roof door opening anim for JJs.
 *
 *  @author: ZivDero
 */
EXPORT_FUNC(_BuildingClass_entry_370_RoofDoorAnim_Patch1)
{
    GET(BuildingClass*, building, EBP);

    const auto btypeext = Extension::Fetch(building->Class);
    if (building->Class->DoorAnim != nullptr && !Should_Open_Roof(building) || btypeext->RoofDoorAnim != nullptr && Should_Open_Roof(building)) {
        return 0x00427CEC;
    }

    return 0x00427E27;
}


EXPORT_FUNC(_BuildingClass_entry_370_RoofDoorAnim_Patch2)
{
    GET(BuildingClass*, building, EBP);

    const BuildingTypeClassExtension* btypeext = Extension::Fetch(building->Class);

    const ShapeSet* shapefile;
    if (Should_Open_Roof(building)) {
        shapefile = btypeext->RoofDoorAnim;
    } else {
        shapefile = building->Class->DoorAnim;
    }

    R->EDX(shapefile);
    return 0x00427DFB;
}


/**
 *  Helper function that handles unlimboing a unit the naval yard has produced.
 *
 *  @author: ZivDero
 */
bool Unlimbo_Naval_Helper(BuildingClass* building, TechnoClass* techno)
{
    if (!building->In_Radio_Contact()) {
        building->Assign_Mission(MISSION_UNLOAD);
    }

    Cell unlimbo_cell = building->Center_Coord().As_Cell();

    /**
     *  If the yard has a rally point set, attempt to place the unit in that direction, next to the naval yard.
     */
    if (building->ArchiveTarget != nullptr) {
        Cell rally = building->ArchiveTarget->Center_Coord().As_Cell();
        DirType direction = Desired_Facing(Point2D(unlimbo_cell.X, unlimbo_cell.Y), Point2D(rally.X, rally.Y));
        FacingType facing = static_cast<FacingType>(direction.Get_Facing<8>());

        while (Map[unlimbo_cell].Cell_Building() == building) {
            unlimbo_cell = Adjacent_Cell(unlimbo_cell, facing);
        }
    }

    /**
     *  If we haven't got a rally point, or the cell we've selected is no good, just pick some cell near the yard that is valid.
     */
    if (building->ArchiveTarget == nullptr || Map[unlimbo_cell].Land_Type() != LAND_WATER || Map[unlimbo_cell].Cell_Techno() != nullptr || !Map.In_Radar(unlimbo_cell)) {
        unlimbo_cell = Map.Nearby_Location(building->Center_Coord().As_Cell(), techno->TClass->Speed);
    }

    /**
     *  Calculate the direction the unit should face - away from the naval yard,
     *  snapped to cardinal directions (looks nicer).
     */
    FacingType facing = static_cast<FacingType>(Desired_Facing(building->Center_Coord(), unlimbo_cell.As_Coord()).Get_Facing<8>());
    Dir256 dir = static_cast<Dir256>(facing * ((DIR_MAX + 1) / FACING_COUNT));

    /**
     *  Unlimbo the unit at that cell.
     */
    if (techno->Unlimbo(Map[unlimbo_cell].Center_Coord(), dir)) {

        /**
         *  If there's a rally point, assign the unit to move there.
         */
        if (building->ArchiveTarget != nullptr) {
            techno->Assign_Destination(building->ArchiveTarget);
            techno->Assign_Mission(MISSION_MOVE);
        }

        /**
         *  Reposition the unit. I'm not exactly sure why this is necessary,
         *  it was copied from YR.
         */
        techno->Mark(MARK_UP);
        techno->PositionCoord = Map[unlimbo_cell].Cell_Coord();
        techno->Mark(MARK_DOWN);

        /**
         *  If this is an AI, give the unit a scatter order so that the AI's ships don't clump at the naval yard.
         */
        if (!techno->House->Is_Human_Player()) {
            techno->Scatter(building->Center_Coord());
        }
        return true;
    }

    return false;
}


/**
 *  This patch handles unlimboing naval yards' production
 *  next to them as opposed to having the units "drive out".
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0042CA98, _BuildingClass_Exit_Object_Naval_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    GET(TechnoClass*, techno, EDI);

    auto type_ext = Extension::Fetch(this_ptr->Class);
    if (type_ext->IsNaval) {
        if (Unlimbo_Naval_Helper(this_ptr, techno)) {
            return 0x0042D7DF; // return 2 - successfully exited
        } else {
            return 0x0042D966; // return 0 - exit failed
        }
    } else {
        // Stolen call
        techno->Assign_Archive_Target(this_ptr->ArchiveTarget);
        return 0x0042CAA6;
    }
}


/**
 *  This patch is part of adding an extra naval queue for the AI.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0042CA35, _BuildingClass_Exit_Object_BuildNavalUnit_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    GET(TechnoClass*, techno, EDI);

    if (techno->RTTI == RTTI_UNIT) {
        TechnoTypeClassExtension* ttype_ext = Extension::Fetch(techno->TClass);
        if (ttype_ext->IsNaval) {
            HouseClassExtension* house_ext = Extension::Fetch(this_ptr->House);
            house_ext->BuildNavalUnit = UNIT_NONE;
        } else {
            this_ptr->House->BuildUnit = UNIT_NONE;
        }
    }

    R->EBP(0xFFFFFFFF);
    return 0x0042CA50;
}


/**
 *  Fixes the bug where a building detaches its light when loading a save.
 *
 *  @author: ZivDero
 */
void BuildingClassExt::_Swizzle_Light_Source()
{
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(LightSource, "LightSource");
}


/**
 *  Handle an incoming message to the building.
 *
 *  @author: 06/09/1994 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
RadioMessageType BuildingClassExt::_Receive_Message(RadioClass* from, RadioMessageType message, long& param)
{
    switch (message) {

        /*
        **  This message is received as a request to attach/load/dock with this building.
        **  Verify that this is allowed and return the appropriate response.
        */
    case RADIO_CAN_LOAD:
        TechnoClass::Receive_Message(from, message, param);
        if (!House->Is_Ally(from)) return RADIO_STATIC;
        if (Mission == MISSION_CONSTRUCTION || Mission == MISSION_DECONSTRUCTION || BState == BSTATE_CONSTRUCTION) return RADIO_NEGATIVE;
        if (!IsOn) return RADIO_NEGATIVE;

        if (Class->IsCanUnitRepair) {
            if (from->RTTI == RTTI_UNIT || from->RTTI == RTTI_AIRCRAFT) {
                if (Transmit_Message(RADIO_ON_DEPOT, from) != RADIO_ROGER) {
                    return RADIO_ROGER;
                }
            }
            return RADIO_NEGATIVE;
        }        

        if ((Class->IsArmory || Class->IsHospital) && from->RTTI == RTTI_INFANTRY) {
            if (Ammo != 0 && Mission != MISSION_REPAIR) {
                return RADIO_ROGER;
            }
            return RADIO_NEGATIVE;
        }
        if (Class->IsHelipad) {

            /*
            **  #FIX: Don't allow aircraft to dock with helipads that aren't listed as their Dock.
            */
            if (from->RTTI == RTTI_AIRCRAFT && static_cast<AircraftClass*>(from)->Class->Dock.Is_Present(Class)) {
                return RADIO_ROGER;
            }
            return RADIO_NEGATIVE;
        }

        /*
        *  Prevents units from requesting to load into the building if it's already in a radio contact with a unit.
        *  This is typically used in order to only allow one unit to set up loading into it.
        *  Originally, it was at the very top of this function to prevent any building from interacting with units while during contact,
        *  however it was moved here in order to allow Helipads, Armories, Hospitals and Service Depots to communicate with
        *  all units without limitation. Only one unit can still be accepted into being loaded at a time.
        */ 
        if (!ScenarioInit && In_Radio_Contact() && Contact_With_Whom() != from) return RADIO_NEGATIVE;

        /**
         *  #issue-129
         *
         *  Fixes a bug where a harvester is able to dock to a refinery that is not
         *  listed in the value of the harvester's Dock= key.
         *
         *  @author: Rampastring
         */
        if (Class->IsDockUnload && from->RTTI == RTTI_UNIT && static_cast<UnitClass*>(from)->Class->IsToHarvest && static_cast<UnitClass*>(from)->Class->Dock.Is_Present(Class) && (ScenarioInit || !Cargo.Is_Something_Attached())) {
            return RADIO_ROGER;
        }

        /**
         *  Replicate the fix above for weeders.
         *
         *  @author: ZivDero
         */
        if (Class->IsWeeder && from->RTTI == RTTI_UNIT && static_cast<UnitClass*>(from)->Class->IsToVeinHarvest && static_cast<UnitClass*>(from)->Class->Dock.Is_Present(Class) && (ScenarioInit || !Cargo.Is_Something_Attached())) {
            return RADIO_ROGER;
        }

        return RADIO_STATIC;

        /*
        **  This message is received when the object has attached itself to this
        **  building.
        */
    case RADIO_IM_IN:
        if (Mission == MISSION_DECONSTRUCTION) {
            return RADIO_NEGATIVE;
        }
        if (Class->IsCanUnitRepair || Class->IsCanUnitReload || Class->IsHospital || Class->IsArmory) {
            IsReadyToCommence = true;
            Assign_Mission(MISSION_REPAIR);
            from->Assign_Mission(MISSION_SLEEP);
            return RADIO_ROGER;
        }
        if (Class->IsDockUnload || Class->IsWeeder) {
            from->Assign_Mission(MISSION_UNLOAD);
            return RADIO_ROGER;
        }
        break;

        /*
        **  Docking maneuver maintenance message. See if new order should be given to the
        **  unit trying to dock.
        */
    case RADIO_DOCKING: {
        TechnoClass::Receive_Message(from, message, param);

        if (!IsOn) {
            return RADIO_NEGATIVE;
        }

        if (Class->IsCanUnitRepair) {
            RadioClass* radio = Contact_With_Whom();
            if (radio != nullptr && radio == from) {
                if (Transmit_Message(RADIO_NEED_REPAIR) == RADIO_NEGATIVE) {
                    return RADIO_NEGATIVE;
                }
            }
        }

        /*
        **  If this building is already in radio contact, then it might
        **  be able to satisfy the request to load by bumping off any
        **  preoccupying task.
        */
        if (Class->IsCanUnitReload) {
            FootClass* radio = static_cast<FootClass*>(Contact_With_Whom());
            if (radio != nullptr && radio != from) {
                if (Transmit_Message(RADIO_ON_DEPOT) == RADIO_ROGER) {
                    if (Transmit_Message(RADIO_ALL_DONE) == RADIO_ROGER) {
                        radio->Assign_Destination(&Map[radio->Nearby_Location(this)]);
                        radio->Assign_Mission(MISSION_MOVE);
                        return RADIO_ROGER;
                    }
                }
                return RADIO_NEGATIVE;
            }
        }

        if (Class->IsHospital || Class->IsArmory) {
            /**
             *  If a unit is asking to go ("dock") into the armory or hospital and they don't have ammo for it, turn it away.
             *  If the armory or hospital has a rally point, tell the unit to move to it instead.
             *  This makes the all units that were ordered to go into it regroup at the rally position.
             */ 
            if (Ammo <= 0) {
                if (ArchiveTarget != nullptr) {
                    auto techno = from->As_Techno();
                    techno->Assign_Mission(MISSION_MOVE);
                    techno->Assign_Destination(ArchiveTarget);
                }
                return RADIO_NEGATIVE;
            }

            if (Contact_With_Whom() != from) {
                if (Transmit_Message(RADIO_NEED_REPAIR) != RADIO_NEGATIVE) {
                    return RADIO_ROGER;
                }
                Transmit_Message(RADIO_RUN_AWAY);
            }
            else {
                param = reinterpret_cast<long>(&Map[Get_Coord()]);
                Transmit_Message(RADIO_MOVE_HERE, param);
            }
            return RADIO_ROGER;
        }

        /*
        **  Establish contact with the object if this building isn't already in contact
        **  with another.
        */
        if (!In_Radio_Contact()) {
            Transmit_Message(RADIO_HELLO, from);
        }

        bool needs_to_move = false;

        if (Contact_With_Whom() != nullptr) {
            if (Class->IsDockUnload || Class->IsWeeder) {
                CellClass* docking_cell = &Map[Docking_Coord()];
                AbstractClass* navcom = static_cast<FootClass*>(Contact_With_Whom())->NavCom;
                if (navcom != nullptr && docking_cell != navcom) {
                    needs_to_move = true;
                }
            }
        }

        if (Contact_With_Whom() != nullptr) {
            if (Class->IsCanUnitRepair) {
                if (Distance_To(Contact_With_Whom()) > CELL_LEPTON / 2) {
                    needs_to_move = true;
                }
            }
        }

        if (Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER || needs_to_move) {
            param = reinterpret_cast<long>(this);
            if (Class->IsDockUnload || Class->IsWeeder) {
                param = reinterpret_cast<long>(&Map[Get_Cell() + Cell(2, 1)]);

                /*
                **  Tell the harvester to move to the docking pad of the building.
                */
                if (Transmit_Message(RADIO_MOVE_HERE, param) == RADIO_YEA_NOW_WHAT) {

                    /*
                    **  Since the harvester is already there, tell it to begin the backup
                    **  procedure now. If it can't, then tell it to get outta here.
                    */
                    Transmit_Message(RADIO_TETHER);
                    if (Transmit_Message(RADIO_BACKUP_NOW, from) != RADIO_ROGER) {
                        from->Scatter(COORD_NONE, true, true);
                    }
                }
            }
            else if (Class->IsHelipad) {
                param = reinterpret_cast<long>(this);
                if (Transmit_Message(RADIO_MOVE_HERE, param) == RADIO_YEA_NOW_WHAT) {
                    Transmit_Message(RADIO_TETHER);
                }
            }
        }
        return RADIO_ROGER;
    }

        /*
        **  If a transport or harvester is requesting permission to head toward, dock
        **  and load/unload, check to make sure that this is allowed given the current
        **  state of the building.
        */
    case RADIO_ARE_REFINERY:
        if (Cargo.Is_Something_Attached() || In_Radio_Contact() || IsInLimbo || House != from->Owner_HouseClass() || (!Class->IsRefinery && !Class->IsCanUnitRepair && !Class->IsWeeder)) {
            return RADIO_NEGATIVE;
        }
        return RADIO_ROGER;

        /*
        **  Someone is telling us that it is starting construction. This should only
        **  occur if this is a construction yard and a building was just placed on
        **  the map.
        */
    case RADIO_BUILDING:
        Assign_Mission(MISSION_REPAIR);
        TechnoClass::Receive_Message(from, message, param);
        return RADIO_ROGER;

        /*
        **  Someone is telling us that they have finished construction. This should
        **  only occur if this is a construction yard and the building that was being
        **  constructed has finished. In this case, stop the construction yard
        **  animation.
        */
    case RADIO_COMPLETE:
        if (Mission != MISSION_DECONSTRUCTION) {
            Assign_Mission(MISSION_GUARD);
            if (Class->IsConstructionYard) {
                End_Anim(BANIM_PRE_PRODUCTION);
                Begin_Anim(BANIM_PRODUCTION, Get_Health_Ratio() <= Rule->ConditionYellow);
            }
        }
        TechnoClass::Receive_Message(from, message, param);
        return RADIO_ROGER;

        /*
        **  This message may occur unexpectedly if the unit in contact with this
        **  building is suddenly destroyed. Handle any cleanup necessary. For example,
        **  a construction yard should stop its construction animation in this case.
        */
    case RADIO_OVER_OUT:
        Begin_Mode(BSTATE_IDLE);
        TechnoClass::Receive_Message(from, message, param);
        return RADIO_ROGER;

        /*
        **  This message is received when an object has completely left
        **  building. Sometimes special cleanup action is required when
        **  this event occurs.
        */
    case RADIO_UNLOADED:
        if (Class->IsCanUnitRepair) {
            if (Distance_To(from) < 0x0180) {
                return RADIO_ROGER;
            }
        }
        TechnoClass::Receive_Message(from, message, param);
        if (Class->IsWeaponsFactory || Class->IsCanUnitRepair) return RADIO_RUN_AWAY;
        return RADIO_ROGER;

    case RADIO_REDRAW:
        if (Class->IsWeaponsFactory) {
            return RADIO_ROGER;
        }
        break;

    default:
        break;
    }

    /*
    **  Pass along the message to the default message handler in the radio itself.
    */
    return TechnoClass::Receive_Message(from, message, param);
}


DEFINE_HOOK(0x004381F8, _BuildingClass_Load_SwizzleLightSource_Patch, 0)
{
    GET(BuildingClassExt*, this_ptr, ESI);

    this_ptr->_Swizzle_Light_Source();

    return 0x00438202;
}


/**
 *  Prevents buildings from catching flames
 *  when rapidly switching between yellow and green
 *  damage states.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x0042B6CC, _BuildingClass_Take_Damage_Prevent_Cumulative_Flame_Spawn_Patch, 0)
{
    GET(Coord *, coord, EAX);
    GET(BuildingClass *, this_ptr, ESI);

    /**
     *  Stolen bytes / code.
     */
    Static_Sound(Rule->BlowupSound, *coord);

    /**
     *  Actual functionality of the hack.
     *  Do not spawn flames on the building if flames were spawned
     *  on it too recently.
     */
    BuildingClassExtension* buildingext = Extension::Fetch(this_ptr);
    if (Frame < buildingext->LastFlameSpawnFrame + RuleExtension->BuildingFlameSpawnBlockFrames) {
        goto past_flame_spawn;
    }

    buildingext->LastFlameSpawnFrame = Frame;

    /**
     *  Continue into applying building flames.
     */
original_code:
    R->EBX(0x7FFF);
    return 0x0042B6E4;

    /**
     *  Skip the game's code block for spawning flames on buildings.
     */
past_flame_spawn:
    return 0x0042B684;
}


BuildingClass* Find_Best_Alternative_Factory(BuildingClass* this_ptr, FootClass* exiting_object)
{
    int closest_distance = INT_MAX;
    BuildingClass* closest_match = nullptr;

    for (int i = 0; i < Buildings.Count(); i++) {
        BuildingClass* bldg = Buildings[i];

        if (bldg->House == this_ptr->House && bldg != this_ptr && bldg->Mission == MISSION_GUARD && !bldg->Factory && bldg->Class->ToBuild == this_ptr->Class->ToBuild && bldg->Is_Powered_On()) {
            // Original TS code, left here for reference. Was part of the above condition
            // if (bldg->Class != this_ptr->Class)
            //     continue;

            const TechnoTypeClass* technotype = exiting_object->TClass;

            // Check ownable, so only factories of a faction that owns the object can
            // build the object
            if ((bldg->Class->Get_Ownable() & technotype->Get_Ownable()) == 0) {
                continue;
            }

            // Check ownable, so only factories of a faction that owns the object can
            // build the object
            if ((bldg->Class->Get_Ownable() & exiting_object->TClass->Get_Ownable()) == 0) {
                continue;
            }

            // All checks have passed. Check the distance to find the closest factory to exit from.
            int distance = this_ptr->Distance_To(bldg);
            if (distance < closest_distance) {
                closest_distance = distance;
                closest_match = bldg;
            }
        }
    }

    return closest_match;
}


/**
 *  Replaces the loop starting from 0x0042CAB9 to improve the alternative
 *  war factory selection logic when a war factory is busy in 3 ways:
 * 
 *  1) The object can now exit from factory buildings of a different type
 *     than what the object was originally produced from.
 *  
 *  2) The above takes speed type into account when finding building to exit from,
 *     preventing ships from exiting from land-based factories and vice-versa.
 * 
 *  3) The logic prefers finding the closest rather than the "first" alternative
 *  factory building.
 * 
 *  @author: Rampastring
 */
DEFINE_HOOK(0x0042CAB9, _BuildingClass_Exit_Object_Factory_Busy_Customized_Alternate_Factory_Seeking_Logic, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    GET(FootClass*, exiting_object, EDI);

    BuildingClass* best_alternative_building = Find_Best_Alternative_Factory(this_ptr, exiting_object);
    if (best_alternative_building != nullptr) {

        /**
         *  Exit from the factory we found.
         */
        R->EBP(best_alternative_building);
        return 0x0042CB28;
    }

    /**
     *  We could not find an alternative factory to exit from,
     *  exit the function and return 1.
     */
    return 0x0042CB16;
}


/**
 *  Allows AI to repair Base Nodes by enabling the IsToRepair flag on those buildings.
 *  Only applies in campaign, and only if the AIRepairBaseNodes under [AI] is set to yes/true. 
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0042A3D1, _BuildingClass_Unlimbo_AI_Repair_Base_Nodes, 5)
{
    GET(BuildingClass*, this_ptr, ESI);

    // Ignore pre-placed buildings
    if (ScenarioInit) {
        return 0;
    }

    if (Session.Type == GAME_NORMAL && !this_ptr->House->Is_Human_Player() && RuleExtension->IsAIRepairBaseNodes) {
        this_ptr->IsToRepair = true;
    }
    
    return 0;
}


/*
*  Patches a portion of BuildingClass::Captured where laser fence connections are updated (or more correctly - removed)
*  At this point, the House of the captured building is NOT updated yet - and belongs to the original owner of this building.
*  This allows us to add an additional check where if a sensor array is captured, the original owner loses that sensor array's sensing capabilities.
* 
*  @author: JoyfulShush
*/
DEFINE_HOOK(0x0042F749, _BuildingClass_Captured_Disable_Sensors, 6)
{
    GET(BuildingClass*, this_ptr, ESI);

    auto building_type = this_ptr->Class;    

    if (building_type->IsSensorArray) {
        this_ptr->Disable_Sensor_Array();
    }

    return 0;
}


/*
 *  Patches a portion of BuildingClass::Captured where a building that gets captured lets players reveal shroud around it with Look().
 *  At this point, the House of the captured building is already updated - and belongs to the new owner of this building.
 *  This allows us to add an additional check where if a sensor array is captured, the new owner gains that sensor array's sensing capabilities.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0042FB9F, _BuildingClass_Captured_Enable_Sensors, 6)
{
    GET(BuildingClass*, this_ptr, ESI);

    auto building_type = this_ptr->Class;

    if (building_type->IsSensorArray) {
        this_ptr->Enable_Sensor_Array();
    }

    return 0;
}


/*
 *  Patches the part of BuildingClass::Repair_AI where a building can no longer be repaired due to a house having insufficient funds.
 *  Typically, it would stop repairs altoghether. However, if the rule for pausing repairs is enabled, then it skips that.
 *
 *  @author: JoyfulShush, Rampastring
 */
DEFINE_HOOK(0x00435A38, _BuildingClass_Repair_AI_Pause_Repairs_Patch, 7)
{
    GET(BuildingClass*, this_ptr, ESI);

    if (this_ptr->House->Is_Human_Player())
    {
        HouseClassExtension* houseext = Extension::Fetch(this_ptr->House);

        if (houseext->IsPauseRepairs) {
            return 0x00435A3F;
        }
    }

    return 0;
}


/*
 *  Reimplements part of BuildingClass::Draw_Overlays where a building determines the wrench frame to use when drawing during repairs.
 *  When the building's repairs are paused, the game draws a specific wrench frame to signal that the repairs are paused.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004288E1, _BuildingClass_Draw_Overlays_Wrench_Shape_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    GET(int, frame, ECX);
    GET(Point2D*, point, EDI);
    GET(Rect*, rect, EBP);

    HouseClassExtension* houseext = Extension::Fetch(this_ptr->House);

    int draw_frame;
    if (this_ptr->House->Is_Human_Player() && houseext->IsPauseRepairs && this_ptr->House->Available_Money() < this_ptr->Class->Repair_Step()) {
        draw_frame = RuleExtension->PausedRepairsFrame;
    } else {
        draw_frame = 6 * (Frame % frame) / (frame - 1);
    }

    Draw_Shape(*LogicalSurface,
        *MouseDrawer,
        (ShapeSet const*)BuildingClass::WrenchShape, 
        draw_frame,
        *point,
        *rect,
        ShapeFlags_Type(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA)        
    );

    return 0x00428925;
}


/*
 *  Patches BuildingClass::Limbo, inside the IsWall check that turns a wall into an overlay.
 *  Replacing the value of Sight to take into account all sight range modifications for this techno.
 *  Particularly relevant to veterancy granting sight range bonuses.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0042A0B0, Building_Class_Unlimbo_Sight_Range_Patch, 0)
{
    GET(Coord*, coord, EBP);
    GET(BuildingClass*, this_ptr, ESI);

    auto building_class_ext = Extension::Fetch(this_ptr);    

    Map.Sight_From(*coord, building_class_ext->Get_Sight_Range(), this_ptr->House);

    return 0x0042A0D7;
}


/**
 *  Main function for patching the hooks.
 */
void BuildingClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    BuildingClassExtension_Init();

    Patch_Jump(0x00439D10, &BuildingClassExt::_Can_Have_Rally_Point);
    Patch_Jump(0x0042D9A0, &BuildingClassExt::_Update_Buildables);
    Patch_Jump(0x00433FB0, &BuildingClassExt::_Crew_Type);
    Patch_Jump(0x00435DA0, &BuildingClassExt::_How_Many_Survivors);
    //Patch_Jump(0x00429220, &BuildingClassExt::_Shape_Number);
    Patch_Jump(0x0042E53C, 0x0042E56F); // Jump a check for the PurchasePrice of a building for spawning its FreeUnit in Grand_Opening
    Patch_Jump(0x00436410, &BuildingClassExt::_Detach_Anim);
    Patch_Jump(0x004275B0, &BuildingClassExt::_Draw_It);
    //Patch_Jump(0x00427CD8, &_BuildingClass_entry_370_RoofDoorAnim_Patch1);
    //Patch_Jump(0x00427DF5, &_BuildingClass_entry_370_RoofDoorAnim_Patch2);
    Patch_Jump(0x00434000, &BuildingClassExt::_Detach_All);
    Patch_Jump(0x0042F590, &BuildingClassExt::_Toggle_Primary);
    Patch_Jump(0x0042C340, &BuildingClassExt::_Assign_Rally_Point);
    Patch_Jump(0x00434FE0, &BuildingClassExt::_Factory_AI);
    Patch_Jump(0x0042EBD0, static_cast<ActionType(BuildingClassExt::*)(ObjectClass const*, bool)>(&BuildingClassExt::_What_Action));
    Patch_Jump(0x0042EED0, static_cast<ActionType(BuildingClassExt::*)(const Cell&, bool, bool) const>(&BuildingClassExt::_What_Action));
    Patch_Jump(0x0043AF60, &BuildingClassExt::_Fetch_Super_Weapon);
    Patch_Jump(0x0043AFC0, &BuildingClassExt::_Fetch_Super_Weapon2);
    Patch_Jump(0x004268C0, &BuildingClassExt::_Receive_Message);
}
