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
#include "cell.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "vinifera_util.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-624
 * 
 *  Implements SpawnsTiberiumType for TerrainTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_AI_Spread_Tiberium_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi);
    GET_REGISTER_STATIC(CellClass *, cell, eax);
    static TerrainTypeClassExtension *terraintypeext;
    static TiberiumType tiberium;

    /**
     *  Fixup stack, as we are replacing a call to CellClass::Spread_Tiberium,
     *  but not the args pushed to the original call.
     */
    _asm { add esp, 0x4 }

    /**
     *  Fetch the tiberium type override if set.
     */
    terraintypeext = TerrainTypeClassExtensions.find(this_ptr->Class);
    if (terraintypeext) {
        tiberium = terraintypeext->SpawnsTiberiumType;

    /**
     *  Original code (which should be TIBERIUM_RIPARIUS).
     */
    } else {
        tiberium = TIBERIUM_FIRST;
    }

    /**
     *  Final check to make sure the tiberium is within expected range.
     */
    if (tiberium == TIBERIUM_NONE || tiberium > Tiberiums.Count()) {
        tiberium = TIBERIUM_FIRST;
    }

    cell->Spread_Tiberium(tiberium, true);

    JMP(0x0064010F);
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
     *  Fetch the extended class instances if they exist.
     */
    terrainext = TerrainClassExtensions.find(this_ptr);
    terraintypeext = TerrainTypeClassExtensions.find(this_ptr->Class);

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
DECLARE_PATCH(_TerrainClass_Unlimbo_LightSource_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, edi);
    static TerrainClassExtension *terrainext;
    static TerrainTypeClassExtension *terraintypeext;
    static TerrainTypeClass *terraintype;
    static LightSourceClass *light;

    terraintype = this_ptr->Class;

    /**
     *  Fetch the extended class instances if they exist.
     */
    terrainext = TerrainClassExtensions.find(this_ptr);
    terraintypeext = TerrainTypeClassExtensions.find(terraintype);

    if (terraintypeext && terraintypeext->IsLightEnabled && terraintypeext->LightIntensity > 0) {

        if (terrainext && !terrainext->LightSource) {

            /**
             *  Create the light source object.
             */
            light = Terrain_New_LightSource(this_ptr);

            if (light) {
                terrainext->LightSource = light;

                /**
                 *  Enable the light source.
                 */
                terrainext->LightSource->Enable();
            }

        }

    }

    /**
     *  Function return.
     */
function_return:
    _asm { mov al, 1 }
    _asm { pop edi }
    _asm { pop esi }
    _asm { add esp, 0x10 }
    _asm { ret 0x8 }
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
DECLARE_PATCH(_TerrainClass_Take_Damage_LightSource_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi);
    static TerrainClassExtension *terrainext;

    /**
     *  Fetch the extended class instance if it exists.
     */
    terrainext = TerrainClassExtensions.find(this_ptr);
    if (terrainext && terrainext->LightSource) {

        /**
         *  This terrain object was destroyed, disable the attached lighting.
         */
        terrainext->LightSource->Disable();
    }

    /**
     *  Stolen bytes/code
     */
    this_ptr->Detach_All(true);
    this_ptr->entry_E4();

    /**
     *  Function return.
     */
    JMP(0x0063F4EF);
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

    Patch_Jump(0x006409C3, &_TerrainClass_Unlimbo_LightSource_Patch);
    Patch_Jump(0x0063F4D9, &_TerrainClass_Take_Damage_LightSource_Patch);
    Patch_Jump(0x00640108, &_TerrainClass_AI_Spread_Tiberium_Patch);
}
