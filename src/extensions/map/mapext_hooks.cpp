/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended MapClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "mapext_hooks.h"

#include "battleui.h"
#include "cell.h"
#include "extension.h"
#include "hooker.h"
#include "map.h"
#include "object.h"
#include "objecttype.h"
#include "rules.h"
#include "syringe.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"


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
    static void _Calculate_Sight_Radius_If_Needed(int target_sight_range);
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
    if (!object) return;

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
        BattleUI.Get_Sidebar().Detach(target);
    }
}

/**
 *  Dynamically populates the radius and occlusion tables if they are not populated for a given radius (sight range).
 *  This allows any desired radius of cells to be revealed when called through MapClass::From_Sight.
 *  Results are cached up to the highest calculated sight range, in which case, we'll simply use the calculated results.
 *
 *  @author: JoyfulShush
 */
void MapClassExt::_Calculate_Sight_Radius_If_Needed(int sight_range)
{
    int current_max_radius = static_cast<int>(RadiusCountTable.size());

    if (sight_range <= current_max_radius - 1) {
        return;
    }

    for (int current_radius = current_max_radius; current_radius <= sight_range; ++current_radius) {
        int search_bounds = current_radius + 2;

        for (int cell_y = -search_bounds; cell_y <= search_bounds; ++cell_y) {
            for (int cell_x = -search_bounds; cell_x <= search_bounds; ++cell_x) {

                int absolute_x = std::abs(cell_x);
                int absolute_y = std::abs(cell_y);

                int octagonal_distance = std::max(absolute_x, absolute_y) + (std::min(absolute_x, absolute_y) / 2);

                if (octagonal_distance == current_radius) {
                    RadiusOffsets.push_back(Cell(cell_x, cell_y));

                    short parent_direction_x = 0;
                    short parent_direction_y = 0;

                    if (absolute_x > absolute_y) {
                        parent_direction_x = (cell_x > 0) ? -1 : 1;
                    } else if (absolute_y > absolute_x) {
                        parent_direction_y = (cell_y > 0) ? -1 : 1;
                    } else {
                        parent_direction_x = (cell_x > 0) ? -1 : 1;
                        parent_direction_y = (cell_y > 0) ? -1 : 1;
                    }

                    OcclusionOffsets.push_back(Cell(parent_direction_x, parent_direction_y));
                }
            }
        }

        RadiusCountTable.push_back(static_cast<int>(RadiusOffsets.size()));
    }
}

/*
 *  Patches MapClass::From_Sight to no longer clamp the sight range to 10.
 *  Implements a dynamic algorithm handling that can compute the cells and counts needed for a given Sight Range.
 *  Used by units when they call Look, and by Reveal by Waypoints, allowing the game to reveal any desired radius.
 *  Additionally, removes the logic of 'incremental' which was intended to be performance saving,
 *  but only worked when height reveals was turned off and sight ranges were small enough.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x00510C9A, _From_Sight_Dynamic_Sight_Range_Patch, 6)
{
    int sight_range = R->EAX();

    MapClassExt::_Calculate_Sight_Radius_If_Needed(sight_range);

    int cell_count = MapClassExt::RadiusCountTable[sight_range];
    Cell* radius_offsets_ptr = MapClassExt::RadiusOffsets.data();
    Cell* occlusion_offsets_ptr = MapClassExt::OcclusionOffsets.data();

    R->ESI(cell_count);

    R->Stack(0x10, radius_offsets_ptr);
    R->Stack(0x4C, occlusion_offsets_ptr);

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
