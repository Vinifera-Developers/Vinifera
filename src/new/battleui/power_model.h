/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Power bar data model. Reads power/drain from HouseClass
 *          and provides rendering state for the view layer.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "stimer.h"
#include "ttimer.h"


/**
 *  Power bar data model. Tracks power output and drain from the player's
 *  house and provides state for rendering the power bar UI.
 */
class PowerModel
{
public:
    PowerModel();

    void Init_Clear();
    void AI();

    void Flash();
    void Redraw() { IsDirty = true; }

    float Power_Fraction() const;
    int Get_Power() const { return PowerOutput; }
    int Get_Drain() const { return PowerDrain; }
    bool Is_Low_Power() const { return PowerDrain > 0 && PowerOutput < PowerDrain; }
    bool Is_Dirty() const { return IsDirty; }
    bool Is_Flashing() const;

    void Clear_Dirty() { IsDirty = false; }

private:
    int PowerOutput;
    int PowerDrain;
    int RecordedPower;
    int RecordedDrain;
    bool IsDirty;
    CDTimerClass<SystemTimerClass> FlashTimer;
};
