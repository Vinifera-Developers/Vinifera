/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Button class for sidebar cameo slots.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "cameo_button.h"

#include "battleui.h"
#include "sidebar_strip_view.h"
#include "scenario.h"


/**
 *  Default constructor.
 *
 *  @author: ZivDero
 */
CameoButtonClass::CameoButtonClass() :
    ControlClass(0, 0, 0, 0, 0,
                 LEFTPRESS | RIGHTPRESS | LEFTUP),
    Strip(nullptr),
    Index(0),
    IsMousedOver(false)
{
}


/**
 *  No-init constructor. Used when constructing inside a container without initialization.
 *
 *  @author: ZivDero
 */
CameoButtonClass::CameoButtonClass(const NoInitClass& x) :
    ControlClass(x),
    Strip(nullptr),
    Index(0),
    IsMousedOver(false)
{
}


/**
 *  Handles button input for cameo clicks: left-click to produce/place,
 *  right-click to suspend/cancel.
 *
 *  @author: ZivDero, Rampastring
 */
bool CameoButtonClass::Action(unsigned flags, KeyNumType& key)
{
    if (Scen->InputLock) {
        return true;
    }

    if (!Strip) {
        return true;
    }

    BattleUI.Get_Sidebar().Handle_Cameo_Action(*Strip, Index, flags);
    ControlClass::Action(flags, key);
    return true;
}


/**
 *  Called when the mouse enters the cameo button region.
 *
 *  @author: ZivDero
 */
void CameoButtonClass::On_Mouse_Enter()
{
    IsMousedOver = true;
}


/**
 *  Called when the mouse leaves the cameo button region.
 *
 *  @author: ZivDero
 */
void CameoButtonClass::On_Mouse_Leave()
{
    IsMousedOver = false;
}


/**
 *  Sets the owning strip and slot index for this button.
 *
 *  @author: ZivDero
 */
void CameoButtonClass::Set_Owner(SidebarStripView& strip, int index)
{
    Strip = &strip;
    Index = index;
}
