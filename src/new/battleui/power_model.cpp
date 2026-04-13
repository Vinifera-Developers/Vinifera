/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          POWER_MODEL.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Power bar data model implementation.
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
