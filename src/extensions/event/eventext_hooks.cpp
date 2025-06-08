/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EVENTEXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended EventClass.
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
#include "eventext_hooks.h"
#include "event.h"
#include "eventext.h"

#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"


/**
 *  This patch intercepts EventClass::Execute and executes the event if it's one of ours.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_EventClass_Execute_New_Events)
{
    GET_REGISTER_STATIC(EventClassExt*, event, esi);

    _asm pushad

    if (event->Is_Vinifera_Event()) {
        event->Execute();
        _asm popad
        JMP(0x00495110); // return
    }

    static EventType etype;
    static int eID;

    etype = event->Type;
    eID = event->ID;

    // continue execution
    _asm popad
    _asm mov al, etype
    _asm mov edi, eID
    JMP_REG(ecx, 0x00494299);
}


/**
 *  Main function for patching the hooks.
 */
void EventClassExtension_Hooks()
{
    Patch_Jump(0x00494294, &_EventClass_Execute_New_Events);
}
