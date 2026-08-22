/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended EventClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "eventext_hooks.h"

#include "connmgr.h"
#include "debughandler.h"
#include "desyncdialog.h"
#include "event.h"
#include "eventext.h"
#include "extension.h"
#include "extension_globals.h"
#include "hooker.h"
#include "house.h"
#include "ipxmgr.h"
#include "mouse.h"
#include "msgbox.h"
#include "netdlg.h"
#include "rules.h"
#include "session.h"
#include "sessionext.h"
#include "syringe.h"
#include "tibsun_functions.h"
#include "version.h"
#include "vinifera_globals.h"


/**
 *  This patch intercepts EventClass::Execute and executes the event if it's one of ours.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00494294, _EventClass_Execute_New_Events, 5)
{
    GET(EventClassExt*, event, ESI);

    if (event->Is_Vinifera_Event()) {
        event->Execute();
        return 0x00495110; // return
    }

    return 0;
}

/**
 *  Keeps a record of which connection ID we received the current packet from.
 *  Used for detecting forged events in Extract_Compressed_Events.
 */
static int CurrentPacketPlayerId;


static bool Is_Valid_Event_Type(EventType type)
{
    return type < EXT_EVENT_COUNT;
}


/**
 *  Validates an uncompressed event packet before queue or timing state is
 *  mutated. ADDPLAYER payloads immediately follow their EventClass record.
 */
static bool Validate_Uncompressed_Events(const void* buf, int bufsize)
{
    if (buf == nullptr || bufsize <= 0 || bufsize > MAX_IPX_PACKET_SIZE) {
        return false;
    }

    int pos = 0;
    while (pos < bufsize) {
        const int remaining = bufsize - pos;
        if (remaining < static_cast<int>(sizeof(EventClass))) {
            return false;
        }

        const auto event = reinterpret_cast<const EventClass*>(static_cast<const char*>(buf) + pos);
        if (!Is_Valid_Event_Type(event->Type) || event->ID != CurrentPacketPlayerId) {
            return false;
        }

        if (pos == 0 && event->Type != EVENT_FRAMEINFO && event->Type != EVENT_FRAMESYNC) {
            return false;
        }

        if (event->Type == EVENT_FRAMESYNC && (pos != 0 || bufsize != sizeof(EventClass))) {
            return false;
        }

        pos += sizeof(EventClass);
        if (event->Type == EVENT_ADDPLAYER) {
            const unsigned long payload_size = event->Data.Variable.Size;
            if (payload_size > MAX_IPX_PACKET_SIZE || payload_size > static_cast<unsigned long>(bufsize - pos)) {
                return false;
            }
            pos += static_cast<int>(payload_size);
        }
    }

    return pos == bufsize;
}


/**
 *  Validates a compressed event packet before extraction. The first record
 *  carries the common FRAMEINFO header; later records contain a type byte and
 *  only that event's payload.
 */
static bool Validate_Compressed_Events(const void* buf, int bufsize)
{
    if (buf == nullptr || bufsize <= 0 || bufsize > MAX_IPX_PACKET_SIZE) {
        return false;
    }

    const int first_record_size = offsetof(EventClass, Data) + sizeof(EventClass::Data.FrameInfo);
    if (bufsize < first_record_size) {
        return false;
    }

    const auto first_event = reinterpret_cast<const EventClass*>(buf);
    if ((first_event->Type != EVENT_FRAMEINFO && first_event->Type != EVENT_FRAMESYNC) || first_event->ID != CurrentPacketPlayerId) {
        return false;
    }

    if (first_event->Type == EVENT_FRAMESYNC) {
        return bufsize == first_record_size;
    }

    int pos = first_record_size;
    while (pos < bufsize) {
        const EventType type = static_cast<EventType>(*(static_cast<const unsigned char*>(buf) + pos));
        if (!Is_Valid_Event_Type(type) || type == EVENT_FRAMEINFO || type == EVENT_FRAMESYNC) {
            return false;
        }
        pos += sizeof(EventClass::Type);

        if (type == EVENT_MEGAMISSION) {
            if (bufsize - pos < static_cast<int>(sizeof(unsigned char))) {
                return false;
            }

            const unsigned int repetitions = *(static_cast<const unsigned char*>(buf) + pos);
            pos += sizeof(unsigned char);
            if (repetitions == 0) {
                return false;
            }

            const int payload_size = EventClassExt::Event_Length(type);
            const int repeated_size = static_cast<int>((repetitions - 1) * sizeof(EventClass::Data.MegaMission.Whom));
            if (payload_size > bufsize - pos || repeated_size > bufsize - pos - payload_size) {
                return false;
            }
            pos += payload_size + repeated_size;
            continue;
        }

        const int payload_size = EventClassExt::Event_Length(type);
        if (payload_size > bufsize - pos) {
            return false;
        }

        if (type == EVENT_ADDPLAYER) {
            unsigned long variable_size = 0;
            std::memcpy(&variable_size, static_cast<const char*>(buf) + pos, sizeof(variable_size));
            if (variable_size > MAX_IPX_PACKET_SIZE || variable_size > static_cast<unsigned long>(bufsize - pos - payload_size)) {
                return false;
            }
            pos += payload_size + static_cast<int>(variable_size);
        } else {
            pos += payload_size;
        }
    }

    return pos == bufsize;
}

/**
 *  Reimplementation of Extract_Uncompressed_Events because the compiler
 *  decided to inline and omit it from the original game binary.
 *
 *  @author: DRD (Red Alert 1 source code)
 *           Rampastring (Tiberian Sun / Vinifera changes).
 */
static int Extract_Uncompressed_Events(void* buf, int bufsize)
{
    int count = 0;
    int pos = 0;
    int leftover = bufsize;
    EventClass* event;

    /**
     *  Loop until there are no more events in the packet
     */
    while (leftover >= sizeof(EventClass)) {

        event = (EventClass*)(((char*)buf) + pos);

        /**
         *  add event to the DoList, only if it's not a FRAMESYNC
         *  (but FRAMEINFO's do get added.)
         */
        if (event->Type != EVENT_FRAMESYNC) {
            event->IsExecuted = 0;

            /**
             *  Special processing for variable-sized events
             */
            if (event->Type == EVENT_ADDPLAYER) {
                event->Data.Variable.Pointer = new char[event->Data.Variable.Size];
                memcpy(event->Data.Variable.Pointer, static_cast<char*>(buf) + pos + sizeof(EventClass), event->Data.Variable.Size);

                pos += event->Data.Variable.Size;
                leftover -= event->Data.Variable.Size;
            }

            if (!DoList.Add(*event)) {
                if (event->Type == EVENT_ADDPLAYER) {
                    delete[] event->Data.Variable.Pointer;
                }
                return (-1);
            }
#ifdef MIRROR_QUEUE
            MirrorList.Add(*event);
#endif

            /**
             *  Keep count of how many events we add to the queue
             */
            count++;
        }

        /**
         *  Point to the next position in the buffer; decrement our 'leftover'
         */
        pos += sizeof(EventClass);
        leftover -= sizeof(EventClass);
    }

    return (count);
}


/**
 *  Reimplementation of Breakup_Receive_Packet because the compiler
 *  decided to inline and omit it from the original game binary.
 *
 *  @author: BRR (Red Alert 1 source code).
 */
static int Breakup_Receive_Packet(void* buf, int bufsize)
{
    int count = 0;

    /*
    **	is there enough leftover for another record
    */
    switch (Session.CommProtocol) {
    case (COMM_PROTOCOL_SINGLE_NO_COMP):
        count = Extract_Uncompressed_Events(buf, bufsize);
        break;

    default:
        count = Extract_Compressed_Events(buf, bufsize);
        break;
    }

    return (count);
}


/**
 *  Replacement of Process_Receive_Packet to record who we received a packet from.
 *
 *  @author: BRR (Red Alert 1 source code)
*            tomsons26, Rampastring (Tiberian Sun changes)
 */
static RetcodeType _Process_Receive_Packet(ConnManClass* net, char* multi_packet_buf, int id, int packetlen, FrameSyncStruct* their, BasicTimerClass<SystemTimerClass>* timer)
{
    EventClass* event;
    int index;
    RetcodeType retcode = RC_NORMAL;
    int i;
    int frame;

    if (net == nullptr || multi_packet_buf == nullptr || their == nullptr || timer == nullptr || id < 0 || id >= MAX_PLAYERS || packetlen <= 0 || packetlen > MAX_IPX_PACKET_SIZE) {
        DEBUG_WARNING("Process_Receive_Packet: Dropping packet with invalid arguments.\n");
        return RC_NORMAL;
    }

    CurrentPacketPlayerId = id;

    const bool packet_is_valid = Session.CommProtocol == COMM_PROTOCOL_SINGLE_NO_COMP
        ? Validate_Uncompressed_Events(multi_packet_buf, packetlen)
        : Validate_Compressed_Events(multi_packet_buf, packetlen);
    if (!packet_is_valid) {
        DEBUG_WARNING("Process_Receive_Packet: Dropping malformed or forged packet from house {}.\n", id);
        return RC_NORMAL;
    }

    /**
     *  Get an event ptr to the incoming message
     */
    event = (EventClass*)multi_packet_buf;

    /**
     *  Get the index of the sender
     */
    index = net->Connection_Index(id);
    if (index < 0 || index >= net->Num_Connections()) {
        DEBUG_WARNING("Process_Receive_Packet: Invalid connection index for house {}.\n", id);
        return RC_NORMAL;
    }

    /**
     *  Compute the other player's frame # (at the time this packet was sent)
     */
    frame = (event->Frame - event->Data.FrameInfo.Delay);
    if (their[index].frame < frame) {

        /**
         *  If the original frame # for this player is -1, it means we've heard
         *  from this player for the 1st time; return the appropriate value.
         */
        if (their[index].frame == -1) {
            retcode = RC_PLAYER_READY;
        }

        their[index].frame = frame;

        if (Session.Type != GAME_INTERNET) {
            Session.field_1B30[index] = 0;
        } else {
            unsigned long f = Ipx.Avg_Response_Time(index) / 2;
            f *= FramesPerSecond;
            Session.field_1B30[index] = (Frame - (f / 60)) - frame;
        }
    }

    /**
     *  Extract the other player's CommandCount.  This count will include
     *  the commands in this packet, if there are any.
     */
    if (event->Data.FrameInfo.CommandCount > their[index].sent) {

        if (abs((int)(their[index].sent - event->Data.FrameInfo.CommandCount)) > 500) {
            FILE* fp;
            fp = fopen("badcount.txt", "wt");
            if (fp) {
                fprintf(fp, "Event Type:%s\n", EventClassExt::Event_Name(event->Type));
                fprintf(fp, "Frame:%d  ID:%d  IsExec:%d\n", event->Frame, event->ID, event->IsExecuted);
                if (event->Type != EVENT_FRAMEINFO) {
                    fprintf(fp, "!!!!!!!!! bad bug, bad bug !!!!!!!!!\n");
                } else {
                    fprintf(fp, "CRC:%x  CommandCount:%d  Delay:%d\n", event->Data.FrameInfo.CRC, event->Data.FrameInfo.CommandCount, event->Data.FrameInfo.Delay);
                }
            }
        }

        their[index].sent = event->Data.FrameInfo.CommandCount;
    }


    /**
     *  If this packet was not a FRAMESYNC packet:
     *  - Add the events in it to our DoList
     *  - Increment our commands-received counter by the number of non-FRAMEINFO packets received
     */
    if (event->Type != EVENT_FRAMESYNC) {
        /**
         *  Break up the packet into its component events.  A returned packet
         *  count of -1 indicates a fatal queue-full error.
         */
        i = Breakup_Receive_Packet(multi_packet_buf, packetlen);
        if (i == -1) {
            return (RC_DOLIST_FULL);
        }
        /**
         *  Compute the actual # commands in the packet by subtracting off the FRAMEINFO event
         */
        if ((event->Type == EVENT_FRAMEINFO) && (i > 0)) {
            i--;
        }

        their[index].recv += (i & 0xFFFF); // TODO shouldn't be needed?????
    }

    /**
     *  If the event was a FRAMESYNC packet, there will be no commands to add,
     *  but we must check the ScenarioCRC value.
     */
    else if (event->Type == EVENT_FRAMESYNC) {
        if (event->Data.FrameInfo.CRC != ScenarioCRC) {
            return (RC_SCENARIO_MISMATCH);
        }
        their[index].timing = *timer;
    }

    return (retcode);
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
        DEBUG_INFO("\nFrame {}: Building Send Packet\n", Frame);
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
                        DEBUG_INFO("      adding Whom:{:x} Mission:{} Target:{:x} Dest:{:x}\n", OutList.First().Data.MegaMission.Whom.Encode(), MissionClass::Mission_Name(OutList.First().Data.MegaMission.Mission), OutList.First().Data.MegaMission.Target.Encode(), OutList.First().Data.MegaMission.Destination.Encode());
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
             *  If the sender ID of this packet does not match the expected player ID,
             *  this is a forged packet. No need to continue - just bail.
             */
            if (eventdata.ID != CurrentPacketPlayerId) {
                DEBUG_ERROR("Extract_Compressed_Events: Forged house ID detected. Expected: {}, actual: {}\n", CurrentPacketPlayerId, eventdata.ID);
                return count;
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
 *  Fixes a cheat in the original game where players are able to issue
 *  commands to technos that are not owned by them.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004946FF, _EventClass_Execute_MEGAMISSION_Prevent_Controlling_Enemy_Units, 0)
{
    enum {
        Continue = 0x0049470A,
        Bail = 0x00495110
    };

    GET(EventClass*, this_ptr, ESI);
    GET(TechnoClass*, techno, EDI);

    // Stolen bytes / code.
    // Jump out if the techno is not active.
    if (!techno->IsActive) {
        return Bail;
    }

    bool hasowncargo = false;

    if (Session.Type != GAME_NORMAL) {
        // In multiplayer, each human player can only control one house.
        if (this_ptr->ID != techno->House->HeapID) {

            // House IDs between event and unit do not match.
            // This might be a crafted event.
            // But before assuming so, check for the object having the event sender's units as cargo.
            // This is necessary so that a player is able to unload units placed inside another house's transport.
            FootClass* cargoobject = techno->Cargo.Attached_Object();
            while (cargoobject != nullptr) {
                if (cargoobject->House->HeapID == this_ptr->ID) {
                    hasowncargo = true;
                    break;
                }
                cargoobject = reinterpret_cast<FootClass*>(cargoobject->Next);
            }

            if (!hasowncargo) {
                return Bail;
            }
        }
    } else {
        // In campaign, the player can control multiple houses.
        // We might as well also fix this exploit for campaign by checking for player control here.
        if (!techno->House->IsPlayerControl) {

            // Also check for cargo in singleplayer. But use IsPlayerControl instead of direct ID comparison
            // due to the human player being able to control multiple houses.
            FootClass* cargoobject = techno->Cargo.Attached_Object();
            while (cargoobject != nullptr) {
                if (cargoobject->House->IsPlayerControl) {
                    hasowncargo = true;
                    break;
                }
                cargoobject = reinterpret_cast<FootClass*>(cargoobject->Next);
            }

            if (!hasowncargo) {
                return Bail;
            }
        }
    }

    // Continue event execution.
    return Continue;
}


/**
 *  Fixes a cheat in the original game where players are able to issue
 *  an IDLE command to technos that are not owned by them.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004949AF, _EventClass_Execute_IDLE_Prevent_Controlling_Enemy_Units, 0)
{
    enum {
        Continue = 0x004949BB,
        Bail = 0x00495110
    };

    GET(EventClass*, this_ptr, ESI);
    GET(TechnoClass*, techno, EAX);

    // Stolen bytes / code.
    // Jump out if the techno is null.
    if (techno == nullptr) {
        return Bail;
    }

    if (Session.Type != GAME_NORMAL) {
        // In multiplayer, each human player can only control one house.
        if (this_ptr->ID != techno->House->HeapID) {
            // ID of owner of techno does not match the ID of whoever generated the event.
            // Exit the function.
            return Bail;
        }
    } else {
        // In campaign, the player can control multiple houses.
        // We might as well also fix this exploit for campaign by checking for player control here.
        if (!techno->House->IsPlayerControl) {
            return Bail;
        }
    }

    // Continue event execution.
    // Set esi to point to the techno and edi to zero, the original
    // game code expects these values.
    R->ESI(techno);
    R->EDI(0);
    return Continue;
}

/**
 *  Patches EventClass::Execute to catch instances of Q-Moving aircrafts.
 *  Unlike ground units, aircraft cannot fire and move at the same time,
 *  and instead have to apply their appropriate firing logic, which is either bombing or curley shuffling.
 *  This means that if a Q-Move order is given to an aircraft, it should abandon its target.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004948A5, _EventClass_Execute_QMove_Aircraft_Patch, 6)
{
    GET(int, mission, EAX);
    GET(FootClass*, this_ptr, EDI);

    if (mission == MISSION_QMOVE && this_ptr->Fetch_RTTI() == RTTI_AIRCRAFT) {
        this_ptr->Assign_Target(nullptr);
    }

    return 0;
}


/**
 *  Patches Destroy_Connection to disable multiplayer saves.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x005751E8, _Destroy_Connection_Disable_Multiplayer_Saves_Patch, 6)
{
    SessionExtension->Disable_Multiplayer_Saves();
    return 0;
}


/**
 *  Patches Queue_AI_Multiplayer to stop processing the game if we are about to load a save.
 *
 *  @author: Rampastring
 */
static short& MySent2 = Make_Global<short>(0x008099F0);
DEFINE_HOOK(0x005B15C5, _Queue_AI_Multiplayer_No_Processing_If_Loading_Save, 6)
{
    if (!PendingMultiplayerSaveLoadTime) {
        return 0;
    }

    DEBUG_INFO("Queue_AI_Multiplayer: Stalling game until game load.\n");

    // Clear command queues
    DoList.Init();
    OutList.Init();

    // Send our commands once
    Send_Packets(&Ipx, Session.MetaPacket, Session.MetaSize, Session.MaxAhead, MySent2);

    // Freeze the game state until it's time to load. Allow the user to scroll around, though.
    while (std::chrono::steady_clock::now() < *PendingMultiplayerSaveLoadTime)
    {
        KeyNumType input;
        int x, y;
        Call_Back();
        Ipx.Service();

        if (SpecialDialog == SDLG_NONE)
        {
            Map.Input(input, x, y);
            if (input) Keyboard_Process(input);
            Map.Render();
        }

        // Don't use more CPU time than necessary.
        Sleep(5);
    }

    DoList.Init();
    OutList.Init();

    DEBUG_INFO("Queue_AI_Multiplayer: Returning to main loop.\n");

    // Queue_AI_Multiplayer is called from the main loop.
    // Return to allow the main loop to finish processing to make sure that all per-frame
    // logic processes are appropriately cleared up for loading a save to be safe.
    // Our post-main-loop hook will load the saved game afterwards.
    return 0x005B1F21;
}


/**
 *  Dump_Packet_Too_Late_Stuff replacement that takes our extended event types into account.
 *
 *  @author: Rampastring
 */
void _Dump_Packet_Too_Late_Stuff(EventClass* event)
{
    DEBUG_INFO("Packet received too late!\n");
    DEBUG_INFO("--------- Event data: -------------------\n");
    DEBUG_INFO("Type:       {}\n", EventClassExt::Event_Name(event->Type));
    DEBUG_INFO("Frame:      {}\n", event->Frame);
    DEBUG_INFO("ID:         {}\n", event->ID);
    DEBUG_INFO("MaxAhead={}\n", Session.MaxAhead);
    DEBUG_INFO("Frame={}\n", Frame);
    DEBUG_INFO("FrameSendRate={}\n", Session.FrameSendRate);
}


/**
 *  Execute_DoList replacement to fix various bugs in the original implementation.
 *
 *  @author: Rampastring
 */
static int _Execute_DoList(int max_houses, HousesType base_house, ConnManClass* net, CDTimerClass<FrameTimerClass>* skip_crc, FrameSyncStruct* their)
{
    Check_Mirror();

    if (Session.Type == GAME_MODEM || Session.Type == GAME_NULL_MODEM) {
        WWMessageBox().Process("Vinifera does not support multiplayer through Modem or Null Modem!", 0);
        Emergency_Exit(0);
        return 0;
    }

    /**
     *  If MPlayerMaxAhead is recomputed such that it increases, the systems
     *  may try to free-run to the new MaxAhead value. If so, they may miss
     *  an event that was generated after the TIMING event was created, but
     *  before it executed; this event will be scheduled with the older,
     *  shorter MaxAhead value. If a system doesn't receive this event, it
     *  may execute past the frame it's scheduled to execute on, creating
     *  a Packet-Received-Too-Late error. To prevent this, find any events
     *  that are scheduled to execute during this "period of vulnerability",
     *  and re-schedule for the end of that period.
     */
    for (int j = 0; j < DoList.Count; j++) {
        if (DoList[j].Type != EVENT_FRAMEINFO && DoList[j].Frame > NewMaxAheadFrame1 && DoList[j].Frame < NewMaxAheadFrame2) {
            DEBUG_INFO("DoList: Moving event from frame {} to frame {}\n", DoList[j].Frame, NewMaxAheadFrame2);
            DoList[j].Frame = NewMaxAheadFrame2;
        }
    }

    /**
     *  Skip the CRC check if we're less than 256 frames into the game; this will
     *  prevent a new game from instantly going out of sync. (For some reason,
     *  FRAMEINFO CRCs are different between players at the start of the game, and
     *  Westwood circumvented it with this hack.)
     */
    bool check_crc = !skip_crc || *skip_crc == 0;

    /**
     *  Only print CRCs once, even if we desynced from multiple players at once.
     *  There's no use writing multiple desync logs for one desync.
     */
    bool print_crcs = true;

    if (check_crc) {

        /**
         *  First, check for desyncs. Loop through all events, and for FRAMEINFO
         *  events to be executed on this frame, check their CRC. Collect all the
         *  players that have desynced this frame before reacting, so that if
         *  several players desync at once we still only show one dialog.
         */
        bool newly_desynced = false;

        for (int i = 0; i < DoList.Count; i++) {

            EventClass& event = DoList[i];

            if (event.ID < Session.Players.Count() && !SessionExtension->Is_Out_of_Sync(event.ID) && Frame == event.Frame && event.Type == EVENT_FRAMEINFO) {
                if (event.Data.FrameInfo.Delay < std::size(CRC)) {

                    int index = ((event.Frame - event.Data.FrameInfo.Delay) & std::size(CRC) - 1);

                    if (CRC[index] != event.Data.FrameInfo.CRC) {

                        SessionExtension->Mark_Player_As_Out_of_Sync(event.ID);
                        newly_desynced = true;

                        if (print_crcs) {
                            Extension::Print_CRCs(&event);
                            print_crcs = false;
                        }

                        DEBUG_WARNING("Player %s has gone out of sync!\n", Houses[event.ID]->IniName.c_str());
                    }
                }
            }
        }

        if (newly_desynced) {

            /**
             *  If a multiplayer save load is already scheduled, the imminent reload
             *  will re-sync everyone - just skip executing events until it happens.
             */
            if (PendingMultiplayerSaveLoadTime) {
                return 1;
            }

            /**
             *  Open the desync dialog and halt the game until the host decides
             *  what to do. The dialog keeps the connections alive in the meantime.
             */
            DesyncDialogOutcomeType outcome = DesyncDialog.Run();

            switch (outcome) {

            /**
             *  Continue without the players that are out of sync with us.
             *  Destroy_Connection queues an EVENT_REMOVEPLAYER for each of them.
             *  This is symmetric: the desynced players drop us in the same way.
             */
            case DESYNC_OUTCOME_CONTINUE:
                for (int id = 0; id < MAX_PLAYERS; id++) {
                    if (SessionExtension->Is_Out_of_Sync(id)) {
                        Destroy_Connection(id, -1);
                    }
                }
                SessionExtension->Update_Master_After_Player_Removal();

                /**
                 *  Reset the desync frame so that a later, separate desync opens
                 *  the dialog again. Keep IsOutOfSync[] set so that any further
                 *  events from the dropped players keep being skipped consistently.
                 */
                SessionExtension->OutOfSyncFrame = -1;

                /**
                 *  Fall through to executing the DoList - the game resumes.
                 */
                break;

            /**
             *  A multiplayer save load has been scheduled. Skip executing events;
             *  After_Main_Loop performs the load at a safe point.
             */
            case DESYNC_OUTCOME_LOAD:
                return 1;

            /**
             *  Sign off to every player and let the caller stop the game.
             */
            case DESYNC_OUTCOME_QUIT: {
                GlobalPacketType packet;
                packet.Command = NET_SIGN_OFF;
                std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name));

                /**
                 *  Send twice for good measure, since these are not acked.
                 */
                for (int attempt = 0; attempt < 2; attempt++) {
                    for (int i = 1; i < Session.Players.Count(); i++) {
                        Ipx.Send_Global_Message(&packet, sizeof(packet), 0, &Session.Players[i]->Address);
                        Ipx.Service();
                    }
                }

                /**
                 *  Make sure the sign-off packets actually go out before we exit.
                 */
                while (Ipx.Global_Num_Send() > 0 && Ipx.Service()) {
                }

                return 0;
            }
            }
        }
    }

    /**
     *  Execute the DoList. Events must be executed in the same order on all
     *  systems; so, execute them in the order of the HouseClass array. This
     *  array is stored in the same order on all systems.
     */
    for (int i = 0; i < Houses.Count(); i++) {

        HousesType house = (HousesType)(i);
        HouseClass* hptr = Houses[house];

        /**
         *  If for some reason this house doesn't exist, skip it. Also, if this
         *  house has exited the game, skip it. (The user can generate events
         *  after he exits, because the exit event is scheduled at least
         *  FrameSendRate*3 frames ahead. If one system gets these packets and
         *  another system doesn't, they'll go out of sync because they aren't
         *  checking the CommandCount for that house, since that house isn't
         *  connected any more.)
         */
        if (!hptr) {
            continue;
        }

        if (!hptr->IsHuman && !hptr->IsPlayerControl) {
            continue;
        }

        for (int j = 0; j < DoList.Count; j++) {

            EventClass& event = DoList[j];

            /**
             *  Skip FRAMEINFO events, we already processed them above.
             */
            if (event.Type == EVENT_FRAMEINFO) {
                continue;
            }

            /**
             *  If this player has been marked as being out of sync, don't execute
             *  their events. Don't check this in campaign games, because there the
             *  player's house ID can be higher than MAX_PLAYERS.
             */
            if (Session.Type != GAME_NORMAL && SessionExtension->Is_Out_of_Sync(event.ID)) {
                continue;
            }

            /**
             *  If this event was from the currently-executing player ID, and it's
             *  time to execute it, execute it.
             */
            if (event.ID == hptr->HeapID && Frame >= event.Frame && !event.IsExecuted) {

                /**
                 *  Error if it's too late to execute this packet!
                 *  (Hack: disable this check for solo or skirmish mode.)
                 *  OPTIONS is also safe because it only changes local UI state
                 *  and events from remote players are ignored below.
                 */
                if (Frame > event.Frame && event.Type != EVENT_FRAMEINFO && event.Type != EVENT_OPTIONS && !Session.Singleplayer_Game()) {
                    _Dump_Packet_Too_Late_Stuff(&event);
                    Session.Suspended++;
                    WWMessageBox().Process(TXT_PACKET_TOO_LATE, TXT_OK);
                    Session.Suspended--;
                    return 0;
                }

                /**
                 *  Only execute EXIT, OPTIONS and PAGEUSER commands if they're from myself.
                 */
                if (event.Type == EVENT_EXIT || event.Type == EVENT_OPTIONS || event.Type == EVENT_PAGEUSER) {

                    if (event.Type == EVENT_EXIT) {

                        int house_count = Houses.Count();

                        /**
                         *  Flag that this house lost because it quit.
                         */
                        HousesType quithouse = HOUSE_NONE;
                        HouseClass* quithptr = nullptr;

                        for (int player = 0; player < house_count; player++) {
                            quithouse = (HousesType)(player);
                            quithptr = Houses[quithouse];
                            if (!quithptr) {
                                continue;
                            }
                            if (quithptr->HeapID == event.ID) {
                                quithptr->IsGiverUpper = true;
                                break;
                            }
                        }

                        /**
                         *  Send the game statistics packet now, since the game is effectively over.
                         */
                        if (Count_Alive_Teams(quithptr) == 1 && SessionExtension->Are_Statistics_Enabled() && !GameStatisticsPacketSent) {
                            Session.SawCompletion = true;
                            Register_Game_End_Time();
                            Send_Statistics_Packet();
                        }

                        if (SessionExtension->Are_Statistics_Enabled() && !GameStatisticsPacketSent && PlayerPtr != nullptr && PlayerPtr == quithptr) {
                            DEBUG_INFO("Sending game results because I quit, but didn't see completion\n");
                            Send_Statistics_Packet();
                        }
                    }

                    if (Debug_Print_Events && event.Type == EVENT_EXIT) {
                        DEBUG_INFO("Exit Event: ID:{} ({}),  Event Frame:{},  My Frame:{}\n", event.ID, (char*)Houses[(HousesType)(event.ID)]->IniName.c_str(), event.Frame, Frame);
                    }

                    if (event.ID == PlayerPtr->HeapID) {

                        event.Execute();

                    } else if (event.Type == EVENT_EXIT) {

                        /**
                         *  If this EXIT event isn't from myself, destroy the connection
                         *  for that player. The HousesType for this event is the connection ID.
                         */
                        if ((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && net) {
                            int index = net->Connection_Index(house);
                            if (index != -1) {
                                for (int k = index; k < net->Num_Connections() - 1; k++) {
                                    their[k] = their[k + 1];
                                }

                                Destroy_Connection(house, 0);
                            }
                        }

                        /**
                         *  Special case for recording playback: turn the house over
                         *  to the computer.
                         */
                        if (Session.Play && DoList[j].Type == EVENT_EXIT) {
                            DEBUG_INFO("Replacing a player with AI for recording playback\n");
                            hptr->IsHuman = false;
                            hptr->IQ = Rule->MaxIQ;
                            hptr->Computer_Paranoid();
                            hptr->IniName = Fetch_String(TXT_COMPUTER);
                            Session.NumPlayers--;
                        }
                    }

                } else {

                    /**
                     *  Execute other commands.
                     */
                    event.Execute();
                }

                /**
                 *  Mark this event as executed.
                 */
                event.IsExecuted = 1;
            }
        }
    }

    return 1;
}


/**
 *  Main function for patching the hooks.
 */
void EventClassExtension_Hooks()
{
    Patch_Jump(0x005B4530, &_Add_Compressed_Events);
    Patch_Jump(0x005B4A40, &_Extract_Compressed_Events);
    Patch_Jump(0x005B3600, &_Process_Receive_Packet);
    Patch_Jump(0x005B4D70, &_Execute_DoList);

    Patch_Jump(0x00494B9A, 0x00494BAA); // Jump over code that prevents deploying with aircraft
}
