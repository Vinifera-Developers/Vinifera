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
#include "event.h"
#include "eventext.h"
#include "extension_globals.h"
#include "hooker.h"
#include "house.h"
#include "ipxmgr.h"
#include "session.h"
#include "sessionext.h"
#include "syringe.h"
#include "version.h"


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
                memcpy(event->Data.Variable.Pointer, ((char*)buf) + sizeof(EventClass), event->Data.Variable.Size);

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

    /**
     *  Record who we received this packet from so we can compare their ID against the events they have sent.
     */
    CurrentPacketPlayerId = id;

    /**
     *  Get an event ptr to the incoming message
     */
    event = (EventClass*)multi_packet_buf;

    /**
     *  Get the index of the sender
     */
    index = net->Connection_Index(id);

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
                DEBUG_ERROR("Extract_Compressed_Events: Forged house ID detected. Expected: {}, actual: {}\n", CurrentPacketPlayerId, event->ID);
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
 *  Main function for patching the hooks.
 */
void EventClassExtension_Hooks()
{
    Patch_Jump(0x005B4530, &_Add_Compressed_Events);
    Patch_Jump(0x005B4A40, &_Extract_Compressed_Events);
    Patch_Jump(0x005B3600, &_Process_Receive_Packet);

    Patch_Jump(0x00494B9A, 0x00494BAA); // Jump over code that prevents deploying with aircraft
}
