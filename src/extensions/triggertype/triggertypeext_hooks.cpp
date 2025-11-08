/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TRIGGERTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TriggerTypeClass.
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
#include "triggertypeext_hooks.h"
#include "triggertype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "syringe.h"


/**
 *  #issue-299
 * 
 *  Fixes the issue with the difficulty flags not being loaded correctly. The
 *  original code only set these values if they were "true", but they are already
 *  initialised to that in the TriggerTypeClass constructor...
 * 
 *  @see: TriggerClass and TActionClass for the other parts of this fix.
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_TriggerTypeClass_Read_INI_Load_Difficulty_Patch)
{
    GET(TriggerTypeClass *, this_ptr, EBP);

    char* token = std::strtok(nullptr, ",");
    this_ptr->IsEnabledEasy = token && std::atoi(token);

    token = std::strtok(nullptr, ",");
    this_ptr->IsEnabledMedium = token && std::atoi(token);

    token = std::strtok(nullptr, ",");
    this_ptr->IsEnabledHard = token && std::atoi(token);

    return 0x0064A337;
}


/**
 *  Main function for patching the hooks.
 */
void TriggerTypeClassExtension_Hooks()
{
    /**
     *  This patch skips the code for setting the enabled state of the
     *  trigger, we have moved this to the TriggerClass constructor now.
     */
    Patch_Jump(0x0064A35A, 0x0064A3A7);
}

declhook(0x0064A2CE, _TriggerTypeClass_Read_INI_Load_Difficulty_Patch, 0);
