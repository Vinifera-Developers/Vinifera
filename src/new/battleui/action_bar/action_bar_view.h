/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Shared action bar view for sidebar-common buttons.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "shapebtn.h"
#include "uicontrol.h"
#include "wwkeyboard.h"

#include <array>


class ActionBarView
{
public:
    ActionBarView() :
        IsActive(false),
        ViewType(SIDEBAR_CLASSIC)
    {
    }

    /**
     *  Lifecycle and layout.
     */
    void Init_Clear();
    void Init_IO();
    void Init_For_House();
    void Shift_Sidebar();
    void Activate(bool enabled);

    /**
     *  Runtime behavior.
     */
    void AI(KeyNumType& key);
    void Draw();
    void Set_Sidebar_View_Type(SidebarViewType view_type) { ViewType = view_type; }

private:
    struct ButtonInfo {
        ShapeButtonClass& Button;
        const SidebarButtonLayout& Layout;
        int ID;
        int TooltipText;
        const std::string& ShapeName;
    };

    std::array<ButtonInfo, 4> Get_Button_Info();

    /**
     *  Tooltip helpers.
     */
    void Register_Tooltips();

    /**
     *  Owned action buttons and state.
     */
    bool IsActive;
    SidebarViewType ViewType;
    ShapeButtonClass RepairButton;
    ShapeButtonClass SellButton;
    ShapeButtonClass PowerButton;
    ShapeButtonClass WaypointButton;
};
