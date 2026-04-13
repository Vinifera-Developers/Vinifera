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

#include "action_bar_view.h"
#include "ibattleui_component.h"
#include "sidebar_model.h"
#include "uicontrol.h"
#include "vinifera_defines.h"

#include <objidl.h>


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
    virtual void Set_Dimensions() override;
    virtual void Shutdown() override;

    virtual const char *Help_Text(int gadget_id) override;

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
    void Flag_Current_Strip_To_Redraw();
    void Flag_Strip_To_Redraw(RTTIType type, ProductionFlags flags);
    bool Change_Tab(int index);
    void Detach(AbstractClass* target);
    void Activate(int control);
    int Visible_Button_Count() const;
    int Visible_Buttons_Per_Column() const;
    void Init_Strips();

    HRESULT Save(IStream* pStm) const override;
    HRESULT Load(IStream* pStm) override;
    void Relink_Factories();

    SidebarModel& Get_Model() { return Model; }
    const SidebarModel& Get_Model() const { return Model; }
    ISidebarView* Get_View() { return ActiveView; }

private:
    ActionBarView ActionBar;
    SidebarModel Model;
    ISidebarView* ActiveView;
};
