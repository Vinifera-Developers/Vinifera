/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          POWER_MODEL.H
 *
 *  @author        ZivDero
 *
 *  @brief         Power bar data model. Reads power/drain from HouseClass
 *                 and provides rendering state for the view layer.
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
