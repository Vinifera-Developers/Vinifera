/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MULTIMISSIONEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended MultiMission class.
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
#include "endgameext_hooks.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Main function for patching the hooks.
 */
void MultiMissionExtension_Hooks()
{
    /**
     *  #issue-8
     *  
     *  Fixes MultiMission "MaxPlayers" incorrectly loaded with "MinPlayers".
     * 
     *  @author: CCHyper
     */
    static const char *TEXT_MAXPLAYERS = "MaxPlayers";
    Patch_Dword(0x005EF124+1, (uintptr_t)TEXT_MAXPLAYERS); // +1 skips "push" opcode
    Patch_Dword(0x005EF5E4+1, (uintptr_t)TEXT_MAXPLAYERS); // +1 skips "push" opcode
}
