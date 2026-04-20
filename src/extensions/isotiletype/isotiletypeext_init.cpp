/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ISOTILETYPEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended IsometricTileTypeClass.
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

#include "always.h"

#include "extension.h"
#include "hooker.h"
#include "isotiletype.h"
#include "isotiletypeext.h"
#include "syringe.h"
#include "vinifera_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F32C4, _IsometricTileTypeClass_Constructor_Patch, 5)
{
    GET(IsometricTileTypeClass *, this_ptr, EBP); // "this" pointer.
    GET_STACK(const char *, ini_name, 0x50); // ini name.

    // IsoTileTypes are not saved to file, so this case is not required.
#if 0
    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }
#endif

    /**
     *  Create an extended class instance.
     */
    Extension::Make<IsometricTileTypeClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F33C6, _IsometricTileTypeClass_Destructor_Patch, 6)
{
    GET(IsometricTileTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<IsometricTileTypeClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F55F2, _IsometricTileTypeClass_Init_Patch, 5)
{
    LEA_STACK(CCINIClass *, ini, 0x30);

    /**
     *  Load static values.
     */
    IsometricTileTypeClassExtension::Init(*ini);

original_code:
    return 0;
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F50AE, _IsometricTileTypeClass_Read_INI_Patch_1, 0)
{
    GET(IsometricTileTypeClass *, this_ptr, EBP);
    LEA_STACK(CCINIClass *, ini, 0x30);
    LEA_STACK(/*const*/ char *, tileset_name, 0x1F8);

    /**
     *  Stolen bytes here.
     */
    R->Stack(0x84, this_ptr);
    R->Stack(0x20, this_ptr);

    /**
     *  Fetch the extension instance.
     */
    auto exttype_ptr = Extension::Fetch(this_ptr);

    /**
     *  Save the tile set name.
     */
    std::strncpy(exttype_ptr->TileSetName, tileset_name, sizeof(exttype_ptr->TileSetName) - 1);

    /**
     *  We can't read the INI right now if we're loading the game because
     *  all the other objects we may require don't exist yet.
     *  We'll read the INIs later.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  Read type class ini.
     */
    exttype_ptr->Read_INI(*ini);

original_code:
    return 0x004F50BD;
}


/**
 *  Patch for reading the extended class members from the ini instance.
 *  
 *  The game in fact uses this patched area for the tile sets that
 *  contain random variations (clear1a, clear1b etc). We need to
 *  read load the ini data here to make sure this linked variation
 *  has the same properties as the main tile set instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F53E9, _IsometricTileTypeClass_Read_INI_Patch_2, 6)
{
    GET(IsometricTileTypeClass *, this_ptr, EBP);
    LEA_STACK(CCINIClass *, ini, 0x30);
    LEA_STACK(/*const*/ char *, tileset_name, 0x1F8);

    /**
     *  Fetch the extension instance.
     */
    auto exttype_ptr = Extension::Fetch(this_ptr);

    /**
     *  Save the tile set name.
     */
    std::strncpy(exttype_ptr->TileSetName, tileset_name, sizeof(exttype_ptr->TileSetName) - 1);

    /**
     *  We can't read the INI right now if we're loading the game because
     *  all the other objects we may require don't exist yet.
     *  We'll read the INIs later.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  Read type class ini.
     */
    exttype_ptr->Read_INI(*ini);

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void IsometricTileTypeClassExtension_Init()
{

}
