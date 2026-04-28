/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Sidebar component.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "action_bar_view.h"
#include "ibattleui_component.h"
#include "sidebar_model.h"
#include "uicontrol.h"


class ISidebarView;
class SidebarStripView;


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
     *  IBattleUIComponent lifecycle.
     */
    virtual void One_Time() override;
    virtual void Init_Clear() override;
    virtual void Init_IO() override;
    virtual void Init_For_House() override;
    virtual void AI(KeyNumType& key, Point2D& mouse) override;
    virtual void Draw() override;
    virtual void Blit(bool complete) override;
    virtual void Shift_Sidebar() override;

    virtual const char *Help_Text(int gadget_id) override;

    /**
     *  Sidebar-specific runtime interface.
     */
    bool Add(RTTIType type, int id);
    bool Scroll(bool up, int column);
    bool Scroll_Page(bool up, int column);
    void Recalc();
    bool Factory_Link(FactoryClass* factory, RTTIType type, int id);
    bool Abandon_Production(RTTIType type, int id);
    bool Handle_Cameo_Action(SidebarStripView& strip, int slot, unsigned& flags);
    bool Is_On_Sidebar(RTTIType type, int id) const;
    bool Change_Tab(int index);
    void Detach(AbstractClass* target);
    void Activate(bool enabled);
    int Visible_Button_Count() const;
    int Visible_Buttons_Per_Column() const;
    SidebarViewType Get_View_Type() const { return ActiveViewType; }

    /**
     *  Persistence and relink helpers.
     */
    HRESULT Save(IStream* pStm) const override;
    HRESULT Load(IStream* pStm) override;
    void Relink_Factories();

    /**
     *  Model and view access.
     */
    SidebarModel& Get_Model() { return Model; }
    const SidebarModel& Get_Model() const { return Model; }
    ISidebarView* Get_View() { return ActiveView; }

private:
    /**
     *  Internal update helpers.
     */
    void Prepare_Drawer();
    void Production_AI();

    /**
     *  Owned models and active view state.
     */
    ActionBarView ActionBar;
    SidebarModel Model;
    ISidebarView* ActiveView;
    SidebarViewType ActiveViewType;
};
