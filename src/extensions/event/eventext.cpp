/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended EventClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "eventext.h"

#include "debughandler.h"
#include "extension.h"
#include "houseext.h"
#include "mouse.h"
#include "protocolzero.h"
#include "session.h"
#include "sessionext.h"
#include "spawnmanager.h"
#include "unit.h"
#include "vinifera_globals.h"


/***************************************************************************
** Table of what data is really used in the EventClass struct for different
** events.  This table must be kept current with the EventType enum.
*/
unsigned char EventClassExt::EventLength[EXT_EVENT_COUNT] = {
    0,                                          // EMPTY
    sizeof(EventClass::Data.Target),            // POWERON
    sizeof(EventClass::Data.Target),            // POWEROFF
    sizeof(EventClass::Data.General),           // ALLY
    sizeof(EventClass::Data.MegaMission),       // MEGAMISSION
    sizeof(EventClass::Data.MegaMission_F),     // MEGAMISSION_F
    sizeof(EventClass::Data.Target),            // IDLE
    sizeof(EventClass::Data.Target),            // SCATTER
    0,                                          // DESTRUCT
    sizeof(EventClass::Data.Target),            // DEPLOY
    sizeof(EventClassExt::Data.NewPlace),       // PLACE
    0,                                          // OPTIONS
    sizeof(EventClass::Data.General),           // GAMESPEED
    sizeof(EventClassExt::Data.Production),     // PRODUCE
    sizeof(EventClassExt::Data.Production),     // SUSPEND
    sizeof(EventClassExt::Data.Production),     // ABANDON
    sizeof(EventClass::Data.Target),            // PRIMARY
    sizeof(EventClass::Data.Special),           // SPECIAL_PLACE
    0,                                          // EXIT
    sizeof(EventClass::Data.Anim),              // ANIMATION
    sizeof(EventClass::Data.Target),            // REPAIR
    sizeof(EventClass::Data.Target),            // SELL
    sizeof(EventClass::Data.SellCell),          // SELLCELL
    sizeof(EventClass::Data.Options),           // SPECIAL
    0,                                          // FRAMESYNC
    0,                                          // MESSAGE
    sizeof(EventClass::Data.FrameInfo.Delay),   // RESPONSE_TIME
    sizeof(EventClass::Data.FrameInfo),         // FRAMEINFO
    0,                                          // SAVEGAME
    sizeof(EventClass::Data.NavCom),            // ARCHIVE
    sizeof(EventClass::Data.Variable.Size),     // ADDPLAYER
    sizeof(EventClass::Data.Timing),            // TIMING
    sizeof(EventClass::Data.ProcessTime),       // PROCESS_TIME
    0,                                          // PAGEUSER
    sizeof(EventClass::Data.General),           // REMOVEPLAYER
    sizeof(EventClass::Data.General),           // LATENCYFUDGE
    sizeof(EventClassExt::Data.PlayerOptions),  // PLAYER_OPTIONS
    sizeof(EventClassExt::Data.ResponseTime2)   // RESPONSE_TIME2
};


char const* EventClassExt::EventNames[EXT_EVENT_COUNT] = {
    "EMPTY",
    "POWERON",
    "POWEROFF",
    "ALLY",
    "MEGAMISSION",
    "MEGAMISSION_F",
    "IDLE",
    "SCATTER",
    "DESTRUCT",
    "DEPLOY",
    "PLACE",
    "OPTIONS",
    "GAMESPEED",
    "PRODUCE",
    "SUSPEND",
    "ABANDON",
    "PRIMARY",
    "SPECIAL_PLACE",
    "EXIT",
    "ANIMATION",
    "REPAIR",
    "SELL",
    "SELLCELL",
    "SPECIAL",
    "FRAMESYNC",
    "MESSAGE",
    "RESPONSE_TIME",
    "FRAMEINFO",
    "SAVEGAME",
    "ARCHIVE",
    "ADDPLAYER",
    "TIMING",
    "PROCESS_TIME",
    "PAGEUSER",
    "REMOVEPLAYER",
    "LATENCYFUDGE",
    "PLAYER_OPTIONS",
    "RESPONSE_TIME_2",
};


/**
 *  EventClassExt constructor for production events.
 *
 *  @author: ZivDero
 */
EventClassExt::EventClassExt(int index, EventType type, RTTIType object, int id, ProductionFlags flags)
{
    DEBUG_INFO("Adding event {}\n", EventNames[type]);

    if (index >= 0) {
        ID = index;
        Type = type;
        Data.Production.Type = object;
        Data.Production.ID = id;
        Data.Production.Flags = flags;
        Frame = ::Frame;
    }
    else {
        ID = -1;
        Type = EVENT_EMPTY;
        Frame = ::Frame;
    }
}


/**
 *  EventClassExt constructor for the PLACE event.
 *
 *  @author: ZivDero
 */
EventClassExt::EventClassExt(int index, EventType type, RTTIType object, Cell const& cell, ProductionFlags flags)
{
    DEBUG_INFO("Adding event {}\n", EventNames[type]);

    if (index >= 0) {
        ID = index;
        Type = type;
        Data.NewPlace.Type = object;
        Data.NewPlace.Where = xCell { cell.X, cell.Y };
        Data.NewPlace.Flags = flags;
        Frame = ::Frame;
    }
    else {
        ID = -1;
        Type = EVENT_EMPTY;
        Frame = ::Frame;
    }
}


/**
 *  EventClassExt constructor for the PLAYER_OPTIONS event.
 *
 *  @author: Rampastring
 */
EventClassExt::EventClassExt(int index, EventType type, bool pausedrepairs)
{
    DEBUG_INFO("Adding event {}\n", EventNames[type]);

    if (index >= 0) {
        ID = index;
        Type = type;
        Data.PlayerOptions.IsPauseRepairs = pausedrepairs;
        Frame = ::Frame;
    } else {
        ID = -1;
        Type = EVENT_EMPTY;
        Frame = ::Frame;
    }
}


/**
 *  EventClassExt constructor for the RESPONSE_TIME2 event.
 *
 *  @author: ZivDero
 */
EventClassExt::EventClassExt(int index, unsigned char max_ahead, LatencyLevelEnum latency_level)
{
    DEBUG_INFO("Adding event RESPONSE_TIME2\n");

    if (index >= 0) {
        ID = index;
        Type = static_cast<EventType>(EXT_EVENT_RESPONSE_TIME2);
        Data.ResponseTime2.MaxAhead = max_ahead;
        Data.ResponseTime2.LatencyLevel = latency_level;
        Frame = ::Frame + Session.MaxAhead;
    } else {
        ID = -1;
        Type = EVENT_EMPTY;
        Frame = ::Frame + Session.MaxAhead;
    }
}


/**
 *  Should this event be handled by our event handler?
 *
 *  @author: ZivDero
 */
bool EventClassExt::Is_Vinifera_Event(EventType type)
{
    if (type >= EXT_EVENT_FIRST && type < EXT_EVENT_COUNT) {
        return true; // This is a Vinifera event
    }

    // We have re-implemented these events, let's handle them ourselves
    switch (type) {
    case EVENT_IDLE:
    case EVENT_PLACE:
    case EVENT_PRODUCE:
    case EVENT_SUSPEND:
    case EVENT_ABANDON:
    case EVENT_SAVEGAME:
    case EVENT_ARCHIVE:
    case EVENT_TIMING:
    case EVENT_REMOVEPLAYER:
        return true;
    }

    // add a check for new events here later
    switch (type) {
    case EXT_EVENT_PLAYER_OPTIONS:
        return true;
    }

    return false;
}


/**
 *  Should this event be handled by our event handler?
 *
 *  @author: ZivDero
 */
bool EventClassExt::Is_Vinifera_Event() const
{
    return Is_Vinifera_Event(Type);
}


/**
 *  Executes the event.
 *
 *  @author: ZivDero
 */
void EventClassExt::Execute()
{
    TechnoClass* techno;
    HouseClassExtension* house_ext = nullptr;

    switch (Type) {
    case EVENT_PLACE:
    case EVENT_PRODUCE:
    case EVENT_SUSPEND:
    case EVENT_ABANDON:
    case EXT_EVENT_PLAYER_OPTIONS:
        if (ID >= static_cast<unsigned int>(Houses.Count()) || Houses[ID] == nullptr) {
            DEBUG_WARNING("Ignoring event {} with invalid house ID {}.\n", Event_Name(Type), ID);
            return;
        }
        house_ext = Extension::Fetch(Houses[ID]);
        break;
    }

    switch (Type) {

         /*
         **  Request that the unit/infantry/aircraft go into idle mode.
         */
    case EVENT_IDLE:
        Do_IDLE();
        break;

        /*
        **  This event will place the specified object at the specified location.
        **  The event is used to place newly constructed buildings down on the map. The
        **  object type is specified. From this object type, the house can determine the
        **  exact factory and real object pointer to use.
        */
    case EVENT_PLACE:
        house_ext->Place_Object(Data.NewPlace.Type, Cell(Data.NewPlace.Where.X, Data.NewPlace.Where.Y), Data.NewPlace.Flags);
        break;

        /*
        **  This event starts production of the specified object type. The house can
        **  determine from the type and ID value, what object to begin production on and
        **  what factory to use.
        */
    case EVENT_PRODUCE:
        house_ext->Begin_Production(Data.Production.Type, Data.Production.ID, false, Data.Production.Flags);
        break;

        /*
        **  This event is generated when the player puts production on hold. From the
        **  object type, the factory can be inferred.
        */
    case EVENT_SUSPEND:
        house_ext->Suspend_Production(Data.Production.Type, Data.Production.Flags);
        break;

        /*
        **  This event is generated when the player cancels production of the specified
        **  object type. From the object type, the exact factory can be inferred.
        */
    case EVENT_ABANDON:
        house_ext->Abandon_Production(Data.Production.Type, Data.Production.ID, Data.Production.Flags);
        break;

        /*
        **  This event is generated when the player broadcasts their preferred player options.
        */
    case EXT_EVENT_PLAYER_OPTIONS:
        house_ext->IsPauseRepairs = Data.PlayerOptions.IsPauseRepairs;
        break;

        /*
        **  Save a multiplayer game (this event is only generated in multiplayer mode).
        **  Only manual multiplayer saves go through the event system - auto-save logic
        **  is handled locally.
        */
    case EVENT_SAVEGAME:
        SessionExtension->Flag_To_Save(true);
        break;

        /*
        **  Update the archive target for this building.
        */
    case EVENT_ARCHIVE:
        techno = Data.NavCom.Whom.As_Techno();
        if (techno && techno->IsActive && techno->Mission != MISSION_DECONSTRUCTION) {
            techno->ArchiveTarget = Data.NavCom.Where.As_Abstract();
        }
        break;

        /*
        **  This event tells all systems to use new timing values. It's like
        **  RESPONSE_TIME, only it works. It's only used with the
        **  COMM_MULTI_E_COMP protocol.
        */
    case EVENT_TIMING:
        Do_TIMING();
        break;

        /*
        **  Removes a player from the game (for any reason).
        */
    case EVENT_REMOVEPLAYER:
        Do_REMOVEPLAYER();
        break;

        /*
        **  New timing event for the spawner.
        */
    case EXT_EVENT_RESPONSE_TIME2:
        ProtocolZero::Handle_Response_Time(*this);
        break;
    }
}


/**
 *  Executes the IDLE event.
 *
 *  @author: ZivDero
 */
void EventClassExt::Do_IDLE()
{
    TechnoClass* techno = Data.Target.Whom.As_Techno();

    if (techno != nullptr && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered) {
        if (techno->Mission == MISSION_CONSTRUCTION || techno->Mission == MISSION_DECONSTRUCTION) {
            return;
        }
        if (!techno->IsOnBridge && Map[techno->PositionCoord].Ramp == RAMP_NONE && techno->Is_On_Elevation()) {
            return;
        }
        if (techno->Is_Foot()) {
            FootClass* foot = static_cast<FootClass*>(techno);
            foot->NavQueue.Clear();
            foot->Clear_Navigation_List();
            foot->CurrentPath = -1;
            foot->NextWaypoint = 0;
            foot->field_224 = Cell(0, 0);
            foot->field_228 = Cell(0, 0);
        }

        techno->Transmit_Message(RADIO_OVER_OUT);
        techno->Assign_Destination(nullptr);
        techno->Assign_Target(nullptr);

        const auto extension = Extension::Fetch(techno);
        if (extension->SpawnManager) {
            extension->SpawnManager->Abandon_Target();
        }

        if (techno->RTTI == RTTI_UNIT && (static_cast<UnitClass*>(techno)->Class->IsToHarvest || static_cast<UnitClass*>(techno)->Class->IsToVeinHarvest)) {
            if (techno->Mission == MISSION_HARVEST || techno->Mission == MISSION_RETURN) {
                techno->Assign_Mission(MISSION_GUARD);
                techno->Commence();
            }
        }
    }
}


/**
 *  Executes the TIMING event.
 *
 *  @author: ZivDero
 */
void EventClassExt::Do_TIMING()
{
    if (Data.Timing.FrameSendRate == 0) {
        DEBUG_WARNING("Ignoring TIMING event with a zero frame-send rate.\n");
        return;
    }

    if (!SessionExtension->ProtocolZeroEnabled) {
        if (Scen->Special.IsFogOfWar) {
            Data.Timing.MaxAhead -= 10;
        }
    }

    /**
     *  If MaxAhead is about to increase, we're vulnerable to a Packet-
     *  Received-Too-Late error, if any system generates an event after
     *  this TIMING event, but before it executes.  So, record the
     *  period of vulnerability's frame start & end values, so we
     *  can reschedule these events to execute after it's over.
     */
    if (Data.Timing.MaxAhead > Session.MaxAhead || Data.Timing.FrameSendRate > Session.FrameSendRate) {
        NewMaxAheadFrame1 = Frame;
        NewMaxAheadFrame2 = Data.Timing.FrameSendRate * ((Data.Timing.FrameSendRate + Data.Timing.MaxAhead + Frame - 1) / Data.Timing.FrameSendRate);
    } else {
        NewMaxAheadFrame1 = 0;
        NewMaxAheadFrame2 = 0;
    }

    Session.DesiredFrameRate = Data.Timing.DesiredFrameRate;
    Session.MaxAhead = Data.Timing.MaxAhead;
    Session.MaxMaxAhead = std::max(Session.MaxMaxAhead, Session.MaxAhead);
    Session.FrameSendRate = Data.Timing.FrameSendRate;
}


/**
 *  Executes the REMOVEPLAYER event.
 *
 *  @author: ZivDero
 */
void EventClassExt::Do_REMOVEPLAYER()
{
    DEBUG_INFO("Executing REMOVEPLAYER event. Frame is {}\n", Frame);
    const int house_id = Data.General.Value;
    if (house_id < 0 || house_id >= Houses.Count() || Houses[house_id] == nullptr) {
        DEBUG_WARNING("Ignoring REMOVEPLAYER event with invalid house ID {}.\n", house_id);
        return;
    }

    HouseClass* house = Houses[house_id];

    if ((Session.Type == GAME_INTERNET && WestwoodOnline_Tournament) || (Session.Type == GAME_IPX && SessionExtension->ExtOptions.IsAutoSurrender)) {
        house->Flag_To_Die();
    } else if (house->Is_Human_Player()) {
        house->AI_Takeover();
    }
}
