/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CDEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended CD class.
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
#include "cdext_hooks.h"
#include "cd.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-513
 * 
 *  Patch to add check for CD::IsFilesLocal in CD::Is_Available
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CD_Is_Available_Local_Files_Patch)
{
	GET_REGISTER_STATIC(CD *, this_ptr, ecx);
	GET_REGISTER_STATIC(DiskID, disk, eax);
	static bool retval;

	/**
	 *  If the CD system has been flagged that the files are local, then
	 *  return true as they are always available.
	 */
	if (CD::IsFilesLocal) {
		retval = true;
		goto function_return;
	}

	/**
	 *  Stolen bytes/code.
	 */
	this_ptr->ThemePlaying = THEME_NONE;

	retval = this_ptr->Force_Available(disk);

function_return:
	_asm { mov al, retval }
	_asm { ret 4 }
}


/**
 *  Main function for patching the hooks.
 */
void CDExtension_Hooks()
{
	Patch_Jump(0x0044E7AE, &_CD_Is_Available_Local_Files_Patch);
}
