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

//#include "ProtocolZero.h"
//#include "ProtocolZero.LatencyLevel.h"
//#include "Spawner.h"
//
//#include <Ext/Event/Body.h>
//#include <Helpers/Macro.h>
//#include <Utilities/Debug.h>
//#include <Unsorted.h>
//#include <EventClass.h>

//
//DEFINE_HOOK(0x4C8011, EventClassExecute__ProtocolZero, 0x8)
//{
//	if (ProtocolZero::Enable)
//		return 0x4C8024;
//
//	return 0;
//}
//
//DEFINE_HOOK(0x64C598, ExecuteDoList__ProtocolZero, 0x6)
//{
//	if (ProtocolZero::Enable)
//	{
//		auto dl = (uint8_t)R->DL();
//
//		if (dl == (uint8_t)EventType::EMPTY)
//			return 0x64C63D;
//
//		if (dl == (uint8_t)EventType::PROCESS_TIME)
//			return 0x64C63D;
//
//		if (dl == (uint8_t)EventTypeExt::ResponseTime2)
//			return 0x64C63D;
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x64771D, QueueAIMultiplayer__ProtocolZero_SetTiming, 0x5)
//DEFINE_HOOK(0x647E6B, QueueAIMultiplayer__ProtocolZero_SetTiming, 0x5)
//{
//	if (ProtocolZero::Enable)
//	{
//		GET(int, NewRetryDelta, EBP);
//		GET(int, NewRetryTimeout, EAX);
//
//		Debug::Log("[Spawner] NewRetryDelta = %d, NewRetryTimeout = %d, FrameSendRate = %d, CurentLatencyLevel = %d\n"
//			, NewRetryDelta
//			, NewRetryTimeout
//			, (int)Game::Network::FrameSendRate
//			, (int)LatencyLevel::CurentLatencyLevel
//		);
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x6476CB, QueueAIMultiplayer__ProtocolZero_ResponseTime, 0x5)
//DEFINE_HOOK(0x647CC5, QueueAIMultiplayer__ProtocolZero_ResponseTime, 0x5)
//{
//	if (ProtocolZero::Enable)
//	{
//		R->EAX(ProtocolZero::WorstMaxAhead);
//		return R->Origin() + 0x5;
//	}
//
//	return 0;
//}


#include "hooker.h"
#include "hooker_macros.h"
#include "latencylevel.h"
#include "protocolzero.h"
#include "tibsun_globals.h"
#include "session.h"

DECLARE_PATCH(_ProtocolZero_Main_Loop)
{
    if (ProtocolZero::Enable)
        ProtocolZero::Send_ResponseTime2();

    // Stolen instructions
    Session.Messages.Manage();
    JMP_REG(ecx, 0x005091AA);
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

DECLARE_PATCH(_ProtocolZero_Queue_AI_Multiplayer_2)
{
    GET_REGISTER_STATIC(char, precalc_desired_frame_rate, eax)

    if (ProtocolZero::Enable)
    {
        precalc_desired_frame_rate = LatencyLevel::NewFrameSendRate;
    }

    // Stolen instructions
    _asm mov [esp+0x36], precalc_desired_frame_rate
    JMP(0x005B1C3B);
}


DECLARE_PATCH(_ProtocolZero_Queue_AI_Multiplayer_3)
{
    static int max_ahead;

    if (ProtocolZero::Enable)
    {
        max_ahead = Session.MaxAhead & 0xFFFF;
        _asm mov edx, max_ahead;
        _asm mov ecx, & OutList;
        JMP(0x005B1BA9);
    }

    _asm mov ecx, &OutList;
    JMP(0x005B1BA6);
}

void ProtocolZero_Patches()
{
    Patch_Jump(0x005091A0, &_ProtocolZero_Main_Loop);
    Patch_Jump(0x005B1A2D, &_ProtocolZero_Queue_AI_Multiplayer_1);
    Patch_Jump(0x005B1C37, &_ProtocolZero_Queue_AI_Multiplayer_2);
    Patch_Jump(0x005B1BA1, &_ProtocolZero_Queue_AI_Multiplayer_3);
}