/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTTRACKER.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         AircraftTrackerClass reimplementation from YR.
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

#include "aircrafttracker.h"

#include <algorithm>

#include "foot.h"
#include "swizzle.h"
#include "cell.h"
#include "extension.h"
#include "footext.h"
#include "mouse.h"
#include "vinifera_saveload.h"


/**
 *  Copies the tracking region to the working set.
 *
 *  @author: ZivDero
 */
void AircraftTrackerClass::Copy_Region(int region)
{
    int count = Regions[region].Count();
    for (int i = 0; i < count; i++) {
        FootClass* foot = Regions[region][i];
        WorkingSet.Add(foot);
    }
}


/**
 *  Fetches a region relative to the given one.
 *
 *  @author: ZivDero
 */
int AircraftTrackerClass::Adjacent_Region(int region, int xoff, int yoff)
{
    int regx = region % 20;
    int regy = region / 20;

    if (xoff + regx >= 0 && xoff + regx > (20 - 1)) {
        regx = (20 - 1);
    }
    else {
        regx = xoff + regx < 0 ? 0 : xoff + regx;
    }

    if (yoff + regy >= 0 && yoff + regy > (20 - 1)) {
        regy = (20 - 1);
    }
    else {
        regy = yoff + regy < 0 ? 0 : yoff + regy;
    }

    return(regx + regy * 20);
}


/**
 *  Gets the region that contains this cell.
 *
 *  @author: ZivDero
 */
int AircraftTrackerClass::Get_Region(Cell cell)
{
    int x = std::max(0, static_cast<int>(cell.X));
    int y = std::max(0, static_cast<int>(cell.Y));

    int regx = std::min(19, x / (Map.MapCellWidth / 20));
    int regy = std::min(19, y / (Map.MapCellHeight / 20));

    return regx + regy * 20;
}


/**
 *  Fetches the targets around this cell in a given range.
 *
 *  @author: ZivDero
 */
void AircraftTrackerClass::Fetch_Targets(CellClass* cellptr, int range)
{
    range = std::max(range, 1);
    int region_range = (range + 19) / 20;

    bool copied_regions[400] = {};

    Cell cell = Coord_Cell(cellptr->Center_Coord());
    int center_region = Get_Region(cell);

    for (int x = -region_range; x <= region_range; x++) {
        for (int y = -region_range; y <= region_range; y++) {
            int adjacent_region = Adjacent_Region(center_region, x, y);
            if (!copied_regions[adjacent_region]) {
                Copy_Region(adjacent_region);
                copied_regions[adjacent_region] = true;
            }
        }
    }
}


/**
 *  Adds the object to the tracker.
 *
 *  @author: ZivDero
 */
void AircraftTrackerClass::Track(FootClass* target)
{
    Cell cell = target->Get_Cell();
    const auto target_ext = Extension::Fetch<FootClassExtension>(target);
    target_ext->Set_Last_Flight_Cell(cell);
    Regions[Get_Region(cell)].Add(target);
}


/**
 *  Removes the object from the tracker.
 *
 *  @author: ZivDero
 */
void AircraftTrackerClass::Untrack(FootClass* target)
{
    const auto target_ext = Extension::Fetch<FootClassExtension>(target);
    Cell cell = target_ext->Get_Last_Flight_Cell();
    target_ext->Set_Last_Flight_Cell(CELL_NONE);
    Regions[Get_Region(cell)].Delete(target);
}


/**
 *  Loads the tracker from the stream.
 *
 *  @author: ZivDero
 */
HRESULT STDMETHODCALLTYPE AircraftTrackerClass::Load(IStream* stream)
{
    for (auto& Region : Regions) {
        HRESULT result = Load_Primitive_Vector(stream, Region, "Regions");
        if (FAILED(result)) {
            return result;
        }
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(Regions[i], "Regions");
    }

    return S_OK;
}


/**
 *  Gets the next target from the working set.
 *
 *  @author: ZivDero
 */
FootClass* AircraftTrackerClass::Get_Target(void)
{
    if (WorkingSet.Count() == 0) {
        return nullptr;
    }

    FootClass* target = WorkingSet[0];
    WorkingSet.Delete(target);
    return target;
}


/**
 *  Clears the tracker.
 *
 *  @author: ZivDero
 */
void AircraftTrackerClass::Clear(void)
{
    for (int i = 0; i < 400; i++) {
        int old = Regions[i].Length();
        Regions[i].Clear();
        Regions[i].Resize(old, nullptr);
    }
    int old = WorkingSet.Length();
    WorkingSet.Clear();
    WorkingSet.Resize(old, nullptr);
}


/**
 *  Saves the tracker to the stream.
 *
 *  @author: ZivDero
 */
HRESULT STDMETHODCALLTYPE AircraftTrackerClass::Save(IStream* stream)
{
    for (auto& Region : Regions) {
        HRESULT result = Save_Primitive_Vector(stream, Region, "Regions");
        if (FAILED(result)) {
            return result;
        }
    }

    return S_OK;
}


/**
 *  Updates an object's position in the tracker.
 *
 *  @author: ZivDero
 */
void AircraftTrackerClass::Update_Position(FootClass* target, Cell oldcell, Cell newcell)
{
    const auto target_ext = Extension::Fetch<FootClassExtension>(target);
    target_ext->Set_Last_Flight_Cell(newcell);
    const int oldregion = Get_Region(oldcell);
    const int newregion = Get_Region(newcell);
    if (oldregion != newregion) {
        Regions[oldregion].Delete(target);
        Regions[newregion].Add(target);
    }
}
