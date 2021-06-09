/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEAMEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TeamClass.
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
#include "teamext_hooks.h"
#include "team.h"
#include "cell.h"
#include "iomap.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-196
 * 
 *  Fixes incorrect cell calculation for the MOVECELL script.
 * 
 *  The original code used outdated code from Red Alert to calculate
 *  the cell position on the map.
 * 
 *  @author: CCHyper (based on research by E1Elite)
 */
DECLARE_PATCH(_TeamClass_AI_MoveCell_FixCellCalc_Patch)
{
    GET_STACK_STATIC(unsigned, argument, esp, 0x24);
    static CellClass *cell;
    static Cell tmpcell;

    /**
     *  Get the cell X and Y position from the script argument.
     */
    tmpcell.X = argument % 1000;
    tmpcell.Y = argument / 1000;

    /**
     *  Fetch the map cell. Added pointer check to make sure the
     *  script didn't have an invalid position.
     */
    cell = &Map[tmpcell];
    if (!cell) {
        goto coordinate_move;
    }

    /**
     *  The Assign_Mission_Target call pushes EAX into the stack
     *  for the cell argument.
     */
    _asm { mov eax, cell }

assign_mission_target:
    JMP_REG(ecx, 0x00622B5F);

coordinate_move:
    JMP(0x00622B19);
}


/**
 *  Main function for patching the hooks.
 */
void TeamClassExtension_Hooks()
{
    Patch_Jump(0x00622B2C, &_TeamClass_AI_MoveCell_FixCellCalc_Patch);
}
