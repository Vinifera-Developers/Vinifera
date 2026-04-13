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
#include "uicontrol.h"


namespace
{
const SidebarSharedLayout& Get_Sidebar_Layout()
{
    static const SidebarSharedLayout default_layout;
    return UIControls != nullptr ? UIControls->SidebarLayout : default_layout;
}
}


void ActionBarView::Init_Clear()
{
    IsActive = false;
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

    ShapeButtonClass* buttons[] = { &Repair, &Sell, &PowerBtn, &Waypoint };
    const int button_ids[] = {
        SidebarClass::BUTTON_REPAIR,
        SidebarClass::BUTTON_SELL,
        SidebarClass::BUTTON_POWER,
        SidebarClass::BUTTON_WAYPOINT
    };
    const int button_x[] = {
        TacticalRect.Width + TacticalRect.X,
        TacticalRect.Width + TacticalRect.X + 27,
        TacticalRect.Width + TacticalRect.X + 54,
        TacticalRect.Width + TacticalRect.X + 81
    };

    for (int i = 0; i < 4; ++i) {
        buttons[i]->X = button_x[i];
        buttons[i]->Y = 148;
        buttons[i]->IsSticky = true;
        buttons[i]->ID = button_ids[i];
        buttons[i]->DrawX = -480;
        buttons[i]->DrawY = 3;
        buttons[i]->DrawnOnSidebarSurface = true;
        buttons[i]->ShapeDrawer = SidebarDrawer;
        buttons[i]->IsPressed = false;
        buttons[i]->IsToggleType = true;
        buttons[i]->ReflectButtonState = true;
    }

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

    const bool was_active = IsActive;
    if (was_active) {
        Activate(0);
    }

    const SidebarSharedLayout& layout = Get_Sidebar_Layout();
    ShapeButtonClass* buttons[] = { &Repair, &Sell, &PowerBtn, &Waypoint };
    const SidebarButtonLayout* button_layouts[] = {
        &layout.RepairButton,
        &layout.SellButton,
        &layout.PowerButton,
        &layout.WaypointButton
    };

    for (int i = 0; i < 4; ++i) {
        buttons[i]->Set_Position(SidebarRect.X + button_layouts[i]->Position.X, SidebarRect.Y + button_layouts[i]->Position.Y);
        buttons[i]->Flag_To_Redraw();
        buttons[i]->DrawX = -SidebarRect.X;
    }

    Register_Tooltips();

    if (was_active) {
        Activate(1);
    }
}


void ActionBarView::Activate(int control)
{
    if (Debug_Map) {
        return;
    }

    IsActive = control != 0;

    if (control) {
        const SidebarSharedLayout& layout = Get_Sidebar_Layout();
        ShapeButtonClass* buttons[] = { &Repair, &Sell, &PowerBtn, &Waypoint };
        const bool button_visible[] = {
            layout.RepairButton.Visible,
            layout.SellButton.Visible,
            layout.PowerButton.Visible,
            layout.WaypointButton.Visible
        };

        for (int i = 0; i < 4; ++i) {
            if (!button_visible[i]) {
                continue;
            }

            buttons[i]->Zap();
            Map.Add_A_Button(*buttons[i]);
        }
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

    const SidebarSharedLayout& layout = Get_Sidebar_Layout();

    if (Map.IsToRedraw || complete) {
        if (layout.RepairButton.Visible) {
            Repair.Draw_Me(true);
        } else {
            Repair.IsDrawn = false;
        }

        if (layout.SellButton.Visible) {
            Sell.Draw_Me(true);
        } else {
            Sell.IsDrawn = false;
        }

        if (layout.PowerButton.Visible) {
            PowerBtn.Draw_Me(true);
        } else {
            PowerBtn.IsDrawn = false;
        }

        if (layout.WaypointButton.Visible) {
            Waypoint.Draw_Me(true);
        } else {
            Waypoint.IsDrawn = false;
        }

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

    const SidebarSharedLayout& layout = Get_Sidebar_Layout();
    ShapeButtonClass* buttons[] = { &Repair, &Sell, &PowerBtn, &Waypoint };
    const SidebarButtonLayout* button_layouts[] = {
        &layout.RepairButton,
        &layout.SellButton,
        &layout.PowerButton,
        &layout.WaypointButton
    };
    const int button_ids[] = {
        SidebarClass::BUTTON_REPAIR,
        SidebarClass::BUTTON_SELL,
        SidebarClass::BUTTON_POWER,
        SidebarClass::BUTTON_WAYPOINT
    };
    const int tooltip_text[] = {
        TXT_REPAIR_MODE,
        TXT_SELL_MODE,
        TXT_POWER_MODE,
        TXT_WAYPOINTMODE
    };

    ToolTip tooltip;
    for (int i = 0; i < 4; ++i) {
        tooltip.ID = button_ids[i];
        ToolTips->Remove(tooltip.ID);

        if (!button_layouts[i]->Visible) {
            continue;
        }

        tooltip.Region = Rect(buttons[i]->X, buttons[i]->Y, buttons[i]->Width, buttons[i]->Height);
        tooltip.Text = tooltip_text[i];
        ToolTips->Add(&tooltip);
    }
}
