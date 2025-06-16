/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VOXELANIMEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended VoxelAnimClass.
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
#include "voxelanimext_hooks.h"
#include "voxelanim.h"
#include "voxelanimtype.h"
#include "voxelanimtypeext.h"
#include "voc.h"
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
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(VoxelAnimClass)
{
    public:
        void _Delete_Me();
};


/**
 *  Implementation of Delete_Me() for VoxelAnimClass.
 */
void VoxelAnimClassExt::_Delete_Me()
{
    /**
     *  #issue-474
     * 
     *  Implements StopSound for VoxelAnimTypes.
     * 
     *  @author: CCHyper
     */
    VoxelAnimTypeClassExtension* voxelanimtypeext = Extension::Fetch(Class);
    if (voxelanimtypeext) {

        /**
         *  Play the StopSound if one has been defined.
         */
        if (voxelanimtypeext->StopSound != VOC_NONE) {
            Static_Sound(voxelanimtypeext->StopSound, Center_Coord());
        }
    }

    ObjectClass::Delete_Me();
}


/**
 *  Main function for patching the hooks.
 */
void VoxelAnimClassExtension_Hooks()
{
    Change_Virtual_Address(0x006D9134, Get_Func_Address(&VoxelAnimClassExt::_Delete_Me));
}
