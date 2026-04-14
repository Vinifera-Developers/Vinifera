/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_CLASSIC_VIEW.H
 *
 *  @author        ZivDero
 *
 *  @brief         Classic sidebar view — two single-column strips side by
 *                 side, matching the vanilla Tiberian Sun layout.
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

#pragma once

#include "sidebar_strip_view.h"
#include "sidebar_view.h"

class ShapeSet;


class ClassicSidebarView : public ISidebarView
{
public:
    static constexpr int COLUMN_COUNT = 2;

    ClassicSidebarView(SidebarModel* model);
    virtual ~ClassicSidebarView() override = default;

    /**
     *  ISidebarView lifecycle and rendering.
     */
    virtual void One_Time() override;
    virtual void Init_Clear() override;
    virtual void Init_IO() override;
    virtual void Init_For_House() override;
    virtual void Shift_Sidebar() override;
    virtual void AI(KeyNumType& input, Point2D& xy) override;
    virtual void Draw() override;
    virtual void Blit(bool complete) override;
    virtual void Activate(int control) override;

    virtual bool Scroll(bool up, int column) override;
    virtual bool Scroll_Page(bool up, int column) override;

    virtual const char* Help_Text(int gadget_id) override;
    virtual void Flag_Strip_To_Redraw() override;
    virtual void Flag_Strip_To_Redraw(RTTIType type, ProductionFlags flags) override;
    virtual int Visible_Button_Count() const override;
    virtual int Visible_Buttons_Per_Column() const override;

private:
    /**
     *  Background layout helpers.
     */
    int Background_Row_Count() const;

    /**
     *  Number of cameo tooltip IDs registered during the last layout pass.
     */
    int RegisteredTooltipCount;

    /**
     *  Owned strip and background state.
     */
    SidebarStripView Strip[COLUMN_COUNT];
    SidebarClass::SBGadgetClass Background;
    const ShapeSet* BackgroundTopShape;
    const ShapeSet* BackgroundMiddleShape;
    const ShapeSet* BackgroundBottomShape;
    const ShapeSet* BackgroundAddonShape;
    const ShapeSet* ClockShape;
    const ShapeSet* RechargeClockShape;
};
