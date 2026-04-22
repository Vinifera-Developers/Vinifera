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

    /**
     * Dynamically generates octagonal distance data up to the requested range.
     * Uses the Octagonal Distance formula: D = max(|x|, |y|) + floor(min(|x|, |y|) / 2)
     */
    static void EnsureRadiusCalculated(int targetSightRange)
    {
        int currentMaxRadius = static_cast<int>(RadiusCountTable.size()) - 1;

        if (targetSightRange <= currentMaxRadius) {
            return;
        }

        for (int currentRadius = currentMaxRadius + 1; currentRadius <= targetSightRange; ++currentRadius) {
            // We search in a bounding box slightly larger than the radius to find all octagonal cells
            int searchBounds = currentRadius + 2;

            for (int cellY = -searchBounds; cellY <= searchBounds; ++cellY) {
                for (int cellX = -searchBounds; cellX <= searchBounds; ++cellX) {
                    int absoluteX = std::abs(cellX);
                    int absoluteY = std::abs(cellY);

                    // The octagonal distance calculation
                    int octagonalDistance = std::max(absoluteX, absoluteY) + (std::min(absoluteX, absoluteY) / 2);

                    if (octagonalDistance == currentRadius) {
                        RadiusOffsets.push_back(Cell(cellX, cellY));

                        // Occlusion logic: the vector pointing back to the origin
                        short parentDirectionX = (cellX == 0) ? 0 : (cellX > 0 ? -1 : 1);
                        short parentDirectionY = (cellY == 0) ? 0 : (cellY > 0 ? -1 : 1);
                        OcclusionOffsets.push_back(Cell(parentDirectionX, parentDirectionY));
                    }
                }
            }
            // Update the prefix sum table with the new total cell count
            RadiusCountTable.push_back(static_cast<int>(RadiusOffsets.size()));
        }
    }
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

std::vector<Cell> getRadiusOffsets(int radius)
{
    std::vector<Cell> offsets;

    // The maximum possible coordinate for a given radius R is R.
    // We iterate through the bounding box of the potential octagon.
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            int absX = std::abs(x);
            int absY = std::abs(y);

            // Octagonal distance formula
            int dist = std::max(absX, absY) + (std::min(absX, absY) / 2);

            if (dist == radius) {
                offsets.push_back(Cell(x, y));
            }
        }
    }
    return offsets;
}

// Hook at: .text:00510C9A (cmp eax, 0Ah)
DEFINE_HOOK(0x510C9A, MapClass_Reveal_Dynamic, 6)
{
    DEBUG_INFO("ENTERED SIGHT LOOKUP\n");

    // EAX contains the current sightrange being processed
    int sightRange = R->EAX();
    GET_STACK(bool, isIncremental, 0x58);

    // Ensure the data for this sightrange exists in our static cache
    MapClassExt::EnsureRadiusCalculated(sightRange);

    // Initial pointers point to the start of our dynamic buffers
    int cellCount = MapClassExt::RadiusCountTable[sightRange];
    Cell* radiusPointer = MapClassExt::RadiusOffsets.data();
    Cell* occlusionPointer = MapClassExt::OcclusionOffsets.data();

    // Incremental reveal logic
    // If not reveal-by-height, only reveal the new "shell" of cells
    if (!Rule->IsRevealByHeight && isIncremental && sightRange > 2) {
        int previousShellCellCount = MapClassExt::RadiusCountTable[sightRange - 3];

        cellCount -= previousShellCellCount;
        radiusPointer += previousShellCellCount;
        occlusionPointer += previousShellCellCount;
    }

    // Write results back to the registers and stack for the game's loop
    R->ESI(cellCount);
    R->Stack<Cell*>(0x10, radiusPointer);    // [esp+48h+ptr]
    R->Stack<Cell*>(0x4C, occlusionPointer); // [esp+48h+coord]

    // Jump to loc_510CFB to resume execution after the patched block
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
