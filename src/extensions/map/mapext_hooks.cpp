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
    static void _CalculateSightRadiusIfNeeded(int targetSightRange);    
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

/**
 * Dynamically populates the radius and occlusion tables if they are not populated for a given radius (sight range).
 * This allows any desired radius of cells to be revealed when called through MapClass::From_Sight.
 * 
 * @author: JoyfulShush
 */
void MapClassExt::_CalculateSightRadiusIfNeeded(int sightRange)
{
    // 1. Identify current progress    
    int currentMaxRadius = static_cast<int>(RadiusCountTable.size());

    // 2. Skip if already calculated
    // The table starts with Radius 0 (size 1), so max radius is size - 1.
    if (sightRange <= currentMaxRadius - 1) {
        return;
    }

    // 3. Generate new shells sequentially
    for (int currentRadius = currentMaxRadius; currentRadius <= sightRange; ++currentRadius) {

        // Define search bounds slightly larger than radius to ensure we find all octagonal corners
        int searchBounds = currentRadius + 2;

        for (int cellY = -searchBounds; cellY <= searchBounds; ++cellY) {
            for (int cellX = -searchBounds; cellX <= searchBounds; ++cellX) {

                int absoluteX = std::abs(cellX);
                int absoluteY = std::abs(cellY);

                // --- THE DISTANCE FORMULA ---
                // D = max(|x|, |y|) + floor(min(|x|, |y|) / 2)                
                int octagonalDistance = std::max(absoluteX, absoluteY) + (std::min(absoluteX, absoluteY) / 2);

                if (octagonalDistance == currentRadius) {
                    // Add the coordinate to the offset list
                    RadiusOffsets.push_back(Cell(cellX, cellY));

                    // --- THE OCCLUSION LOGIC ---
                    // The engine expects the parent vector to point toward (0,0).
                    // We prioritize the dominant axis to match the original game's lookup table.
                    short parentDirectionX = 0;
                    short parentDirectionY = 0;

                    if (absoluteX > absoluteY) {
                        // Horizontal is dominant: Point left if we are right, or right if we are left.
                        parentDirectionX = (cellX > 0) ? -1 : 1;
                    } else if (absoluteY > absoluteX) {
                        // Vertical is dominant: Point up if we are down, or down if we are up.
                        parentDirectionY = (cellY > 0) ? -1 : 1;
                    } else {
                        // Perfectly diagonal: Point diagonally back to center.
                        parentDirectionX = (cellX > 0) ? -1 : 1;
                        parentDirectionY = (cellY > 0) ? -1 : 1;
                    }

                    OcclusionOffsets.push_back(Cell(parentDirectionX, parentDirectionY));
                }
            }
        }

        // 4. Update the Count Table
        // The size of RadiusOffsets after building the shell becomes the new total count.
        RadiusCountTable.push_back(static_cast<int>(RadiusOffsets.size()));
    }
}

/*
* Patches MapClass::From_Sight to no longer clamp the sight range to 10.
* Implements a dynamic algorithm handling that can compute the cells and counts needed for a given Sight Range.
* Used by units when they call Look, and by Reveal by Waypoints, allowing the game to reveal any desired radius.
* Additionally, removes the logic of 'incremental' which was intended to be performance saving,
* but only worked when height reveals was turned off and sight ranges were small enough.
* 
* @author: JoyfulShush
*/
DEFINE_HOOK(0x510C9A, _From_Sight_Dynamic_Sight_Range_Patch, 6)
{
    // EAX contains the sightrange of the unit currently being processed
    int sightRange = R->EAX();

    /*
    * Retrieve the 'incremental' flag from the stack [esp + 0x58]
    * Incremental = true means the unit is moving and only needs to reveal the "new" edge.
    */    
    GET_STACK(bool, isIncremental, 0x58);

    /*
    * 1. Dynamic Generation
    * Calculate the cells that need to be revealed for the provided sight range
    * If the sight range was already calculated from previous operations, they would be already cached
    */    
    MapClassExt::_CalculateSightRadiusIfNeeded(sightRange);

    // 2. Data Preparation
    int cellCount = MapClassExt::RadiusCountTable[sightRange];
    Cell* radiusPointer = MapClassExt::RadiusOffsets.data();
    Cell* occlusionPointer = MapClassExt::OcclusionOffsets.data();

    /*
    * 3. Register and Stack Cleanup
    * ESI must contain the 'count' for the reveal loop
    */    
    R->ESI(cellCount);

    // The game expects the specific 'ptr' and 'coord' variables to be set on the stack
    R->Stack<Cell*>(0x10, radiusPointer);    // [esp+48h+ptr]
    R->Stack<Cell*>(0x4C, occlusionPointer); // [esp+48h+coord]

    /*
    * 4. Flow Control
    * Return the address of loc_510CFB to skip the original hardcoded array lookups
    */    
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
