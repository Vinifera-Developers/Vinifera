/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for protocol zero.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "protocolzero_hooks.h"

#include "debughandler.h"
#include "event.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "ipxmgr.h"
#include "latencylevel.h"
#include "protocolzero.h"
#include "scenario.h"
#include "session.h"
#include "sessionext.h"
#include "spawner.h"
#include "syringe.h"
#include "tibsun_globals.h"

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


/**
 *  Patch to log network parameters.
 *
 *  @author: ZivDero
 */
void IPXManagerClassExt::_Set_Timing(unsigned long retrydelta, unsigned long maxretries, unsigned long timeout, bool global)
{
    if (SessionExtension->ProtocolZeroEnabled) {
        DEBUG_INFO("[Spawner] NewRetryDelta = {}, NewRetryTimeout = {}, FrameSendRate = {}, CurentLatencyLevel = {}\n", retrydelta, maxretries, Session.FrameSendRate, (int)LatencyLevel::CurentLatencyLevel);
    }

    /**
     *  Vanilla function.
     */
    DEBUG_INFO("RetryDelta = {}\n", retrydelta);
    DEBUG_INFO("MaxAhead is {}\n", Session.MaxAhead);

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


/**
 *  Patch IPXManagerClass to return our MaxAhead when Protocol 0 is active.
 *
 *  @author: ZivDero
 */
unsigned long IPXManagerClassExt::_Response_Time()
{
    if (SessionExtension->ProtocolZeroEnabled && !ProtocolZero::GetRealMaxAhead) {
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


/**
 *  Convenient patch in Main_Loop to send a Response Time event.
 *
 *  @author: ZivDero
 */
bool MessageListClassExt::_Manage()
{
    if (SessionExtension->ProtocolZeroEnabled) ProtocolZero::Send_Response_Time();

    return MessageListClass::Manage();
}


/**
 *  Skip checking MySent, if Protocol 0 is active.
 *
 *  @author: ZivDero
 */
static short& MySent = Make_Global<short>(0x008099F0);
DEFINE_HOOK(0x005B1A2D, _ProtocolZero_Queue_AI_Multiplayer_1, 0)
{
    if (SessionExtension->ProtocolZeroEnabled || MySent >= 5) {
        return 0x005B1A3B;
    }

    return 0x005B1C4C;
}


/**
 *  Patch adding the Timing event, taking Protocol 0 into consideration.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005B1BF1, _ProtocolZero_Queue_AI_Multiplayer_2, 0)
{
    DEBUG_INFO("[Spawner] Sending precalculated network timings on frame {}\n", Frame);

    EventClass ev;
    ev.Type = EVENT_TIMING;
    ev.Data.Timing.DesiredFrameRate = Session.PrecalcDesiredFrameRate;
    ev.Data.Timing.MaxAhead = Session.PrecalcMaxAhead;
    ev.Data.Timing.FrameSendRate = SessionExtension->ProtocolZeroEnabled ? LatencyLevel::NewFrameSendRate : Session.PrecalcDesiredFrameRate > 30 ? 10 : 5;

    OutList.Add(ev);
    Session.PrecalcMaxAhead = 0;
    Session.PrecalcDesiredFrameRate = 0;

    return 0x005B1C4C;
}


/**
 *  Patch adding the Timing event, taking Protocol 0 into consideration.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005B1B7A, _ProtocolZero_Queue_AI_Multiplayer_3, 0)
{
    GET(int, max_ahead, EDI);

    EventClass ev;
    ev.Type = EVENT_TIMING;
    ev.Data.Timing.DesiredFrameRate = Session.DesiredFrameRate;
    ev.Data.Timing.MaxAhead = SessionExtension->ProtocolZeroEnabled ? Session.MaxAhead : (max_ahead + (Scen->Special.IsFogOfWar ? 10 : 0));
    ev.Data.Timing.FrameSendRate = Session.FrameSendRate;

    OutList.Add(ev);

    return 0x005B1BB9;
}


/**
 *  If Protocol 0 is enabled, allow some types of packets to come in late.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005B4EA5, _ProtocolZero_ExecuteDoList, 5)
{
    GET(EventClass*, event, ESI);

    if (SessionExtension->ProtocolZeroEnabled) {
        if (event->Type == EVENT_EMPTY) {
            return 0x005B4EB7;
        }

        if (event->Type == EVENT_PROCESS_TIME) {
            return 0x005B4EB7;
        }

        if (event->Type == EXT_EVENT_RESPONSE_TIME2) {
            return 0x005B4EB7;
        }
    }

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void ProtocolZero_Hooks()
{
    Patch_Call(0x005091A5, &MessageListClassExt::_Manage);
    Patch_Jump(0x004F05B0, &IPXManagerClassExt::_Set_Timing);
    Patch_Jump(0x004F0F00, &IPXManagerClassExt::_Response_Time);
}
