/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TRIGGEREXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TriggerClass.
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
#include "sideext_hooks.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "triggertype.h"
#include "scenario.h"
#include "session.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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
DECLARE_PATCH(_TriggerClass_Constructor_Enabled_For_Difficulty_Patch)
{
    GET_REGISTER_STATIC(TriggerClass *, this_ptr, esi);

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
          || (Scen->CDifficulty == DIFF_EASY && !this_ptr->Class-IsEnabled>Hard)))) {

            this_ptr->IsEnabled = false;
        }
    }

    JMP(0x00649188);
}


/**
 *  Main function for patching the hooks.
 */
void TriggerClassExtension_Hooks()
{
    Patch_Jump(0x00649171, &_TriggerClass_Constructor_Enabled_For_Difficulty_Patch);
}
