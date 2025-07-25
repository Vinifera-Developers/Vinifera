/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended BuildingClass.
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
#include "buildingext_hooks.h"

#include <algorithm>
#include "buildingext_init.h"
#include "buildingext.h"
#include "buildingtypeext.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "vinifera_util.h"
#include "building.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "unit.h"
#include "unitext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
#include "anim.h"
#include "animext.h"
#include "house.h"
#include "housetype.h"
#include "map.h"
#include "mouse.h"
#include "cell.h"
#include "bsurface.h"
#include "dsurface.h"
#include "convert.h"
#include "drawshape.h"
#include "infantrytype.h"
#include "unit.h"
#include "unittype.h"
#include "rules.h"
#include "voc.h"
#include "iomap.h"
#include "spritecollection.h"
#include "extension.h"
#include "sideext.h"
#include "fatal.h"
#include "asserthandler.h"
#include "bullettype.h"
#include "coord.h"
#include "debughandler.h"
#include "event.h"
#include "factory.h"
#include "session.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "houseext.h"
#include "jumpjetlocomotion.h"
#include "rulesext.h"
#include "super.h"
#include "supertypeext.h"
#include "vox.h"
#include "tactical.h"
#include "vinifera_saveload.h"
#include "weapontype.h"


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
    if (this->Class->CanUnitRepair)
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
    if (House == PlayerPtr && !IsInLimbo && IsDiscoveredByPlayer && IsPowerOn)
    {
        switch (Class->ToBuild)
        {
        case RTTI_AIRCRAFTTYPE:
            for (int i = 0; i < AircraftTypes.Count(); i++)
            {
                if (PlayerPtr->Can_Build(AircraftTypes[i], false, true) && AircraftTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)
                {
                    Map.Add(RTTI_AIRCRAFTTYPE, i);
                }
            }
            break;

        case RTTI_BUILDINGTYPE:
            for (int i = 0; i < BuildingTypes.Count(); i++)
            {
                if (PlayerPtr->Can_Build(BuildingTypes[i], false, true) && BuildingTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)
                {
                    Map.Add(RTTI_BUILDINGTYPE, i);
                }
            }
            break;

        case RTTI_INFANTRYTYPE:
            for (int i = 0; i < InfantryTypes.Count(); i++)
            {
                if (PlayerPtr->Can_Build(InfantryTypes[i], false, true) && InfantryTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)
                {
                    Map.Add(RTTI_INFANTRYTYPE, i);
                }
            }
            break;

        case RTTI_UNITTYPE:
            for (int i = 0; i < UnitTypes.Count(); i++)
            {
                if (PlayerPtr->Can_Build(UnitTypes[i], false, true) && UnitTypes[i]->Who_Can_Build_Me(true, false, RuleExtension->IsRecheckPrerequisites, PlayerPtr) != nullptr)
                {
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
                    if (Class->CanUnitRepair) {
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
        if (radio != nullptr && radio->Techno_Type_Class()->Locomotor == __uuidof(JumpjetLocomotionClass)) {
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
        if (shapenum < Class->GateStages / 2) {
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
            bool altdown = WWKeyboard->Down(Options.KeyForceMove1) || WWKeyboard->Down(Options.KeyForceMove2);
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
                        if (Map[cell].Passability != PASSABLE_OK && !(Extension::Fetch(Class)->IsNaval && Map[cell].Passability == PASSABLE_WATER)) {
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
                if (Map[cell].Passability != PASSABLE_OK && !(Extension::Fetch(Class)->IsNaval && Map[cell].Passability == PASSABLE_WATER)) {
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
DECLARE_PATCH(_BuildingClass_Draw_Overlays_Fetch_Factory_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);

    static FactoryClass* factory;
    static BuildingTypeClassExtension const* type_ext;
    type_ext = Extension::Fetch(this_ptr->Class);
    factory = Extension::Fetch(this_ptr->House)->Fetch_Factory(this_ptr->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);

    _asm mov eax, factory
    JMP_REG(ecx, 0x00428AC4);
}


/**
 *  Replaces an inlined call of Detach_Anim with a direct call.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingClass_Detach_Detach_Anim_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    GET_REGISTER_STATIC(AnimClass*, anim, ecx);

    this_ptr->Detach_Anim(anim);

    JMP(0x00433F84);
}


/**
 *  #issue-204
 * 
 *  Implements ReloadRate for AircraftTypes, allowing each aircraft to have
 *  its own independent ammo reloading rate when docked with a helipad.
 * 
 *  @author: CCHyper
 */
static int Building_Radio_Reload_Rate(BuildingClass *this_ptr)
{
    AircraftClass *radio = reinterpret_cast<AircraftClass *>(this_ptr->Contact_With_Whom());
    AircraftTypeClassExtension *radio_class_ext = Extension::Fetch(radio->Class);

    return radio_class_ext->ReloadRate * TICKS_PER_MINUTE;
}


DECLARE_PATCH(_BuildingClass_Mission_Repair_ReloadRate_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, ebp);
    static int time;

    time = Building_Radio_Reload_Rate(this_ptr);

    _asm { mov eax, time }
    JMP_REG(edi, 0x0043260F);
}


/**
 *  #issue-966
 *
 *  Assigns destination to a unit when it's leaving a service depot.
 *
 *  @author: Rampastring
 */
bool _BuildingClass_Mission_Repair_Assign_Unit_Destination(BuildingClass *building, TechnoClass *techno, bool clear_archive) {
    Cell exitcell;
    CellClass* cellptr;

    AbstractClass * target = nullptr;

    if (building->ArchiveTarget != nullptr)
    {
        bool is_object = Target_As_Object(building->ArchiveTarget) != nullptr;

        if (Target_Legal(building->ArchiveTarget, is_object))
            target = building->ArchiveTarget;
    }

    if (target == nullptr)
    {
        /**
         *  Stolen bytes/code.
         *  Reimplements original game behaviour.
         */
        exitcell = building->Find_Exit_Cell(reinterpret_cast<TechnoClass*>(building->Radio));

        if (exitcell.X == 0 && exitcell.Y == 0) {
            /**
             *  Failed to find valid exit cell.
             */
            return false;
        }

        cellptr = &Map[exitcell];
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
DECLARE_PATCH(_BuildingClass_Mission_Repair_Assign_Rally_Destination_When_No_Repair_Needed)
{
    GET_REGISTER_STATIC(BuildingClass*, building, ebp);
    GET_REGISTER_STATIC(TechnoClass*, techno, esi);

    _BuildingClass_Mission_Repair_Assign_Unit_Destination(building, techno, true);

    /**
     *  Set mission delay and return, regardless of whether the
     *  destination assignment succeeded.
     */
    JMP_REG(ecx, 0x004324DF);
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
DECLARE_PATCH(_BuildingClass_Mission_Repair_Assign_Rally_Destination_After_Repair_Complete)
{
    GET_REGISTER_STATIC(BuildingClass *, building, ebp);
    GET_REGISTER_STATIC(TechnoClass *, techno, esi);

    if (_BuildingClass_Mission_Repair_Assign_Unit_Destination(building, techno, false)) {
        goto success;
    } else {
        goto fail_return;
    }

    /**
     *  The unit destination was applied successfully.
     */
success:
    _asm { mov eax, 1 }
    JMP_REG(edi, 0x00431E27);

    /**
     *  Return from the function without assigning any location for the unit to move into.
     */
fail_return:
    JMP(0x00431E90);
}


/**
 *  #issue-26
 * 
 *  Adds functionality for the produce cash per-frame logic.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingClass_AI_ProduceCash_Patch)
{
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    static BuildingClassExtension *ext_ptr;

    /**
     *  Fetch the extension instance.
     */
    ext_ptr = Extension::Fetch(this_ptr);

    ext_ptr->Produce_Cash_AI();

    /**
     *  Stolen bytes/code here.
     */
original_code:

    /**
     *  Animation per frame update.
     */
    this_ptr->Animation_AI();

    JMP(0x00429A9D);
}


/**
 *  #issue-26
 * 
 *  Grants cash bonus and starts the cash timer on building capture.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingClass_Captured_ProduceCash_Patch)
{
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    GET_STACK_STATIC(HouseClass *, newowner, esp, 0x58);
    static BuildingClassExtension *ext_ptr;
    static BuildingTypeClassExtension *exttype_ptr;

    /**
     *  Fetch the extension instances.
     */
    ext_ptr = Extension::Fetch(this_ptr);
    exttype_ptr = Extension::Fetch(this_ptr->Class);

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

    JMP(0x0042F68E);
}


/**
 *  #issue-26
 * 
 *  Starts the cash timer on building placement complete (the "grand opening").
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingClass_Grand_Opening_ProduceCash_Patch)
{
    GET_STACK_STATIC8(bool, captured, esp, 0x40);
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    static BuildingClassExtension *ext_ptr;
    static BuildingTypeClassExtension *exttype_ptr;

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
    JMP(0x0042E197);

    /**
     *  Function return.
     */
function_return:
    JMP(0x0042E9DF);

    /**
     *  Else case from "HasOpened" check.
     */
has_opened_else:
    JMP(0x0042E4C7);
}


/**
 *  #issue-65
 * 
 *  Gate lowering and rising sound overrides for buildings.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingClass_Mission_Open_Gate_Open_Sound_Patch)
{
    GET_REGISTER_STATIC(Coord *, coord, eax);
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    static BuildingTypeClass *buildingtype;
    static BuildingTypeClassExtension *buildingtypeext;
    static VocType voc;

    buildingtype = this_ptr->Class;

    /**
     *  Fetch the default gate lowering sound.
     */
    voc = Rule->GateDownSound;

    /**
     *  Fetch the extension instance.
     */
    buildingtypeext = Extension::Fetch(buildingtype);

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

    JMP_REG(edx, 0x00433BC8);
}

DECLARE_PATCH(_BuildingClass_Mission_Open_Gate_Close_Sound_Patch)
{
    GET_REGISTER_STATIC(Coord *, coord, eax);
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    static BuildingTypeClass *buildingtype;
    static BuildingTypeClassExtension *buildingtypeext;
    static VocType voc;

    buildingtype = this_ptr->Class;

    /**
     *  Fetch the default gate rising sound.
     */
    voc = Rule->GateUpSound;

    /**
     *  Fetch the extension instance.
     */
    buildingtypeext = Extension::Fetch(buildingtype);

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
    JMP(0x00433C81);
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
    BuildingTypeClassExtension *buildingtypeext;

    /**
     *  Fetch the extension instance.
     */
    buildingtypeext = Extension::Fetch(static_cast<const BuildingTypeClass*>(building->TClass));

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
         || (buildingtypeext->ShakePixelYLo > 0 || buildingtypeext->ShakePixelYHi > 0)) {

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

DECLARE_PATCH(_BuildingClass_Explode_ShakeScreen_Division_BugFix_Patch)
{
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    static int shakes;

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
    _asm { xor edi, edi }

    JMP_REG(edx, 0x0042B27F);
}


/**
 *  #issue-72
 * 
 *  Fixes the bug where the wrong palette used to draw the cameo of the object
 *  being produced above a enemy spied factory building.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingClass_Draw_Spied_Cameo_Palette_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, factory_obj, eax);
    GET_REGISTER_STATIC(Point2D *, pos_xy, edi);
    GET_REGISTER_STATIC(Rect *, window_rect, ebp);
    static const TechnoTypeClass *technotype;
    static TechnoTypeClassExtension *technotypeext;
    static const ShapeSet *cameo_shape;
    static Surface *pcx_image;
    static Rect pcxrect;

    technotype = factory_obj->TClass;

    /**
     *  #issue-487
     * 
     *  Adds support for PCX/PNG cameo icons.
     * 
     *  @author: CCHyper
     */
    technotypeext = Extension::Fetch(technotype);
    if (technotypeext->CameoImageSurface) {

        /**
         *  Draw the cameo pcx image.
         */
        pcxrect.X = window_rect->X + pos_xy->X;
        pcxrect.Y = window_rect->Y + pos_xy->Y;
        pcxrect.Width = technotypeext->CameoImageSurface->Get_Width();
        pcxrect.Height = technotypeext->CameoImageSurface->Get_Height();

        SpriteCollection.Draw(pcxrect, *LogicSurface, *technotypeext->CameoImageSurface);

    } else {

        cameo_shape = technotype->Get_Cameo_Data();

        /**
         *  Draw the cameo shape.
         * 
         *  Original code used NormalDrawer, which is the old Red Alert shape
         *  drawer, so we need to use CameoDrawer here for the correct palette.
         */
        Draw_Shape(*LogicSurface, *CameoDrawer, cameo_shape, 0, *pos_xy, *window_rect, SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_NORMAL);
    }

    JMP(0x00428B13);
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
DECLARE_PATCH(_BuildingClass_Assign_Target_No_Deconstruction_With_Null_UndeploysInto)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    static BuildingTypeClass* buildingtype;

    if (this_ptr->Class->UndeploysInto == nullptr) {

        /**
         *  This building cannot undeploy. Exit the function.
         */
        JMP(0x0042C58C);
    }

    /**
     *  Stolen bytes / code.
     *  Assign MISSION_DECONSTRUCTION and exit.
     */
    this_ptr->Assign_Mission(MISSION_DECONSTRUCTION);
    this_ptr->Commence();
    JMP(0x0042C63A);
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
 *  #issue-129
 *
 *  Fixes a bug where a harvester is able to dock to a refinery that is not
 *  listed in the value of the harvester's Dock= key.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_BuildingClass_Receive_Message_Only_Allow_Dockable_Harvester_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    GET_REGISTER_STATIC(UnitClass*, unit, edi);

    if (!Is_Allowed_Harvester(this_ptr, unit)) {
        JMP(0x0042696C); // Return RADIO_NEGATIVE
    }

    // Stolen bytes / code
    if (!this_ptr->Cargo.Is_Something_Attached()) {
        JMP(0x0042707B); // Return RADIO_ROGER
    }

    // Continue function execution beyond harvester-to-dock check
    JMP(0x00426A8C);
}


/**
 *  #issue-445
 *
 *  Fixes a bug where crew wouldn't come out of sold/destroyed construction yards
 *  (or buildings that undeploy, to be more specific).
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingClass_Mission_Deconstruction_ConYard_Survivors_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);

    // Unfortunately, it seems like Mission_Deconstruction does not know if the building was sold or is undeploying
    // So we're going to
    // a) check if this building doesn't ever undeploy
    // b) if it does undeploy, check that it has no archive target (the place you order them to move is set as the archive target, although rally points are also set as archive targets, so it may have side-effects)
    // c) ensure that this isn't artillery/icbm/etc.
    if ((this_ptr->ArchiveTarget == nullptr || this_ptr->Class->UndeploysInto == nullptr) && !this_ptr->Class->Is_Deployable())
    {
        // Process crew
        JMP(0x00430CE4);
    }

    // Don't process crew
    JMP(0x00430EEA);
}


/**
 *  Fixes a bug where if when undeploying a construction yard an
 *  MCV couldn't be placed, it would stay limboed.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingClass_Mission_Deconstruction_ConYard_Unlimbo_Patch)
{
    GET_REGISTER_STATIC(UnitClass*, mcv, ebp);
    LEA_STACK_STATIC(Coord*, coords, esp, 0x40);
    GET_REGISTER_STATIC(Dir256, dir, eax);

    static bool result;

    ScenarioInit++;
    result = mcv->Unlimbo(*coords, dir);
    ScenarioInit--;

    if (result)
    {
        JMP(0x00430A1A);
    }
    else
    {
        delete mcv;
        JMP(0x00430B37);
    }
}


/**
 *  Fixes a bug where you could receive double the amount of survivors
 *  if a building that was being sold got destroyed,
 *  or free survivors by undeploying a building that was being sold.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingClass_Mission_Deconstruction_Double_Survivors_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);

    // We've already ejected the survivors, don't eject them any more.
    this_ptr->IsSurvivorless = true;

    // Stolen instructions
    this_ptr->Status = 2;
    this_ptr->Begin_Mode(BSTATE_CONSTRUCTION);

    JMP(0x00430F3B);
}


/**
 *  Patch to not assign archive targets to buildings currently being sold.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_EventClass_Execute_Archive_Selling_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, techno, edi);
    GET_REGISTER_STATIC(AbstractClass *, target, eax);

    // Don't assign an archive target if currently selling
    if (techno->Mission != MISSION_DECONSTRUCTION) {
        techno->Assign_Archive_Target(target);
    }

    JMP(0x00494372);
}


/**
 *  Patch in BuildingClass::Captured to not count captured DontScore buildings.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingClass_Captured_DontScore_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    static BuildingTypeClassExtension* ext;

    ext = Extension::Fetch(this_ptr->Class);
    if ((Session.Type == GAME_INTERNET || Session.Type == GAME_IPX) && !ext->IsDontScore)
    {
        JMP(0x0042F7A3);
    }

    JMP(0x0042F7BB);
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
DECLARE_PATCH(_BuildingClass_Grand_Opening_Assign_FreeUnit_LastDockedBuilding_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    GET_REGISTER_STATIC(UnitClass*, unit, edi);
    static UnitClassExtension* unitext;

    unitext = Extension::Fetch(unit);
    unitext->LastDockedBuilding = this_ptr;

    /**
     *  Continue the FreeUnit down-placing process.
     */
    _asm { movsx   eax, bp }
    _asm { movsx   ecx, bx }
    JMP_REG(edx, 0x0042E5FB);
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


DECLARE_PATCH(_BuildingClass_Mission_Missile_INITIAL_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    static int delay;

    delay = _BuildingClass_Mission_Missile_INITIAL(this_ptr);

    this_ptr->IsToDisplay = true;

    _asm mov eax, delay
    JMP_REG(edi, 0x00432721);
}



DECLARE_PATCH(_BuildingClass_Mission_Missile_DOOR_OPENING_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    static int delay;
    
    delay = _BuildingClass_Mission_Missile_DOOR_OPENING(this_ptr);

    _asm mov eax, delay
    JMP_REG(edi, 0x0043274C);
}


DECLARE_PATCH(_BuildingClass_Mission_Missile_LAUNCH_DOWN_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    static int delay;
    
    delay = _BuildingClass_Mission_Missile_LAUNCH_DOWN(this_ptr);
    JMP_REG(edi, 0x0043296C);
}


/**
 *  Implements `MissileLaunchedVoice` for missile SWs.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingClass_Mission_Missile_LAUNCH_DOWN_Voice_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    static SuperWeaponTypeClassExtension* super_ext;

    super_ext = Extension::Fetch(SuperWeaponTypes[this_ptr->field_298]);
    if (super_ext->VoxMissileLaunched != VOX_NONE) {
        Speak(super_ext->VoxMissileLaunched);
    }

    JMP(0x00432943);
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
        if (radio != nullptr && radio->Techno_Type_Class()->Locomotor == __uuidof(JumpjetLocomotionClass)) {
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
DECLARE_PATCH(_BuildingClass_entry_370_RoofDoorAnim_Patch1)
{
    GET_REGISTER_STATIC(BuildingClass*, building, ebp);
    const BuildingTypeClassExtension* btypeext;

    btypeext = Extension::Fetch(building->Class);

    if (building->Class->DoorAnim != nullptr && !Should_Open_Roof(building) || btypeext->RoofDoorAnim != nullptr && Should_Open_Roof(building)) {
        JMP(0x00427CEC);
    }

    JMP(0x00427E27);
}


DECLARE_PATCH(_BuildingClass_entry_370_RoofDoorAnim_Patch2)
{
    GET_REGISTER_STATIC(BuildingClass*, building, ebp);
    const BuildingTypeClassExtension* btypeext;
    const ShapeSet* shapefile;

    _asm pushad

    btypeext = Extension::Fetch(building->Class);

    if (Should_Open_Roof(building)) {
        shapefile = btypeext->RoofDoorAnim;
    } else {
        shapefile = building->Class->DoorAnim;
    }

    _asm popad
    _asm mov edx, shapefile

    JMP_REG(ecx, 0x00427DFB);
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
     *  Unlimbo the unit at that cell.
     */
    if (techno->Unlimbo(Map[unlimbo_cell].Center_Coord())) {

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
DECLARE_PATCH(_BuildingClass_Exit_Object_Naval_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    GET_REGISTER_STATIC(TechnoClass*, techno, edi);
    static BuildingTypeClassExtension* type_ext;

    type_ext = Extension::Fetch(this_ptr->Class);
    if (type_ext->IsNaval) {
        if (Unlimbo_Naval_Helper(this_ptr, techno)) {
            JMP(0x0042D7DF); // return 2 - successfully exited
        } else {
            JMP(0x0042D966); // return 0 - exit failed
        }
    } else {
        // Stolen call
        techno->Assign_Archive_Target(this_ptr->ArchiveTarget);
        JMP(0x0042CAA6);
    }
}


/**
 *  This patch is part of adding an extra naval queue for the AI.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingClass_Exit_Object_BuildNavalUnit_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    GET_REGISTER_STATIC(TechnoClass*, techno, edi);

    static TechnoTypeClassExtension* ttype_ext;
    static HouseClassExtension* house_ext;

    if (techno->RTTI == RTTI_UNIT) {
        ttype_ext = Extension::Fetch(techno->TClass);
        if (ttype_ext->IsNaval) {
            house_ext = Extension::Fetch(this_ptr->House);
            house_ext->BuildNavalUnit = UNIT_NONE;
        } else {
            this_ptr->House->BuildUnit = UNIT_NONE;
        }
    }

    _asm mov ebp, 0xFFFFFFFF

    JMP(0x0042CA50);
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

DECLARE_PATCH(_BuildingClass_Load_SwizzleLightSource_Patch)
{
    GET_REGISTER_STATIC(BuildingClassExt*, this_ptr, esi);

    this_ptr->_Swizzle_Light_Source();

    JMP(0x00438202);
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

    Patch_Jump(0x00428AD3, &_BuildingClass_Draw_Spied_Cameo_Palette_Patch);
    Patch_Jump(0x0042B250, &_BuildingClass_Explode_ShakeScreen_Division_BugFix_Patch);
    Patch_Jump(0x00433BB5, &_BuildingClass_Mission_Open_Gate_Open_Sound_Patch);
    Patch_Jump(0x00433C6F, &_BuildingClass_Mission_Open_Gate_Close_Sound_Patch);
    Patch_Jump(0x00429A96, &_BuildingClass_AI_ProduceCash_Patch);
    Patch_Jump(0x0042F67D, &_BuildingClass_Captured_ProduceCash_Patch);
    Patch_Jump(0x0042E179, &_BuildingClass_Grand_Opening_ProduceCash_Patch);
    Patch_Jump(0x004325F9, &_BuildingClass_Mission_Repair_ReloadRate_Patch);
    Patch_Jump(0x0043266C, &_BuildingClass_Mission_Repair_ReloadRate_Patch);
    Patch_Jump(0x00432184, &_BuildingClass_Mission_Repair_Assign_Rally_Destination_When_No_Repair_Needed);
    Patch_Jump(0x00431DAB, &_BuildingClass_Mission_Repair_Assign_Rally_Destination_After_Repair_Complete);
    Patch_Jump(0x0042C624, &_BuildingClass_Assign_Target_No_Deconstruction_With_Null_UndeploysInto);
    Patch_Jump(0x00426A7E, &_BuildingClass_Receive_Message_Only_Allow_Dockable_Harvester_Patch);
    Patch_Jump(0x00439D10, &BuildingClassExt::_Can_Have_Rally_Point);
    Patch_Jump(0x0042D9A0, &BuildingClassExt::_Update_Buildables);
    Patch_Jump(0x00433FB0, &BuildingClassExt::_Crew_Type);
    Patch_Jump(0x00435DA0, &BuildingClassExt::_How_Many_Survivors);
    Patch_Jump(0x00430CC2, &_BuildingClass_Mission_Deconstruction_ConYard_Survivors_Patch);
    Patch_Jump(0x00430A01, &_BuildingClass_Mission_Deconstruction_ConYard_Unlimbo_Patch);
    Patch_Jump(0x00430F2B, &_BuildingClass_Mission_Deconstruction_Double_Survivors_Patch);
    Patch_Jump(0x0049436A, &_EventClass_Execute_Archive_Selling_Patch);
    Patch_Jump(0x0042F799, &_BuildingClass_Captured_DontScore_Patch);
    Patch_Jump(0x0042E5F5, &_BuildingClass_Grand_Opening_Assign_FreeUnit_LastDockedBuilding_Patch);
    //Patch_Jump(0x00429220, &BuildingClassExt::_Shape_Number);
    Patch_Jump(0x0042E53C, 0x0042E56F); // Jump a check for the PurchasePrice of a building for spawning its FreeUnit in Grand_Opening
    Patch_Jump(0x00436410, &BuildingClassExt::_Detach_Anim);
    Patch_Jump(0x004275B0, &BuildingClassExt::_Draw_It);
    Patch_Jump(0x00433F1D, &_BuildingClass_Detach_Detach_Anim_Patch);
    Patch_Jump(0x00432709, &_BuildingClass_Mission_Missile_INITIAL_Patch);
    Patch_Jump(0x00432729, &_BuildingClass_Mission_Missile_DOOR_OPENING_Patch);
    Patch_Jump(0x00432957, &_BuildingClass_Mission_Missile_LAUNCH_DOWN_Patch);
    Patch_Jump(0x00432937, &_BuildingClass_Mission_Missile_LAUNCH_DOWN_Voice_Patch);
    //Patch_Jump(0x00427CD8, &_BuildingClass_entry_370_RoofDoorAnim_Patch1);
    //Patch_Jump(0x00427DF5, &_BuildingClass_entry_370_RoofDoorAnim_Patch2);
    Patch_Jump(0x00428AA4, &_BuildingClass_Draw_Overlays_Fetch_Factory_Patch);
    Patch_Jump(0x00434000, &BuildingClassExt::_Detach_All);
    Patch_Jump(0x0042F590, &BuildingClassExt::_Toggle_Primary);
    Patch_Jump(0x0042C340, &BuildingClassExt::_Assign_Rally_Point);
    Patch_Jump(0x00434FE0, &BuildingClassExt::_Factory_AI);
    Patch_Jump(0x0042EBD0, static_cast<ActionType(BuildingClassExt::*)(ObjectClass const*, bool)>(&BuildingClassExt::_What_Action));
    Patch_Jump(0x0042EED0, static_cast<ActionType(BuildingClassExt::*)(const Cell&, bool, bool) const>(&BuildingClassExt::_What_Action));
    Patch_Jump(0x0042CA98, &_BuildingClass_Exit_Object_Naval_Patch);
    Patch_Jump(0x0042CA35, &_BuildingClass_Exit_Object_BuildNavalUnit_Patch);
    Patch_Jump(0x0043AF60, &BuildingClassExt::_Fetch_Super_Weapon);
    Patch_Jump(0x0043AFC0, &BuildingClassExt::_Fetch_Super_Weapon2);
    Patch_Jump(0x004381F8, &_BuildingClass_Load_SwizzleLightSource_Patch);
}
