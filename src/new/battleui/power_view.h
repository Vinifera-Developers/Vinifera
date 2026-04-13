/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          POWER_VIEW.H
 *
 *  @author        ZivDero
 *
 *  @brief         Power bar view. Renders the animated pip-based power bar
 *                 on the sidebar surface. Ported from the original Tiberian
 *                 Sun PowerClass rendering and animation logic.
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


class PowerModel;
struct ShapeSet;


/**
 *  Power bar view. Renders and animates the power bar on the sidebar
 *  surface using pips (POWERP.SHP). The animation system incrementally
 *  adjusts pip counts toward desired levels, providing a smooth visual
 *  transition when power/drain changes.
 */
class PowerView
{
public:
    enum PowerEnums {
        GADGET_POWER = 999,
        POWER_X = 8,
        POWER_Y = 25,
        POWER_WIDTH = 12,
        POWER_PIP_HEIGHT = 4,
        POWER_PIP_EMPTY = 0,
        POWER_PIP_GREEN = 1,
        POWER_PIP_YELLOW = 2,
        POWER_PIP_RED = 3,
        POWER_PIP_WHITE = 4,
    };

    PowerView();

    void One_Time();
    void Init_Clear();
    void Init_For_House();
    void Set_Dimensions();
    void AI();
    void Draw(bool complete);
    void Flash_Power();

    const char *Help_Text(int gadget_id);

    void Set_Model(PowerModel *model) { Model = model; }

    bool IsToRedraw;

private:
    int Max_Power_Height() const;
    int Desired_Power_Height() const;
    int Desired_Levels(int &green, int &yellow, int &red) const;
    int Update_Delay() const;
    void Remove_Pip();
    void Add_Pip();

    PowerModel *Model;

    CDTimerClass<SystemTimerClass> FlashTimer;
    int FlashCount;

    CDTimerClass<SystemTimerClass> UpdateTimer;
    int GreenPipCount;
    int YellowPipCount;
    int RedPipCount;

    bool HasChanged;

    int RecordedDrain;
    int RecordedPower;

    static const ShapeSet *PowerPipShape;
};
