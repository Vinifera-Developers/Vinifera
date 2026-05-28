/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Protocol zero.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "protocolzero.h"

#include <algorithm>

#include "eventext.h"
#include "house.h"
#include "ipxmgr.h"
#include "latencylevel.h"
#include "session.h"
#include "sessionext.h"
#include "spawner.h"

#include "debughandler.h"


bool ProtocolZero::GetRealMaxAhead = false;
unsigned int ProtocolZero::WorstMaxAhead = 24;


/**
 *  Sends a Response Time event.
 *
 *  @author: Belonit
 */
void ProtocolZero::Send_Response_Time()
{
    if (!SessionExtension->ProtocolZeroEnabled || Session.Singleplayer_Game()) {
        return;
    }

    static int NextSendFrame = 6 * SendResponseTimeInterval;

    /**
     *  It is not yet time to send a Response Time event.
     */
    if (Frame <= NextSendFrame) {
        return;
    }

    /**
     *  IPXManagerClass::Response_Time is patched to return ProtocolZero::MaxAhead,
     *  so to get the real MaxAhead we set this bool to true just for this call.
     */
    GetRealMaxAhead = true;
    const unsigned int ipxResponseTime = Ipx.Response_Time();
    GetRealMaxAhead = false;

    /**
     *  Create the event.
     */
    EventClassExt event(PlayerPtr->HeapID, static_cast<unsigned char>(ipxResponseTime + 1), LatencyLevel::From_Response_Time(ipxResponseTime));

    /**
     *  Send it!
     */
    if (OutList.Add(event.As_Event())) {
        NextSendFrame = Frame + SendResponseTimeInterval;
        DEBUG_INFO("[Spawner] Player {} sending response time of {}, LatencyMode = {}, Frame = {}\n", event.ID, event.Data.ResponseTime2.MaxAhead, (int)event.Data.ResponseTime2.LatencyLevel, Frame);
    } else {
        NextSendFrame++;
    }
}


/**
 *  Executes a Response Time event.
 *
 *  @author: Belonit
 */
void ProtocolZero::Handle_Response_Time(EventClassExt& event)
{
    if (!SessionExtension->ProtocolZeroEnabled || Session.Singleplayer_Game()) {
        return;
    }


    if (event.Data.ResponseTime2.MaxAhead == 0) {
        DEBUG_INFO("[Spawner] Returning because event.MaxAhead == 0\n");
        return;
    }

    static unsigned int PlayerMaxAheads[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    static unsigned char PlayerLatencyMode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    static unsigned int PlayerLastTimingFrame[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    /**
     *  Save the info we got from the event.
     */
    PlayerMaxAheads[event.ID] = event.Data.ResponseTime2.MaxAhead;
    PlayerLatencyMode[event.ID] = event.Data.ResponseTime2.LatencyLevel;
    PlayerLastTimingFrame[event.ID] = event.Frame;

    /**
     *  Now loop all the players and find the worst one latency-wise.
     */
    unsigned char latency_mode = 0;
    unsigned int max_ahead = 0;

    for (size_t i = 0; i < std::size(PlayerMaxAheads); i++) {
        if (PlayerLastTimingFrame[i] + SendResponseTimeInterval * 4 < Frame) {
            PlayerMaxAheads[i] = 0;
            PlayerLatencyMode[i] = 0;
        } else {
            max_ahead = PlayerMaxAheads[i] > max_ahead ? PlayerMaxAheads[i] : max_ahead;
            latency_mode = std::max(PlayerLatencyMode[i], latency_mode);
        }
    }

    /**
     *  The worst determines the settings for all the players.
     */
    WorstMaxAhead = max_ahead;
    LatencyLevel::Apply(latency_mode);
}
