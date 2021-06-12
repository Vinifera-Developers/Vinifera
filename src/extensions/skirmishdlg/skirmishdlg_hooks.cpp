/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SKIRMISHDLG_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks and patches for the Skirmish Dialog.
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
#include "skirmishdlg_hooks.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "session.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include <Commctrl.h>

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-324
 * 
 *  When the game is running in developer mode, allow Skirmish games to be
 *  started with no AI house(s). This would make testing of features and
 *  mechanics without interference from the AI house(s).
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SkirmishDialog_InitDialog_AIPlayers_Patch)
{
    GET_REGISTER_STATIC(HWND, hAICountSlider, ebp);
    static int initial_pos;

    /**
     *  Set the AI Count slider range.
     * 
     *  In developer mode, the slider range is set to allow 0.
     * 
     *  #NOTE: Changed to be available in non-developer mode due to popular vote.
     */
    //if (Vinifera_DeveloperMode) {
        SendMessage(hAICountSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, (MAX_PLAYERS-1)));
    //} else {
    //    SendMessage(hAICountSlider, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(1, (MAX_PLAYERS-1)));
    //}

#ifdef NDEBUG
    /**
     *  Set the slider initial value.
     */
    initial_pos = Session.Options.AIPlayers;
    if (initial_pos <= 1) {
        initial_pos = 1;
    }
    SendMessage(hAICountSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)initial_pos);
#else
    /**
     *  Set the slider position to 0 for debug builds.
     */
    SendMessage(hAICountSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0);
#endif

    JMP(0x005F7782);
}


/**
 *  Main function for patching the hooks.
 */
void SkirmishDialog_Hooks()
{
    Patch_Jump(0x005F7759, &_SkirmishDialog_InitDialog_AIPlayers_Patch);
}
