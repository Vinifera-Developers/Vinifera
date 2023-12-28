/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          LOGICEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended LogicClass.
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
#include "logicext_hooks.h"
#include "logic.h"
#include "fatal.h"
#include "tibsun_globals.h"
#include "vinifera_defines.h"
#include "tag.h"
#include "scenario.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  x
 * 
 *  @author: CCHyper
 */
static bool Logic_LogicTriggers_Spring(TagClass *tag)
{
    if (!tag) {
        return false;
    }

    /**
     *	Mission timer state queries.
     */
    if (!Scen->MissionTimer.Expired()) {
        if (tag->Spring(TEventType(TEVENT_MISSION_TIMER_LESS_THAN))) return true;

        if (tag->Spring(TEventType(TEVENT_MISSION_TIMER_GREATER_THAN))) return true;

        if (tag->Spring(TEventType(TEVENT_MISSION_TIMER_EQUALS))) return true;
    }

    return false;
}


/**
 *  x
 */
DECLARE_PATCH(_LogicClass_AI_LogicTriggers_Loop_Patch)
{
    GET_REGISTER(TagClass *, tag, esi);

    /**
     *  Spring any new events. If one is sucessfully sprung, the loop will "continue".
     */
    if (Logic_LogicTriggers_Spring(tag)) {
        goto continue_loop;
    }

    /**
     *  Stolen bytes/code.
     */
    JMP_REG(ecx, 0x00506C1E);

continue_loop:
    JMP_REG(ecx, 0x00506C5E);
}


/**
 *  Main function for patching the hooks.
 */
void LogicClassExtension_Hooks()
{
    Patch_Jump(0x00506C18, &_LogicClass_AI_LogicTriggers_Loop_Patch);
}
