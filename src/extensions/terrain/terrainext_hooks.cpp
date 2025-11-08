/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TERRAINEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TerrainClass.
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
#include "terrainext_hooks.h"
#include "terrainext_init.h"
#include "terrainext.h"
#include "terraintypeext.h"
#include "terrain.h"
#include "terraintype.h"
#include "lightsource.h"
#include "vinifera_util.h"
#include "extension.h"
#include "scenario.h"
#include "mouse.h"
#include "fatal.h"
#include "rules.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "syringe.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(TerrainClass)
{
public:
    void _AI();
};


/**
 *  Replacement for TerrainClass::AI.
 *
 *  @author: ZivDero
 */
void TerrainClassExt::_AI()
{
    ObjectClass::AI();

    if (Class->IsAnimated) {
        if (Fetch_Rate() == 0) {
            if (Probability_Of(Class->AnimationProbability)) {
                Set_Stage(0);
                Set_Rate(Class->AnimationRate);
            }
        }
    }

    if (StageClass::Graphic_Logic()) {

        /**
         *  If the terrain object is in the process of crumbling, then when at the
         *  last stage of the crumbling animation, delete the terrain object.
         */
        if (IsCrumbling && Fetch_Stage() == (((ShapeSet const*)Class->Get_Image_Data())->Get_Count()) - 1) {
            Delete_Me();
            return;
        }

        if (Class->IsTiberiumSpawn && Class->IsAnimated && Fetch_Stage() == (((ShapeSet const*)Class->Get_Image_Data())->Get_Count() / 2)) {
            Set_Stage(0);
            Set_Rate(0);
            Extension::Fetch(this)->Spread_Tiberium();
        }
    }

    if (IsOnFire) {
        if (abs(Scen->RandomNumber()) % 100 == 0) {
            CellClass& cellptr = Map[Get_Coord()];
            for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
                TerrainClass* terrain = cellptr.Adjacent_Cell(facing).Cell_Terrain();
                if (terrain && !terrain->IsOnFire && Probability_Of2(Rule->TreeFlammability)) {
                    terrain->Catch_Fire();
                }
            }
        }
    }
}


/**
 *  Create a light source instances for terrain object.
 * 
 *  @author: CCHyper
 */
static LightSourceClass *Terrain_New_LightSource(TerrainClass *this_ptr)
{
    if (!this_ptr) {
        return nullptr;
    }
    
    TerrainClassExtension *terrainext;
    TerrainTypeClassExtension *terraintypeext;
    LightSourceClass *light;

    /**
     *  Fetch the extension instance.
     */
    terrainext = Extension::Fetch(this_ptr);
    terraintypeext = Extension::Fetch(this_ptr->Class);

    /**
     *  Create the light source object at the terrain coord.
     */
    light = new LightSourceClass(
                    this_ptr->Center_Coord(),
                    terraintypeext->LightVisibility,
                    terraintypeext->LightIntensity,
                    terraintypeext->LightRedTint,
                    terraintypeext->LightGreenTint,
                    terraintypeext->LightBlueTint
                );
    ASSERT(light != nullptr);

    return light;
}


/**
 *  #issue-452
 * 
 *  Create the light source object when the terrain is placed
 *  into the game world.
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_TerrainClass_Unlimbo_LightSource_Patch)
{
    GET(TerrainClass *, this_ptr, EDI);

    TerrainTypeClass* terraintype = this_ptr->Class;

    /**
     *  Fetch the extension instances.
     */
    TerrainClassExtension* terrainext = Extension::Fetch(this_ptr);
    TerrainTypeClassExtension* terraintypeext = Extension::Fetch(terraintype);

    if (terraintypeext->IsLightEnabled && terraintypeext->LightIntensity > 0) {

        if (!terrainext->LightSource) {

            /**
             *  Create the light source object.
             */
            LightSourceClass* light = Terrain_New_LightSource(this_ptr);

            if (light) {
                terrainext->LightSource = light;

                /**
                 *  Enable the light source.
                 */
                terrainext->LightSource->Enable();
            }

        }

    }

function_return:
    return 0;
}


/**
 *  #issue-452
 * 
 *  Disable the light source object when the terrain object is destroyed.
 * 
 *  #NOTE: This patch is within and at the end of the RESULT_DESTROYED (4) branch
 *         returned from ObjectClass::Take_Damage().
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_TerrainClass_Take_Damage_LightSource_Patch)
{
    GET(TerrainClass *, this_ptr, ESI);

    /**
     *  Fetch the extension instance.
     */
    TerrainClassExtension* terrainext = Extension::Fetch(this_ptr);
    if (terrainext->LightSource) {

        /**
         *  This terrain object was destroyed, disable the attached lighting.
         */
        terrainext->LightSource->Disable();
    }

    /**
     *  Stolen bytes/code
     */
    this_ptr->Detach_All(true);
    this_ptr->Delete_Me();

    /**
     *  Function return.
     */
    return 0x0063F4EF;
}


/**
 *  Main function for patching the hooks.
 */
void TerrainClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TerrainClassExtension_Init();

    Patch_Jump(0x0063FFB0, &TerrainClassExt::_AI);
}

declhook(0x006409C3, _TerrainClass_Unlimbo_LightSource_Patch, 0x7);
declhook(0x0063F4D9, _TerrainClass_Take_Damage_LightSource_Patch, 0x6);
