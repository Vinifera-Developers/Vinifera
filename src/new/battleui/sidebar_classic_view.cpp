/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_CLASSIC_VIEW.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Classic sidebar view implementation.
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

#include "sidebar_classic_view.h"

#include "cameo_button.h"
#include "sidebar_model.h"
#include "sidebar_render_utils.h"
#include "power_model.h"

#include "convert.h"
#include "drawshape.h"
#include "fetchres.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "options.h"
#include "palette.h"
#include "power.h"
#include "sidebar.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"


/**
 *  ClassicSidebarView constructor.
 *
 *  @author: ZivDero
 */
ClassicSidebarView::ClassicSidebarView(SidebarModel* model) :
    ISidebarView(model),
    Strip()
{
}


/**
 *  One-time initialization. Loads shapes and initializes strips.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::One_Time()
{
    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].One_Time(i);
    }

    /**
     *  Load the clock shapes.
     */
    SidebarClass::StripClass::RechargeClockShape = MFCD::RetrieveT<ShapeSet>("RCLOCK2.SHP");
    SidebarClass::StripClass::ClockShape = MFCD::RetrieveT<ShapeSet>("GCLOCK2.SHP");
}


/**
 *  Clears state for a new scenario.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Init_Clear()
{
    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].Init_Clear();
    }
}


/**
 *  Initializes IO gadgets. Sets up action buttons and strip gadgets.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Init_IO()
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

    /**
     *  Initialize the two strips as single-column mode.
     */
    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].Init_IO(i, 1);
    }

    /**
     *  Link strip views to model categories.
     */
    if (Model->Category_Count() >= COLUMN_COUNT) {
        Strip[0].Set_Category(&Model->Get_Category(0));
        Strip[1].Set_Category(&Model->Get_Category(1));
    }

    Set_Dimensions();
}


/**
 *  Loads house-specific shapes for the sidebar and buttons.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Init_For_House()
{
    PaletteClass pal("SIDEBAR.PAL");

    delete SidebarDrawer;
    SidebarDrawer = new ConvertClass(&pal, &pal, VisibleSurface, 1);

    Sell.Set_Shape(MFCD::RetrieveT<ShapeSet>("SELL.SHP"));
    Sell.ShapeDrawer = SidebarDrawer;

    PowerBtn.Set_Shape(MFCD::RetrieveT<ShapeSet>("POWER.SHP"));
    PowerBtn.ShapeDrawer = SidebarDrawer;

    Waypoint.Set_Shape(MFCD::RetrieveT<ShapeSet>("WAYP.SHP"));
    Waypoint.ShapeDrawer = SidebarDrawer;

    Repair.Set_Shape(MFCD::RetrieveT<ShapeSet>("REPAIR.SHP"));
    Repair.ShapeDrawer = SidebarDrawer;

    SidebarClass::SidebarShape = MFCD::RetrieveT<ShapeSet>("SIDE1.SHP");
    SidebarClass::SidebarMiddleShape = MFCD::RetrieveT<ShapeSet>("SIDE2.SHP");
    SidebarClass::SidebarBottomShape = MFCD::RetrieveT<ShapeSet>("SIDE3.SHP");
    SidebarClass::SidebarAddonShape = MFCD::RetrieveT<ShapeSet>("ADDON.SHP");

    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].Init_For_House(i);
    }
}


/**
 *  Recalculates positions of all buttons and strips.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Set_Dimensions()
{
    Background.Set_Position(SidebarRect.X + 16, TacticalRect.Y);
    Background.Flag_To_Redraw();

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

    /**
     *  Position the two strips — vanilla layout: buildings left, units right.
     */
    Strip[0].Set_Dimensions(COLUMN_ONE_X, COLUMN_ONE_Y);
    Strip[1].Set_Dimensions(COLUMN_TWO_X, COLUMN_TWO_Y);

    /**
     *  Set up tooltips for the select buttons.
     */
    if (ToolTips) {
        ToolTip tooltip;

        for (int i = 0; i < 100; i++) {
            ToolTips->Remove(1000 + i);
        }

        for (int col = 0; col < COLUMN_COUNT; col++) {
            for (int i = 0; i < Strip[col].MaxVisibleCount; i++) {
                CameoButtonClass* btn = Strip[col].SelectButtons[i];
                tooltip.Region = Rect(btn->X, btn->Y, btn->Width, btn->Height);
                tooltip.ID = 1000 + col * Strip[col].MaxVisibleCount + i;
                tooltip.Text = TXT_NONE;
                ToolTips->Add(&tooltip);
            }
        }

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
}


/**
 *  Per-frame logic. Processes both strips and action button inputs.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::AI(KeyNumType& input, Point2D& xy)
{
    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].AI(input, xy);
    }
}


/**
 *  Draws the entire classic sidebar — background, action buttons, and strips.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Draw(bool complete)
{
    Surface* oldsurface = LogicalSurface;
    LogicalSurface = SidebarSurface;

    Rect rect(0, 0, SidebarSurface->Get_Width(), SidebarSurface->Get_Height());

    if (Map.IsSidebarActive && (Map.IsToRedraw || complete) && !Debug_Map) {
        if (complete || Strip[0].IsToRedraw || Strip[1].IsToRedraw) {
            /**
             *  Draw the sidebar background shapes.
             */
            int y = SidebarRect.Y;
            Point2D xy(0, y);
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarShape, 0, xy, rect, SHAPE_WIN_REL);
            y += SidebarClass::SidebarShape->Get_Height();

            int rows = Visible_Buttons_Per_Column();
            for (int i = 0; i < rows; i++, y += SidebarClass::SidebarMiddleShape->Get_Height()) {
                xy = Point2D(0, y);
                Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarMiddleShape, 0, xy, rect, SHAPE_WIN_REL);
            }

            xy = Point2D(0, y);
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarBottomShape, 0, xy, rect, SHAPE_WIN_REL);

            xy = Point2D(0, y + SidebarClass::SidebarBottomShape->Get_Height());
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarAddonShape, 0, xy, rect, SHAPE_WIN_REL);

            Strip[0].IsToRedraw = true;
            Strip[1].IsToRedraw = true;
        }

        /**
         *  Draw action buttons.
         */
        Repair.Draw_Me(true);
        Sell.Draw_Me(true);
        PowerBtn.Draw_Me(true);
        Waypoint.Draw_Me(true);

        RedrawSidebar = true;
    }

    /**
     *  Draw the strips.
     */
    if (Map.IsSidebarActive) {
        for (int i = 0; i < COLUMN_COUNT; i++) {
            Strip[i].Draw(*SidebarSurface, rect, complete);
        }
    }

    /**
     *  Check if action buttons drew themselves.
     */
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

    if (ToolTips) {
        ToolTips->Force_Redraw(true);
    }

    Map.IsToRedraw = false;
    Map.IsToFullRedraw = false;

    LogicalSurface = oldsurface;
}


/**
 *  Blits the sidebar surface to the visible screen.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Blit(bool complete)
{
    Map.Blit_Sidebar(complete);
}


/**
 *  Activates or deactivates the sidebar.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Activate(int control)
{
    if (control) {
        Repair.Zap();
        Map.Add_A_Button(Repair);
        Sell.Zap();
        Map.Add_A_Button(Sell);
        PowerBtn.Zap();
        Map.Add_A_Button(PowerBtn);
        Waypoint.Zap();
        Map.Add_A_Button(Waypoint);
        Background.Zap();
        Map.Add_A_Button(Background);

        for (int i = 0; i < COLUMN_COUNT; i++) {
            Strip[i].Activate();
        }
    } else {
        Map.Remove_A_Button(Repair);
        Map.Remove_A_Button(Sell);
        Map.Remove_A_Button(PowerBtn);
        Map.Remove_A_Button(Waypoint);
        Map.Remove_A_Button(Background);

        for (int i = 0; i < COLUMN_COUNT; i++) {
            Strip[i].Deactivate();
        }
    }
}


/**
 *  Scrolls a specific column.
 *
 *  @author: ZivDero
 */
bool ClassicSidebarView::Scroll(bool up, int column)
{
    if (column >= 0 && column < COLUMN_COUNT) {
        return Strip[column].Scroll(up);
    }
    return false;
}


/**
 *  Page-scrolls a specific column.
 *
 *  @author: ZivDero
 */
bool ClassicSidebarView::Scroll_Page(bool up, int column)
{
    if (column >= 0 && column < COLUMN_COUNT) {
        return Strip[column].Scroll_Page(up);
    }
    return false;
}


/**
 *  Flags the active strips for redraw.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Flag_Current_Strip_To_Redraw()
{
    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].Flag_To_Redraw();
    }
}


/**
 *  Flags the routed classic column for redraw.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Flag_Strip_To_Redraw(RTTIType type, ProductionFlags flags)
{
    (void)flags;

    int column = 1;
    if (type == RTTI_BUILDINGTYPE || type == RTTI_BUILDING) {
        column = 0;
    }

    if (column >= 0 && column < COLUMN_COUNT) {
        Strip[column].Flag_To_Redraw();
    }
}


/**
 *  Returns tooltip text for a cameo slot in classic layout.
 *
 *  @author: ZivDero
 */
const char* ClassicSidebarView::Help_Text(int gadget_id)
{
    int offset = gadget_id - 1000;
    if (offset < 0) {
        return nullptr;
    }

    for (int column = 0; column < COLUMN_COUNT; column++) {
        SidebarStripView& strip = Strip[column];
        if (offset >= strip.MaxVisibleCount) {
            offset -= strip.MaxVisibleCount;
            continue;
        }

        BuildCategory* category = strip.Get_Category();
        if (category == nullptr) {
            return nullptr;
        }

        int item_index = strip.TopIndex + offset;
        if (item_index < 0 || item_index >= category->Items.Count()) {
            return nullptr;
        }

        return Format_Cameo_Tooltip(category->Items[item_index]);
    }

    return nullptr;
}


/**
 *  Returns the total number of visible buttons across both classic columns.
 *
 *  @author: ZivDero
 */
int ClassicSidebarView::Visible_Button_Count() const
{
    return Visible_Buttons_Per_Column() * COLUMN_COUNT;
}


/**
 *  Returns the number of visible buttons in one classic column.
 *
 *  @author: ZivDero
 */
int ClassicSidebarView::Visible_Buttons_Per_Column() const
{
    if (SidebarSurface != nullptr && SidebarClass::SidebarShape != nullptr) {
        return (SidebarRect.Height
                - SidebarClass::SidebarBottomShape->Get_Height()
                - SidebarClass::SidebarShape->Get_Height())
               / SidebarClass::SidebarMiddleShape->Get_Height();
    }
    return SidebarClass::StripClass::MAX_VISIBLE;
}


/**
 *  Handles action button input and synchronizes visual state with mode flags.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Action_Bar_AI(KeyNumType& key)
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
