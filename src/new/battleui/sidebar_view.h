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
#include "wwkeyboard.h"
class SidebarModel;
class PowerModel;


/**
 *  Abstract interface for sidebar views. A view is responsible for all
 *  rendering and input handling of the sidebar UI. The data it displays
 *  comes from SidebarModel and PowerModel references.
 */
class ISidebarView
{
public:
    ISidebarView(SidebarModel* model, PowerModel* power) : Model(model), Power(power) {}
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

    virtual int Max_Visible() const = 0;

protected:
    SidebarModel* Model;
    PowerModel* Power;
};
