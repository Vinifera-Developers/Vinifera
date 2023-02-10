/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TechnoTypeClass.
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
#include "technotypeext_hooks.h"
#include "technotypeext.h"
#include "technotype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-90
 *
 *  Disables the bugged bonus range for arcing projectiles.
 *
 *  Author: Rampastring
 */
DECLARE_PATCH(_TechnoTypeClass_In_Range_Disable_Arcing_Bonus_Range_Patch)
{
    JMP(0x0063D6AA);
}


/**
 *  Main function for patching the hooks.
 */
void TechnoTypeClassExtension_Hooks()
{
    Patch_Jump(0x0063D5A7, &_TechnoTypeClass_In_Range_Disable_Arcing_Bonus_Range_Patch);
}
