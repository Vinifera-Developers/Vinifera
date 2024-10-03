/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PROTOCOLZERO_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for protocol zero.
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

#include "protocolzero_hooks.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "latencylevel.h"
#include "protocolzero.h"
#include "tibsun_globals.h"
#include "session.h"
#include "viniferaevent/viniferaevent.h"
#include "ipxmgr.h"
#include "scenario.h"
#include "spawner.h"

/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class IPXManagerClassExt : public IPXManagerClass
{
public:
    void _Set_Timing(unsigned long retrydelta, unsigned long maxretries, unsigned long timeout, bool global = true);
    unsigned long _Response_Time();
};


void IPXManagerClassExt::_Set_Timing(unsigned long retrydelta, unsigned long maxretries, unsigned long timeout, bool global)
{
    if (ProtocolZero::Enable) {
        DEBUG_INFO("[Spawner] NewRetryDelta = %d, NewRetryTimeout = %d, FrameSendRate = %d, CurentLatencyLevel = %d\n"
            , retrydelta
            , maxretries
            , Session.FrameSendRate
            , LatencyLevel::CurentLatencyLevel
        );
    }

    /**
     *  Vanilla function.
     */
    DEBUG_INFO("RetryDelta = %d\n", retrydelta);
    DEBUG_INFO("MaxAhead is %d\n", Session.MaxAhead);

    RetryDelta = retrydelta;
    MaxRetries = maxretries;
    Timeout = timeout;

    if (global) {
        Set_External_Timing(RetryDelta, MaxRetries, Timeout);
    }

    for (int i = 0; i < NumConnections; i++) {
        Connection[i]->Set_Retry_Delta(RetryDelta);
        Connection[i]->Set_Max_Retries(MaxRetries);
        Connection[i]->Set_TimeOut(Timeout);
    }
}


unsigned long IPXManagerClassExt::_Response_Time()
{
    if (ProtocolZero::Enable && !ProtocolZero::GetRealMaxAhead) {
        return ProtocolZero::WorstMaxAhead;
    }

    // Vanilla function
    unsigned long maxresp = 0;

    for (int i = 0; i < NumConnections; i++) {
        unsigned long resp = Connection[i]->Queue->Avg_Response_Time();
        if (resp > maxresp) {
            maxresp = resp;
        }
    }

    return maxresp;
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class MessageListClassExt : public MessageListClass
{
public:
    bool _Manage();
};


bool MessageListClassExt::_Manage()
{
    if (ProtocolZero::Enable)
        ProtocolZero::Send_Response_Time();

    return MessageListClass::Manage();
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class EventClassExt : public EventClass
{
public:
    void _Execute_Timing();
};


void EventClassExt::_Execute_Timing()
{
    if (!ProtocolZero::Enable)
        Data.Timing.MaxAhead -= Scen->SpecialFlags.IsFogOfWar ? 10 : 0;

    if (Data.Timing.MaxAhead > Session.MaxAhead || Data.Timing.FrameSendRate > Session.FrameSendRate)
    {
        NewMaxAheadFrame1 = Frame;
        NewMaxAheadFrame2 = Data.Timing.FrameSendRate * ((Data.Timing.FrameSendRate + Data.Timing.MaxAhead + Frame - 1) / Data.Timing.FrameSendRate);
    }
    else
    {
        NewMaxAheadFrame1 = 0;
        NewMaxAheadFrame2 = 0;
    }

    Session.DesiredFrameRate = Data.Timing.DesiredFrameRate;
    Session.MaxAhead = Data.Timing.MaxAhead;
    if (Session.MaxAhead > Session.MaxMaxAhead)
        Session.MaxMaxAhead = Session.MaxAhead;
    Session.FrameSendRate = Data.Timing.FrameSendRate;
}


DECLARE_PATCH(_ProtocolZero_EventClass_Execute)
{
    GET_REGISTER_STATIC(EventClassExt*, e, esi);

    e->_Execute_Timing();

    JMP(0x004950AD);
}


static short& MySent = Make_Global<short>(0x008099F0);
DECLARE_PATCH(_ProtocolZero_Queue_AI_Multiplayer_1)
{
    if (ProtocolZero::Enable || MySent >= 5)
    {
        JMP(0x005B1A3B);
    }

    JMP(0x005B1C4C);
}


static void Add_Timing_Event()
{
    DEBUG_INFO("[Spawner] Sending precalculated network timings on frame %d\n", Frame);

    EventClass ev;
    ev.Type = EVENT_TIMING;
    ev.Data.Timing.DesiredFrameRate = Session.PrecalcDesiredFrameRate;
    ev.Data.Timing.MaxAhead = Session.PrecalcMaxAhead;
    ev.Data.Timing.FrameSendRate = ProtocolZero::Enable ? LatencyLevel::NewFrameSendRate :
        Session.PrecalcDesiredFrameRate > 30 ? 10 : 5;

    OutList.Add(ev);
    Session.PrecalcMaxAhead = 0;
    Session.PrecalcDesiredFrameRate = 0;
}


DECLARE_PATCH(_ProtocolZero_Queue_AI_Multiplayer_2)
{
    Add_Timing_Event();
    JMP(0x005B1C4C);
}


static void Add_Timing_Event_2(int max_ahead)
{
    EventClass ev;
    ev.Type = EVENT_TIMING;
    ev.Data.Timing.DesiredFrameRate = Session.DesiredFrameRate;
    ev.Data.Timing.MaxAhead = ProtocolZero::Enable ? Session.MaxAhead : (max_ahead + Scen->SpecialFlags.IsFogOfWar ? 10 : 0);
    ev.Data.Timing.FrameSendRate = Session.FrameSendRate;

    OutList.Add(ev);
}


DECLARE_PATCH(_ProtocolZero_Queue_AI_Multiplayer_3)
{
    GET_REGISTER_STATIC(int, max_ahead, edi);
    _asm push esi

    Add_Timing_Event_2(max_ahead);

    _asm pop esi
    JMP(0x005B1BB9);
}


DECLARE_PATCH(_ProtocolZero_ExecuteDoList)
{
    GET_REGISTER_STATIC(EventClass*, event, esi)

    if (ProtocolZero::Enable)
    {
        if (event->Type == EVENT_EMPTY)
            goto continue_execution;

        if (event->Type == EVENT_PROCESS_TIME)
            goto continue_execution;

        if (event->Type == VEVENT_RESPONSE_TIME_2)
            goto continue_execution;
    }

    _asm mov eax, Session
    _asm mov eax, [eax]
    JMP_REG(ecx, 0x005B4EAA);

    continue_execution:
    JMP(0x005B4EB7);
}


void ProtocolZero_Hooks()
{
    Patch_Call(0x005091A5, &MessageListClassExt::_Manage);
    Patch_Jump(0x005B1A2D, &_ProtocolZero_Queue_AI_Multiplayer_1);
    Patch_Jump(0x005B1BF1, &_ProtocolZero_Queue_AI_Multiplayer_2);
    Patch_Jump(0x005B1B7A, &_ProtocolZero_Queue_AI_Multiplayer_3);
    Patch_Jump(0x00495013, &_ProtocolZero_EventClass_Execute);
    Patch_Jump(0x005B4EA5, &_ProtocolZero_ExecuteDoList);
    Patch_Jump(0x004F05B0, &IPXManagerClassExt::_Set_Timing);
    Patch_Jump(0x004F0F00, &IPXManagerClassExt::_Response_Time);
}
