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
#include "buildingext_init.h"
#include "buildingext.h"
#include "buildingtypeext.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "vinifera_util.h"
#include "building.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
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
#include "unittype.h"
#include "rules.h"
#include "voc.h"
#include "iomap.h"
#include "spritecollection.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class BuildingClassExt final : public BuildingClass
{
public:
    bool _Can_Have_Rally_Point();
    void _Update_Buildables();
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
                if (PlayerPtr->Can_Build(AircraftTypes[i], false, true) && AircraftTypes[i]->Who_Can_Build_Me(true, true, true, PlayerPtr) != nullptr)
                {
                    Map.Add(RTTI_AIRCRAFTTYPE, i);
                }
            }
            break;

        case RTTI_BUILDINGTYPE:
            for (int i = 0; i < BuildingTypes.Count(); i++)
            {
                if (PlayerPtr->Can_Build(BuildingTypes[i], false, true) && BuildingTypes[i]->Who_Can_Build_Me(true, true, true, PlayerPtr) != nullptr)
                {
                    Map.Add(RTTI_BUILDINGTYPE, i);
                }
            }
            break;

        case RTTI_INFANTRYTYPE:
            for (int i = 0; i < InfantryTypes.Count(); i++)
            {
                if (PlayerPtr->Can_Build(InfantryTypes[i], false, true) && InfantryTypes[i]->Who_Can_Build_Me(true, true, true, PlayerPtr) != nullptr)
                {
                    Map.Add(RTTI_INFANTRYTYPE, i);
                }
            }
            break;

        case RTTI_UNITTYPE:
            for (int i = 0; i < UnitTypes.Count(); i++)
            {
                if (PlayerPtr->Can_Build(UnitTypes[i], false, true) && UnitTypes[i]->Who_Can_Build_Me(true, true, true, PlayerPtr) != nullptr)
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

    TARGET target = nullptr;

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
        newowner->field_4F0 = true;
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
    Sound_Effect(voc, *coord);

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
    Sound_Effect(voc, *coord);

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
    buildingtypeext = Extension::Fetch<BuildingTypeClassExtension>(building->Techno_Type_Class());

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
    static TechnoTypeClass *technotype;
    static TechnoTypeClassExtension *technotypeext;
    static const ShapeFileStruct *cameo_shape;
    static Surface *pcx_image;
    static Rect pcxrect;

    technotype = factory_obj->Techno_Type_Class();

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
        CC_Draw_Shape(LogicSurface, CameoDrawer, cameo_shape, 0, pos_xy, window_rect, ShapeFlagsType(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_NORMAL));
    }

    JMP(0x00428B13);
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
    Patch_Jump(0x00439D10, &BuildingClassExt::_Can_Have_Rally_Point);
    Patch_Jump(0x0042D9A0, &BuildingClassFake::_Update_Buildables);
}
