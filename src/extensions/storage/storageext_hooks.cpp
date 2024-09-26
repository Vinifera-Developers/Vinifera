/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          STORAGEEXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended StorageClass.
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
#include "storageext_hooks.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "storageext.h"


/**
 *  Main function for patching the hooks.
 */
void StorageClassExtension_Hooks()
{
    /**
     *  Patch all the methods of StorageClass to our new extension class.
     *  Operators '+' and '-' are not patched because they are not used in the game,
     *  and require us to instantiate a new class, which we cannot do
     *  (because we now store the amounts in a DVC that belongs to the owner class)
     */
    Patch_Jump(0x0060AD80, &StorageClassExt::Get_Total_Value);
    Patch_Jump(0x0060ADB0, &StorageClassExt::Get_Total_Amount);
    Patch_Jump(0x0060ADD0, &StorageClassExt::Get_Amount);
    Patch_Jump(0x0060ADE0, &StorageClassExt::Increase_Amount);
    Patch_Jump(0x0060AE00, &StorageClassExt::Decrease_Amount);
    Patch_Jump(0x0060AFA0, &StorageClassExt::First_Used_Slot);
    Patch_Jump(0x0060AE90, &StorageClassExt::operator+=);
    Patch_Jump(0x0060AF50, &StorageClassExt::operator-=);
}
