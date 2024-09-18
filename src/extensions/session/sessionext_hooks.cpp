/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SESSIONEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended SessionClass.
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
#include "sessionext_hooks.h"
#include "sessionext_init.h"
#include "sessionext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Main function for patching the hooks.
 */
void SessionClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SessionClassExtension_Init();

    /**
     *  #issue-218
     *
     *  Changes the default value of SessionClass 0x1D91 (IsGDI) from "1" to "0".. This is
     *  because we now use it as a HouseType index, and need it to default to the first index.
     */
    Patch_Byte(0x005ED06B+1, 0x85); // changes "dl" (1) to "al" (0)
}
