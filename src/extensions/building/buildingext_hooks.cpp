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
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "building.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "dsurface.h"
#include "convert.h"
#include "drawshape.h"
#include "rules.h"
#include "voc.h"
#include "iomap.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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

DECLARE_PATCH(_BuildingClass_Explode_ShakeScreen_Division_BugFix_Patch)
{
    GET_REGISTER_STATIC(BuildingClass *, this_ptr, esi);
    static int shakes;

    BuildingClass_Shake_Screen(this_ptr);

    /**
     *  Continue execution of function.
     */
continue_function:
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
    static const ShapeFileStruct *cameo_shape;

    technotype = factory_obj->Techno_Type_Class();
    cameo_shape = technotype->Get_Cameo_Data();

    /**
     *  Draw the cameo shape.
     * 
     *  Original code used NormalDrawer, which is the old Red Alert shape
     *  drawer, so we need to use CameoDrawer here for the correct palette.
     */
    CC_Draw_Shape(TempSurface, CameoDrawer, cameo_shape, 0, pos_xy, window_rect, ShapeFlagsType(SHAPE_CENTER|SHAPE_400|SHAPE_ALPHA|SHAPE_NORMAL));

    JMP(0x00428B13);
}


/**
 *  Main function for patching the hooks.
 */
void BuildingClassExtension_Hooks()
{
    Patch_Jump(0x00428AD3, &_BuildingClass_Draw_Spied_Cameo_Palette_Patch);
    Patch_Jump(0x0042B250, &_BuildingClass_Explode_ShakeScreen_Division_BugFix_Patch);
    Patch_Jump(0x00433BB5, &_BuildingClass_Mission_Open_Gate_Open_Sound_Patch);
    Patch_Jump(0x00433C6F, &_BuildingClass_Mission_Open_Gate_Close_Sound_Patch);
}
