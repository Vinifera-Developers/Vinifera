/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Protocol zero latency level class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once


enum LatencyLevelEnum : unsigned char {
    LATENCY_LEVEL_INITIAL = 0,

    LATENCY_LEVEL_1 = 1,
    LATENCY_LEVEL_2 = 2,
    LATENCY_LEVEL_3 = 3,
    LATENCY_LEVEL_4 = 4,
    LATENCY_LEVEL_5 = 5,
    LATENCY_LEVEL_6 = 6,
    LATENCY_LEVEL_7 = 7,
    LATENCY_LEVEL_8 = 8,
    LATENCY_LEVEL_9 = 9,

    LATENCY_LEVEL_MAX = LATENCY_LEVEL_9,
    LATENCY_SIZE = 1 + LATENCY_LEVEL_MAX
};


/**
 *  LatencyLevel
 *
 *  This class is contains methods for working with Protocol 0 latency levels.
 */
class LatencyLevel
{
public:
    LatencyLevel() = delete;

    static LatencyLevelEnum CurentLatencyLevel;
    static unsigned char NewFrameSendRate;

    static void Reset();
    static void Apply(LatencyLevelEnum new_latency_level);
    static void Apply(unsigned char new_latency_level) { Apply(static_cast<LatencyLevelEnum>(new_latency_level)); }
    static unsigned int Get_Max_Ahead(LatencyLevelEnum latency_level);
    static const char* Get_Latency_Message(LatencyLevelEnum latency_level);
    static LatencyLevelEnum From_Response_Time(unsigned int response_time);
};
