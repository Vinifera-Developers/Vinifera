/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Shared action bar view for sidebar-common buttons.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "action_bar_view.h"

#include "battleui.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "sidebar.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"


namespace
{
/**
 *  Returns the active sidebar layout config for the selected view type.
 *
 *  @author: ZivDero
 */
const BattleSidebarLayoutBase& Get_Sidebar_Layout(SidebarViewType view_type)
{
    return UIControls->Get_Battle_Sidebar_Config(view_type);
}
}


/**
 *  Returns the action bar button table bound to the active sidebar layout.
 *
 *  @author: ZivDero
 */
std::array<ActionBarView::ButtonInfo, 4> ActionBarView::Get_Button_Info()
{
    const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);
    return { {
        { RepairButton,   layout.RepairButton,   SidebarClass::BUTTON_REPAIR,   TXT_REPAIR_MODE,  layout.RepairButtonShape },
        { SellButton,     layout.SellButton,     SidebarClass::BUTTON_SELL,     TXT_SELL_MODE,    layout.SellButtonShape },
        { PowerButton,    layout.PowerButton,    SidebarClass::BUTTON_POWER,    TXT_POWER_MODE,   layout.PowerButtonShape },
        { WaypointButton, layout.WaypointButton, SidebarClass::BUTTON_WAYPOINT, TXT_WAYPOINTMODE, layout.WaypointButtonShape },
    } };
}


/***************************************************************************
**  Lifecycle and layout
***************************************************************************/


/**
 *  Resets action bar button state for a new scenario.
 *
 *  @author: ZivDero
 */
void ActionBarView::Init_Clear()
{
    IsActive = false;

    for (const auto& info : Get_Button_Info()) {
        info.Button.IsPressed = false;
        if (info.Button.IsOn) info.Button.Turn_Off();
    }
}


/**
 *  Initializes action bar button gadgets and baseline placement.
 *
 *  @author: ZivDero
 */
void ActionBarView::Init_IO()
{
    if (Debug_Map) {
        return;
    }

    const int base_x = TacticalRect.Width + TacticalRect.X;
    int i = 0;
    for (const auto& info : Get_Button_Info()) {
        ShapeButtonClass& btn = info.Button;
        btn.X = base_x + i * 27;
        btn.Y = 148;
        btn.IsSticky = true;
        btn.ID = info.ID;
        btn.DrawX = -480;
        btn.DrawY = 3;
        btn.DrawnOnSidebarSurface = true;
        btn.ShapeDrawer = SidebarDrawer;
        btn.IsPressed = false;
        btn.IsToggleType = true;
        btn.ReflectButtonState = true;
        ++i;
    }

    // The waypoint button is always enabled, so enable it now.
    // Other buttons may be disabled
    WaypointButton.Enable();
}


/**
 *  Loads house-specific action bar button art.
 *
 *  @author: ZivDero
 */
void ActionBarView::Init_For_House()
{
    if (Debug_Map) {
        return;
    }

    for (const auto& info : Get_Button_Info()) {
        info.Button.Set_Shape(MFCD::RetrieveT<ShapeSet>(info.ShapeName.c_str()));
        info.Button.ShapeDrawer = SidebarDrawer;
    }
}


/**
 *  Reflows action bar button positions from the active layout config.
 *
 *  @author: ZivDero
 */
void ActionBarView::Shift_Sidebar()
{
    if (Debug_Map) {
        return;
    }

    const bool was_active = IsActive;
    if (was_active) {
        Activate(false);
    }

    for (const auto& info : Get_Button_Info()) {
        info.Button.Set_Position(SidebarRect.X + info.Layout.Position.X, SidebarRect.Y + info.Layout.Position.Y);
        info.Button.Flag_To_Redraw();
        info.Button.DrawX = -SidebarRect.X;
    }

    Register_Tooltips();

    if (was_active) {
        Activate(true);
    }
}


/**
 *  Activates or deactivates the action bar gadgets.
 *
 *  @author: ZivDero
 */
void ActionBarView::Activate(bool enabled)
{
    if (Debug_Map) {
        return;
    }

    IsActive = enabled;

    if (enabled) {
        for (const auto& info : Get_Button_Info()) {
            if (info.Layout.IsVisible) {
                info.Button.Zap();
                Map.Add_A_Button(info.Button);
            }
        }
    } else {
        for (const auto& info : Get_Button_Info()) {
            Map.Remove_A_Button(info.Button);
        }
    }
}


/***************************************************************************
**  Runtime behavior
***************************************************************************/


/**
 *  Handles action bar button input and keeps toggle states in sync.
 *
 *  @author: ZivDero
 */
void ActionBarView::AI(KeyNumType& key)
{
    if (key == (SidebarClass::BUTTON_REPAIR | KN_BUTTON)) {
        Map.Repair_Mode_Control(-1);
    }

    if (key == (SidebarClass::BUTTON_POWER | KN_BUTTON)) {
        Map.Power_Mode_Control(-1);
    }

    if (key == (SidebarClass::BUTTON_WAYPOINT | KN_BUTTON)) {
        Map.Waypoint_Mode_Control(-1, false);
    }

    if (key == (SidebarClass::BUTTON_SELL | KN_BUTTON)) {
        Map.Sell_Mode_Control(-1);
    }

    if (!Map.IsRepairMode && RepairButton.IsOn) {
        RepairButton.Turn_Off();
    }

    if (!Map.IsSellMode && SellButton.IsOn) {
        SellButton.Turn_Off();
    }

    if (!Map.IsPowerMode && PowerButton.IsOn) {
        PowerButton.Turn_Off();
    }

    if (!Map.IsWaypointMode && WaypointButton.IsOn) {
        WaypointButton.Turn_Off();
    }
}


/**
 *  Draws the visible action bar buttons onto the sidebar surface.
 *
 *  @author: ZivDero
 */
void ActionBarView::Draw()
{
    if (!Map.IsSidebarActive || Debug_Map || SidebarSurface == nullptr) {
        return;
    }

    Surface* oldsurface = LogicalSurface;
    LogicalSurface = SidebarSurface;

    for (const auto& info : Get_Button_Info()) {
        if (info.Layout.IsVisible) {
            info.Button.Draw_Me(true);
        }
    }

    LogicalSurface = oldsurface;
}


/**
 *  Registers tooltips for the currently visible action bar buttons.
 *
 *  @author: ZivDero
 */
void ActionBarView::Register_Tooltips()
{
    if (ToolTips == nullptr) {
        return;
    }

    ToolTip tooltip;
    for (const auto& info : Get_Button_Info()) {
        tooltip.ID = info.ID;
        ToolTips->Remove(tooltip.ID);

        if (!info.Layout.IsVisible) {
            continue;
        }

        tooltip.Region = Rect(info.Button.X, info.Button.Y, info.Button.Width, info.Button.Height);
        tooltip.Text = info.TooltipText;
        ToolTips->Add(&tooltip);
    }
}
