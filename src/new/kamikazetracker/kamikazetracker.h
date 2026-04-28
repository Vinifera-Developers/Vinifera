/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  KamikazeTrackerClass reimplementation from YR.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "ftimer.h"
#include "ttimer.h"
#include "vector.h"

class AircraftClass;
class AbstractClass;
class CellClass;
class KamikazeTrackerClass;
struct IStream;


class KamikazeTrackerClass {
public:
    struct KamikazeControl {
        AircraftClass* Aircraft;
        CellClass* Cell;
    };

    KamikazeTrackerClass() : UpdateTimer(100), Controls() { }
    KamikazeTrackerClass(const NoInitClass& noinit) : UpdateTimer(noinit), Controls(noinit) { }
    ~KamikazeTrackerClass();

    HRESULT STDMETHODCALLTYPE Load(IStream* pStm);
    HRESULT STDMETHODCALLTYPE Save(IStream* pStm, BOOL fClearDirty);

    void Add(AircraftClass* aircraft, AbstractClass * target);
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
