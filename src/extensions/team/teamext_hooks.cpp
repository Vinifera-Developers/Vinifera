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
#include "teamtype.h"
#include "cell.h"
#include "foot.h"
#include "technotype.h"
#include "iomap.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-231
 * 
 *  Fixes a bug where transports do not not return to their loading cell (home)
 *  when they have finished unloading and are flagged with "TransportsReturnOnUnload".
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TeamClass_AI_Log_Transport_Return_Patch)
{
    GET_REGISTER_STATIC(TeamClass *, this_ptr, esi);
    static Cell cell;
    static FootClass *i;

    /**
     *  Iterate over all members of this team and clear the archive cell.
     */
    for (i = this_ptr->Member; i; i = i->Member) {

        /**
         *  ...Unless its a transport that has been flagged to return after unload.
         */
        if (this_ptr->Class->TransportsReturnOnUnload && i->Techno_Type_Class()->MaxPassengers > 0) {
            if (i->ArchiveTarget) {
                cell.X = i->Coord.X / 256;
                cell.Y = i->Coord.Y / 256;
                DEBUG_INFO("Transport \"%s\" just received orders to go home to %d,%d after unloading.\n", i->Name(), cell.X, cell.Y);
#ifndef NDEBUG
                /**
                 *  #DEBUG: Jump to the position of the transport in question.
                 */
                Map.Set_Tactical_Position(i->Coord);
#endif
            }

        } else {

            /**
             *  Not a "real" transport, clear the archive cell.
             */
            i->ArchiveTarget = nullptr;
        }
    }

    /**
     *  Continue function flow.
     */
    JMP_REG(ecx, 0x0062297C);
}


/**
 *  #issue-231
 * 
 *  Fixes a bug where transports do not not return to their loading cell (home)
 *  when they have finished unloading and are flagged with "TransportsReturnOnUnload".
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TeamClass_TMission_Load_Assign_Archive_Patch)
{
    GET_STACK_STATIC(TeamClass *, this_ptr, esp, 0x10);
    GET_REGISTER_STATIC(FootClass *, trans, ebp); // Pointer to the first member of the team.
    static Cell cell;

    /**
     *  This this team is flagged that transports should return to
     *  the location they loaded at, then backup the current cell
     *  to the transport archive so it knows where to return to.
     */
    if (this_ptr->Class->TransportsReturnOnUnload) {
        trans->ArchiveTarget = (TARGET)&Map[trans->Coord];
        cell.X = trans->Coord.X / 256;
        cell.Y = trans->Coord.Y / 256;
        DEV_DEBUG_INFO("Transport \"%s\" loaded, returning to %d,%d after unloading.\n", trans->Name(), cell.X, cell.Y);
#ifndef NDEBUG
        /**
         *  #DEBUG: Jump to the position of the transport in question.
         */
        Map.Set_Tactical_Position(trans->Coord);
#endif
    }

    /**
     *  Stolen bytes/code.
     */
    this_ptr->IsNextMission = true;

    JMP_REG(ecx, 0x00625FD3);
}


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
    static CellClass* cell;
    static Cell tmpcell;

    /**
     *  Get the cell X and Y position from the script argument.
     */
    if (NewINIFormat < 4) {
        tmpcell.X = argument % 256;
        tmpcell.Y = argument / 256;
    } else {
        tmpcell.X = argument % 1000;
        tmpcell.Y = argument / 1000;
    }

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

    /**
     *  This patch writes a short jump to some padding, where we then
     *  jump from. This is because the area we need to patch is only
     *  4 bytes, and a long jump needs 5.
     */
    Patch_Word(0x00625FCF, 0x37EB);
    Patch_Jump(0x00626008, &_TeamClass_TMission_Load_Assign_Archive_Patch);

    Patch_Jump(0x00622962, &_TeamClass_AI_Log_Transport_Return_Patch);
}
