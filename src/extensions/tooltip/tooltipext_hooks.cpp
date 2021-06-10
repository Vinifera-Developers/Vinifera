/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TOOLTIPEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ToolTipManager.
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
#include "tooltipext_hooks.h"
#include "vinifera_globals.h"
#include "tooltip.h"
#include "cctooltip.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Patch to kill the tooltip timer when the developer option is enabled.
 * 
 *  @see: CursorPositionCommandClass.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ToolTipManager_Message_Handler_CursorPosition_Patch)
{
    GET_REGISTER_STATIC(ToolTipManager *, this_ptr, esi);

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

    JMP(0x006473E3);

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

    JMP(0x006474D2);
}


/**
 *  Main function for patching the hooks.
 */
void ToolTipManagerExtension_Hooks()
{
    Patch_Jump(0x006473D4, &_ToolTipManager_Message_Handler_CursorPosition_Patch);
}
