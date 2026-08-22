/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended EventClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "event.h"
#include "footext.h"
#include "latencylevel.h"


/**
 *  This is the extended EventClass. It doesn't literally extend or replace the vanilla EventClass,
 *  but instead provides us with a way to send custom payloads via the event system as well as create new/
 *  replace implementations of vanilla events.
 */
class EventClassExt
{
    friend void EventClassExtension_Hooks();

public:
    EventClassExt() { Type = EVENT_EMPTY; }
    EventClassExt(int index, EventType type, RTTIType object, int id, ProductionFlags flags);
    EventClassExt(int index, EventType type, RTTIType object, Cell const& cell, ProductionFlags flags);
    EventClassExt(int id, unsigned char max_ahead, LatencyLevelEnum latency_level);
    EventClassExt(int index, EventType type, bool pausedRepairs);

    int operator==(const EventClassExt& q) const { return std::memcmp(this, &q, sizeof(q)) == 0; }
    int operator!=(const EventClassExt& q) const { return std::memcmp(this, &q, sizeof(q)) != 0; }

    EventClass* As_Event_Ptr() { return reinterpret_cast<EventClass*>(this); }
    EventClass& As_Event() { return reinterpret_cast<EventClass&>(*this); }

    static bool Is_Vinifera_Event(EventType type);
    bool Is_Vinifera_Event() const;

    void Execute();

    void Do_IDLE();
    void Do_TIMING();
    void Do_REMOVEPLAYER();

    static const char* Event_Name(EventType event) { return event >= EVENT_EMPTY && event < EXT_EVENT_COUNT ? EventNames[event] : ""; }
    static unsigned char Event_Length(EventType event) { return event >= EVENT_EMPTY && event < EXT_EVENT_COUNT ? EventLength[event] : 0; }

#pragma pack(1) // We need this so bools/bits are not aligned.
public:
    EventType Type;
    long Frame;
    bool IsExecuted;
    unsigned ID;

    union {
        struct {
            int Value;
        } General;

        struct {
            xTargetClass Whom;
        } Target;

        struct {
            xTargetClass Whom;
            xTargetClass Where;
        } NavCom;

        struct {
            unsigned short DesiredFrameRate;
            unsigned short MaxAhead;
            unsigned char FrameSendRate;
        } Timing;

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

        struct {
            bool            IsPauseRepairs;
        } PlayerOptions;

        struct ResponseTime2 {
            unsigned char MaxAhead;
            LatencyLevelEnum LatencyLevel;
        } ResponseTime2;

        char Padding[sizeof(EventClass::Data)];
    } Data;
#pragma pack()

private:
    static unsigned char EventLength[EXT_EVENT_COUNT];
    static char const* EventNames[EXT_EVENT_COUNT];
};

/**
 *  Ensure that our class has the same size and layout as the vanilla class.
 */
static_assert(sizeof(EventClassExt) == sizeof(EventClass), "EventClassExt must match EventClass in size!");
static_assert(sizeof(EventClassExt::Data) == sizeof(EventClass::Data), "EventClassExt::Data must match EventClass::Data in size!");
static_assert(offsetof(EventClassExt, Data) == offsetof(EventClass, Data), "EventClassExt::Data must be at the same offset as in EventClass!");
