/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Power bar view implementation. Ported from the original
 *          Tiberian Sun PowerClass rendering and animation logic.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "stimer.h"
#include "ttimer.h"
#include "uicontrol.h"


class PowerModel;
class ShapeSet;


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
    void Shift_Sidebar();
    void Set_Height(int pixels);
    void AI();
    void Draw();
    void Flash_Power();
    void Set_Sidebar_View_Type(SidebarViewType view_type) { ViewType = view_type; }

    const char *Help_Text(int gadget_id);

    void Set_Model(PowerModel *model) { Model = model; }

private:
    int Current_Power() const;
    int Current_Drain() const;
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
    int PixelHeight;

    bool HasChanged;
    SidebarViewType ViewType;

    static const ShapeSet *PowerPipShape;
};
