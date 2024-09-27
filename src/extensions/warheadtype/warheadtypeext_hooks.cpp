/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WARHEADTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended WarheadTypeClass.
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
#include "warheadtypeext_hooks.h"
#include "warheadtypeext_init.h"
#include "warheadtypeext.h"
#include "unittype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "hooker.h"


/**
 *  Main function for patching the hooks.
 */
void WarheadTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    WarheadTypeClassExtension_Init();

    /**
     *  Skip reading verses in the vanilla function to prevent crashes when there are not enough specified.
     */
    Patch_Jump(0x0066F3F4, 0x0066F4A4);
}
