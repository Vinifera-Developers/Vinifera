/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTTRACKER.H
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
#pragma once

#include "vector.h"
#include <unknwn.h>
#include "tibsun_defines.h"

class AbstractClass;
class FootClass;
class CellClass;


class AircraftTrackerClass {
public:
    AircraftTrackerClass() { }
    ~AircraftTrackerClass() { };

    FootClass* Get_Target();
    void Fetch_Targets(CellClass* cellptr, int range);

    void Track(FootClass* target);
    void Untrack(FootClass* target);
    void Update_Position(FootClass* target, Cell oldcell, Cell newcell);

    void Clear();

    HRESULT STDMETHODCALLTYPE Load(IStream* pStm);
    HRESULT STDMETHODCALLTYPE Save(IStream* pStm);

    AircraftTrackerClass(const AircraftTrackerClass&) = delete;
    AircraftTrackerClass& operator= (const AircraftTrackerClass&) = delete;

private:
    int Get_Region(Cell cell);
    void Copy_Region(int region);
    int Adjacent_Region(int region, int dy, int dx);

public:
    DynamicVectorClass<FootClass*> Regions[400];
    DynamicVectorClass<FootClass*> WorkingSet;
};
