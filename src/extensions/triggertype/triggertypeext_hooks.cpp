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
#include "extension.h"

#include "hooker.h"
#include "syringe.h"
#include "tevent.h"
#include "vinifera_defines.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TriggerTypeClass)
{
public:
    bool _Is_Tied_To_Global(int global) const;
    bool _Is_Tied_To_Local(int local) const;
};


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
DEFINE_HOOK(0x0064A2CE, _TriggerTypeClass_Read_INI_Load_Difficulty_Patch, 0)
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


static bool Is_Global_Variable_Event(TEventType event)
{
    switch (event) {
    case TEVENT_GLOBAL_SET:
    case TEVENT_GLOBAL_CLEAR:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_CONSTANT:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_GLOBAL:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_LOCAL:
    case EXT_TEVENT_GLOBAL_EQUALS_CONSTANT:
    case EXT_TEVENT_GLOBAL_EQUALS_GLOBAL:
    case EXT_TEVENT_GLOBAL_EQUALS_LOCAL:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_CONSTANT:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_GLOBAL:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_LOCAL:
    case EXT_TEVENT_GLOBAL_LESS_THAN_CONSTANT:
    case EXT_TEVENT_GLOBAL_LESS_THAN_GLOBAL:
    case EXT_TEVENT_GLOBAL_LESS_THAN_LOCAL:
        return true;
    }

    return false;
}


static bool Is_Local_Variable_Event(TEventType event)
{
    switch (event) {
    case TEVENT_LOCAL_SET:
    case TEVENT_LOCAL_CLEAR:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_CONSTANT:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_GLOBAL:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_LOCAL:
    case EXT_TEVENT_LOCAL_EQUALS_CONSTANT:
    case EXT_TEVENT_LOCAL_EQUALS_GLOBAL:
    case EXT_TEVENT_LOCAL_EQUALS_LOCAL:
    case EXT_TEVENT_LOCAL_GREATER_THAN_CONSTANT:
    case EXT_TEVENT_LOCAL_GREATER_THAN_GLOBAL:
    case EXT_TEVENT_LOCAL_GREATER_THAN_LOCAL:
    case EXT_TEVENT_LOCAL_LESS_THAN_CONSTANT:
    case EXT_TEVENT_LOCAL_LESS_THAN_GLOBAL:
    case EXT_TEVENT_LOCAL_LESS_THAN_LOCAL:
        return true;
    }

    return false;
}


/**
 *  Re-implementations of Is_Tied_To_Global and Is_Tied_To_Local to
 *  ensure timers are properly reset for global and local events.
 *
 *  @author: ZivDero, tomsons26
 */
bool TriggerTypeClassExt::_Is_Tied_To_Global(int global) const
{
    bool tied = false;
    TEventClass* event = Event;
    while (event != nullptr) {
        if (Is_Global_Variable_Event(event->Event) && event->Data.Value == global) {
            tied = true;
            break;
        }
        event = event->Next;
    }
    return tied;
}


bool TriggerTypeClassExt::_Is_Tied_To_Local(int local) const
{
    bool tied = false;
    TEventClass* event = Event;
    while (event != nullptr) {
        if (Is_Local_Variable_Event(event->Event) && event->Data.Value == local) {
            tied = true;
            break;
        }
        event = event->Next;
    }
    return tied;
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

    Patch_Jump(0x00649FB0, &TriggerTypeClassExt::_Is_Tied_To_Global);
    Patch_Jump(0x00649FF0, &TriggerTypeClassExt::_Is_Tied_To_Local);
}
