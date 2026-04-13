/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_VIEW.H
 *
 *  @author        ZivDero
 *
 *  @brief         Abstract sidebar view interface. Both ClassicSidebarView
 *                 and TabbedSidebarView implement this to provide different
 *                 sidebar layouts over the same data model.
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

#include "point.h"
#include "sidebar.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
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
    virtual void Set_Dimensions() = 0;
    virtual void AI(KeyNumType& input, Point2D& xy) = 0;
    virtual void Draw(bool complete) = 0;
    virtual void Blit(bool complete) = 0;
    virtual void Activate(int control) = 0;

    virtual bool Scroll(bool up, int column) = 0;
    virtual bool Scroll_Page(bool up, int column) = 0;

    virtual const char* Help_Text(int gadget_id) { return nullptr; }
    virtual void Flag_Current_Strip_To_Redraw() {}
    virtual void Flag_Strip_To_Redraw(RTTIType type, ProductionFlags flags) { (void)type; (void)flags; }
    virtual int Visible_Button_Count() const = 0;
    virtual int Visible_Buttons_Per_Column() const = 0;

    virtual bool Change_Tab(int index) { return false; }
    virtual void Action_Bar_AI(KeyNumType& key) {}

protected:
    SidebarModel* Model;
};
