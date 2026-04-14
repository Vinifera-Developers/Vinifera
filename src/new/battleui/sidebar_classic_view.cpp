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

#include "drawshape.h"
#include "fetchres.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "options.h"
#include "power.h"
#include "sidebar.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"


namespace
{
/**
 *  Builds the configured strip layout for the left or right classic column.
 *
 *  @author: ZivDero
 */
SidebarStripView::StripLayout Build_Classic_Strip_Layout(bool left_strip)
{
    const SidebarClassicLayout& layout = UIControls->ClassicSidebarLayoutConfig;
    SidebarStripView::StripLayout strip_layout;

    strip_layout.Position = left_strip ? layout.LeftStripPosition : layout.RightStripPosition;
    strip_layout.VisibleRows = layout.VisibleRows;
    strip_layout.RowPitch = layout.RowPitch;
    strip_layout.CameoSize = layout.CameoSize;
    strip_layout.CameoNameOffset = layout.CameoNameOffset;
    strip_layout.CameoTextOffset = layout.CameoTextOffset;
    strip_layout.QueueCountOffset = layout.QueueCountOffset;

    if (left_strip) {
        strip_layout.UpButtonPosition = layout.LeftUpButtonPosition;
        strip_layout.DownButtonPosition = layout.LeftDownButtonPosition;
        strip_layout.IsUpButtonVisible = layout.IsLeftUpButtonVisible;
        strip_layout.IsDownButtonVisible = layout.IsLeftDownButtonVisible;
    } else {
        strip_layout.UpButtonPosition = layout.RightUpButtonPosition;
        strip_layout.DownButtonPosition = layout.RightDownButtonPosition;
        strip_layout.IsUpButtonVisible = layout.IsRightUpButtonVisible;
        strip_layout.IsDownButtonVisible = layout.IsRightDownButtonVisible;
    }

    return strip_layout;
}
}


/***************************************************************************
**  Lifecycle and layout
***************************************************************************/


/**
 *  ClassicSidebarView constructor.
 *
 *  @author: ZivDero
 */
ClassicSidebarView::ClassicSidebarView(SidebarModel* model) :
    ISidebarView(model),
    Strip(),
    BackgroundTopShape(nullptr),
    BackgroundMiddleShape(nullptr),
    BackgroundBottomShape(nullptr),
    BackgroundAddonShape(nullptr),
    ClockShape(nullptr),
    RechargeClockShape(nullptr)
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

}


/**
 *  Loads house-specific shapes for the classic sidebar.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Init_For_House()
{
    const SidebarClassicLayout& layout = UIControls->ClassicSidebarLayoutConfig;

    BackgroundTopShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarShape.c_str());
    BackgroundMiddleShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarMiddleShape.c_str());
    BackgroundBottomShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarBottomShape.c_str());
    BackgroundAddonShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarAddonShape.c_str());
    ClockShape = MFCD::RetrieveT<ShapeSet>(layout.ClockShape.c_str());
    RechargeClockShape = MFCD::RetrieveT<ShapeSet>(layout.RechargeClockShape.c_str());

    SidebarStripView::StripArt strip_art;
    strip_art.ScrollUpButtonShape = MFCD::RetrieveT<ShapeSet>(layout.ScrollUpButtonShape.c_str());
    strip_art.ScrollDownButtonShape = MFCD::RetrieveT<ShapeSet>(layout.ScrollDownButtonShape.c_str());
    strip_art.DarkenShape = MFCD::RetrieveT<ShapeSet>(layout.DarkenShape.c_str());
    strip_art.ClockShape = ClockShape;
    strip_art.RechargeClockShape = RechargeClockShape;
    strip_art.BackgroundTopHeight = BackgroundTopShape != nullptr ? BackgroundTopShape->Get_Height() : 0;
    strip_art.BackgroundBottomHeight = BackgroundBottomShape != nullptr ? BackgroundBottomShape->Get_Height() : 0;

    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].Set_Art(strip_art);
        Strip[i].Init_For_House();
    }
}


/**
 *  Reflows the classic sidebar layout.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Shift_Sidebar()
{
    Background.Set_Position(SidebarRect.X + 16, TacticalRect.Y);
    Background.Flag_To_Redraw();

    /**
     *  Position the two strips — vanilla layout: buildings left, units right.
     */
    Strip[0].Set_Layout(Build_Classic_Strip_Layout(true));
    Strip[0].Shift_Sidebar();
    Strip[1].Set_Layout(Build_Classic_Strip_Layout(false));
    Strip[1].Shift_Sidebar();

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

    }
}


/***************************************************************************
**  Runtime behavior
***************************************************************************/


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
void ClassicSidebarView::Draw()
{
    if (!Map.IsSidebarActive || Debug_Map || SidebarSurface == nullptr) {
        return;
    }

    Surface* oldsurface = LogicalSurface;
    LogicalSurface = SidebarSurface;

    if (BackgroundTopShape == nullptr
        || BackgroundMiddleShape == nullptr
        || BackgroundBottomShape == nullptr
        || BackgroundAddonShape == nullptr) {
        LogicalSurface = oldsurface;
        return;
    }

    Rect rect(0, 0, SidebarSurface->Get_Width(), SidebarSurface->Get_Height());

    /**
     *  Repaint the sidebar background every active frame so the surface is
     *  fully refreshed before blitting.
     */
    int y = SidebarRect.Y;
    Point2D xy(0, y);
    Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundTopShape, 0, xy, rect, SHAPE_WIN_REL);
    y += BackgroundTopShape->Get_Height();

    int rows = Background_Row_Count();
    for (int i = 0; i < rows; i++, y += BackgroundMiddleShape->Get_Height()) {
        xy = Point2D(0, y);
        Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundMiddleShape, 0, xy, rect, SHAPE_WIN_REL);
    }

    xy = Point2D(0, y);
    Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundBottomShape, 0, xy, rect, SHAPE_WIN_REL);

    xy = Point2D(0, y + BackgroundBottomShape->Get_Height());
    Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundAddonShape, 0, xy, rect, SHAPE_WIN_REL);

    RedrawSidebar = true;

    /**
     *  Draw the strips.
     */
    for (int i = 0; i < COLUMN_COUNT; i++) {
        Strip[i].Draw(*SidebarSurface, rect);
    }

    if (ToolTips) {
        ToolTips->Force_Redraw(true);
    }

    Map.IsToRedraw = false;
    Map.IsToFullRedraw = false;

    LogicalSurface = oldsurface;
}


/**
 *  Returns how many background middle rows fit in the current sidebar height.
 *
 *  @author: ZivDero
 */
int ClassicSidebarView::Background_Row_Count() const
{
    if (SidebarSurface != nullptr
        && BackgroundTopShape != nullptr
        && BackgroundMiddleShape != nullptr
        && BackgroundBottomShape != nullptr) {
        return (SidebarRect.Height
                - BackgroundBottomShape->Get_Height()
                - BackgroundTopShape->Get_Height())
               / BackgroundMiddleShape->Get_Height();
    }

    return SidebarClass::StripClass::MAX_VISIBLE;
}


/***************************************************************************
**  View queries and control
***************************************************************************/


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
        Background.Zap();
        Map.Add_A_Button(Background);

        for (int i = 0; i < COLUMN_COUNT; i++) {
            Strip[i].Activate();
        }
    } else {
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
 *  Flags the visible classic strips for redraw.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Flag_Strip_To_Redraw()
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

        const BuildItem* item = strip.Get_Visible_Item(offset);
        if (item == nullptr) {
            return nullptr;
        }

        return Format_Cameo_Tooltip(*item);
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
    return Strip[0].Visible_Button_Count() + Strip[1].Visible_Button_Count();
}


/**
 *  Returns the number of visible buttons in one classic column.
 *
 *  @author: ZivDero
 */
int ClassicSidebarView::Visible_Buttons_Per_Column() const
{
    return Strip[0].Visible_Buttons_Per_Column();
}


