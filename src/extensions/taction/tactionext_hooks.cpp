/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TriggerClass.
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
#include "tactionext_hooks.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Main function for patching the hooks.
 */
void TActionClassExtension_Hooks()
{
    /**
     *  #issue-674
     * 
     *  Fixes a bug where the game would crash when TACTION_WAKEUP_GROUP was
     *  executed but the game was not able to match the Group to the triggers
     *  group. This was because the game was searching the Foots vector with
     *  the count of the Technos vector, and in cases where the Group did
     *  not match, the game would crash trying to search out of bounds.
     * 
     *  @author: CCHyper
     */
    Patch_Dword(0x00619552+2, (0x007E4820+4)); // Foot vector to Technos vector.
}
