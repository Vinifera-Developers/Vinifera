/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_COMPONENT.H
 *
 *  @author        ZivDero
 *
 *  @brief         Sidebar component. Orchestrates the sidebar model, power
 *                 model, and active view implementation.
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

#include "battleui_component.h"
#include "power_model.h"
#include "sidebar_model.h"
#include "sidebar_config.h"


class ISidebarView;


/**
 *  Top-level sidebar component. Owns the data models and the active view,
 *  and implements the IBattleUIComponent lifecycle interface.
 */
class SidebarComponent : public IBattleUIComponent
{
public:
    SidebarComponent();
    ~SidebarComponent();

    /**
     *  IBattleUIComponent
     */
    virtual void One_Time() override;
    virtual void Init_Clear() override;
    virtual void Init_IO() override;
    virtual void Init_For_House() override;
    virtual void AI(KeyNumType& key, Point2D& mouse) override;
    virtual void Draw(bool complete) override;
    virtual void Blit(bool complete) override;
    virtual void Shutdown() override;

    /**
     *  Sidebar-specific interface.
     */
    bool Add(RTTIType type, int id);
    bool Scroll(bool up, int column);
    bool Scroll_Page(bool up, int column);
    void Recalc();
    bool Factory_Link(FactoryClass* factory, RTTIType type, int id);
    bool Abandon_Production(RTTIType type, int id);
    bool Is_On_Sidebar(RTTIType type, int id) const;
    void Activate(int control);
    void Set_Dimensions();
    int Max_Visible() const;
    void Init_Strips();

    SidebarModel& Get_Model() { return Model; }
    const SidebarModel& Get_Model() const { return Model; }
    PowerModel& Get_Power() { return Power; }
    const PowerModel& Get_Power() const { return Power; }
    ISidebarView* Get_View() { return ActiveView; }

private:
    SidebarModel Model;
    PowerModel Power;
    ISidebarView* ActiveView;
};


extern SidebarComponent Sidebar;
