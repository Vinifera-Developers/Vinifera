/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks and patches for the Skirmish Dialog.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "skirmishdlg_hooks.h"

#include "hooker.h"
#include "session.h"
#include "syringe.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"

#include <Commctrl.h>


/**
 *  #issue-346
 *
 *  Fixes a limitation where returning to the Skirmish dialog after a game
 *  clamps the chosen side between 0 (GDI) and 1 (NOD). This means the player
 *  would be forced back to index 1 (NOD) on the combo box if they played as
 *  a new side which was added in a mod for example.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005F7812, _SkirmishDialog_InitDialog_RestoreSideIndex_Patch, 0)
{
    GET(HWND, hSideComboBox, EDI);

    /**
     *  Clamp the chosen House index within the range of known houses. If the
     *  value is out of range for any reason, set back to index 0 (GDI).
     */
    int side_index = Session.House;
    if (side_index >= HouseTypes.Count()) {
        side_index = 0;
    }
    
    /**
     *  Set the combo box entry.
     */
    SendMessage(hSideComboBox, CB_SETCURSEL, (WPARAM)side_index, (LPARAM)0);

    return 0x005F782C;
}


/**
 *  #issue-324
 * 
 *  When the game is running in developer mode, allow Skirmish games to be
 *  started with no AI house(s). This would make testing of features and
 *  mechanics without interference from the AI house(s).
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005F7759, _SkirmishDialog_InitDialog_AIPlayers_Patch, 0)
{
    GET(HWND, hAICountSlider, EBP);

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
    int initial_pos = Session.Options.AIPlayers;
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

    return 0x005F7782;
}


/**
 *  Main function for patching the hooks.
 */
void SkirmishDialog_Hooks()
{

}
