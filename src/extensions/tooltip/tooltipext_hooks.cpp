/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended ToolTipManager.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "tooltipext_hooks.h"

#include "syringe.h"
#include "tooltip.h"
#include "vinifera_globals.h"


/**
 *  Patch to kill the tooltip timer when the developer option is enabled.
 *
 *  @see: CursorPositionCommandClass.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x006473D4, _ToolTipManager_Message_Handler_CursorPosition_Patch, 0)
{
    GET(ToolTipManager *, this_ptr, ESI);

    /**
     *  If the cursor position command is activated, skip all
     *  tooltip timer loading as this it show have no delay.
     */
    if (Vinifera_Developer_ShowCursorPosition) {
        goto set_tooltip;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    /**
     *  Kill the tooltip timer
     */
    KillTimer(this_ptr->Window, ToolTipManager::TIMER_ID);

    return 0x006473E3;

set_tooltip:
    /**
     *  Record the new mouse position.
     */
    GetCursorPos((LPPOINT)&this_ptr->LastMousePos);
    ScreenToClient(this_ptr->Window, (LPPOINT)&this_ptr->LastMousePos);

    /**
     *  Find the tooltip instance which is defined for this region and
     *  and assign it to the current tooltip pointer.
     */
    this_ptr->CurrentToolTip = this_ptr->Find_From_Pos(this_ptr->LastMousePos);
            
    if (this_ptr->Process()) {
        SetTimer(this_ptr->Window, ToolTipManager::TIMER_ID, this_ptr->ToolTipLifetime, nullptr);
    }

    return 0x006474D2;
}


/**
 *  Main function for patching the hooks.
 */
void ToolTipManagerExtension_Hooks()
{

}
