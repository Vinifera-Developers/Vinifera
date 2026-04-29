/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Battle UI component framework. Defines the lifecycle
 *          interface for UI components and the system that drives them.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "power_component.h"
#include "sidebar_component.h"

#include <objidl.h>


/**
 *  Top-level system that owns and drives all battle UI components.
 */
class BattleUISystem
{
public:
    BattleUISystem() = default;
    ~BattleUISystem();

    /**
     *  Lifecycle and frame orchestration.
     */
    void One_Time();
    void Init_Clear();
    void Init_IO();
    void Init_For_House();
    void AI(KeyNumType &key, Point2D &mouse);
    void Draw();
    void Blit(bool complete);
    void Shift_Sidebar();

    /**
     *  Queries and persistence.
     */
    const char *Help_Text(int gadget_id);

    HRESULT Save(IStream *pStm) const;
    HRESULT Load(IStream *pStm);

    /**
     *  Component access.
     */
    SidebarComponent &Get_Sidebar() { return Sidebar; }
    const SidebarComponent &Get_Sidebar() const { return Sidebar; }

private:
    /**
     *  Owned battle UI components.
     */
    SidebarComponent Sidebar;
    PowerComponent Power;
};


extern BattleUISystem BattleUI;
