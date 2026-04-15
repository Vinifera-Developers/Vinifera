/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Classic sidebar view — two single-column strips side by
 *          side, matching the vanilla Tiberian Sun layout.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "sidebar.h"
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
