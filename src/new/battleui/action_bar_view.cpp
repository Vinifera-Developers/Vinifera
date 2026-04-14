/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ACTION_BAR_VIEW.CPP
 *
 *  @author        ZivDero
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

#include "battleui_component.h"

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
    RepairButton.IsPressed = false;
    SellButton.IsPressed = false;
    PowerButton.IsPressed = false;
    WaypointButton.IsPressed = false;

    if (RepairButton.IsOn) RepairButton.Turn_Off();
    if (SellButton.IsOn) SellButton.Turn_Off();
    if (PowerButton.IsOn) PowerButton.Turn_Off();
    if (WaypointButton.IsOn) WaypointButton.Turn_Off();
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

    ShapeButtonClass* buttons[] = { &RepairButton, &SellButton, &PowerButton, &WaypointButton };
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

    const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);

    SellButton.Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.SellButtonShape.c_str()));
    SellButton.ShapeDrawer = SidebarDrawer;

    PowerButton.Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.PowerButtonShape.c_str()));
    PowerButton.ShapeDrawer = SidebarDrawer;

    WaypointButton.Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.WaypointButtonShape.c_str()));
    WaypointButton.ShapeDrawer = SidebarDrawer;

    RepairButton.Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.RepairButtonShape.c_str()));
    RepairButton.ShapeDrawer = SidebarDrawer;
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
        Activate(0);
    }

    const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);
    ShapeButtonClass* buttons[] = { &RepairButton, &SellButton, &PowerButton, &WaypointButton };
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


/**
 *  Activates or deactivates the action bar gadgets.
 *
 *  @author: ZivDero
 */
void ActionBarView::Activate(int control)
{
    if (Debug_Map) {
        return;
    }

    IsActive = control != 0;

    if (control) {
        const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);
        ShapeButtonClass* buttons[] = { &RepairButton, &SellButton, &PowerButton, &WaypointButton };
        const bool button_visible[] = {
            layout.RepairButton.IsVisible,
            layout.SellButton.IsVisible,
            layout.PowerButton.IsVisible,
            layout.WaypointButton.IsVisible
        };

        for (int i = 0; i < 4; ++i) {
            if (!button_visible[i]) {
                continue;
            }

            buttons[i]->Zap();
            Map.Add_A_Button(*buttons[i]);
        }
    } else {
        Map.Remove_A_Button(RepairButton);
        Map.Remove_A_Button(SellButton);
        Map.Remove_A_Button(PowerButton);
        Map.Remove_A_Button(WaypointButton);
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

    const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);

    if (layout.RepairButton.IsVisible) {
        RepairButton.Draw_Me(true);
    }

    if (layout.SellButton.IsVisible) {
        SellButton.Draw_Me(true);
    }

    if (layout.PowerButton.IsVisible) {
        PowerButton.Draw_Me(true);
    }

    if (layout.WaypointButton.IsVisible) {
        WaypointButton.Draw_Me(true);
    }

    RepairButton.IsDrawn = false;
    SellButton.IsDrawn = false;
    PowerButton.IsDrawn = false;
    WaypointButton.IsDrawn = false;

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

    const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);
    ShapeButtonClass* buttons[] = { &RepairButton, &SellButton, &PowerButton, &WaypointButton };
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

        if (!button_layouts[i]->IsVisible) {
            continue;
        }

        tooltip.Region = Rect(buttons[i]->X, buttons[i]->Y, buttons[i]->Width, buttons[i]->Height);
        tooltip.Text = tooltip_text[i];
        ToolTips->Add(&tooltip);
    }
}
