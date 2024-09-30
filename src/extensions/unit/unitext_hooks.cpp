/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITEXT_HOOKS.H
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended UnitClass.
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
#include "unitext_hooks.h"
#include "unitext_init.h"
#include "tibsun_inline.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "technotype.h"
#include "technotypeext.h"
#include "warheadtype.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "target.h"
#include "rules.h"
#include "iomap.h"
#include "voc.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "verses.h"
#include "warheadtypeext.h"
#include "weapontype.h"


#if 0
/**
 *  #issue-510
 * 
 *  This patch fixes a bug where harvesters that are idling on a bridge with
 *  tiberium below it would begin erroneously harvesting the patch below it.
 * 
 *  @author: CCHyper
 */
static CellClass *Unit_Get_Current_Cell(UnitClass *this_ptr) { return &Map[this_ptr->Get_Coord()]; }
DECLARE_PATCH(_UnitClass_Mission_Harvest_Block_Harvesting_On_Bridge_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    static CellClass *cellptr;

    /**
     *  Perform a check to see if the cell we occupy contains a bridge.
     */
    cellptr = Unit_Get_Current_Cell(this_ptr);
    if (cellptr && cellptr->Is_Bridge_Here()) {
        goto function_return;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    _asm { mov eax, [esi+0x360] } // this->Class
    _asm { mov ecx, [eax+0x268] } // Class->Dock.ActiveCount
    JMP_REG(ebp, 0x00654AB6);

function_return:
    JMP(0x00654AA3);
}
#endif


#if 0
/**
 *  #issue-510
 * 
 *  This patch fixes a bug where harvesters that are idling on a bridge with
 *  tiberium below it would begin erroneously harvesting the patch below it.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Enter_Idle_Mode_Block_Harvesting_On_Bridge_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    static CellClass *cellptr;

    /**
     *  Code before here assumes we are a harvester of some kind.
     */

    /**
     *  Perform a check to see if the cell we occupy contains a bridge.
     */
    cellptr = Unit_Get_Current_Cell(this_ptr);
    if (cellptr && cellptr->Is_Bridge_Here()) {
        goto function_return;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    static MissionType mission;
    mission = this_ptr->Get_Mission();
    _asm { mov eax, mission }
    JMP_REG(edi, 0x00650559);

function_return:
    JMP_REG(ecx, 0x0065066A);
}
#endif


/**
 *  #issue-510
 * 
 *  This patch fixes the bug where the user could action a harvester to 
 *  target tiberium that was 'hidden' below a bridge by forcing any orders
 *  on a bridge with tiberium below it to be move orders.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, edi);
    GET_REGISTER_STATIC(Cell *, cell, esi);
    static CellClass *cellptr;
    static ActionType action;

    /**
     *  Code before here assumes we are a harvester of some kind.
     */

    action = ACTION_HARVEST;

    /**
     *  If the user has the mouse over a cell that contains tiberium, but also
     *  contains a bridge above it, force the MOVE action.
     */
    cellptr = &Map[*cell];
    if (cellptr && cellptr->Is_Bridge_Here()) {
        action = ACTION_MOVE;
    }

    _asm { mov eax, action }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x10 }
    _asm { ret 0x0C }
}


/**
 *  #issue-421
 * 
 *  Implements IdleRate for UnitTypes.
 *
 *  This replaces the WalkFrames branch as we needed to move FiringSyncDelay
 *  before so it has the highest priority over what shape frame to use.
 * 
 *  @author: CCHyper
 */
static bool Locomotion_Is_Moving(UnitClass *this_ptr) { return this_ptr->Locomotion->Is_Moving(); }
DECLARE_PATCH(_UnitClass_Draw_Shape_IdleRate_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    GET_REGISTER_STATIC(int, facing, ebx);
    GET_REGISTER_STATIC(ShapeFileStruct *, shape, edi);
    static UnitTypeClassExtension *unittypeext;
    static const UnitTypeClass *unittype;
    static int frame;

    unittype = reinterpret_cast<const UnitTypeClass *>(this_ptr->Techno_Type_Class());
    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    if (!Locomotion_Is_Moving(this_ptr)) {
        if (this_ptr->FiringSyncDelay >= 0) {
            frame = (this_ptr->FiringSyncDelay/2)
                + this_ptr->Class->StartFiringFrame
                + (this_ptr->Class->FiringFrames * facing);

            goto continue_to_draw;
        }
    }

    if (!Locomotion_Is_Moving(this_ptr)) {
        if (this_ptr->DeathCounter >= 0) {

            static int death_frame;

            death_frame = (this_ptr->DeathCounter / unittype->DeathFrameRate);
            if (death_frame >= (unittype->DeathFrames-1)) {
                death_frame = (unittype->DeathFrames-1);
            }

            frame = death_frame + unittype->StartDeathFrame;

            goto continue_to_draw;
        }
    }

    if (Locomotion_Is_Moving(this_ptr)) {
        frame = unittype->StartWalkFrame
            + (this_ptr->TotalFramesWalked % unittype->WalkFrames)
            + (unittype->WalkFrames * facing);

        goto continue_to_draw;
    }

    /**
     *  Unit is not moving, so if the unit has a idle animation rate, use this.
     */
    if (!Locomotion_Is_Moving(this_ptr) && unittypeext->IdleRate > 0) {
        frame = unittypeext->StartIdleFrame
            + (this_ptr->TotalFramesWalked % unittypeext->IdleFrames)
            + (unittypeext->IdleFrames * facing);

        goto continue_to_draw;
    }

    if (this_ptr->field_34D) {
        if (unittype->StandingFrames > 0) {
            frame = unittype->StartStandFrame + (facing * unittype->StandingFrames);

        } else {
            frame = unittype->StartWalkFrame + (facing * unittype->WalkFrames);
        }

        goto continue_to_draw;
    }

    /**
     *  Continue to the shape drawing.
     */
continue_to_draw:
    _asm { mov ebx, frame }
    _asm { mov edi, shape }     // Restore EDI register (shape file pointer).
    JMP(0x006531FB);
}


/**
 *  #issue-264
 * 
 *  Implements LeaveTransportSound for this unit is unloading its passengers.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Mission_Unload_Transport_Detach_Sound_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    static FootClass *passenger;
    static TechnoTypeClassExtension *radio_technotypeext;

    /**
     *  Do we have a sound to play when passengers leave us? If so, play it now.
     */
    radio_technotypeext = Extension::Fetch<TechnoTypeClassExtension>(this_ptr->Techno_Type_Class());
    if (radio_technotypeext->LeaveTransportSound != VOC_NONE) {
        Sound_Effect(radio_technotypeext->LeaveTransportSound, this_ptr->Coord);
    }

    /**
     *  Stolen bytes/code.
     * 
     *  Are we a part of a team? If so, make any passengers we unload part of it too.
     */
    if (this_ptr->Team) {
        goto add_to_team;
    }

    /**
     *  Finished unloading passengers.
     */
finish_up:
    JMP(0x006543BB);

    /**
     *  Add this passenger to my team.
     */
add_to_team:
    _asm { mov ebp, passenger } // Restore EBP pointer.
    JMP(0x006543A3);
}


/**
 *  #issue-188
 * 
 *  Adds support for custom (per-type) unloading class when a harvester is unloading at a refinery.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Draw_It_Unloading_Harvester_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    GET_REGISTER_STATIC(UnitTypeClass *, unittype, eax);
    static const UnitTypeClass *unloading_class;
    static const UnitTypeClassExtension *unittypeext;

    /**
     *  The code just before this backs up the current Class, so we
     *  don't need to worry about doing that here.
     */

    /**
     *  Are we currently unloading at a refinery?
     */
    if (this_ptr->IsDumping) {

        /**
         *  Is this unit some type of harvester that is unloading?
         * 
         *  The original code only checked for "IsToHarvest".
         */
        if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {

            unloading_class = nullptr;

            /**
             *  Fetch the default unloading class.
             * 
             *  If this is a weed harvester that is unloading, then they need
             *  a special case to ensure they do not switch unless defined as
             *  they do not have a unloading graphics switch in the original
             *  Tiberian Sun when they enter the facility.
             */
            if (!unittype->IsToVeinHarvest) {
                unloading_class = Rule->UnloadingHarvester;
            }

            /**
             *  Fetch the unloading class from the extended class instance if it exists.
             */
            unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);
            if (unittypeext->UnloadingClass) {
                if (unittypeext->UnloadingClass->Kind_Of() == RTTI_UNITTYPE) {
                    unloading_class = reinterpret_cast<const UnitTypeClass *>(unittypeext->UnloadingClass);
                }
            }

            /**
             *  Only switch the graphic if the unloading class is valid.
             */
            if (unloading_class) {
                this_ptr->Class = const_cast<UnitTypeClass *>(unloading_class);
            }

        }

    }

    JMP(0x00653DA5);
}


/**
 *  Returns the graphic shape number based on the input current facing and desired facing count.
 * 
 *  @author: CCHyper
 */
static int Facing_To_Frame_Number(FacingClass &facing, int facings_count)
{
    int shape_number = 0;

    /**
     *  Fetch the current facing value in the required units.
     */
    switch (facings_count) {

        case 8:
            shape_number = Dir_To_8(facing.Current());
            break;

        case 16:
            shape_number = Dir_To_16(facing.Current());
            break;

        case 32:
            shape_number = Dir_To_32(facing.Current());
            break;

        case 64:
            shape_number = Dir_To_64(facing.Current());
            break;

        default:
            shape_number = 0;
            break;
    };

    return shape_number;
}


/**
 *  #issue-#214
 * 
 *  Support for additional facings counts for units with shape graphics.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Draw_Shape_Primary_Facing_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(const UnitTypeClass *, unittype, eax);
    //const UnitTypeClass *unittype;
    static int shape_number;

    /**
     *  #NOTE:
     *  Using either of these causes a memory leak for some reason...
     *  So we now just fetch EAX which is a UnitTypeClass instance already.
     */
    //unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Techno_Type_Class());
    //unittype = this_ptr->Class;

    /**
     *  Fetch the frame index for current turret facing.
     */
    shape_number = Facing_To_Frame_Number(this_ptr->PrimaryFacing, unittype->Facings);

    /**
     *  EBX == desired shape number.
     */
    _asm { mov ebx, shape_number }

    JMP(0x00653114);
}


/**
 *  #issue-#214
 * 
 *  Support for additional facings counts for units with shape graphics.
 * 
 *  This function replaces and reimplements the call to Draw_Object();
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Draw_Shape_Turret_Facing_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(const void *, shape, edi);
    static const UnitTypeClass *unittype;
    static const UnitTypeClassExtension *unittypeext;
    static int shape_number;
    static int frame_number;
    static int turret_facings;
    static int start_turret_frame;

    frame_number = 0;

    unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Techno_Type_Class());
    
    /**
     *  All turrets have 32 facings in Tiberian Sun.
     */
    turret_facings = 32;

    /**
     *  Turret frames start directly after the facing frames.
     */
    start_turret_frame = unittype->Facings * unittype->WalkFrames;

    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    /**
     *  #issue-393
     * 
     *  Allow the custom turret facings.
     * 
     *  @author: CCHyper
     */
    turret_facings = unittypeext->TurretFacings;

    /**
     *  Fetch the frame index for current turret facing.
     */
    shape_number = Facing_To_Frame_Number(this_ptr->SecondaryFacing, turret_facings);

    /**
     *  Now adjust the frame index based on the units walk frames.
     */

    /**
     *  #issue-389
     * 
     *  Allow the starting turret frame index to be defined.
     * 
     *  @author: CCHyper
     */
    if (unittypeext && unittypeext->StartTurretFrame != -1) {
        frame_number = unittypeext->StartTurretFrame + (shape_number % turret_facings);
    } else {
        frame_number = start_turret_frame + (shape_number % turret_facings);
    }

    /**
     *  The location we jump back to pushes EAX into the stack for
     *  the call to Draw_Object().
     */
    _asm { mov eax, frame_number }

    /**
     *  Restore some registers to make sure nothing got reused and all is good.
     */
    _asm { mov edi, shape }
    _asm { mov ecx, [this_ptr] }
    _asm { mov ebx, [ecx] }

    JMP_REG(edx, 0x006537AE);
}


/**
 *  #issue-334
 * 
 *  Fixes a division by zero crash when Rule->ShakeScreen is zero
 *  and a unit dies/explodes.
 * 
 *  @author: CCHyper
 */
static void UnitClass_Shake_Screen(UnitClass *unit)
{
    UnitTypeClassExtension *unittypeext;

    /**
     *  Fetch the extension instance.
     */
    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unit->Techno_Type_Class());

    /**
     *  #issue-414
     * 
     *  Can this unit shake the screen when it is destroyed?
     * 
     *  @author: CCHyper
     */
    if (unittypeext->IsShakeScreen) {

        /**
         *  If this unit has screen shake values defined, then set the blitter
         *  offset values. GScreenClass::Blit will handle the rest for us.
         */
        if ((unittypeext->ShakePixelXLo > 0 || unittypeext->ShakePixelXHi > 0)
         || (unittypeext->ShakePixelYLo > 0 || unittypeext->ShakePixelYHi > 0)) {

            if (unittypeext->ShakePixelXLo > 0 || unittypeext->ShakePixelXHi > 0) {
                Map.ScreenX = Sim_Random_Pick(unittypeext->ShakePixelXLo, unittypeext->ShakePixelXHi);
            }
            if (unittypeext->ShakePixelYLo > 0 || unittypeext->ShakePixelYHi > 0) {
                Map.ScreenY = Sim_Random_Pick(unittypeext->ShakePixelYLo, unittypeext->ShakePixelYHi);
            }

        } else {

            /**
             *  Very strong units that have an explosion will also rock the
             *  screen when they are destroyed.
             */
            if (unit->Class->MaxStrength > Rule->ShakeScreen) {

                /**
                 *  Make sure both the screen shake factor and the units strength
                 *  are valid before performing the division.
                 */
                if (Rule->ShakeScreen > 0 && unit->Class->MaxStrength > 0) {

                    int shakes = std::min<int>(unit->Class->MaxStrength / (Rule->ShakeScreen/2), 6);

                    /**
                     *  #issue-414
                     * 
                     *  Restores the vertical screen shake when a strong unit is destroyed.
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

DECLARE_PATCH(_UnitClass_Explode_ShakeScreen_Division_BugFix_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, edi);

    /**
     *  Stolen bytes/code.
     */
    _asm { pop ebx }

    UnitClass_Shake_Screen(this_ptr);

    /**
     *  Return from the function.
     */
function_return:
    JMP_REG(ecx, 0x0065B581);
}


/**
 *  #issue-#6
 * 
 *  A "quality of life" patch for harvesters so they auto harvest
 *  when they have just been kicked out of the war factory.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(TARGET, target, esi);
    static BuildingClass *building_contact;
    static UnitTypeClass *unittype;

    /**
     *  Is the unit we are processing a harvester?
     */
    unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Class_Of());
    if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {

        /**
         *  Order the unit to harvest.
         */
        this_ptr->Assign_Mission(MISSION_HARVEST);

        goto continue_check_scatter;
    }

    /**
     *  Stolen bytes/code from here on, continues function flow.
     */

    /**
     *  Find out if the target is a building. (flagged to not use dynamic_cast).
     */
continue_function:
    building_contact = Target_As_Building(target, false);

    /**
     *  This is real ugly, but we replace the dynamic_cast in the original
     *  location and we need to return to just after its stack fixup.
     */
    _asm { mov ebp, this_ptr }
    _asm { mov ecx, [ebp+0x0EC] } // this->House
    _asm { mov eax, building_contact }

    JMP_REG(edx, 0x006517DB);

continue_check_scatter:
    _asm { mov ebp, this_ptr }
    JMP_REG(ecx, 0x0065194E);
}


/**
 *  Replaces Verses (Modifier) of the Warhead with the one from the extension.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_Jellyfish_AI_Armor_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, target, esi);
    GET_STACK_STATIC(WeaponTypeClass*, weapon, esp, 0x20);
    GET_STACK_STATIC(WarheadTypeClass*, warhead, esp, 0x14);

    static int damage;
    damage = weapon->Attack * Verses::Get_Modifier(target->Techno_Type_Class()->Armor, warhead);

    _asm
    {
        mov eax, damage
        mov ebx, warhead
    }

    JMP_REG(ecx, 0x0064F2DF);
}


/**
 *  Main function for patching the hooks.
 */
void UnitClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    UnitClassExtension_Init();

    Patch_Jump(0x006517BE, &_UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch);
    Patch_Jump(0x0065B547, &_UnitClass_Explode_ShakeScreen_Division_BugFix_Patch);
    Patch_Jump(0x006530EB, &_UnitClass_Draw_Shape_Primary_Facing_Patch);
    Patch_Jump(0x006537A8, &_UnitClass_Draw_Shape_Turret_Facing_Patch);
    Patch_Jump(0x00653D7F, &_UnitClass_Draw_It_Unloading_Harvester_Patch);
    Patch_Jump(0x00654399, &_UnitClass_Mission_Unload_Transport_Detach_Sound_Patch);
    Patch_Jump(0x00653114, &_UnitClass_Draw_Shape_IdleRate_Patch);
    Patch_Jump(0x00656623, &_UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch); // IsToHarvest
    Patch_Jump(0x0065665D, &_UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch); // IsToVeinHarvest
    Patch_Jump(0x0064F2D6, &_UnitClass_Jellyfish_AI_Armor_Patch);
    //Patch_Jump(0x0065054F, &_UnitClass_Enter_Idle_Mode_Block_Harvesting_On_Bridge_Patch); // Removed, keeping code for reference.
    //Patch_Jump(0x00654AB0, &_UnitClass_Mission_Harvest_Block_Harvesting_On_Bridge_Patch); // Removed, keeping code for reference.
}
