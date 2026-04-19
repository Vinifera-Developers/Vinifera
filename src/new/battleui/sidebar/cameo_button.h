/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Button class for sidebar cameo slots.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "gcntrl.h"
#include "ihoverable_gadget.h"


class SidebarStripView;


/**
 *  A sidebar cameo slot button. Delegates click handling to the owning
 *  SidebarStripView and tracks hover state for highlight rendering.
 */
class CameoButtonClass : public ControlClass, public IHoverableGadget
{
public:
    CameoButtonClass();
    CameoButtonClass(const NoInitClass& x);

    bool Action(unsigned flags, KeyNumType& key) override;

    void On_Mouse_Enter() override;
    void On_Mouse_Leave() override;

    void Set_Owner(SidebarStripView& strip, int index);

public:
    SidebarStripView* Strip;
    int Index;
    bool IsMousedOver;
};
