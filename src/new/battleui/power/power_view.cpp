/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Power bar view implementation. Ported from the original
 *          Tiberian Sun PowerClass rendering and animation logic.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "power_view.h"

#include "battleui.h"
#include "building.h"
#include "convert.h"
#include "drawshape.h"
#include "fetchres.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "power_model.h"
#include "shapeset.h"
#include "surface.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"

#include <algorithm>


const ShapeSet *PowerView::PowerPipShape = nullptr;


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


/**
 *  Returns the power bar's sidebar-relative position for the given view type.
 *
 *  @author: ZivDero
 */
TPoint2D<int> Get_Power_Bar_Position(SidebarViewType view_type)
{
    return Get_Sidebar_Layout(view_type).PowerBarPosition;
}


/**
 *  Returns the power bar width in pixels for the given view type.
 *
 *  @author: ZivDero
 */
int Get_Power_Bar_Width(SidebarViewType view_type)
{
    return Get_Sidebar_Layout(view_type).PowerBarWidth;
}


/**
 *  Returns the height of one power pip in pixels for the given view type.
 *
 *  @author: ZivDero
 */
int Get_Power_Pip_Height(SidebarViewType view_type)
{
    return Get_Sidebar_Layout(view_type).PowerPipHeight;
}
}


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
PowerView::PowerView() :
    Model(nullptr),
    FlashTimer(0),
    FlashCount(0),
    UpdateTimer(0),
    GreenPipCount(0),
    YellowPipCount(0),
    RedPipCount(0),
    PixelHeight(0),
    HasChanged(false),
    ViewType(SIDEBAR_CLASSIC)
{
}


/**
 *  One-time initialization. Called once at game startup.
 *
 *  @author: ZivDero
 */
void PowerView::One_Time()
{
}


/**
 *  Clears all state for a new scenario.
 *
 *  @author: ZivDero
 */
void PowerView::Init_Clear()
{
    FlashTimer = 0;
    FlashCount = 0;
    UpdateTimer = 0;
    GreenPipCount = 0;
    YellowPipCount = 0;
    RedPipCount = 0;
    PixelHeight = 0;
    HasChanged = false;
}


/**
 *  Loads art assets for the power bar.
 *
 *  @author: ZivDero
 */
void PowerView::Init_For_House()
{
    const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);
    PowerPipShape = MFCD::RetrieveT<ShapeSet>(layout.PowerPipShape.c_str());
}


/**
 *  Registers the power bar tooltip region.
 *
 *  @author: ZivDero
 */
void PowerView::Shift_Sidebar()
{
    if (ToolTips != nullptr) {
        const TPoint2D<int> power_bar_position = Get_Power_Bar_Position(ViewType);
        ToolTip tt;
        tt.Text = TXT_NONE;
        tt.ID = GADGET_POWER;
        tt.Region.X = SidebarRect.X + power_bar_position.X;
        tt.Region.Y = SidebarRect.Y + power_bar_position.Y;
        tt.Region.Width = Get_Power_Bar_Width(ViewType);
        tt.Region.Height = PixelHeight;

        ToolTips->Remove(tt.ID);
        ToolTips->Add(&tt);
    }
}


/**
 *  Sets the available pixel height for the power bar and flags a redraw.
 *
 *  @author: ZivDero
 */
void PowerView::Set_Height(int pixels)
{
    pixels = std::max(1, pixels);
    if (PixelHeight != pixels) {
        PixelHeight = pixels;
        HasChanged = true;
    }
}


/**
 *  Triggers the power bar flash effect.
 *
 *  @author: ZivDero
 */
void PowerView::Flash_Power()
{
    FlashCount = 10;
    FlashTimer = 3;
}


/**
 *  Returns the maximum number of pips that fit in the power bar area.
 *
 *  @author: ZivDero
 */
int PowerView::Max_Power_Height() const
{
    const BattleSidebarLayoutBase& layout = Get_Sidebar_Layout(ViewType);
    return (PixelHeight + layout.PowerBarHeightAdjust) / Get_Power_Pip_Height(ViewType);
}


/**
 *  Returns the player's current power output.
 *
 *  @author: ZivDero
 */
int PowerView::Current_Power() const
{
    if (Model != nullptr) {
        return Model->Get_Power();
    }

    return 0;
}


/**
 *  Returns the player's current power drain.
 *
 *  @author: ZivDero
 */
int PowerView::Current_Drain() const
{
    if (Model != nullptr) {
        return Model->Get_Drain();
    }

    return 0;
}


/**
 *  Returns the desired total pip count based on current power/drain.
 *
 *  @author: ZivDero
 */
int PowerView::Desired_Power_Height() const
{
    int max_pips = Max_Power_Height();
    int drain = Current_Drain();
    int power = Current_Power();

    int empty_pips = static_cast<int>(400.0 / (drain + power + 400.0) * max_pips);
    empty_pips = std::max(empty_pips, 0);
    empty_pips = std::min(empty_pips, max_pips - 1);

    return max_pips - empty_pips;
}


/**
 *  Returns the animation speed delay based on how close we are
 *  to the desired level.
 *
 *  @author: ZivDero
 */
int PowerView::Update_Delay() const
{
    int desired_pips = Desired_Power_Height();

    int current_pips = GreenPipCount + YellowPipCount + RedPipCount;
    current_pips = std::min(current_pips, desired_pips);

    if (desired_pips == 0) {
        return 0;
    }

    return static_cast<int>(static_cast<double>(current_pips) / static_cast<double>(desired_pips) * 5.0);
}


/**
 *  Computes the desired pip distribution (green/yellow/red).
 *  Returns the maximum pip count.
 *
 *  @author: ZivDero
 */
int PowerView::Desired_Levels(int &green, int &yellow, int &red) const
{
    int max_pips = Max_Power_Height();
    int desired_pips = Desired_Power_Height();
    const double power = Current_Power();
    const double drain = Current_Drain();

    double power_delta = power - drain;

    double green_power = 0.0;
    double yellow_power = 100.0;

    if (power_delta < 0.0) {
        yellow_power = 0.0;
        green_power = 0.0;
    } else {
        if (power_delta < 100.0) {
            yellow_power = power_delta;
        }
        green_power = power_delta - yellow_power;
    }

    double red_fraction = 1.0;
    double green_fraction = 0.0;
    double yellow_fraction = 0.0;

    double total_power = drain + yellow_power + green_power;

    if (total_power > 0.0) {
        red_fraction = drain / total_power;
        green_fraction = green_power / total_power;
        yellow_fraction = yellow_power / total_power;
    }

    red = static_cast<int>(desired_pips * red_fraction);
    yellow = static_cast<int>(desired_pips * yellow_fraction);
    green = static_cast<int>(desired_pips * green_fraction);

    red += static_cast<int>(
        (desired_pips * green_fraction - green)
        + (desired_pips * yellow_fraction - yellow)
        + (desired_pips * red_fraction - red)
        + 0.01);

    return max_pips;
}


/**
 *  Removes one pip, preferring to remove from colors that are
 *  over their desired level.
 *
 *  @author: ZivDero
 */
void PowerView::Remove_Pip()
{
    int green, red, yellow;
    Desired_Levels(green, yellow, red);

    if (GreenPipCount > green) {
        GreenPipCount--;
    } else if (RedPipCount > red) {
        RedPipCount--;
    } else if (YellowPipCount > yellow) {
        YellowPipCount--;
    }
}


/**
 *  Adds one pip, preferring to add to colors that are under
 *  their desired level.
 *
 *  @author: ZivDero
 */
void PowerView::Add_Pip()
{
    int green, red, yellow;
    Desired_Levels(green, yellow, red);

    if (RedPipCount < red) {
        RedPipCount++;
    } else if (GreenPipCount < green) {
        GreenPipCount++;
    } else if (YellowPipCount < yellow) {
        YellowPipCount++;
    }
}


/**
 *  Per-frame update. Handles flash timing and incremental pip
 *  adjustment toward desired levels.
 *
 *  @author: ZivDero
 */
void PowerView::AI()
{
    if (!Map.IsSidebarActive) {
        return;
    }

    const bool model_dirty = Model != nullptr && Model->Is_Dirty();

    if (!HasChanged && FlashCount > 0) {
        if (FlashTimer == 0) {
            FlashCount--;
            Map.Redraw_Sidebar();
            FlashTimer = 3;
        }
    }

    /**
     *  If the recorded power or drain value has changed we need to adjust for it.
     */
    if (model_dirty || HasChanged) {

        Map.Redraw_Sidebar();

        /**
         *  Flag to flash the top of the power bar if we're adjusting the bar height.
         */
        if (model_dirty) {
            HasChanged = true;
            Flash_Power();
        }

        int green, yellow, red;
        Desired_Levels(green, yellow, red);

        if (UpdateTimer == 0) {
            HasChanged = false;

            /**
             *  If we need to move the red level height then do so.
             */
            if (RedPipCount != red) {
                HasChanged = true;

                if (RedPipCount > red) {
                    RedPipCount--;
                    Add_Pip();
                } else {
                    RedPipCount++;
                    Remove_Pip();
                }

            /**
             *  If we need to move the green level height then do so.
             */
            } else if (GreenPipCount != green) {
                HasChanged = true;

                if (GreenPipCount > green) {
                    GreenPipCount--;
                    Add_Pip();
                } else {
                    GreenPipCount++;
                    Remove_Pip();
                }

            /**
             *  If we need to move the yellow level height then do so.
             */
            } else if (YellowPipCount != yellow) {
                HasChanged = true;

                if (YellowPipCount > yellow) {
                    YellowPipCount--;
                    Add_Pip();
                } else {
                    YellowPipCount++;
                    Remove_Pip();
                }
            }

            if (HasChanged) {
                UpdateTimer = Update_Delay();
            }
        }

        if (Model != nullptr) {
            Model->Clear_Dirty();
        }
    }
}


/**
 *  Renders the power bar to the sidebar surface.
 *
 *  @author: ZivDero
 */
void PowerView::Draw()
{
    if (!Map.IsSidebarActive) {
        return;
    }

    if (PowerPipShape == nullptr || SidebarSurface == nullptr) {
        return;
    }

    Rect rect = SidebarSurface->Get_Rect();
    const TPoint2D<int> power_bar_position = Get_Power_Bar_Position(ViewType);
    const int power_pip_height = Get_Power_Pip_Height(ViewType);
    int x = power_bar_position.X;
    int y = SidebarRect.Y + power_bar_position.Y;

    int num = Max_Power_Height() - RedPipCount - YellowPipCount - GreenPipCount;

    int index;
    for (index = 0; index < num; index++) {
        Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_EMPTY, Point2D(x, y), rect, SHAPE_WIN_REL);
        y += power_pip_height;
    }

    index = 0;
    if (FlashCount > 0) {
        if ((FlashCount % 2) == 0) {
            Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_WHITE, Point2D(x, y), rect, SHAPE_WIN_REL);
            y += power_pip_height;
            index++;
        }
    }

    if (GreenPipCount > 0) {
        while (index < GreenPipCount) {
            Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_GREEN, Point2D(x, y), rect, SHAPE_WIN_REL);
            y += power_pip_height;
            index++;
        }
        index = 0;
    }

    if (YellowPipCount > 0) {
        while (index < YellowPipCount) {
            Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_YELLOW, Point2D(x, y), rect, SHAPE_WIN_REL);
            y += power_pip_height;
            index++;
        }
        index = 0;
    }

    if (RedPipCount > 0) {
        while (index < RedPipCount) {
            Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_RED, Point2D(x, y), rect, SHAPE_WIN_REL);
            y += power_pip_height;
            index++;
        }
        index = 0;
    }
}


/**
 *  Returns help text for the power bar tooltip.
 *
 *  @author: ZivDero
 */
const char *PowerView::Help_Text(int gadget_id)
{
    static char _str[128];

    if (gadget_id == GADGET_POWER) {
        std::sprintf(_str, Fetch_String(TXT_POWER_DRAIN), Current_Power(), Current_Drain());
        return _str;
    }

    return nullptr;
}
