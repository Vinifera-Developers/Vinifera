/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EVENTEXT.H
 *
 *  @author        ZivDero
 *
 *  @brief         Extended EventClass class.
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
#pragma once

#include "footext.h"
#include "aircraft.h"
#include "event.h"


/**
 *  This is the extended EventClass. It doesn't literally extend or replace the vanilla EventClass,
 *  but instead provides us with a way to send custom payloads via the event system as well as create new/
 *  replace implementations of vanilla events.
 */
class EventClassExt
{
public:
    EventClassExt() { Type = EVENT_EMPTY; }
    EventClassExt(int index, EventType type, RTTIType object, int id, ProductionFlags flags);
    EventClassExt(int index, EventType type, RTTIType object, Cell const& cell, ProductionFlags flags);

    int operator==(const EventClassExt& q) const { return std::memcmp(this, &q, sizeof(q)) == 0; }
    int operator!=(const EventClassExt& q) const { return std::memcmp(this, &q, sizeof(q)) != 0; }

    EventClass* As_Event_Ptr() { return reinterpret_cast<EventClass*>(this); }
    EventClass& As_Event() { return reinterpret_cast<EventClass&>(*this); }

    static bool Is_Vinifera_Event(EventType type);
    bool Is_Vinifera_Event() const;

    void Execute();

    // We don't yet have new events, implement when necessary
    //static const char* Event_Name(EventType event);
    //static unsigned char Event_Length(EventType event);

#pragma pack(1) // We need this so bools/bits are not aligned.
public:
    EventType Type;
    unsigned Frame;
    bool IsExecuted;
    int ID;

    union {
        struct {
            RTTIType        Type;
            int             ID;
            ProductionFlags Flags;
        } Production;

        struct {
            RTTIType        Type;
            xCell           Where;
            ProductionFlags Flags;
        } NewPlace;

        char Padding[sizeof(EventClass::Data)];
    } Data;
#pragma pack()
};

/**
 *  Ensure that our class has the same size and layout as the vanilla class.
 */
static_assert(sizeof(EventClassExt) == sizeof(EventClass), "EventClassExt must match EventClass in size!");
static_assert(sizeof(EventClassExt::Data) == sizeof(EventClass::Data), "EventClassExt::Data must match EventClass::Data in size!");
static_assert(offsetof(EventClassExt, Data) == offsetof(EventClass, Data), "EventClassExt::Data must be at the same offset as in EventClass!");
