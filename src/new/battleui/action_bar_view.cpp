/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ACTION_BAR_VIEW.CPP
 *
 *  @author        OpenAI
 *
 *  @brief         Shared action bar view for sidebar-common buttons.
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

#include "action_bar_view.h"

#include "fetchres.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "palette.h"
#include "sidebar.h"
#include "tibsun_globals.h"
#include "tooltip.h"


void ActionBarView::Init_Clear()
{
    Repair.IsPressed = false;
    Sell.IsPressed = false;
    PowerBtn.IsPressed = false;
    Waypoint.IsPressed = false;

    if (Repair.IsOn) Repair.Turn_Off();
    if (Sell.IsOn) Sell.Turn_Off();
    if (PowerBtn.IsOn) PowerBtn.Turn_Off();
    if (Waypoint.IsOn) Waypoint.Turn_Off();
}


void ActionBarView::Init_IO()
{
    if (Debug_Map) {
        return;
    }

    Repair.X = TacticalRect.Width + TacticalRect.X;
    Sell.X = TacticalRect.Width + TacticalRect.X + 27;
    PowerBtn.X = TacticalRect.Width + TacticalRect.X + 54;
    Waypoint.X = TacticalRect.Width + TacticalRect.X + 81;

    Repair.IsSticky = true;
    Repair.ID = SidebarClass::BUTTON_REPAIR;
    Repair.Y = 148;
    Repair.DrawX = -480;
    Repair.DrawY = 3;
    Repair.DrawnOnSidebarSurface = true;
    Repair.ShapeDrawer = SidebarDrawer;
    Repair.IsPressed = false;
    Repair.IsToggleType = true;
    Repair.ReflectButtonState = true;

    Sell.IsSticky = true;
    Sell.ID = SidebarClass::BUTTON_SELL;
    Sell.Y = 148;
    Sell.DrawX = -480;
    Sell.DrawY = 3;
    Sell.DrawnOnSidebarSurface = true;
    Sell.ShapeDrawer = SidebarDrawer;
    Sell.IsPressed = false;
    Sell.IsToggleType = true;
    Sell.ReflectButtonState = true;

    PowerBtn.IsSticky = true;
    PowerBtn.ID = SidebarClass::BUTTON_POWER;
    PowerBtn.Y = 148;
    PowerBtn.DrawX = -480;
    PowerBtn.DrawY = 3;
    PowerBtn.DrawnOnSidebarSurface = true;
    PowerBtn.ShapeDrawer = SidebarDrawer;
    PowerBtn.IsPressed = false;
    PowerBtn.IsToggleType = true;
    PowerBtn.ReflectButtonState = true;

    Waypoint.IsSticky = true;
    Waypoint.ID = SidebarClass::BUTTON_WAYPOINT;
    Waypoint.Y = 148;
    Waypoint.DrawX = -480;
    Waypoint.DrawY = 3;
    Waypoint.DrawnOnSidebarSurface = true;
    Waypoint.ShapeDrawer = SidebarDrawer;
    Waypoint.IsPressed = false;
    Waypoint.IsToggleType = true;
    Waypoint.ReflectButtonState = true;
    Waypoint.Enable();
}


void ActionBarView::Init_For_House()
{
    if (Debug_Map) {
        return;
    }

    Sell.Set_Shape(MFCD::RetrieveT<ShapeSet>("SELL.SHP"));
    Sell.ShapeDrawer = SidebarDrawer;

    PowerBtn.Set_Shape(MFCD::RetrieveT<ShapeSet>("POWER.SHP"));
    PowerBtn.ShapeDrawer = SidebarDrawer;

    Waypoint.Set_Shape(MFCD::RetrieveT<ShapeSet>("WAYP.SHP"));
    Waypoint.ShapeDrawer = SidebarDrawer;

    Repair.Set_Shape(MFCD::RetrieveT<ShapeSet>("REPAIR.SHP"));
    Repair.ShapeDrawer = SidebarDrawer;
}


void ActionBarView::Set_Dimensions()
{
    if (Debug_Map) {
        return;
    }

    Repair.Set_Position(SidebarRect.X + SidebarClass::BUTTON_REPAIR_X_OFFSET, SidebarRect.Y + SidebarClass::BUTTON_REPAIR_Y_OFFSET);
    Repair.Flag_To_Redraw();
    Repair.DrawX = -SidebarRect.X;

    Sell.Set_Position(Repair.X + SidebarClass::BUTTON_SELL_X_OFFSET, Repair.Y);
    Sell.Flag_To_Redraw();
    Sell.DrawX = -SidebarRect.X;

    PowerBtn.Set_Position(Sell.X + SidebarClass::BUTTON_POWER_X_OFFSET, Sell.Y);
    PowerBtn.Flag_To_Redraw();
    PowerBtn.DrawX = -SidebarRect.X;

    Waypoint.Set_Position(PowerBtn.X + SidebarClass::BUTTON_WAYPOINT_X_OFFSET, PowerBtn.Y);
    Waypoint.Flag_To_Redraw();
    Waypoint.DrawX = -SidebarRect.X;

    Register_Tooltips();
}


void ActionBarView::Activate(int control)
{
    if (Debug_Map) {
        return;
    }

    if (control) {
        Repair.Zap();
        Map.Add_A_Button(Repair);
        Sell.Zap();
        Map.Add_A_Button(Sell);
        PowerBtn.Zap();
        Map.Add_A_Button(PowerBtn);
        Waypoint.Zap();
        Map.Add_A_Button(Waypoint);
    } else {
        Map.Remove_A_Button(Repair);
        Map.Remove_A_Button(Sell);
        Map.Remove_A_Button(PowerBtn);
        Map.Remove_A_Button(Waypoint);
    }
}


void ActionBarView::AI(KeyNumType& key)
{
    if (PlayerPtr->CurBuildings > 0) {
        Map.Activate_Repair(true);
    } else {
        Map.Activate_Repair(false);
    }

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

    if (!Map.IsRepairMode && Repair.IsOn) {
        Repair.Turn_Off();
    }

    if (!Map.IsSellMode && Sell.IsOn) {
        Sell.Turn_Off();
    }

    if (!Map.IsPowerMode && PowerBtn.IsOn) {
        PowerBtn.Turn_Off();
    }

    if (!Map.IsWaypointMode && Waypoint.IsOn) {
        Waypoint.Turn_Off();
    }
}


void ActionBarView::Draw(bool complete)
{
    if (!Map.IsSidebarActive || Debug_Map || SidebarSurface == nullptr) {
        return;
    }

    Surface* oldsurface = LogicalSurface;
    LogicalSurface = SidebarSurface;

    if (Map.IsToRedraw || complete) {
        Repair.Draw_Me(true);
        Sell.Draw_Me(true);
        PowerBtn.Draw_Me(true);
        Waypoint.Draw_Me(true);
        RedrawSidebar = true;
    }

    if (Repair.IsDrawn) {
        RedrawSidebar = true;
        Repair.IsDrawn = false;
    }
    if (Sell.IsDrawn) {
        RedrawSidebar = true;
        Sell.IsDrawn = false;
    }
    if (PowerBtn.IsDrawn) {
        RedrawSidebar = true;
        PowerBtn.IsDrawn = false;
    }
    if (Waypoint.IsDrawn) {
        RedrawSidebar = true;
        Waypoint.IsDrawn = false;
    }

    LogicalSurface = oldsurface;
}


void ActionBarView::Register_Tooltips()
{
    if (ToolTips == nullptr) {
        return;
    }

    ToolTip tooltip;

    tooltip.Region = Rect(Repair.X, Repair.Y, Repair.Width, Repair.Height);
    tooltip.ID = SidebarClass::BUTTON_REPAIR;
    tooltip.Text = TXT_REPAIR_MODE;
    ToolTips->Remove(tooltip.ID);
    ToolTips->Add(&tooltip);

    tooltip.Region = Rect(Sell.X, Sell.Y, Sell.Width, Sell.Height);
    tooltip.ID = SidebarClass::BUTTON_SELL;
    tooltip.Text = TXT_SELL_MODE;
    ToolTips->Remove(tooltip.ID);
    ToolTips->Add(&tooltip);

    tooltip.Region = Rect(PowerBtn.X, PowerBtn.Y, PowerBtn.Width, PowerBtn.Height);
    tooltip.ID = SidebarClass::BUTTON_POWER;
    tooltip.Text = TXT_POWER_MODE;
    ToolTips->Remove(tooltip.ID);
    ToolTips->Add(&tooltip);

    tooltip.Region = Rect(Waypoint.X, Waypoint.Y, Waypoint.Width, Waypoint.Height);
    tooltip.ID = SidebarClass::BUTTON_WAYPOINT;
    tooltip.Text = TXT_WAYPOINTMODE;
    ToolTips->Remove(tooltip.ID);
    ToolTips->Add(&tooltip);
}
