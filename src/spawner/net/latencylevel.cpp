/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Protocol zero latency level class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "latencylevel.h"
#include <algorithm>
#include "colorscheme.h"
#include "debughandler.h"
#include "house.h"
#include "protocolzero.h"
#include "rules.h"
#include "session.h"
#include "sessionext.h"


LatencyLevelEnum LatencyLevel::CurentLatencyLevel = LATENCY_LEVEL_INITIAL;
unsigned char LatencyLevel::NewFrameSendRate = 3;


/**
 *  Resets Protocol Zero latency state for a new or reloaded session.
 */
void LatencyLevel::Reset()
{
    CurentLatencyLevel = LATENCY_LEVEL_INITIAL;
    NewFrameSendRate = 3;
}


/**
 *  Sets the desired latency level.
 *
 *  @author: Belonit
 */
void LatencyLevel::Apply(LatencyLevelEnum new_latency_level)
{
    new_latency_level = std::min(new_latency_level, LATENCY_LEVEL_MAX);

    auto max_latency_level = static_cast<LatencyLevelEnum>(SessionExtension->ProtocolZeroMaxLatencyLevel);
    new_latency_level = std::min(new_latency_level, max_latency_level);

    if (new_latency_level <= CurentLatencyLevel) return;

    DEBUG_INFO("[Spawner] Player {}, LatencyMode ({}, {}) Frame = {}\n", PlayerPtr->IniName, (int)new_latency_level, (int)CurentLatencyLevel, Frame);

    CurentLatencyLevel = new_latency_level;
    NewFrameSendRate = static_cast<unsigned char>(new_latency_level);
    Session.PrecalcDesiredFrameRate = 60;
    Session.PrecalcMaxAhead = Get_Max_Ahead(new_latency_level);
    Session.Messages.Add_Message(nullptr, 0, Get_Latency_Message(new_latency_level), Fetch_Scheme_Index_By_Name("White"), TPF_USE_GRAD_PAL | TPF_FULLSHADOW | TPF_6PT_GRAD, static_cast<int>(Rule->MessageDelay * TICKS_PER_MINUTE / 2));
}


/**
 *  Gets the max ahead for the given latency level.
 *
 *  @author: Belonit
 */
unsigned int LatencyLevel::Get_Max_Ahead(LatencyLevelEnum latency_level)
{
    static const int maxAhead[] = {/* 0 */ 1,
                                   /* 1 */ 4,
                                   /* 2 */ 6,
                                   /* 3 */ 9,
                                   /* 4 */ 12,
                                   /* 5 */ 15,
                                   /* 6 */ 18,
                                   /* 7 */ 21,
                                   /* 8 */ 24,
                                   /* 9 */ 27};

    return maxAhead[latency_level];
}


/**
 *  Gets the chat message for the given latency level.
 *
 *  @author: Belonit
 */
const char* LatencyLevel::Get_Latency_Message(LatencyLevelEnum latency_level)
{
    const char* message[] = {
        /* 0 */ "Network: Latency mode set to: 0 - Initial", // Players should never see this, if they do, then it's a bug
        /* 1 */ "Network: Latency mode set to: 1 - Best",
        /* 2 */ "Network: Latency mode set to: 2 - Super",
        /* 3 */ "Network: Latency mode set to: 3 - Excellent",
        /* 4 */ "Network: Latency mode set to: 4 - Very Good",
        /* 5 */ "Network: Latency mode set to: 5 - Good",
        /* 6 */ "Network: Latency mode set to: 6 - Decent",
        /* 7 */ "Network: Latency mode set to: 7 - Mediocre",
        /* 8 */ "Network: Latency mode set to: 8 - Bad",
        /* 9 */ "Network: Latency mode set to: 9 - Worst",
    };

    return message[latency_level];
}


/**
 *  Gets the latency level for the given response time.
 *
 *  @author: Belonit
 */
LatencyLevelEnum LatencyLevel::From_Response_Time(unsigned int response_time)
{
    for (char i = LATENCY_LEVEL_1; i < LATENCY_LEVEL_MAX; i++) {
        if (response_time <= Get_Max_Ahead(static_cast<LatencyLevelEnum>(i))) {
            return static_cast<LatencyLevelEnum>(i);
        }
    }

    return LATENCY_LEVEL_MAX;
}
