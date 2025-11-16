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
#include "house.h"
#include "session.h"
#include "syringe.h"
#include "version.h"


/**
 *  This patch intercepts EventClass::Execute and executes the event if it's one of ours.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00494294, _EventClass_Execute_New_Events, 0x5)
{
    GET(EventClassExt*, event, ESI);

    if (event->Is_Vinifera_Event()) {
        event->Execute();
        return 0x00495110; // return
    }

    return 0;
}


/**
 *  Replacement of Add_Compressed_Events to support new events lengths.
 *
 *  @author: tomsons26, ZivDero
 */
static int _Add_Compressed_Events(void* buf, int bufsize, int frame_delay, int size, int cap, int& processed)
{
    int num = 0;                        // Number of events processed
    unsigned char eventtype;            // Type of event being compressed
    EventClass prevevent;               // Last event processed
    int datasize;                       // Size of element plucked from event union
    int storedsize;                     // Actual number of bytes stored from event
    unsigned char* unitsptr = nullptr;  // Pointer to buffer position to store MegaMission rep count
    unsigned char numunits = 0;         // MegaMission rep count value
    bool missiondup = false;            // Flag: is this event a MegaMission repeat?

    /**
     *  Clear previous event.
     */
    memset(&prevevent, 0, sizeof(EventClass));

    if (Debug_Print_Events) {
        DEBUG_INFO("\nFrame %d: Building Send Packet\n", Frame);
    }

    /**
     *  Loop until there are no more events, we've processed our max number of events, or the buffer is full.
     */
    while (OutList.Count && num < cap) {

        eventtype = OutList.First().Type;
        datasize = EventClassExt::Event_Length(static_cast<EventType>(eventtype));

        /**
         *  For a variable-sized event, pull the size from the event; otherwise,
         *  the size will be the data element size plus the event type value.
         *  (The other data elements in the event, Frame, ID, etc, are stored
         *  in the packet header.)
         */
        if (eventtype == EVENT_ADDPLAYER) {
            storedsize = datasize + sizeof(EventClass::Type) + OutList.First().Data.Variable.Size;
        } else {
            storedsize = datasize + sizeof(EventClass::Type);
        }

        /**
         *  MegaMission compression: MegaMissions are stored as:
         *   EventType
         *   Rep Count
         *   MegaMission structure (event #1 only)
         *   Whom #2
         *   Whom #3
         *   Whom #4
         *   ...
         *   Whom #n
         */
        if (prevevent.Type == EVENT_MEGAMISSION) {

            /**
             *  If previous & current events are both MegaMissions:
             */
            if (eventtype == EVENT_MEGAMISSION) {

                /**
                 *  If the Mission, Target, & Destination are the same, compress
                 *  the events into one:
                 *  - Change datasize to the size of the 'Whom' field only
                 *  - Set total number of bytes to store to the size of the 'Whom' only
                 *  - Increment the MegaMission rep count
                 *  - Set the MegaMission rep flag
                 */
                if (OutList.First().Data.MegaMission.Mission == prevevent.Data.MegaMission.Mission &&
                    OutList.First().Data.MegaMission.Target == prevevent.Data.MegaMission.Target &&
                    OutList.First().Data.MegaMission.Destination == prevevent.Data.MegaMission.Destination) {

                    if (Debug_Print_Events) {
                        DEBUG_INFO("      adding Whom:%x Mission:%s Target:%x Dest:%x\n", OutList.First().Data.MegaMission.Whom.Encode(), MissionClass::Mission_Name(OutList.First().Data.MegaMission.Mission), OutList.First().Data.MegaMission.Target.Encode(), OutList.First().Data.MegaMission.Destination.Encode());
                    }

                    datasize = sizeof(prevevent.Data.MegaMission.Whom);
                    storedsize = datasize;
                    numunits++;
                    missiondup = true;
                }

                /**
                 *  Data doesn't match; start a new run of MegaMissions:
                 *  - Store previous MegaMission rep count
                 *  - Init 'unitsptr' to buffer pos after next EventType
                 *  - Set total number of bytes to store to 'datasize' + sizeof(EventType) + sizeof(numunits)
                 *  - Init the MegaMission rep count to 1
                 *  - Clear the MegaMission rep flag
                 */
                else {
                    if (Debug_Print_Events) {
                        DEBUG_INFO("  New MEGAMISSION run:\n");
                    }

                    *unitsptr = numunits;
                    unitsptr = static_cast<unsigned char*>(buf) + size + sizeof(EventClass::Type);
                    storedsize += sizeof(numunits);
                    numunits = 1;
                    missiondup = false;
                }
            }

            /**
             *  Previous event was a MegaMission, but this one isn't: end the
             *  run of MegaMissions:
             *  - Store previous MegaMission rep count
             *  - Clear variables
             */
            else {
                *unitsptr = numunits;   // save number of events in our run
                unitsptr = nullptr;     // init other values
                numunits = 0;
                missiondup = false;
            }
        }

        /**
         *  The previous event is not a MEGAMISSION but the current event is:
         *  Set up a new run of MegaMissions:
         *  - Init 'unitsptr' to buffer pos after next EventType
         *  - Set total number of bytes to store to 'datasize' + sizeof(EventType) + sizeof(numunits)
         *  - Init the MegaMission rep count to 1
         *  - Clear the MegaMission rep flag
         */
        else if (eventtype == EVENT_MEGAMISSION) {
            if (Debug_Print_Events) {
                DEBUG_INFO("  New MEGAMISSION run:\n");
            }

            unitsptr = static_cast<unsigned char*>(buf) + size + sizeof(EventClass::Type);
            storedsize += sizeof(numunits);
            numunits = 1;
            missiondup = false;
        }

        /**
         *  Will the next event exceed the size of the buffer? If so, stop compressing.
         */
        if (size + storedsize > bufsize) break;

        /**
         *  Set the event's frame delay (this is protocol-dependent)
         */
        if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
            OutList.First().Frame = (Frame + frame_delay + (Session.FrameSendRate - 1)) / Session.FrameSendRate * Session.FrameSendRate;
        } else {
            OutList.First().Frame = Frame + frame_delay;
        }

        /**
         *  Set the event's ID
         */
        OutList.First().ID = PlayerPtr->HeapID;

        /**
         *  Transfer the event in OutList to DoList, un-queue the OutList event.
         *  If the DoList is full, stop transferring immediately.
         */
        OutList.First().IsExecuted = 0;
        if (!DoList.Add(OutList.First())) {
            break;
        }

        /**
         *  Compress the event into the send packet buffer
         */
        switch (eventtype) {

            /**
             *  RESPONSE_TIME: just use the Delay field of the FrameInfo union
             */
        case EVENT_RESPONSE_TIME:
            *reinterpret_cast<unsigned char*>(static_cast<char*>(buf) + size) = eventtype;
            memcpy(static_cast<char*>(buf) + size + sizeof(EventClass::Type), &OutList.First().Data.FrameInfo.Delay, datasize);
            size += datasize + sizeof(EventClass::Type);
            break;

            /**
             *  MEGAMISSION:
             *  Repeated mission in a run:
             *   - Update the rep count (in case we break out)
             *   - Copy the Whom field only
             *  1st mission in a run:
             *   - Init the rep count (in case we break out)
             *   - Set the EventType
             *   - Copy the MegaMission structure, leaving room for 'numunits'
             */
        case EVENT_MEGAMISSION:
            if (missiondup) {
                *unitsptr = numunits;
                memcpy(static_cast<char*>(buf) + size, &OutList.First().Data.MegaMission.Whom, datasize);
                size += datasize;
            } else {
                *unitsptr = numunits;
                *reinterpret_cast<unsigned char*>(static_cast<char*>(buf) + size) = eventtype;
                memcpy(static_cast<char*>(buf) + size + sizeof(EventClass::Type) + sizeof(numunits), &OutList.First().Data.MegaMission, datasize);
                size += datasize + sizeof(EventClass::Type) + sizeof(numunits);
            }
            break;

            /**
             *  Variable-sized packets: Copy the packet Size & the buffer
             */
        case EVENT_ADDPLAYER:
            *reinterpret_cast<unsigned char*>(static_cast<char*>(buf) + size) = eventtype;
            memcpy(static_cast<char*>(buf) + size + sizeof(EventClass::Type), &OutList.First().Data.Variable.Size, datasize);
            size += datasize + sizeof(EventClass::Type);
            memcpy(static_cast<char*>(buf) + size, OutList.First().Data.Variable.Pointer, OutList.First().Data.Variable.Size);
            size += OutList.First().Data.Variable.Size;
            break;

            /**
             *  Default case: Just copy over the data field from the union
             */
        default:
            *reinterpret_cast<unsigned char*>(static_cast<char*>(buf) + size) = eventtype;
            memcpy(static_cast<char*>(buf) + size + sizeof(EventClass::Type), &OutList.First().Data, datasize);
            size += datasize + sizeof(EventClass::Type);
            break;
        }

        /**
         *  Update number of events processed
         */
        num++;

        /**
         *  Update 'prevevent'
         */
        memcpy(&prevevent, &OutList.First(), sizeof(EventClass));

        /**
         *  Go to the next event to process
         */
        OutList.Next();
    }

    processed = num;
    return size;
}

/**
 *  Replacement of Extract_Compressed_Events to support new events lengths.
 *
 *  @author: tomsons26, ZivDero
 */
static int _Extract_Compressed_Events(void* buf, int bufsize)
{
    int pos = 0;                // Current buffer parsing position
    int leftover = bufsize;     // Number of bytes left to process
    EventClass* event;          // Event pointer for parsing buffer
    int count = 0;              // Number of events processed
    int datasize = 0;           // Size of data to copy
    EventClass eventdata;       // Stores Frame, ID, etc
    unsigned char numunits = 0; // Number of units stored in compressed MegaMissions

    /**
     *  Clear work event structure
     */
    memset(&eventdata, 0, sizeof(EventClass));

    /**
     *  Assume the first event is a FRAMEINFO event
     *  Init 'datasize' to the amount of data to copy, minus the EventType value
     *  For the 1st packet only, this will include all info before the Data
     *  union, plus the size of the FrameInfo structure, minus the EventType size.
     */
    datasize = offsetof(EventClass, Data) + sizeof(EventClass::Data.FrameInfo) - sizeof(EventClass::Type);
    event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);

    while (leftover >= datasize + sizeof(EventClass::Type)) {

        /**
         *  Add event to the DoList, only if it's not a FRAMESYNC
         *  (but FRAMEINFO's do get added.)
         */
        if (event->Type != EVENT_FRAMESYNC) {

            /**
             *  Initialize the common data from the FRAMEINFO event
             *  keeping IsExecuted 0
             */
            if (event->Type == EVENT_FRAMEINFO) {
                eventdata.Frame = event->Frame;
                eventdata.ID = event->ID;

                /**
                 *  Adjust position past the common data
                 */
                pos += offsetof(EventClass, Data) - sizeof(EventClass::Type);
                leftover -= offsetof(EventClass, Data) - sizeof(EventClass::Type);
            }

            /**
             *  If MEGAMISSION event get the number of units (events to generate)
             */
            else if (event->Type == EVENT_MEGAMISSION) {
                numunits = *(static_cast<unsigned char*>(buf) + pos + sizeof(eventdata.Type));
                pos += sizeof(numunits);
                leftover -= sizeof(numunits);
            }

            /**
             *  Clear the union data portion of the event
             */
            memset(&eventdata.Data, 0, sizeof(eventdata.Data));
            eventdata.Type = event->Type;
            datasize = EventClassExt::Event_Length(eventdata.Type);

            switch (eventdata.Type) {
            case EVENT_RESPONSE_TIME:
                memcpy(&eventdata.Data.FrameInfo.Delay, static_cast<char*>(buf) + pos + sizeof(EventClass::Type), datasize);
                break;
            case EVENT_ADDPLAYER:
                memcpy(&eventdata.Data.Variable.Size, static_cast<char*>(buf) + pos + sizeof(EventClass::Type), datasize);
                eventdata.Data.Variable.Pointer = new char[eventdata.Data.Variable.Size];
                memcpy(eventdata.Data.Variable.Pointer, static_cast<char*>(buf) + pos + sizeof(EventClass::Type) + datasize, eventdata.Data.Variable.Size);
                pos += eventdata.Data.Variable.Size;
                leftover -= eventdata.Data.Variable.Size;
                break;
            case EVENT_MEGAMISSION:
                memcpy(&eventdata.Data.MegaMission, static_cast<char*>(buf) + pos + sizeof(EventClass::Type), datasize);
                if (numunits > 1) {
                    pos += datasize + sizeof(EventClass::Type);
                    leftover -= datasize + sizeof(EventClass::Type);
                    datasize = sizeof(eventdata.Data.MegaMission.Whom);
                    while (numunits) {
                        if (!DoList.Add(eventdata)) {
                            return -1;
                        }

                        /**
                         *  Keep count of how many events we add to the queue
                         */
                        count++;
                        numunits--;
                        memcpy(&eventdata.Data.MegaMission.Whom, static_cast<char*>(buf) + pos, datasize);

                        /**
                         *  If one unit left fall through to normal code
                         */
                        if (numunits == 1) {
                            datasize -= sizeof(EventClass::Type);
                            break;
                        } else {
                            pos += datasize;
                            leftover -= datasize;
                        }
                    }
                }
                break;
            default:
                memcpy(&eventdata.Data, static_cast<char*>(buf) + pos + sizeof(EventClass::Type), datasize);
                break;
            }

            if (!DoList.Add(eventdata)) {
                if (eventdata.Type == EVENT_ADDPLAYER) {
                    delete[] eventdata.Data.Variable.Pointer;
                }
                return -1;
            }

            /**
             *  Keep count of how many events we add to the queue
             */
            count++;

            pos += datasize + sizeof(EventClass::Type);
            leftover -= datasize + sizeof(EventClass::Type);

            if (leftover) {
                event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);
                datasize = EventClassExt::Event_Length(event->Type);
                if (event->Type == EVENT_MEGAMISSION) {
                    datasize += sizeof(numunits);
                }
            }
        }

        /**
         *  FRAMESYNC event: This should be the only event in the buffer,
         *  and it will be uncompressed.
         */
        else {
            pos += datasize + sizeof(EventClass::Type);
            leftover -= datasize + sizeof(EventClass::Type);
            event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);

            /**
             *  Size of FRAMESYNC event - EventType size
             */
            datasize = offsetof(EventClass, Data) + sizeof(EventClass::Data.FrameInfo) - sizeof(EventClass::Type);
        }
    }

    return count;
}


/**
 *  Main function for patching the hooks.
 */
void EventClassExtension_Hooks()
{
    //Patch_Jump(0x005B4530, &_Add_Compressed_Events);
    //Patch_Jump(0x005B4A40, &_Extract_Compressed_Events);

    Patch_Dword(0x005B45E2 + 2, reinterpret_cast<uint32_t>(&EventClassExt::EventLength));
    Patch_Dword(0x005B4AED + 2, reinterpret_cast<uint32_t>(&EventClassExt::EventLength));
    Patch_Dword(0x005B4CF8 + 2, reinterpret_cast<uint32_t>(&EventClassExt::EventLength));

    Patch_Jump(0x00494B9A, 0x00494BAA); // Jump over code that prevents deploying with aircraft
}

