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


/**
 *  Patch EventClass::Execute to execute our new events.
 *
 *  @author: ZivDero
 */
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


/**
 *  Patch event length in Add_Compressed_Events.
 *
 *  @author: ZivDero
 */
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


/**
 *  Extract_Compressed_Events -- extracts events from a packet.
 *
 *  @author: 11/21/1995 DRD - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
static int Vinifera_Extract_Compressed_Events(void* buf, int bufsize)
{
    int pos = 0;                    // current buffer parsing position
    int leftover = bufsize;         // # bytes left to process
    EventClass* event;              // event ptr for parsing buffer
    int count = 0;                  // # events processed
    int datasize = 0;               // size of data to copy
    EventClass eventdata;           // stores Frame, ID, etc
    unsigned char numunits = 0;     // # units stored in compressed MegaMissions

    /**
     *  Clear work event structure.
     */
    std::memset(&eventdata, 0, sizeof(EventClass));

    /**
     *  Assume the first event is a FRAMEINFO event
     *  Init 'datasize' to the amount of data to copy, minus the EventType value
     *  For the 1st packet only, this will include all info before the Data
     *  union, plus the size of the FrameInfo structure, minus the EventType size.
     */
    datasize = (offsetof(EventClass, Data) + sizeof(EventClass::Data.FrameInfo)) - sizeof(EventType);
    event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);

    while (leftover >= datasize + (int)sizeof(EventType))
    {
        /**
         *  Add event to the DoList, only if it's not a FRAMESYNC
         *  (but FRAMEINFO's do get added.)
         */
        if (event->Type != EVENT_FRAMESYNC)
        {
            /**
             *  Initialize the common data from the FRAMEINFO event.
             *  keeping IsExecuted 0
             */
            if (event->Type == EVENT_FRAMEINFO)
            {
                eventdata.Frame = event->Frame;
                eventdata.ID = event->ID;

                /**
                 *  Adjust position past the common data.
                 */
                pos += offsetof(EventClass, Data) - sizeof(EventType);
                leftover -= offsetof(EventClass, Data) - sizeof(EventType);
            }

            /**
             *  If MEGAMISSION event get the number of units (events to generate).
             */
            else if (event->Type == EVENT_MEGAMISSION)
            {
                numunits = *(static_cast<unsigned char*>(buf) + pos + sizeof(eventdata.Type));
                pos += sizeof(numunits);
                leftover -= sizeof(numunits);
            }

            /**
             *  Clear the union data portion of the event.
             */
            memset(&eventdata.Data, 0, sizeof(eventdata.Data));
            eventdata.Type = event->Type;
            datasize = ViniferaEventClass::Event_Length(eventdata.Type);

            switch (eventdata.Type)
            {
            case EVENT_RESPONSE_TIME:
                memcpy(&eventdata.Data.FrameInfo.Delay, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);
                break;

            case EVENT_ADDPLAYER:
                memcpy(&eventdata.Data.Variable.Size, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);

                eventdata.Data.Variable.Pointer = new char[eventdata.Data.Variable.Size];
                memcpy(eventdata.Data.Variable.Pointer, static_cast<char*>(buf) + pos + sizeof(EventType) + datasize, eventdata.Data.Variable.Size);

                pos += eventdata.Data.Variable.Size;
                leftover -= eventdata.Data.Variable.Size;

                break;

            case EVENT_MEGAMISSION:
                memcpy(&eventdata.Data.MegaMission, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);

                if (numunits > 1)
                {
                    pos += datasize + sizeof(EventType);
                    leftover -= datasize + sizeof(EventType);
                    datasize = sizeof(eventdata.Data.MegaMission.Whom);

                    while (numunits)
                    {
                        if (!DoList.Add(eventdata))
                            return -1;

                        /**
                         *  Keep count of how many events we add to the queue.
                         */
                        count++;
                        numunits--;
                        memcpy(&eventdata.Data.MegaMission.Whom, static_cast<char*>(buf) + pos, datasize);

                        /**
                         *  If one unit left fall thru to normal code.
                         */
                        if (numunits == 1)
                        {
                            datasize -= sizeof(EventType);
                            break;
                        }
                        else
                        {
                            pos += datasize;
                            leftover -= datasize;
                        }
                    }
                }
                break;

            default:
                memcpy(&eventdata.Data, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);
                break;
            }

            if (!DoList.Add(eventdata))
            {
                if (eventdata.Type == EVENT_ADDPLAYER)
                    delete[] eventdata.Data.Variable.Pointer;

                return -1;
            }

            /**
             *  Keep count of how many events we add to the queue.
             */
            count++;

            pos += datasize + sizeof(EventType);
            leftover -= datasize + sizeof(EventType);

            if (leftover)
            {
                event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);
                datasize = ViniferaEventClass::Event_Length(event->Type);
                if (event->Type == EVENT_MEGAMISSION)
                    datasize += sizeof(numunits);
            }
        }
        /**
         *  FRAMESYNC event: This >should< be the only event in the buffer,
         *  and it will be uncompressed.
         */
        else
        {
            pos += datasize + sizeof(EventType);
            leftover -= datasize + sizeof(EventType);
            event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);

            /**
             *  Size of FRAMESYNC event - EventType size.
             */
            datasize = offsetof(EventClass, Data) + sizeof(EventClass::Data.FrameInfo) - sizeof(EventType);
        }
    }

    return count;

}


/**
 *  Main function for patching the hooks.
 */
void ViniferaEvent_Hooks()
{
    Patch_Jump(0x00494294, &_EventClass_Execute_ViniferaEvent);
    Patch_Jump(0x005B45D5, &_Add_Compressed_Events_ViniferaEvent_Length);
    Patch_Jump(0x005B4A40, &Vinifera_Extract_Compressed_Events);
}
