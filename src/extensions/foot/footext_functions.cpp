/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FOOTEXT_FUNCTIONS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the supporting functions for the extended FootClass.
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
#include "footext_functions.h"
#include "unitext_functions.h"
#include "unit.h"
#include "unittype.h"
#include "unitext.h"
#include "building.h"
#include "buildingtype.h"
#include "cell.h"
#include "map.h"
#include "mouse.h"
#include "house.h"
#include "technotype.h"
#include "rulesext.h"
#include "session.h"
#include "tibsun_inline.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "tibsun_defines.h"
#include "debughandler.h"
#include "asserthandler.h"



void _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(FootClass* this_ptr, Cell& cell_coords, Cell* besttiberiumcell, int* besttiberiumvalue, UnitClassExtension* unitext)
{
    if (this_ptr->Tiberium_Check(cell_coords)) {

        CellClass* cell = &Map[cell_coords];
        int tiberiumvalue = cell->Get_Tiberium_Value();

        /**
        *  #issue-203
        *
        *  Consider distance to refinery when selecting the next tiberium patch to harvest.
        *  Prefer the most resourceful tiberium patch, but if there's a tie, prefer one that's
        *  closer to our refinery.
        *
        *  @author: Rampastring
        */
        if (unitext && unitext->LastDockedBuilding && unitext->LastDockedBuilding->IsActive && !unitext->LastDockedBuilding->IsInLimbo) {
            tiberiumvalue *= 100;
            tiberiumvalue -= ::Distance(cell_coords, unitext->LastDockedBuilding->Get_Cell());
        }

        if (tiberiumvalue > * besttiberiumvalue)
        {
            *besttiberiumvalue = tiberiumvalue;
            *besttiberiumcell = cell_coords;
        }
    }
}


/**
 *  Smarter replacement for the Search_For_Tiberium method.
 *  Makes harvesters consider the distance to their refinery when
 *  looking for the cell of tiberium to harvest.
 *
 *  @author: Rampastring
 */
Cell Vinifera_FootClass_Search_For_Tiberium(FootClass* this_ptr, int rad, bool a2)
{
    if (!this_ptr->Owning_House()->Is_Human_Control() &&
        this_ptr->What_Am_I() == RTTI_UNIT &&
        ((UnitClass*)this_ptr)->Class->IsToHarvest &&
        a2 &&
        Session.Type != GAME_NORMAL)
    {
        /**
         *  Use weighted tiberium-seeking algorithm for AI in multiplayer.
         */

        return this_ptr->Search_For_Tiberium_Weighted(rad);
    }

    Coordinate center_coord = this_ptr->Center_Coord();
    Cell cell_coords = Coord_Cell(center_coord);
    Cell unit_cell_coords = cell_coords;

    if (Map[unit_cell_coords].Land_Type() == LAND_TIBERIUM) {

        /**
         *  If we're already standing on tiberium, then we don't need to move anywhere.
         */

        return unit_cell_coords;
    }

    int besttiberiumvalue = -1;
    Cell besttiberiumcell = Cell(0, 0);

    UnitClassExtension* unitext = nullptr;
    if (this_ptr->What_Am_I() == RTTI_UNIT) {
        unitext = UnitClassExtensions.find((UnitClass*)this_ptr);
    }

    /**
     *  Perform a ring search outward from the center.
     */
    for (int radius = 1; radius < rad; radius++) {
        for (int x = -radius; x <= radius; x++) {

            cell_coords = Cell(unit_cell_coords.X + x, unit_cell_coords.Y - radius);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this_ptr, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);

            cell_coords = Cell(unit_cell_coords.X + x, unit_cell_coords.Y + radius);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this_ptr, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);

            cell_coords = Cell(unit_cell_coords.X - radius, unit_cell_coords.Y + x);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this_ptr, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);

            cell_coords = Cell(unit_cell_coords.X + radius, unit_cell_coords.Y + x);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this_ptr, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);
        }

        if (besttiberiumvalue != -1)
            break;
    }

    return besttiberiumcell;
}