/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Protocol zero.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once

#include "tibsun_defines.h"

class EventClassExt;


/**
 *  ProtocolZero
 *
 *  This class is contains methods and the state of Protocol 0.
 */
class ProtocolZero
{
private:
    static constexpr int SendResponseTimeInterval = 30;
    static int NextSendFrame;
    static unsigned int PlayerMaxAheads[MAX_PLAYERS];
    static unsigned char PlayerLatencyModes[MAX_PLAYERS];
    static int PlayerLastTimingFrames[MAX_PLAYERS];

public:
    static bool GetRealMaxAhead;
    static unsigned int WorstMaxAhead;

    static void Reset();
    static void Send_Response_Time();
    static void Handle_Response_Time(EventClassExt& event);
};
