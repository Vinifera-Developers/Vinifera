/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Classic sidebar view implementation. Two single-column strips
 *          side by side, matching the vanilla Tiberian Sun layout.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sidebar_classic_view.h"

#include "battleui.h"
#include "cameo_button.h"
#include "drawshape.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "power.h"
#include "sidebar.h"
#include "sidebar_model.h"
#include "sidebar_render_utils.h"
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
    strip_layout.RowSpacing = layout.RowSpacing;
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
    RegisteredTooltipCount(0),
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
    RegisteredTooltipCount = 0;

    for (auto& strip : Strip) {
        strip.Init_Clear();
    }
}


/**
 *  Initializes IO gadgets. Sets up strips and links them to model categories.
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

    for (auto& strip : Strip) {
        strip.Set_Art(strip_art);
        strip.Init_For_House();
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
        int tooltip_id = 1000;

        for (int i = 0; i < RegisteredTooltipCount; i++) {
            ToolTips->Remove(1000 + i);
        }

        for (auto& strip : Strip) {
            for (int i = 0; i < strip.MaxVisibleCount; i++) {
                CameoButtonClass* btn = strip.SelectButtons[i];
                tooltip.Region = Rect(btn->X, btn->Y, btn->Width, btn->Height);
                tooltip.ID = tooltip_id++;
                tooltip.Text = TXT_NONE;
                ToolTips->Add(&tooltip);
            }
        }

        RegisteredTooltipCount = tooltip_id - 1000;
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
    for (auto& strip : Strip) {
        strip.AI(input, xy);
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

    /**
     *  Draw the strips.
     */
    for (auto& strip : Strip) {
        strip.Draw(*SidebarSurface, rect);
    }

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
void ClassicSidebarView::Activate(bool enabled)
{
    if (enabled) {
        Background.Zap();
        Map.Add_A_Button(Background);

        for (auto& strip : Strip) {
            strip.Activate();
        }
    } else {
        Map.Remove_A_Button(Background);

        for (auto& strip : Strip) {
            strip.Deactivate();
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
    bool scrolled = false;
    if (column < 0) {
        for (auto& strip : Strip) {
            scrolled |= strip.Scroll(up);
        }
    } else if (column < COLUMN_COUNT) {
        scrolled = Strip[column].Scroll(up);
    }
    return scrolled;
}


/**
 *  Page-scrolls a specific column.
 *
 *  @author: ZivDero
 */
bool ClassicSidebarView::Scroll_Page(bool up, int column)
{
    bool scrolled = false;
    if (column < 0) {
        for (auto& strip : Strip) {
            scrolled |= strip.Scroll_Page(up);
        }
    } else if (column < COLUMN_COUNT) {
        scrolled = Strip[column].Scroll_Page(up);
    }
    return scrolled;
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

    for (auto& strip : Strip) {
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


/**
 *  Captures per-strip visible-item state before the sidebar model recalcs.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Prepare_Model_Recalc()
{
    for (auto& strip : Strip) {
        strip.Prepare_Model_Recalc();
    }
}


/**
 *  Restores per-strip scroll positions after the sidebar model recalcs.
 *
 *  @author: ZivDero
 */
void ClassicSidebarView::Finish_Model_Recalc()
{
    for (auto& strip : Strip) {
        strip.Finish_Model_Recalc();
    }
}


