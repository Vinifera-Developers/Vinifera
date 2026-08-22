/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TriggerClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "hooker.h"
#include "scenario.h"
#include "session.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "triggertype.h"


/**
 *  #issue-299
 *
 *  Fixes the issue with the current difficulty not being checked
 *  when enabling triggers.
 *
 *  @see: TriggerTypeClass and TActionClass for the other parts of this fix.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00649171, _TriggerClass_Constructor_Enabled_For_Difficulty_Patch, 0)
{
    GET(TriggerClass *, this_ptr, ESI);

    /**
     *  This is direct port of the code from Red Alert 2, which looks to fix this issue.
     */

    if (this_ptr->Class) {

        this_ptr->Reset_Timer_Events();

        /**
         *  Set this trigger to be disabled if;
         *    - The class instance is marked as inactive.
         *    - It is marked as disabled for this current mission difficulty.
         *
         *  Rampastring: Check CDifficulty instead of Difficulty.
         *  Also, in non-campaign games, consider all difficulties enabled regardless of what the trigger specifies.
         */
        if (!this_ptr->Class->Enabled ||
          (Session.Type == GAME_NORMAL && 
          ((Scen->CDifficulty == DIFF_HARD && !this_ptr->Class->IsEnabledEasy)
          || (Scen->CDifficulty == DIFF_NORMAL && !this_ptr->Class->IsEnabledMedium)
          || (Scen->CDifficulty == DIFF_EASY && !this_ptr->Class->IsEnabledHard)))) {

            this_ptr->IsEnabled = false;
        }
    }

    return 0x00649188;
}


/**
 *  Main function for patching the hooks.
 */
void TriggerClassExtension_Hooks()
{

}
