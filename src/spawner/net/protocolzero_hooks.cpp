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


void IPXManagerClassExt::_Set_Timing(unsigned long retrydelta, unsigned long maxretries,
    unsigned long timeout, bool global)
{
    if (ProtocolZero::Enable) {
        DEBUG_INFO("[Spawner] NewRetryDelta = %d, NewRetryTimeout = %d, FrameSendRate = %d, CurentLatencyLevel = %d\n"
            , retrydelta
            , maxretries
            , Session.FrameSendRate
            , LatencyLevel::CurentLatencyLevel
        );
    }

    // Vanilla function
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
    if (ProtocolZero::Enable) {
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
    GET_REGISTER_STATIC(unsigned char, precalc_desired_frame_rate, al)

    if (ProtocolZero::Enable)
    {
        precalc_desired_frame_rate = LatencyLevel::NewFrameSendRate;
        _asm mov al, precalc_desired_frame_rate
        JMP_REG(ecx, 0x005B1C2D);
    }

    // Stolen instructions
    _asm
    {
        mov al, precalc_desired_frame_rate
        and al, 5
        push ecx
        add al, 5
    }

    JMP_REG(ecx, 0x005B1C2D);
}


DECLARE_PATCH(_ProtocolZero_Queue_AI_Multiplayer_3)
{
    static unsigned int max_ahead;

    if (ProtocolZero::Enable)
    {
        max_ahead = Session.MaxAhead & 0xFFFF;
        _asm mov edx, max_ahead;
    }

    // Stolen instructions
    _asm mov [esp+0x34], dx
    JMP(0x005B1BB4);
}


DECLARE_PATCH(_ProtocolZero_EventClass_Execute)
{
    // Don't subtract 10 from MaxAhead
	if (ProtocolZero::Enable)
	{
        JMP(0x0049502B);
	}

    // Stolen instructions
	_asm
    {
        mov eax, [edx]
        and eax, 0x800
    }

    JMP_REG(ecx, 0x00495020);
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

        if (event->Type == VINIFERA_EVENT_RESPONSETIME2)
            goto continue_execution;
    }

    _asm mov eax, Session
    JMP_REG(ecx, 0x005B4EAA);

    continue_execution:
    JMP(0x005B4EB7);
}


void ProtocolZero_Hooks()
{
    Patch_Jump(0x005091A0, &_ProtocolZero_Main_Loop);
    Patch_Jump(0x005B1A2D, &_ProtocolZero_Queue_AI_Multiplayer_1);
    Patch_Jump(0x005B1C28, &_ProtocolZero_Queue_AI_Multiplayer_2);
    Patch_Jump(0x005B1BAF, &_ProtocolZero_Queue_AI_Multiplayer_3);
    Patch_Jump(0x00495019, &_ProtocolZero_EventClass_Execute);
    Patch_Jump(0x005B4EA5, &_ProtocolZero_ExecuteDoList);
    Patch_Jump(0x004F05B0, &IPXManagerClassExt::_Set_Timing);
    Patch_Jump(0x004F0F00, &IPXManagerClassExt::_Response_Time);
}
