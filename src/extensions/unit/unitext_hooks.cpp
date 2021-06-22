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
#include "tibsun_inline.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "unittypeext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "unit.h"
#include "unittype.h"
#include "target.h"
#include "rules.h"
#include "iomap.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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
    //unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Class_Of());
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

    unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Class_Of());
    
    /**
     *  All turrets have 32 facings in Tiberian Sun.
     */
    turret_facings = 32;

    /**
     *  Turret frames start directly after the facing frames.
     */
    start_turret_frame = unittype->Facings;

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
    unittypeext = UnitTypeClassExtensions.find(unittype);
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
    TechnoTypeClass *technotype;
    TechnoTypeClassExtension *technotypeext;

    /**
     *  Fetch the extended techno type instance if it exists.
     */
    technotype = unit->Techno_Type_Class();
    technotypeext = TechnoTypeClassExtensions.find(technotype);

    /**
     *  #issue-414
     * 
     *  Can this unit shake the screen when it is destroyed?
     * 
     *  @author: CCHyper
     */
    if (technotypeext && technotypeext->IsShakeScreen) {

        /**
         *  If this unit has screen shake values defined, then set the blitter
         *  offset values. GScreenClass::Blit will handle the rest for us.
         */
        if ((technotypeext->ShakePixelXLo > 0 || technotypeext->ShakePixelXHi > 0)
         || (technotypeext->ShakePixelYLo > 0 || technotypeext->ShakePixelYHi > 0)) {

            if (technotypeext->ShakePixelXLo > 0 || technotypeext->ShakePixelXHi > 0) {
                Map.ScreenX = Sim_Random_Pick(technotypeext->ShakePixelXLo, technotypeext->ShakePixelXHi);
            }
            if (technotypeext->ShakePixelYLo > 0 || technotypeext->ShakePixelYHi > 0) {
                Map.ScreenY = Sim_Random_Pick(technotypeext->ShakePixelYLo, technotypeext->ShakePixelYHi);
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
 *  Main function for patching the hooks.
 */
void UnitClassExtension_Hooks()
{
    Patch_Jump(0x006517BE, &_UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch);
    Patch_Jump(0x0065B547, &_UnitClass_Explode_ShakeScreen_Division_BugFix_Patch);
    Patch_Jump(0x006530EB, &_UnitClass_Draw_Shape_Primary_Facing_Patch);
    Patch_Jump(0x006537A8, &_UnitClass_Draw_Shape_Turret_Facing_Patch);
}
