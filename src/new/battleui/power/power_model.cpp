/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Power bar data model. Reads power/drain from HouseClass
 *          and provides rendering state for the view layer.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "power_model.h"

#include "house.h"
#include "tibsun_globals.h"


static constexpr int FLASH_DURATION = 7;


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
PowerModel::PowerModel() :
    PowerOutput(0),
    PowerDrain(0),
    RecordedPower(0),
    RecordedDrain(0),
    IsDirty(true),
    FlashTimer(0)
{
}


/**
 *  Resets the power model to initial state.
 *
 *  @author: ZivDero
 */
void PowerModel::Init_Clear()
{
    PowerOutput = 0;
    PowerDrain = 0;
    RecordedPower = 0;
    RecordedDrain = 0;
    IsDirty = true;
    FlashTimer = 0;
}


/**
 *  Updates power data from the player's house. Called every frame.
 *  Sets dirty flag when values change.
 *
 *  @author: ZivDero
 */
void PowerModel::AI()
{
    if (PlayerPtr == nullptr) {
        return;
    }

    PowerOutput = PlayerPtr->Power;
    PowerDrain = PlayerPtr->Drain;

    if (PowerOutput != RecordedPower || PowerDrain != RecordedDrain) {
        RecordedPower = PowerOutput;
        RecordedDrain = PowerDrain;
        IsDirty = true;
    }
}


/**
 *  Triggers the power bar flash effect.
 *
 *  @author: ZivDero
 */
void PowerModel::Flash()
{
    FlashTimer = FLASH_DURATION;
    IsDirty = true;
}


/**
 *  Returns the power fraction (output / drain), clamped to [0.0, 1.0].
 *
 *  @author: ZivDero
 */
float PowerModel::Power_Fraction() const
{
    if (PowerDrain <= 0) {
        return 1.0f;
    }

    if (PowerOutput <= 0) {
        return 0.0f;
    }

    float fraction = static_cast<float>(PowerOutput) / static_cast<float>(PowerDrain);
    if (fraction > 1.0f) {
        fraction = 1.0f;
    }

    return fraction;
}


/**
 *  Is the power bar currently in a flash state?
 *
 *  @author: ZivDero
 */
bool PowerModel::Is_Flashing() const
{
    return FlashTimer.Is_Active();
}
