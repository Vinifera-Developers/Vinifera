/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ACTION_BAR_VIEW.H
 *
 *  @author        OpenAI
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

#pragma once

#include "shapebtn.h"
#include "sidebar.h"
#include "uicontrol.h"
#include "wwkeyboard.h"


class ActionBarView
{
public:
    ActionBarView() :
        IsActive(false),
        ViewType(SIDEBAR_CLASSIC)
    {
    }

    void Init_Clear();
    void Init_IO();
    void Init_For_House();
    void Set_Dimensions();
    void Activate(int control);
    void AI(KeyNumType& key);
    void Draw();
    void Set_Sidebar_View_Type(SidebarViewType view_type) { ViewType = view_type; }

private:
    void Register_Tooltips();

    bool IsActive;
    SidebarViewType ViewType;
    ShapeButtonClass RepairButton;
    ShapeButtonClass SellButton;
    ShapeButtonClass PowerButton;
    ShapeButtonClass WaypointButton;
};
