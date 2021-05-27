/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUGFIX_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for all bug fixes.
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
#include "bugfix_hooks.h"
#include "bugfixes.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-244
 * 
 *  Changes the default value of "AllowHiResModes" to "true".
 * 
 *  @author: CCHyper
 */
static void _OptionsClass_Constructor_AllowHiResModes_Default_Patch()
{
    Patch_Byte(0x005899D6+1, 0x50); // "cl" (zero) to "dl" (1)
}


/**
 *  Main function for patching the hooks.
 */
void BugFix_Hooks()
{
    _OptionsClass_Constructor_AllowHiResModes_Default_Patch();
}
