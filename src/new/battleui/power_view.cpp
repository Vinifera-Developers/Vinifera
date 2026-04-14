/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          POWER_VIEW.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Power bar view implementation. Ported from the original
 *                 Tiberian Sun PowerClass rendering and animation logic.
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

#include "power_view.h"

#include "power_model.h"

#include "building.h"
#include "buildingtype.h"
#include "convert.h"
#include "drawshape.h"
#include "fetchres.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "sidebar.h"
#include "shapeset.h"
#include "surface.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"

#include <algorithm>
#include <cstdio>


const ShapeSet *PowerView::PowerPipShape = nullptr;


namespace
{
const BattleSidebarLayoutBase& Get_Sidebar_Layout(SidebarViewType view_type)
{
    return UIControls->Get_Battle_Sidebar_Config(view_type);
}


TPoint2D<int> Get_Power_Bar_Position(SidebarViewType view_type)
{
    return Get_Sidebar_Layout(view_type).PowerBarPosition;
}


int Get_Power_Bar_Width(SidebarViewType view_type)
{
    return Get_Sidebar_Layout(view_type).PowerBarWidth;
}


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
    IsToRedraw(false),
    FlashTimer(0),
    FlashCount(0),
    UpdateTimer(0),
    GreenPipCount(0),
    YellowPipCount(0),
    RedPipCount(0),
    VisibleButtonsPerColumn(SidebarClass::StripClass::MAX_VISIBLE),
    IsChanged(false),
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
    VisibleButtonsPerColumn = SidebarClass::StripClass::MAX_VISIBLE;
    IsChanged = false;
    IsToRedraw = false;
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
        tt.Region.Height = Get_Sidebar_Layout(ViewType).CameoSize.Y * VisibleButtonsPerColumn;

        ToolTips->Remove(tt.ID);
        ToolTips->Add(&tt);
    }
}


void PowerView::Set_Visible_Buttons_Per_Column(int count)
{
    count = std::max(1, count);
    if (VisibleButtonsPerColumn != count) {
        VisibleButtonsPerColumn = count;
        IsChanged = true;
        IsToRedraw = true;
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
    return Get_Sidebar_Layout(ViewType).CameoSize.Y * VisibleButtonsPerColumn / Get_Power_Pip_Height(ViewType);
}


int PowerView::Current_Power() const
{
    if (Model != nullptr) {
        return Model->Get_Power();
    }

    return PlayerPtr != nullptr ? PlayerPtr->Power : 0;
}


int PowerView::Current_Drain() const
{
    if (Model != nullptr) {
        return Model->Get_Drain();
    }

    return PlayerPtr != nullptr ? PlayerPtr->Drain : 0;
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
    if (current_pips > desired_pips) {
        current_pips = desired_pips;
    }

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

    if (!IsChanged && FlashCount > 0) {
        if (FlashTimer == 0) {
            IsToRedraw = true;
            FlashCount--;
            Map.Redraw_Sidebar();
            FlashTimer = 3;
        }
    }

    /**
     *  If the recorded power or drain value has changed we need to adjust for it.
     */
    if (model_dirty || IsChanged) {

        IsToRedraw = true;
        Map.Redraw_Sidebar();

        /**
         *  Flag to flash the top of the power bar if we're adjusting the bar height.
         */
        if (model_dirty) {
            IsChanged = true;
            Flash_Power();
        }

        int green, yellow, red;
        Desired_Levels(green, yellow, red);

        if (UpdateTimer == 0) {
            IsChanged = false;

            /**
             *  If we need to move the red level height then do so.
             */
            if (RedPipCount != red) {
                IsChanged = true;

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
                IsChanged = true;

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
                IsChanged = true;

                if (YellowPipCount > yellow) {
                    YellowPipCount--;
                    Add_Pip();
                } else {
                    YellowPipCount++;
                    Remove_Pip();
                }
            }

            if (IsChanged) {
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

    IsToRedraw = false;
    RedrawSidebar = true;

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
