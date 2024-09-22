/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERAEVENT_HOOKS.H
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the Vinifera event class.
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

#include "viniferaevent_hooks.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "viniferaevent.h"


DECLARE_PATCH(_EventClass_Execute_ViniferaEvent)
{
    GET_REGISTER_STATIC(ViniferaEventClass*, vevent, esi);
    static EventType eventtype;
    static int id;

    _asm pushad

    if (ViniferaEventClass::Is_Vinifera_Event(vevent->Type))
    {
        vevent->Execute();
        _asm popad
        JMP(0x00495110); // return from function
    }

    eventtype = static_cast<EventType>(vevent->Type);
    id = vevent->ID;

    _asm popad

    // Stolen instructions
    _asm mov al, eventtype
    _asm mov edi, id
    JMP_REG(ebx, 0x00494299);
}


DECLARE_PATCH(_Add_Compressed_Events_ViniferaEvent_Length)
{
    GET_REGISTER_STATIC(unsigned char, eventtype, cl);
    static unsigned char eventlength;

    _asm pushad

    if (ViniferaEventClass::Is_Vinifera_Event(static_cast<ViniferaEventType>(eventtype)))
    {
        eventlength = ViniferaEventClass::Event_Length(static_cast<ViniferaEventType>(eventtype));
    }
    else
    {
        eventlength = EventClass::Event_Length(static_cast<EventType>(eventtype));
    }

    if (eventtype == EVENT_ADDPLAYER)
    {
        _asm popad
        _asm mov bl, eventlength
        JMP_REG(esi, 0x005B45EA);
    }
    else
    {
        _asm popad
        _asm mov bl, eventlength
        JMP_REG(esi, 0x005B45F3);
    }
}


DECLARE_PATCH(_Extract_Compressed_Events_ViniferaEvent_Length1)
{
    GET_REGISTER_STATIC(unsigned int, eventtype, ecx);
    static unsigned char eventlength;

    _asm pushad

    if (ViniferaEventClass::Is_Vinifera_Event(static_cast<ViniferaEventType>(eventtype)))
    {
        eventlength = ViniferaEventClass::Event_Length(static_cast<ViniferaEventType>(eventtype));
    }
    else
    {
        eventlength = EventClass::Event_Length(static_cast<EventType>(eventtype));
    }

    _asm popad
    _asm mov bl, eventlength

    JMP_REG(esi, 0x005B4AF3);
}


DECLARE_PATCH(_Extract_Compressed_Events_ViniferaEvent_Length2)
{
    GET_REGISTER_STATIC(unsigned int, eventtype, ecx);
    static unsigned char eventlength;

    _asm pushad

    if (ViniferaEventClass::Is_Vinifera_Event(static_cast<ViniferaEventType>(eventtype)))
    {
        eventlength = ViniferaEventClass::Event_Length(static_cast<ViniferaEventType>(eventtype));
    }
    else
    {
        eventlength = EventClass::Event_Length(static_cast<EventType>(eventtype));
    }

    _asm popad
    _asm mov bl, eventlength

    JMP_REG(esi, 0x005B4CFE);
}



void ViniferaEvent_Hooks()
{
    Patch_Jump(0x00494294, &_EventClass_Execute_ViniferaEvent);
    Patch_Jump(0x005B45D5, &_Add_Compressed_Events_ViniferaEvent_Length);
    Patch_Jump(0x005B4AED, &_Extract_Compressed_Events_ViniferaEvent_Length1);
    Patch_Jump(0x005B4CF8, &_Extract_Compressed_Events_ViniferaEvent_Length2);
}
