/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Abstract sidebar view interface. Both ClassicSidebarView
 *          and TabbedSidebarView implement this to provide different
 *          sidebar layouts over the same data model.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "point.h"
#include "wwkeyboard.h"

class SidebarModel;


/**
 *  Abstract interface for sidebar views. A view is responsible for all
 *  rendering and input handling of the sidebar UI. The data it displays
 *  comes from the SidebarModel reference.
 */
class ISidebarView
{
public:
    ISidebarView(SidebarModel* model) : Model(model) {}
    virtual ~ISidebarView() = default;

    virtual void One_Time() = 0;
    virtual void Init_Clear() = 0;
    virtual void Init_IO() = 0;
    virtual void Init_For_House() = 0;
    virtual void Shift_Sidebar() = 0;
    virtual void AI(KeyNumType& input, Point2D& xy) = 0;
    virtual void Draw() = 0;
    virtual void Blit(bool complete) = 0;
    virtual void Activate(int control) = 0;

    virtual bool Scroll(bool up, int column) = 0;
    virtual bool Scroll_Page(bool up, int column) = 0;

    virtual const char* Help_Text(int gadget_id) { return nullptr; }
    virtual int Visible_Button_Count() const = 0;
    virtual int Visible_Buttons_Per_Column() const = 0;

    virtual bool Change_Tab(int index) { return false; }
    virtual void Notify_Production_Complete(int category_index) { (void)category_index; }

protected:
    SidebarModel* Model;
};
