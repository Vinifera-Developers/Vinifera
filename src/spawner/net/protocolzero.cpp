/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PROTOCOLZERO.CPP
 *
 *  @author        Belonit, ZivDero
 *
 *  @brief         Protocol zero.
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

#include "protocolzero.h"

#include "latencylevel.h"
#include "spawner.h"
#include "viniferaevent/viniferaevent.h"
#include "house.h"
#include "session.h"
#include "ipxmgr.h"

#include "debughandler.h"


bool ProtocolZero::Enable = false;
bool ProtocolZero::GetRealMaxAhead = false;
unsigned int ProtocolZero::WorstMaxAhead = 24;
unsigned char ProtocolZero::MaxLatencyLevel = 0xff;


/**
 *  Sends a Response Time event.
 *
 *  @author: Belonit
 */
void ProtocolZero::Send_Response_Time()
{
    if (Enable == false || Session.Singleplayer_Game())
        return;

    static int NextSendFrame = 6 * SendResponseTimeInterval;

    /**
     *  It is not yet time to send a Response Time event.
     */
    if (Frame <= NextSendFrame)
        return;

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
    ViniferaEventClass event;
    event.Type = VEVENT_RESPONSE_TIME_2;
    event.ID = PlayerPtr->Get_Heap_ID();
    event.Frame = Frame + Session.MaxAhead;
    event.Data.ResponseTime2.MaxAhead = static_cast<unsigned char>(ipxResponseTime + 1);
    event.Data.ResponseTime2.LatencyLevel = LatencyLevel::From_Response_Time(ipxResponseTime);

    /**
     *  Send it!
     */
    if (OutList.Add(event.As_Event()))
    {
        NextSendFrame = Frame + SendResponseTimeInterval;
        DEBUG_INFO("[Spawner] Player %d sending response time of %u, LatencyMode = %d, Frame = %d\n"
            , event.ID
            , event.Data.ResponseTime2.MaxAhead
            , event.Data.ResponseTime2.LatencyLevel
            , Frame
        );
    }
    else
    {
        NextSendFrame++;
    }
}


/**
 *  Executes a Response Time event.
 *
 *  @author: Belonit
 */
void ProtocolZero::Handle_Response_Time(ViniferaEventClass* event)
{
    if (Enable == false || Session.Singleplayer_Game())
        return;

    if (event->Data.ResponseTime2.MaxAhead == 0)
    {
        DEBUG_INFO("[Spawner] Returning because event->MaxAhead == 0\n");
        return;
    }

    static unsigned int PlayerMaxAheads[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static unsigned char PlayerLatencyMode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static unsigned int PlayerLastTimingFrame[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    /**
     *  Save the info we got from the event.
     */
    PlayerMaxAheads[event->ID] = event->Data.ResponseTime2.MaxAhead;
    PlayerLatencyMode[event->ID] = event->Data.ResponseTime2.LatencyLevel;
    PlayerLastTimingFrame[event->ID] = event->Frame;

    /**
     *  Now loop all the players and find the worst one latency-wise.
     */
    unsigned char latency_mode = 0;
    unsigned int max_ahead = 0;

    for (size_t i = 0; i < std::size(PlayerMaxAheads); i++)
    {
        if (PlayerLastTimingFrame[i] + SendResponseTimeInterval * 4 < Frame)
        {
            PlayerMaxAheads[i] = 0;
            PlayerLatencyMode[i] = 0;
        }
        else
        {
            max_ahead = PlayerMaxAheads[i] > max_ahead ? PlayerMaxAheads[i] : max_ahead;
            if (PlayerLatencyMode[i] > latency_mode)
                latency_mode = PlayerLatencyMode[i];
        }
    }

    /**
     *  The worst determines the settings for all the players.
     */
    WorstMaxAhead = max_ahead;
    LatencyLevel::Apply(latency_mode);
}
