/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TERRAINEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended TerrainClass.
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
#include "terrainext.h"
#include "terrain.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "vinifera_globals.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "syringe.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_TerrainClass_Default_Constructor_Patch)
{
    GET(TerrainClass *, this_ptr, ESI); // Current "this" pointer.

    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  Create an extended class instance.
     */
    Extension::Make<TerrainClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  We need do this before the unlimbo otherwise finding extension data
 *  will fail if you patch Unlimbo.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_TerrainClass_Constructor_Patch)
{
    GET(TerrainClass *, this_ptr, ESI); // Current "this" pointer.

    /**
     *  Create an extended class instance.
     */
    Extension::Make<TerrainClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_TerrainClass_Scalar_Destructor_Patch)
{
    GET(TerrainClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<TerrainClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void TerrainClassExtension_Init()
{

}

declhook(0x0063F88C, _TerrainClass_Default_Constructor_Patch, 0x5);
declhook(0x0063F556, _TerrainClass_Constructor_Patch, 0x7);
declhook(0x00640C3D, _TerrainClass_Scalar_Destructor_Patch, 0x6);
