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
#include "house.h"
#include "housetype.h"
#include "bsurface.h"
#include "dsurface.h"
#include "convert.h"
#include "drawshape.h"
#include "rules.h"
#include "voc.h"
#include "iomap.h"
#include "spritecollection.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-531
 * 
 *  This patch allows buildings with the IsNavalYard property to be assigned
 *  as "Primary" seperate from other UnitType factories.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingClass_Toggle_Primary_NavalYard_Check_Patch)
{
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    GET_REGISTER_STATIC(BuildingClass *, building, eax);
    static BuildingTypeClassExtension *this_buildingtypeext;
    static BuildingTypeClassExtension *i_buildingtypeext;

    /**
     *  Find the type class extension instance.
     */
    this_buildingtypeext = BuildingTypeClassExtensions.find(this_ptr->Class);
    i_buildingtypeext = BuildingTypeClassExtensions.find(building->Class);

    /**
     *  If we found another naval yard, remove its leadership status so we can
     *  assign it to ourself.
     */
    if (this_buildingtypeext && i_buildingtypeext
     && this_buildingtypeext->IsNavalYard && i_buildingtypeext->IsNavalYard) {

        building->IsLeader = false;

    } else {
        building->IsLeader = false;
    }

continue_loop:
    JMP(0x0042F61A);
}


/**
 *  #issue-531
 * 
 *  This patch allows buildings with the IsNavalYard property to place rally
 *  points on water rather than on land.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingClass_Set_Rally_To_Point_NavalYard_Check_Patch)
{
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    GET_REGISTER_STATIC(SpeedType, speed, ebx);
    GET_REGISTER_STATIC(MZoneType, mzone, edi);
    static BuildingTypeClassExtension *buildingtypeext;
    static BuildingTypeClass *buildingtype;

    buildingtype = this_ptr->Class;

    /**
     *  Stolen bytes/code.
     */
    if (buildingtype->ToBuild == RTTI_AIRCRAFTTYPE) {
        speed = SPEED_WINGED;
        mzone = MZONE_FLYER;

    } else {

        /**
         *  Find the type class extension instance.
         */
        buildingtypeext = BuildingTypeClassExtensions.find(buildingtype);
        if (buildingtypeext) {

            /**
             *  If this is a factory that produces units, but is flagged as a shipyard, then
             *  change the zone flags to scan for water regions only.
             */
            if (buildingtype->ToBuild == RTTI_UNITTYPE && buildingtypeext->IsNavalYard) {
                speed = SPEED_AMPHIBIOUS;
                mzone = MZONE_AMPHIBIOUS_DESTROYER;
            }

        }

    }

    /**
     *  Assign the zone flags to the expected registers.
     */
    _asm { mov eax, speed }
    _asm { mov dword ptr [esp+0x14], eax }

    _asm { mov edi, mzone }

    JMP(0x0042C38E);
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
     *  Find the extension instances.
     */
    ext_ptr = BuildingClassExtensions.find(this_ptr);
    if (ext_ptr) {
        ext_ptr->Produce_Cash_AI();
    }

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
     *  Find the extension instances.
     */
    ext_ptr = BuildingClassExtensions.find(this_ptr);
    exttype_ptr = BuildingTypeClassExtensions.find(this_ptr->Class);
    if (!ext_ptr || !exttype_ptr) {
        goto original_code;
    }

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
     *  Find the extension instances.
     */
    ext_ptr = BuildingClassExtensions.find(this_ptr);
    exttype_ptr = BuildingTypeClassExtensions.find(this_ptr->Class);
    if (!ext_ptr || !exttype_ptr) {
        goto continue_function;
    }

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
     *  Fetch the class extension if it exists.
     */
    buildingtypeext = BuildingTypeClassExtensions.find(buildingtype);
    if (buildingtypeext) {

        /**
         *  Does this building have a custom gate lowering sound? If so, use it.
         */
        if (buildingtypeext->GateDownSound != VOC_NONE) {
            voc = buildingtypeext->GateDownSound;
        }
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
     *  Fetch the class extension if it exists.
     */
    buildingtypeext = BuildingTypeClassExtensions.find(buildingtype);
    if (buildingtypeext) {

        /**
         *  Does this building have a custom gate rising sound? If so, use it.
         */
        if (buildingtypeext->GateUpSound != VOC_NONE) {
            voc = buildingtypeext->GateUpSound;
        }
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
    TechnoTypeClass *technotype;
    TechnoTypeClassExtension *technotypeext;

    /**
     *  Fetch the extended techno type instance if it exists.
     */
    technotype = building->Techno_Type_Class();
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
         *  If this building has screen shake values defined, then set the blitter
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
    technotypeext = TechnoTypeClassExtensions.find(technotype);
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
    Patch_Jump(0x0042C37F, &_BuildingClass_Set_Rally_To_Point_NavalYard_Check_Patch);
    Patch_Jump(0x0042F614, &_BuildingClass_Toggle_Primary_NavalYard_Check_Patch);
}
