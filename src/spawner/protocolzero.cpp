/**
*  yrpp-spawner
*
*  Copyright(C) 2023-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "protocolzero.h"
#include "latencylevel.h"

#include "spawner.h"
#include "viniferaevent/viniferaevent.h"
#include "debughandler.h"
#include "house.h"
#include "session.h"
#include "ipxmgr.h"

bool ProtocolZero::Enable = false;
int ProtocolZero::WorstMaxAhead = 24;
unsigned char ProtocolZero::MaxLatencyLevel = 0xff;

void ProtocolZero::Send_ResponseTime2()
{
	if (Session.Type == GAME_NORMAL)
		return;

	static int NextSendFrame = 6 * SendResponseTimeInterval;

	if (NextSendFrame >= Frame)
		return;

	const int ipxResponseTime = Ipx->Response_Time();
	if (ipxResponseTime <= -1)
		return;

	ViniferaEventClass event;
	event.Type = VINIFERA_EVENT_RESPONSETIME2;
	event.ID = PlayerPtr->Get_Heap_ID();
	event.Frame = Frame + Session.MaxAhead;
	event.Data.ResponseTime2.MaxAhead = (char)ipxResponseTime + 1;
	event.Data.ResponseTime2.LatencyLevel = (char)LatencyLevel::FromResponseTime((char)ipxResponseTime);

	if (OutList.Add(event.As_Event()))
	{
		NextSendFrame = Frame + SendResponseTimeInterval;
		DEBUG_INFO("[Spawner] Player %d sending response time of %d, LatencyMode = %d, Frame = %d\n"
			, event.ID
			, event.Data.ResponseTime2.MaxAhead
			, event.Data.ResponseTime2.LatencyLevel
			, Frame
		);
	}
	else
	{
		++NextSendFrame;
	}
}

void ProtocolZero::HandleResponseTime2(ViniferaEventClass* event)
{
	if (ProtocolZero::Enable == false || Session.Type == GAME_NORMAL)
		return;

	if (event->Data.ResponseTime2.MaxAhead == 0)
	{
		DEBUG_INFO("[Spawner] Returning because event->MaxAhead == 0\n");
		return;
	}

	static int32_t PlayerMaxAheads[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static uint8_t PlayerLatencyMode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static int32_t PlayerLastTimingFrame[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	int32_t houseIndex = event->ID;
	PlayerMaxAheads[houseIndex] = (int32_t)event->Data.ResponseTime2.MaxAhead;
	PlayerLatencyMode[houseIndex] = event->Data.ResponseTime2.LatencyLevel;
	PlayerLastTimingFrame[houseIndex] = event->Frame;

	uint8_t setLatencyMode = 0;
	int maxMaxAheads = 0;

	for (char i = 0; i < (char)std::size(PlayerMaxAheads); ++i)
	{
		if (Frame >= (PlayerLastTimingFrame[i] + (SendResponseTimeInterval * 4)))
		{
			PlayerMaxAheads[i] = 0;
			PlayerLatencyMode[i] = 0;
		}
		else
		{
			maxMaxAheads = PlayerMaxAheads[i] > maxMaxAheads ? PlayerMaxAheads[i] : maxMaxAheads;
			if (PlayerLatencyMode[i] > setLatencyMode)
				setLatencyMode = PlayerLatencyMode[i];
		}
	}

	ProtocolZero::WorstMaxAhead = maxMaxAheads;
	LatencyLevel::Apply(setLatencyMode);
}
