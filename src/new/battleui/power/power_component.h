/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Power bar component.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "ibattleui_component.h"
#include "power_model.h"
#include "power_view.h"


/**
 *  Power bar component. Owns the data model and view for the power bar
 *  and registers with BattleUISystem for lifecycle management.
 */
class PowerComponent : public IBattleUIComponent
{
public:
    PowerComponent();
    ~PowerComponent() = default;

    /**
     *  IBattleUIComponent
     */
    virtual void One_Time() override;
    virtual void Init_Clear() override;
    virtual void Init_IO() override;
    virtual void Init_For_House() override;
    virtual void AI(KeyNumType &key, Point2D &mouse) override;
    virtual void Draw() override;
    virtual void Blit(bool complete) override;
    virtual void Shift_Sidebar() override;

    virtual const char *Help_Text(int gadget_id) override;

    void Set_Height(int pixels);
    void Set_Sidebar_View_Type(SidebarViewType view_type);
    void Flash_Power();

    PowerModel &Get_Model() { return Model; }
    const PowerModel &Get_Model() const { return Model; }

private:
    PowerModel Model;
    PowerView View;
};
