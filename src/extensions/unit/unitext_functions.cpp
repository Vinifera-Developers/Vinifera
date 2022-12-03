/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITEXT_FUNCTIONS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the supporting functions for the extended UnitClass.
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
#include "unitext_functions.h"
#include "unit.h"
#include "unittype.h"
#include "building.h"
#include "buildingtype.h"
#include "technotype.h"
#include "rulesext.h"
#include "tibsun_inline.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "debughandler.h"
#include "asserthandler.h"


 /**
  *  Finds the nearest docking bay for a specific unit.
  *
  *  @author: Rampastring
  */
void UnitClassExtension_Find_Nearest_Refinery(UnitClass* this_ptr, BuildingClass** building_addr, int* distance_addr, bool include_reserved)
{
    int nearest_refinery_distance = INT_MAX;
    BuildingClass* nearest_refinery = nullptr;

    /**
     *  Find_Docking_Bay looks also through occupied docking bays if ScenarioInit is set
     */
    if (include_reserved) {
        ScenarioInit++;
    }

    for (int i = 0; i < this_ptr->Class->Dock.Count(); i++) {
        BuildingTypeClass* dockbuildingtype = this_ptr->Class->Dock[i];

        BuildingClass* dockbuilding = this_ptr->Find_Docking_Bay(dockbuildingtype, false, false);
        if (dockbuilding == nullptr)
            continue;

        int distance = this_ptr->Distance(dockbuilding);

        if (distance < nearest_refinery_distance) {
            nearest_refinery_distance = distance;
            nearest_refinery = dockbuilding;
        }
    }

    if (include_reserved) {
        ScenarioInit--;
    }

    *building_addr = nearest_refinery;
    *distance_addr = nearest_refinery_distance;
}