/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          LATENCYLEVEL.H
 *
 *  @author        Belonit, ZivDero
 *
 *  @brief         Protocol zero latency level class.
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
#pragma once

#include <cstdint>

enum LatencyLevelEnum : uint8_t
{
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
class LatencyLevel
{
public:
    static LatencyLevelEnum CurentLatencyLevel;
    static uint8_t NewFrameSendRate;

    static void Apply(LatencyLevelEnum new_latency_level);
    static void __forceinline Apply(uint8_t newLatencyLevel)
    {
        Apply(static_cast<LatencyLevelEnum>(newLatencyLevel));
    }

    static int Get_MaxAhead(LatencyLevelEnum latencyLevel);
    static const char* Get_Latency_Message(LatencyLevelEnum latencyLevel);
    static LatencyLevelEnum From_Response_Time(uint8_t rspTime);
};
