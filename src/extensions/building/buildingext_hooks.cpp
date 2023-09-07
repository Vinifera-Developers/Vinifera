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
#include "unit.h";
#include "unitext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
#include "house.h"
#include "housetype.h"
#include "map.h"
#include "mouse.h"
#include "houseext.h"
#include "cell.h"
#include "bsurface.h"
#include "dsurface.h"
#include "convert.h"
#include "drawshape.h"
#include "rules.h"
#include "rulesext.h"
#include "scenario.h"
#include "scenarioext.h"
#include "terrain.h"
#include "terraintype.h"
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
static class BuildingClassFake final : public BuildingClass
{
public:
    bool _Can_Have_Rally_Point();
};


bool BuildingClassFake::_Can_Have_Rally_Point()
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
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, ebp);
    static int time;

    time = Building_Radio_Reload_Rate(this_ptr);

    _asm { mov eax, time }
    JMP_REG(edi, 0x0043260F);
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

        SpriteCollection.Draw(pcxrect, *TempSurface, *technotypeext->CameoImageSurface);

    } else {

        cameo_shape = technotype->Get_Cameo_Data();

        /**
         *  Draw the cameo shape.
         * 
         *  Original code used NormalDrawer, which is the old Red Alert shape
         *  drawer, so we need to use CameoDrawer here for the correct palette.
         */
        CC_Draw_Shape(TempSurface, CameoDrawer, cameo_shape, 0, pos_xy, window_rect, ShapeFlagsType(SHAPE_CENTER|SHAPE_400|SHAPE_ALPHA|SHAPE_NORMAL));
    }

    JMP(0x00428B13);
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
original_code:
    _asm { movsx   eax, bp }
    _asm { movsx   ecx, bx }
    JMP_REG(edx, 0x0042E5FB);
}


/**
 *  DTA-specific patch. Prevents buildings from catching flames
 *  when rapidly switching between damage yellow and green
 *  damage states.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_BuildingClass_Take_Damage_Prevent_Cumulative_Flame_Spawn_Patch)
{
    GET_REGISTER_STATIC(Coordinate *, coord, eax);
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    static BuildingClassExtension *buildingext;

    /**
     *  Stolen bytes / code.
     */
    Sound_Effect(Rule->BlowupSound, *coord);

    /**
     *  Actual functionality of the hack.
     *  Do not spawn flames on the building if flames were spawned
     *  on it too recently.
     */
    buildingext = Extension::Fetch<BuildingClassExtension>(this_ptr);
    if (Frame < buildingext->LastFlameSpawnFrame + RuleExtension->BuildingFlameSpawnBlockFrames) {
        goto past_flame_spawn;
    }

    buildingext->LastFlameSpawnFrame = Frame;

    /**
     *  Continue into applying building flames.
     */
original_code:
    _asm { mov  ebx, 7FFFh }
    JMP(0x0042B6E4);

    /**
     *  Skip the game's code block for spawning flames on buildings.
     */
past_flame_spawn:
    JMP(0x0042B684);
}


void Mark_Expansion_As_Done(HouseClass* house) {
    HouseClassExtension* ext = Extension::Fetch<HouseClassExtension>(house);

    if (ext->NextExpansionPointLocation.X == 0 || ext->NextExpansionPointLocation.Y == 0)
        return;

    ext->NextExpansionPointLocation = Cell(0, 0);
}

int Try_Place(BuildingClass* building, Cell cell) 
{
    HouseClass* owner = building->House;
    HouseClassExtension* ext = Extension::Fetch<HouseClassExtension>(owner);

    int ret = building->Class->Flush_For_Placement(cell, owner);
    if (ret == 1) {
        return 1;
    } else if (ret == 2) {
        //cell = Map.Nearby_Location(target_cell, SPEED_TRACK, -1, building->Class->MZone, false, building->Class->Width(), building->Class->Height(), true);
        return 1;
    }

    Cell final_placement_cell = cell;
    // final_placement_cell = Map.Nearby_Location(cell, SPEED_TRACK, -1, building->Class->MZone, false, building->Class->Width(), building->Class->Height(), true, false, false, true, closest);
    Coordinate coord = Cell_Coord(final_placement_cell);

    if (building->Unlimbo(coord)) {
        owner->BuildStructure = BUILDING_NONE;

        // This is necessary or the building's build-up anim is played twice.
        // RA doesn't do this, must be a difference somewhere in the engine.
        building->Assign_Mission(MISSION_CONSTRUCTION);
        building->Commence();

        int close_enough = 15;

        // Check if we placed a refinery.
        // If yes, check if we were expanding. If yes, the expanding is done.
        // If no but we're close to an expansion field, then flag us to build a refinery as our next building.
        if (building->Class->IsRefinery) {
            if (ext->NextExpansionPointLocation.X != 0 && ext->NextExpansionPointLocation.Y != 0) {
                BuildingClassExtension* buildingext = Extension::Fetch<BuildingClassExtension>(building);
                buildingext->AssignedExpansionPoint = ext->NextExpansionPointLocation;
            }

            Mark_Expansion_As_Done(owner);
            ext->ShouldBuildRefinery = false;
        } 
        else if (ext->NextExpansionPointLocation.X > 0 &&
            ext->NextExpansionPointLocation.Y > 0 &&
            ::Distance(Coord_Cell(building->Center_Coord()), ext->NextExpansionPointLocation) < close_enough) 
        {
            ext->ShouldBuildRefinery = true;
        }

        return 2;
    }

    return 0;
}

/**
 *  Fetches a house's base area as a rectangle.
 *  We can use this as a rough zone for placing new buildings.
 */
Rect Get_Base_Rect(HouseClass* house, int adjacency, int width, int height)
{
    int x = INT_MAX;
    int y = INT_MAX;
    int right = INT_MIN;
    int bottom = INT_MIN;

    for (int i = 0; i < Buildings.Count(); i++) {
        BuildingClass* building = Buildings[i];

        if (!building->IsActive || building->IsInLimbo || building->House != house) {
            continue;
        }

        Cell buildingcell = building->Get_Cell();
        if (buildingcell.X < x)
            x = buildingcell.X;

        if (buildingcell.Y < y)
            y = buildingcell.Y;

        int buildingright = buildingcell.X + building->Class->Width() - 1;
        if (buildingright > right)
            right = buildingright;

        int buildingbottom = buildingcell.Y + building->Class->Height() - 1;
        if (buildingbottom > bottom)
            bottom = buildingbottom;
    }

    x -= adjacency;
    x -= width;
    y -= adjacency;
    y -= height;
    right += adjacency + width;
    bottom += adjacency + height;

    return Rect(x, y, right - x, bottom - y);
}

/**
 *  Checks whether a cell should be evaluated for AI building placement.
 */
bool Should_Evaluate_Cell_For_Placement(Cell cell, BuildingClass* building)
{
    bool retvalue = false;

    int adjacency = building->Class->Adjacent + 1;

    for (int i = 0; i < Buildings.Count(); i++) {
        BuildingClass* otherbuilding = Buildings[i];

        if (!otherbuilding->IsActive || 
            otherbuilding->IsInLimbo ||
            otherbuilding->Class->IsInvisibleInGame) {
            continue;
        }

        // We don't want the AI to get stuck.
        // Make sure that the building would leave at least 1 free cell to its neighbours.
        Cell origin = otherbuilding->Get_Cell();
        Cell* occupy = otherbuilding->Occupy_List(true);
        bool allowance = true;
        while (occupy->X != REFRESH_EOL && occupy->Y != REFRESH_EOL) {
            Cell sum = origin + *occupy;

            // The new building is close enough if even one 
            // of its cells would be close enough to the cells
            // of the current "other" building.
            Cell* newoccupy = building->Occupy_List(true);
            while (newoccupy->X != REFRESH_EOL && newoccupy->Y != REFRESH_EOL) {
                Cell newsum = cell + *newoccupy;

                int xdiff = newsum.X - sum.X;
                int ydiff = newsum.Y - sum.Y;
                xdiff = ABS(xdiff);
                ydiff = ABS(ydiff);

                if (xdiff < 2 && ydiff < 2) {
                    // This foundation cell too close to the compared building
                    allowance = false;
                    break;
                }

                newoccupy++;
            }

            if (!allowance) {
                break;
            }

            occupy++;
        }

        // If the building was too close to an existing building, just bail out.
        // No need to check anything else.
        if (!allowance) {
            retvalue = false;
            break;
        }

        // For the proximity check, check that this building
        // is owned by us and that it extends the adjacency range.
        if (otherbuilding->House != building->House ||
            !otherbuilding->Class->IsBase) {
            continue;
        }

        // Check that the cell would be close enough for the building placement 
        // to pass the proximity check if the building was on the cell.
        // This is only necessary to check if the building hasn't already
        // passed the proximity check.
        if (!retvalue) {

            bool pass = false;

            origin = otherbuilding->Get_Cell();
            occupy = otherbuilding->Occupy_List(true);
            while (occupy->X != REFRESH_EOL && occupy->Y != REFRESH_EOL) {
                Cell sum = origin + *occupy;

                // The new building is close enough if even one 
                // of its cells would be close enough to the cells
                // of the current "other" building.
                Cell* newoccupy = building->Occupy_List(true);
                while (newoccupy->X != REFRESH_EOL && newoccupy->Y != REFRESH_EOL) {
                    Cell newsum = cell + *newoccupy;

                    int xdiff = newsum.X - sum.X;
                    int ydiff = newsum.Y - sum.Y;
                    xdiff = ABS(xdiff);
                    ydiff = ABS(ydiff);

                    if (xdiff <= adjacency && ydiff <= adjacency) {
                        // This foundation cell is close enough to the compared building.
                        pass = true;
                        break;
                    }

                    newoccupy++;
                }

                if (pass) {
                    break;
                }

                occupy++;
            }

            if (pass)
                retvalue = true;
        }
    }

    return retvalue;
}

/**
 *  Evaluates a rectangle from the map with a value generator 
 *  function and finds the best cell for placing down a building.
 *  The best cell is one that has the LOWEST value and that allows legal
 *  building placement.
 */
Cell Find_Best_Building_Placement_Cell(Rect basearea, BuildingClass* building, int (*valuegenerator)(Cell, BuildingClass*))
{
    int lowestrating = INT_MAX;
    Cell bestcell = Cell(0, 0);

    // Check the resolution of the scan. If our base area is huge, we can't check as precisely
    // or we'll cause into performance issues.
    int resolution;
    int rescells = 2000;
    int areasize = basearea.Width * basearea.Height;
    resolution = 1 + (areasize / rescells);

    for (int y = basearea.Y; y < basearea.Y + basearea.Height; y += resolution) {
        for (int x = basearea.X; x < basearea.X + basearea.Width; x += resolution) {
            Cell cell = Cell(x, y);

            // Skip cells that are outside of the visible map area.
            if (!Map.In_Radar(cell))
                continue;

            // Skip cells where we couldn't legally place the building on.
            // TODO: Manually check the cells? Currently our own units also block placement.
            if (!building->Class->Legal_Placement(cell, building->House))
                continue;

            // Check whether this cell is fine by proximity rules.
            if (!Should_Evaluate_Cell_For_Placement(cell, building))
                continue;

            int value = valuegenerator(cell, building);
            if (value < lowestrating) {
                lowestrating = value;
                bestcell = cell;
            }
        }
    }

    return bestcell;
}

int Refinery_Placement_Cell_Value(Cell cell, BuildingClass* building) 
{
    HouseClass* owner = building->House;
    HouseClassExtension* houseext = Extension::Fetch<HouseClassExtension>(owner);

    // If we have nowhere to expand, then just try placing it somewhere central, hopefully it's safe there.
    if (houseext->NextExpansionPointLocation.X <= 0 || houseext->NextExpansionPointLocation.Y <= 0) {
        Cell center = owner->Base_Center();
        return ::Distance(cell, center);
    }

    // For refinery placement, we can basically make the value equal to the distance
    // that the refinery has to our next expansion point.
    return ::Distance(cell, houseext->NextExpansionPointLocation);
}

/**
 *  Calculates the best refinery placement location.
 */
Cell Get_Best_Refinery_Placement_Position(BuildingClass* building)
{
    int adjacency = building->Class->Adjacent + 1;
    Rect basearea = Get_Base_Rect(building->House, adjacency, building->Class->Width(), building->Class->Height());
    return Find_Best_Building_Placement_Cell(basearea, building, Refinery_Placement_Cell_Value);
}

int Near_Base_Center_Placement_Position_Value(Cell cell, BuildingClass* building)
{
    HouseClass* owner = building->House;
    Cell center = owner->Base_Center();
    return ::Distance(cell, center);
}

int Near_Enemy_Placement_Position_Value(Cell cell, BuildingClass* building)
{
    HouseClass* owner = building->House;
    HouseClass* enemy = nullptr;

    if (owner->Enemy != HOUSE_NONE) {
        enemy = HouseClass::As_Pointer(owner->Enemy);
    }

    // If we have no enemy, then place it as close to the center of the map as possible.
    // Most commonly we are on the edge of a map, so if we place towards the center,
    // it doesn't go terribly wrong.
    if (enemy == nullptr) {
        Point2D mapcenter = Map.MapLocalSize.Center_Point();
        Cell mapcenter_cell = Cell(mapcenter.X, mapcenter.Y);
        return ::Distance(cell, mapcenter_cell);
    }

    return ::Distance(cell, enemy->Base_Center());
}

int Near_ConYard_Placement_Position_Value(Cell cell, BuildingClass* building)
{
    Cell conyardcell = Cell(0, 0);
    if (building->House->ConstructionYards.Count() > 0) {
        conyardcell = building->House->ConstructionYards[0]->Get_Cell();
    } else {
        // Fallback
        Point2D mapcenter = Map.MapLocalSize.Center_Point();
        Cell mapcenter_cell = Cell(mapcenter.X, mapcenter.Y);
        conyardcell = mapcenter_cell;
    }

    return SHRT_MAX - ::Distance(cell, conyardcell);
}

int Far_From_Enemy_Placement_Position_Value(Cell cell, BuildingClass* building)
{
    HouseClass* owner = building->House;
    HouseClass* enemy = nullptr;

    if (owner->Enemy != HOUSE_NONE) {
        enemy = HouseClass::As_Pointer(owner->Enemy);
    }

    // If we have no enemy, then just place it near the base center.
    if (enemy == nullptr) {
        Cell center = owner->Base_Center();
        return ::Distance(cell, center);
    }

    return SHRT_MAX - ::Distance(cell, enemy->Base_Center());
}

Cell Get_Best_SuperWeapon_Building_Placement_Position(BuildingClass* building)
{
    int adjacency = building->Class->Adjacent + 1;
    Rect basearea = Get_Base_Rect(building->House, adjacency, building->Class->Width(), building->Class->Height());
    return Find_Best_Building_Placement_Cell(basearea, building, Far_From_Enemy_Placement_Position_Value);
}

int Towards_Expansion_Placement_Cell_Value(Cell cell, BuildingClass* building)
{
    HouseClass* owner = building->House;
    HouseClassExtension* houseext = Extension::Fetch<HouseClassExtension>(owner);

    // If we have nowhere to expand, then just try placing it somewhere that's far from our base.
    if (houseext->NextExpansionPointLocation.X <= 0 || houseext->NextExpansionPointLocation.Y <= 0) {
        Cell center = owner->Base_Center();
        return SHRT_MAX - ::Distance(cell, center);
    }

    HouseClass* enemy = nullptr;

    if (owner->Enemy != HOUSE_NONE) {
        enemy = HouseClass::As_Pointer(owner->Enemy);
    }

    int enemydistance = 0;
    if (enemy != nullptr && enemy->ConstructionYards.Count() > 0) {
        enemydistance = ::Distance(cell, enemy->ConstructionYards[0]->Get_Cell());
    }

    // Otherwise, we can basically make the value equal to the distance
    // that the building has to our next expansion point.
    // Also, secondarily take distance into enemy into account.
    return ::Distance(cell, houseext->NextExpansionPointLocation) * 100 + enemydistance;
}

Cell Get_Best_Expansion_Placement_Position(BuildingClass* building)
{
    int adjacency = building->Class->Adjacent + 1;
    Rect basearea = Get_Base_Rect(building->House, adjacency, building->Class->Width(), building->Class->Height());

    HouseClass* owner = building->House;
    HouseClassExtension* houseext = Extension::Fetch<HouseClassExtension>(owner);

    Cell bestcell = Find_Best_Building_Placement_Cell(basearea, building, Towards_Expansion_Placement_Cell_Value);

    if (houseext->NextExpansionPointLocation.X > 0 &&
        houseext->NextExpansionPointLocation.Y > 0 &&
        !houseext->ShouldBuildRefinery) {

        // If we can't get closer to the expansion point with this building,
        // then we are as close to the expansion point as possible and should build a refinery
        // as our next building.

        // To perform this check, fetch the nearest distance any of our buildings has to the expansion point,
        // and perform a comparison to the best cell.

        int nearestdistance = INT_MAX;
        Cell nearestcell = Cell(0, 0);

        for (int i = 0; i < Buildings.Count(); i++)
        {
            BuildingClass* otherbuilding = Buildings[i];

            if (!otherbuilding->IsActive ||
                otherbuilding->IsInLimbo ||
                otherbuilding->Class->IsInvisibleInGame ||
                otherbuilding->House != owner) {
                continue;
            }

            int distance = ::Distance(houseext->NextExpansionPointLocation, otherbuilding->Get_Cell());
            if (distance < nearestdistance) {
                nearestdistance = distance;
                nearestcell = otherbuilding->Get_Cell();
            }
        }

        int newdistance = ::Distance(houseext->NextExpansionPointLocation, bestcell);
        if (newdistance >= nearestdistance) {
            houseext->ShouldBuildRefinery = true;
        }
    }
    
    return bestcell;
}

int Barracks_Placement_Cell_Value(Cell cell, BuildingClass* building)
{
    // A barracks is best built close to the opponent.
    HouseClass* owner = building->House;
    HouseClassExtension* houseext = Extension::Fetch<HouseClassExtension>(owner);

    int expand_distance = 0;
    // If we are expanding, consider distance to expansion location as barracks are great for expanding.
    if (houseext->NextExpansionPointLocation.X > 0 && houseext->NextExpansionPointLocation.Y > 0) {
        expand_distance = ::Distance(cell, houseext->NextExpansionPointLocation);
        expand_distance *= 3; // Give expansion distance more weight than distance to enemy
    }

    HouseClass* enemy = nullptr;

    if (owner->Enemy != HOUSE_NONE) {
        enemy = HouseClass::As_Pointer(owner->Enemy);
    }

    if (enemy != nullptr) {
        return ::Distance(cell, enemy->Base_Center()) + expand_distance;
    }

    // If we do not have an opponent, then just consider expansion.
    if (expand_distance > 0) {
        return expand_distance;
    }

    // If we do not have an opponent AND do not expand, just place it somewhere on our base outskirts.
    Cell center = owner->Base_Center();
    return SHRT_MAX - ::Distance(cell, center);

    // TODO: Distance to other barracks of our house could be a good factor too.
    // It would require us to go through all buildings though... which might give a significant perf hit.
    // Maybe a static list of barracks so we could only fetch it once?
}

int WarFactory_Placement_Cell_Value(Cell cell, BuildingClass* building)
{
    // War factories are typically valuable.
    // It might be best to place them not close to the enemy, but around our base center so they're safe.

    return Near_Base_Center_Placement_Position_Value(cell, building);
}

int Helipad_Placement_Cell_Value(Cell cell, BuildingClass* building)
{
    // Helipads don't need to be very close to the enemy.
    // Place them as far from the enemy as possible.
    return Far_From_Enemy_Placement_Position_Value(cell, building);
}

/**
 *  Calculates the best factory placement location.
 */
Cell Get_Best_Factory_Placement_Position(BuildingClass* building) 
{
    int adjacency = building->Class->Adjacent + 1;
    Rect basearea = Get_Base_Rect(building->House, adjacency, building->Class->Width(), building->Class->Height());

    if (building->Class->ToBuild == RTTI_INFANTRYTYPE)
        return Find_Best_Building_Placement_Cell(basearea, building, Barracks_Placement_Cell_Value);
    else if (building->Class->ToBuild == RTTI_UNITTYPE)
        return Find_Best_Building_Placement_Cell(basearea, building, WarFactory_Placement_Cell_Value);
    else if (building->Class->ToBuild == RTTI_AIRCRAFTTYPE)
        return Find_Best_Building_Placement_Cell(basearea, building, Helipad_Placement_Cell_Value);
    else
        return Find_Best_Building_Placement_Cell(basearea, building, Near_Base_Center_Placement_Position_Value);
}

static Cell attackcell;

int Near_AttackCell_Cell_Value(Cell cell, BuildingClass* building) 
{
    return ::Distance(cell, attackcell);
}

Cell Get_Best_Defense_Placement_Position(BuildingClass* building)
{
    HouseClass* owner = building->House;
    HouseClassExtension* houseext = Extension::Fetch<HouseClassExtension>(owner);

    attackcell = Cell(0, 0);

    int adjacency = building->Class->Adjacent + 1;
    Rect basearea = Get_Base_Rect(building->House, adjacency, building->Class->Width(), building->Class->Height());

    // If we were attacked recently, then place the defense near a damaged building of ours if one exists.
    if (owner->LATime + TICKS_PER_MINUTE > Frame) {
        for (int i = 0; i < Buildings.Count(); i++) {
            BuildingClass* otherbuilding = Buildings[i];

            if (!otherbuilding->IsActive ||
                otherbuilding->IsInLimbo ||
                otherbuilding->Class->IsInvisibleInGame ||
                otherbuilding->House != owner) {
                continue;
            }

            if (otherbuilding->Strength < otherbuilding->Class->MaxStrength) {
                attackcell = otherbuilding->Get_Cell();
                break;
            }
        }
    }

    if (attackcell.X > 0 && attackcell.Y > 0) {
        return Find_Best_Building_Placement_Cell(basearea, building, Near_AttackCell_Cell_Value);
    }

    // If we are expanding, then it's likely we should build defenses towards the expansion node.
    if (houseext->NextExpansionPointLocation.X > 0 && houseext->NextExpansionPointLocation.Y > 0 && Percent_Chance(50)) {
        return Find_Best_Building_Placement_Cell(basearea, building, Towards_Expansion_Placement_Cell_Value);
    }

    HouseClass* enemy = nullptr;
    if (owner->Enemy != HOUSE_NONE) {
        enemy = HouseClass::As_Pointer(owner->Enemy);
    }

    // Place some defenses to the backline.
    if (Percent_Chance(20)) {
        return Find_Best_Building_Placement_Cell(basearea, building, Near_ConYard_Placement_Position_Value);
    }

    if (enemy == nullptr || Percent_Chance(30)) {
        return Find_Best_Building_Placement_Cell(basearea, building, Near_Base_Center_Placement_Position_Value);
    }

    return Find_Best_Building_Placement_Cell(basearea, building, Near_Enemy_Placement_Position_Value);
}

Cell Get_Best_Sensor_Placement_Position(BuildingClass* building)
{
    int adjacency = building->Class->Adjacent + 1;
    Rect basearea = Get_Base_Rect(building->House, adjacency, building->Class->Width(), building->Class->Height());
    return Find_Best_Building_Placement_Cell(basearea, building, Near_Base_Center_Placement_Position_Value);
}

Cell Get_Best_Placement_Position(BuildingClass* building) 
{
    if (building->Class->IsRefinery) {
        return Get_Best_Refinery_Placement_Position(building);
    }

    if (building->Class->SuperWeapon != SPECIAL_NONE || building->Class->SuperWeapon2 != SPECIAL_NONE) {
        return Get_Best_SuperWeapon_Building_Placement_Position(building);
    }

    if (building->Class->ToBuild != RTTI_NONE) {
        return Get_Best_Factory_Placement_Position(building);
    }

    if (building->Class->Fetch_Weapon_Info(WEAPON_SLOT_PRIMARY).Weapon != nullptr) {
        return Get_Best_Defense_Placement_Position(building);
    }

    if (building->Class->IsSensorArray) {
        return Get_Best_Sensor_Placement_Position(building);
    }

    return Get_Best_Expansion_Placement_Position(building);
}

int BuildingClass_Exit_Object_Custom_Position(BuildingClass* building)
{
    HouseClass* owner = building->House;
    HouseClassExtension* ext = Extension::Fetch<HouseClassExtension>(owner);

    Cell placement_cell = Get_Best_Placement_Position(building);

    // If we couldn't find any place for the building, refund it.
    if (placement_cell.X <= 0 || placement_cell.Y <= 0) {
        return 0;
    }

    int returnvalue = Try_Place(building, placement_cell); 

    return returnvalue;
}

DECLARE_PATCH(_BuildingClass_Exit_Object_Seek_Building_Position)
{
    GET_REGISTER_STATIC(BuildingClass*, base, edi);
    static int retvalue;
    retvalue = 0;

    if (!RuleExtension->IsUseAdvancedAI || base->Class->PowersUpBuilding[0] != '\0') {

        // Stolen bytes / code
        _asm { mov eax, [esi+0ECh]}
        JMP_REG(ecx, 0x0042D3BE);
    }

    retvalue = BuildingClass_Exit_Object_Custom_Position(base);

    // Reconstruct function epilogue
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { mov eax, dword ptr ds:retvalue }
    _asm { pop ebx }
    _asm { add esp, 0F8h }
    _asm { retn 4 }
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
    Patch_Jump(0x0042E5F5, &_BuildingClass_Grand_Opening_Assign_FreeUnit_LastDockedBuilding_Patch);
    Patch_Jump(0x00432184, &_BuildingClass_Mission_Repair_Assign_Rally_Destination_When_No_Repair_Needed);
    Patch_Jump(0x00431DAB, &_BuildingClass_Mission_Repair_Assign_Rally_Destination_After_Repair_Complete);
    Patch_Jump(0x00439D10, &BuildingClassFake::_Can_Have_Rally_Point);
    Patch_Jump(0x004325F9, &_BuildingClass_Mission_Repair_ReloadRate_Patch);
    Patch_Jump(0x0043266C, &_BuildingClass_Mission_Repair_ReloadRate_Patch);
    Patch_Jump(0x0042B6CC, &_BuildingClass_Take_Damage_Prevent_Cumulative_Flame_Spawn_Patch);
    Patch_Jump(0x0042D3B8, &_BuildingClass_Exit_Object_Seek_Building_Position);
}
