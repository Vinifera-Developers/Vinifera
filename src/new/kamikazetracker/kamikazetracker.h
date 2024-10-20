/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          KAMIKAZETRACKER.H
 *
 *  @authors       ZivDero
 *
 *  @brief         KamikazeTrackerClass reimplementation from YR.
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

#include "abstract.h"
#include "ftimer.h"
#include "ttimer.h"
#include "vector.h"

class AircraftClass;
class CellClass;
class KamikazeTrackerClass;


class KamikazeTrackerClass {
public:
    struct KamikazeControl {
        AircraftClass* Aircraft;
        CellClass* Cell;
    };

    KamikazeTrackerClass() noexcept : UpdateTimer(100), Controls() { }
    ~KamikazeTrackerClass();

    HRESULT STDMETHODCALLTYPE Load(IStream* pStm);
    HRESULT STDMETHODCALLTYPE Save(IStream* pStm, BOOL fClearDirty);

    void Add(AircraftClass* aircraft, TARGET target);
    void AI();
    void Detach(AircraftClass const* aircraft);
    void Clear();

    KamikazeTrackerClass(const KamikazeTrackerClass&) = delete;
    KamikazeTrackerClass& operator= (const KamikazeTrackerClass&) = delete;

public:
    /**
     *  The timer that controls how often the tracker should perform its AI function.
     */
    CDTimerClass<FrameTimerClass> UpdateTimer;

    /**
     *  The vector that contains all kamikaze controls.
     */
    DynamicVectorClass<KamikazeControl*> Controls;
};
