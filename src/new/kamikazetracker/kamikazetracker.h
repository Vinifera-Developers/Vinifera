/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  KamikazeTrackerClass reimplementation from YR.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "detach_listener.h"
#include "ftimer.h"
#include "ttimer.h"
#include "vector.h"

class AircraftClass;
class AbstractClass;
class CellClass;
class KamikazeTrackerClass;
struct IStream;


class KamikazeTrackerClass : public Vinifera::Detach::Listener<AircraftClass>
{
public:
    struct KamikazeControl {
        AircraftClass* Aircraft;
        CellClass* Cell;
    };

    KamikazeTrackerClass() : Vinifera::Detach::Listener<AircraftClass>(), UpdateTimer(100), Controls() { }
    KamikazeTrackerClass(const NoInitClass& noinit) : Vinifera::Detach::Listener<AircraftClass>(noinit), UpdateTimer(noinit), Controls(noinit) { }
    ~KamikazeTrackerClass();

    HRESULT STDMETHODCALLTYPE Load(IStream* pStm);
    HRESULT STDMETHODCALLTYPE Save(IStream* pStm, BOOL fClearDirty);

    void Add(AircraftClass* aircraft, AbstractClass * target);
    void AI();
    void On_Detach(AircraftClass *aircraft, bool all) override;
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
