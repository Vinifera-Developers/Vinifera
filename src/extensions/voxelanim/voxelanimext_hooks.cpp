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

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class VoxelAnimClassFake final : public VoxelAnimClass
{
    public:
        void _entry_E4();
};


/**
 *  Implementation of entry_E4() for VoxelAnimClass.
 */
void VoxelAnimClassFake::_entry_E4()
{
    VoxelAnimTypeClassExtension *voxelanimtypeext = nullptr;

    /**
     *  #issue-474
     * 
     *  Implements StopSound for VoxelAnimTypes.
     * 
     *  @author: CCHyper
     */
    voxelanimtypeext = VoxelAnimTypeClassExtensions.find(Class);
    if (voxelanimtypeext) {

        /**
         *  Play the StopSound if one has been defined.
         */
        if (voxelanimtypeext->StopSound != VOC_NONE) {
            Sound_Effect(voxelanimtypeext->StopSound, Center_Coord());
        }
    }

    ObjectClass::entry_E4();
}


/**
 *  Main function for patching the hooks.
 */
void VoxelAnimClassExtension_Hooks()
{
    Change_Virtual_Address(0x006D9134, Get_Func_Address(&VoxelAnimClassFake::_entry_E4));
}
