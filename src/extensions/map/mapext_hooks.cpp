/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MAPEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended MapClass.
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

#include "always.h"

#include "mapext_hooks.h"

#include "cell.h"
#include "extension.h"
#include "hooker.h"
#include "map.h"
#include "object.h"
#include "objecttype.h"
#include "sidebarext.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"
#include "syringe.h"
#include "rules.h"
#include "debughandler.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class MapClassExt final : public MapClass
{
public:
    void _Place_Down(Cell& cell, ObjectClass* object);
    void _Pick_Up(Cell& cell, ObjectClass* object);
    void _Detach(AbstractClass* target, bool all);

    // Cumulative cell counts for each radius
    static std::vector<int> RadiusCountTable;

    // Ordered list of all relative cell coordinates
    static std::vector<Cell> RadiusOffsets;

    // Vectors pointing toward the parent cell for occlusion logic
    static std::vector<Cell> OcclusionOffsets;

    static void EnsureRadiusCalculated(int targetSightRange);    
};

// Initialize static members with the values for Radius 0
std::vector<int> MapClassExt::RadiusCountTable = {1};
std::vector<Cell> MapClassExt::RadiusOffsets = {Cell(0, 0)};
std::vector<Cell> MapClassExt::OcclusionOffsets = {Cell(0, 0)};


/**
 *  Fixes a buffer overflow crash in the original MapClass::Place_Down
 *  where the game accessed the cell array before checking that the cell
 *  number is within bounds.
 *
 *  @author: Rampastring
 */
void MapClassExt::_Place_Down(Cell& cell, ObjectClass* object)
{
    if (!object)
        return;

    if (object->Class_Of()->IsFootprint && object->In_Which_Layer() == LAYER_GROUND) {
        Cell xlist[32];
        List_Copy(object->Occupy_List(), std::size(xlist), xlist);
        Cell const* list = xlist;
        while (*list != REFRESH_EOL) {
            Cell newcell = cell + *list++;
            int cellnum = newcell.As_Cell_Number();

            if (cellnum >= 0 && cellnum < MAP_CELL_TOTAL && Array[cellnum] != nullptr) {
                (*this)[newcell].Occupy_Down(object, object->IsOnBridge);
                (*this)[newcell].Recalc_Attributes();
            }
        }
    }
}


/**
 *  Fixes a buffer overflow crash in the original MapClass::Pick_Up
 *  where the game accessed the cell array before checking that the cell
 *  number is within bounds.
 *
 *  @author: Rampastring
 */
void MapClassExt::_Pick_Up(Cell& cell, ObjectClass* object)
{
    if (!object) return;

    if (object->Class_Of()->IsFootprint && object->In_Which_Layer() == LAYER_GROUND) {
        Cell xlist[32];
        List_Copy(object->Occupy_List(), std::size(xlist), xlist);
        Cell const* list = xlist;
        while (*list != REFRESH_EOL) {
            Cell newcell = cell + *list++;
            int cellnum = newcell.As_Cell_Number();

            if (cellnum >= 0 && cellnum < MAP_CELL_TOTAL && Array[cellnum] != nullptr) {
                (*this)[newcell].Occupy_Up(object, object->IsOnBridge);
                (*this)[newcell].Recalc_Attributes();
            }
        }
    }
}


/**
 *  Proxy for MapClass::Detach
 *
 *  @author: ZivDero
 */
void MapClassExt::_Detach(AbstractClass* target, bool all)
{
    MapClass::Detach(target, all);

    if (target->RTTI == RTTI_FACTORY) {
        SidebarExtension->Detach(target);
    }
}

void MapClassExt::EnsureRadiusCalculated(int targetSightRange)
{
    // 1. Check how much we have already calculated
    // Subtract 1 because the 0th index is Radius 0
    int currentMaxRadius = static_cast<int>(RadiusCountTable.size()) - 1;

    // 2. If we've already calculated this range, do nothing
    if (targetSightRange <= currentMaxRadius) {
        return;
    }

    // 3. Iteratively calculate every new radius shell required
    for (int currentRadius = currentMaxRadius + 1; currentRadius <= targetSightRange; ++currentRadius) {

        // We define a search area large enough to capture all octagonal corners.
        // Octagonal distance grows slower than Manhattan, so R+2 is a safe bounds.
        int searchBounds = currentRadius + 2;

        for (int cellY = -searchBounds; cellY <= searchBounds; ++cellY) {
            for (int cellX = -searchBounds; cellX <= searchBounds; ++cellX) {

                int absoluteX = std::abs(cellX);
                int absoluteY = std::abs(cellY);

                // The Octagonal Distance formula:
                // This creates the 8-sided shape used by the game engine.
                int octagonalDistance = std::max(absoluteX, absoluteY) + (std::min(absoluteX, absoluteY) / 2);

                // 4. If this cell's distance matches the current shell we are building
                if (octagonalDistance == currentRadius) {

                    // Add the relative coordinate to our offset list
                    RadiusOffsets.push_back(Cell(cellX, cellY));

                    // 5. Calculate Occlusion (Line of Sight parenting)
                    // The occlusion vector points from the current cell back toward the origin.
                    // If a cell at (5, 5) is occluded, the game looks at (5-1, 5-1) to see if it's blocked.
                    short parentDirectionX = (cellX == 0) ? 0 : (cellX > 0 ? -1 : 1);
                    short parentDirectionY = (cellY == 0) ? 0 : (cellY > 0 ? -1 : 1);

                    OcclusionOffsets.push_back(Cell(parentDirectionX, parentDirectionY));
                }
            }
        }

        // 6. Finalize the shell
        // Record the current total size of the offsets list.
        // This allows the game to know exactly where one radius ends and the next begins.
        RadiusCountTable.push_back(static_cast<int>(RadiusOffsets.size()));
    }
}

/**
 * Hook at 0x510C9A: The entry point for calculating Fog of War reveal parameters.
 * Original instruction: cmp eax, 0Ah (Checks if sight > 10)
 */
DEFINE_HOOK(0x510C9A, MapClass_Reveal_Dynamic, 6)
{
    // EAX contains the sightrange of the unit currently being processed
    int sightRange = R->EAX();

    // Retrieve the 'incremental' flag from the stack [esp + 0x58]
    // Incremental = true means the unit is moving and only needs to reveal the "new" edge.
    GET_STACK(bool, isIncremental, 0x58);

    // 1. Dynamic Generation
    // Ensure our static vectors are large enough for this specific unit's sight
    MapClassExt::EnsureRadiusCalculated(sightRange);

    // 2. Data Preparation
    // By default, we assume we are revealing the full circle (Radius 0 to sightRange)
    int cellCount = MapClassExt::RadiusCountTable[sightRange];
    Cell* radiusPointer = MapClassExt::RadiusOffsets.data();
    Cell* occlusionPointer = MapClassExt::OcclusionOffsets.data();

    // 3. Incremental Logic
    // If Rule->IsRevealByHeight is false, and it's an incremental update,
    // we skip all inner cells and only provide the pointers for the outermost shell.
    if (!Rule->IsRevealByHeight && isIncremental && sightRange > 2) {

        // The number of cells in all shells BEFORE the current one
        int previousShellsTotal = MapClassExt::RadiusCountTable[sightRange - 3];

        // Subtract inner count: we only want to process the difference
        cellCount -= previousShellsTotal;

        // Offset the pointers so the game starts reading at the beginning of the new shell
        radiusPointer += previousShellsTotal;
        occlusionPointer += previousShellsTotal;
    }

    // 4. Register and Stack Cleanup
    // ESI must contain the 'count' for the reveal loop
    R->ESI(cellCount);

    // The game expects the specific 'ptr' and 'coord' variables to be set on the stack
    R->Stack<Cell*>(0x10, radiusPointer);    // [esp+48h+ptr]
    R->Stack<Cell*>(0x4C, occlusionPointer); // [esp+48h+coord]

    // 5. Flow Control
    // Return the address of loc_510CFB to skip the original hardcoded array lookups
    return 0x510CFB;
}


/**
 *  Main function for patching the hooks.
 */
void MapClassExtension_Hooks()
{
    Patch_Jump(0x00511070, &MapClassExt::_Place_Down);
    Patch_Jump(0x005111B0, &MapClassExt::_Pick_Up);
    Patch_Call(0x00648BCF, &MapClassExt::_Detach);
    Patch_Call(0x00648B67, &MapClassExt::_Detach);
}
