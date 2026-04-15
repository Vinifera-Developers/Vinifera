/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Lifecycle interface for battle UI components.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "point.h"
#include "wwkeyboard.h"


/**
 *  Lifecycle interface for battle UI components (sidebar, radar, etc.).
 */
class IBattleUIComponent
{
public:
    virtual ~IBattleUIComponent() = default;

    virtual void One_Time() = 0;
    virtual void Init_Clear() = 0;
    virtual void Init_IO() = 0;
    virtual void Init_For_House() = 0;
    virtual void AI(KeyNumType &key, Point2D &mouse) = 0;
    virtual void Draw() = 0;
    virtual void Blit(bool complete) = 0;
    virtual void Shift_Sidebar() = 0;
    virtual void Shutdown() = 0;

    virtual const char *Help_Text(int gadget_id) { return nullptr; }

    virtual HRESULT Save(IStream *pStm) const { return S_OK; }
    virtual HRESULT Load(IStream *pStm) { return S_OK; }
};
