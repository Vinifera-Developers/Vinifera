/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  AircraftTrackerClass reimplementation from YR.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "tibsun_defines.h"
#include "vector.h"

class AbstractClass;
class FootClass;
class CellClass;
struct IStream;


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
