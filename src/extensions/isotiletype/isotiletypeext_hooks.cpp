/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ISOTILETYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended IsometricTileTypeClass.
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
#include "isotiletypeext_hooks.h"
#include "isotiletypeext_init.h"
#include "isotiletype.h"
#include "isotiletypeext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "extension.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(IsometricTileTypeClass)
{
public:
    const ShapeSet * _Get_Image_Data();
};


/**
 *  Reimplementation of IsometricTileTypeClass::Get_Image_Data with added assertion.
 * 
 *  @author: CCHyper
 */
const ShapeSet * IsometricTileTypeClassExt::_Get_Image_Data()
{
    if (Image) {
        return Image;
    }

    if (IsFileLoaded) {
        Load_Image_Data();
    }
    
    if (Image == nullptr) {
        DEBUG_WARNING("IsoTile %s has NULL image data!\n", Name());
    }

    return Image;
}


/**
 *  Main function for patching the hooks.
 */
void IsometricTileTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    IsometricTileTypeClassExtension_Init();

    Patch_Jump(0x004F3570, &IsometricTileTypeClassExt::_Get_Image_Data);
}
