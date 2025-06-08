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
#include "debughandler.h"
#include "session.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "jumpjetlocomotion.h"
#include "rulesext.h"
#include "super.h"
#include "supertypeext.h"
#include "vox.h"
#include "tactical.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class BuildingClassExt : public BuildingClass
{
public:
    bool _Can_Have_Rally_Point();
    void _Update_Buildables();
    const InfantryTypeClass* _Crew_Type() const;
    int _How_Many_Survivors() const;
    int _Shape_Number() const;
    void _Detach_Anim(AnimClass* anim);
    void _Draw_It(Point2D const& xdrawpoint, Rect const& xcliprect);
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
    const int engineer_chance = Extension::Fetch<BuildingTypeClassExtension>(Class)->EngineerChance;
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

    const int count = (Class->Cost_Of(House) * Rule->SurvivorFraction) / divisor;
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
            shapenum = (Class->Anims[BSTATE_CONSTRUCTION].Start + Class->Anims[BSTATE_CONSTRUCTION].Count - 1) - shapenum;
        }

        /**
         *  If the building is deconstructing, then the display frame progresses
         *  from the end to the beginning. Reverse the shape number accordingly.
         */
        if (Mission == MISSION_DECONSTRUCTION) {
            shapenum = (Class->Anims[BState].Start + Class->Anims[BState].Count - 1) - shapenum;
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

    const auto type_ext = Extension::Fetch<BuildingTypeClassExtension>(Class);
    if (type_ext->IsHideDuringSpecialAnim && (Anims[BANIM_SPECIAL_ONE] || Anims[BANIM_SPECIAL_TWO] || Anims[BANIM_SPECIAL_THREE])) return;

    bool open_roof = false;
    if (Get_Mission() == MISSION_UNLOAD) {
        TechnoClass* radio = Contact_With_Whom();
        if (radio != nullptr && (radio->TClass->Locomotor == __uuidof(JumpjetLocomotionClass))) {
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

        shapenum += (HealthRatio <= Rule->ConditionYellow ? (Class->GateStages + 1) : 0);
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
    Point2D zsizeoffset((Class->Width() * CELL_LEPTON_W) - CELL_LEPTON_W, (Class->Height() * CELL_LEPTON_H) - CELL_LEPTON_H);
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
 *  Replaces an inlined call of Detach_Anim with a direct call.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_BuildingCLass_Detach_Detach_Anim_Patch)
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
    AircraftTypeClassExtension *radio_class_ext = Extension::Fetch<AircraftTypeClassExtension>(radio->Class);

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
    techno->field_20C = 0;
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
    ext_ptr = Extension::Fetch<BuildingClassExtension>(this_ptr);

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
    ext_ptr = Extension::Fetch<BuildingClassExtension>(this_ptr);
    exttype_ptr = Extension::Fetch<BuildingTypeClassExtension>(this_ptr->Class);

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
    ext_ptr = Extension::Fetch<BuildingClassExtension>(this_ptr);
    exttype_ptr = Extension::Fetch<BuildingTypeClassExtension>(this_ptr->Class);

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
    GET_REGISTER_STATIC(Coordinate *, coord, eax);
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
    buildingtypeext = Extension::Fetch<BuildingTypeClassExtension>(buildingtype);

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
    GET_REGISTER_STATIC(Coordinate *, coord, eax);
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
    buildingtypeext = Extension::Fetch<BuildingTypeClassExtension>(buildingtype);

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
    buildingtypeext = Extension::Fetch<BuildingTypeClassExtension>(building->TClass);

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
        if ((buildingtypeext->ShakePixelXLo > 0 || buildingtypeext->ShakePixelXHi > 0)
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
    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);
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
    LEA_STACK_STATIC(Coordinate*, coords, esp, 0x40);
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

    ext = Extension::Fetch<BuildingTypeClassExtension>(this_ptr->Class);
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

    unitext = Extension::Fetch<UnitClassExtension>(unit);
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

    super_ext = Extension::Fetch<SuperWeaponTypeClassExtension>(SuperWeaponTypes[this_ptr->field_298]);
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
        if (radio != nullptr && (radio->TClass->Locomotor == __uuidof(JumpjetLocomotionClass))) {
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

    btypeext = Extension::Fetch<BuildingTypeClassExtension>(building->Class);

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

    btypeext = Extension::Fetch<BuildingTypeClassExtension>(building->Class);

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
    Patch_Jump(0x00433F1D, &_BuildingCLass_Detach_Detach_Anim_Patch);
    Patch_Jump(0x00432709, &_BuildingClass_Mission_Missile_INITIAL_Patch);
    Patch_Jump(0x00432729, &_BuildingClass_Mission_Missile_DOOR_OPENING_Patch);
    Patch_Jump(0x00432957, &_BuildingClass_Mission_Missile_LAUNCH_DOWN_Patch);
    Patch_Jump(0x00432937, &_BuildingClass_Mission_Missile_LAUNCH_DOWN_Voice_Patch);
    //Patch_Jump(0x00427CD8, &_BuildingClass_entry_370_RoofDoorAnim_Patch1);
    //Patch_Jump(0x00427DF5, &_BuildingClass_entry_370_RoofDoorAnim_Patch2);
}
