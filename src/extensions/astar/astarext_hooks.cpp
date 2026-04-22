/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ASTAREXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended AStarClass.
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
#include "astarext_hooks.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "rules.h"


/**
 *  Main function for patching the hooks.
 */
void AStarClassExtension_Hooks()
{
    // Patch some pathfinding related arrays to fix pathfinding crashes on very large maps with complex paths

    // Double the size of RegularQueue
    Patch_Byte(0x0041B53F + 3, 0x08);
    Patch_Byte(0x0041B550 + 5, 0x02);

    // Double the size of HierQueue
    Patch_Byte(0x0041B591 + 1, 0x84);  // Allocation size 0x9C44 -> 0x13884 (40,004 -> 80,004 bytes)
    Patch_Byte(0x0041B591 + 2, 0x38);
    Patch_Byte(0x0041B591 + 3, 0x01);
    Patch_Byte(0x0041B5A2 + 3, 0x20);  // Capacity 0x2710 -> 0x4E20 (10,000 -> 20,000)
    Patch_Byte(0x0041B5A2 + 4, 0x4E);

    // Double size of RegularOpenNodePool
    Patch_Byte(0x0041B5D0 + 3, 0x20);
    Patch_Byte(0x0041B5E7 + 3, 0x02);
    Patch_Byte(0x0041B5FA + 4, 0x20);

    // Double the size of ???
    Patch_Byte(0x0041B604 + 3, 0x30);
    Patch_Byte(0x0041B618 + 4, 0x30);
    Patch_Byte(0x0041B625 + 4, 0x30);
    Patch_Byte(0x0041B630 + 4, 0x20);

    // Double the size of HierQueueHeap
    Patch_Byte(0x0041B6D0 + 3, 0x04);
    Patch_Byte(0x0041B6D0 + 2, 0xE2);

    /**
     *  AStarClass::Clear - update sentinel offsets to match doubled buffer sizes.
     */
    Patch_Byte(0x0041B3A9 + 4, 0x30);  // [eax+0x180000] -> [eax+0x300000]
    Patch_Byte(0x0041B3B3 + 4, 0x20);  // [edx+0x100000] -> [edx+0x200000]

    /**
     *  AStarClass::Create_Node - update count/sentinel offsets.
     */
    Patch_Byte(0x0041B256 + 4, 0x20);  // [eax+0x100000] read  -> [eax+0x200000]
    Patch_Byte(0x0041B269 + 4, 0x20);  // [eax+0x100000] write -> [eax+0x200000]
    Patch_Byte(0x0041B272 + 4, 0x30);  // [eax+0x180000] read  -> [eax+0x300000]
    Patch_Byte(0x0041B27C + 4, 0x30);  // [eax+0x180000] write -> [eax+0x300000]

    /**
     *  AStarClass::Find_Path_Hierarchical - fix sign extension of unsigned
     *  16-bit zone/region indices. Values >= 32768 would become negative
     *  when sign-extended, producing wild array indices and crashes.
     *  Change movsx (0F BF) to movzx (0F B7).
     */
    Patch_Byte(0x0041D133, 0xB7);  // movsx -> movzx word at 0x0041D132
    Patch_Byte(0x0041D153, 0xB7);  // movsx -> movzx word at 0x0041D152
}
