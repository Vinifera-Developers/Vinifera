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
};


/**
 *  EventClassExt constructor for production events.
 *
 *  @author: ZivDero
 */
EventClassExt::EventClassExt(int index, EventType type, RTTIType object, int id, ProductionFlags flags)
{
    DEBUG_INFO("Adding event %s\n", EventNames[type]);

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
    DEBUG_INFO("Adding event %s\n", EventNames[type]);

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
 *  Should this event be handled by our event handler?
 *
 *  @author: ZivDero
 */
bool EventClassExt::Is_Vinifera_Event(EventType type)
{
    // We have re-implemented these events, let's handle them ourselves
    switch (type) {
    case EVENT_PLACE:
    case EVENT_PRODUCE:
    case EVENT_SUSPEND:
    case EVENT_ABANDON:
        return true;
    }

    // add a check for new events here later

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
    HouseClass* house = Houses[ID];
    HouseClassExtension* house_ext = Extension::Fetch(house);

    switch (Type) {

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
    }
}
