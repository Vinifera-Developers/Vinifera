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

enum LatencyLevelEnum : unsigned char
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
    LatencyLevel() = delete;

    static LatencyLevelEnum CurentLatencyLevel;
    static unsigned char NewFrameSendRate;

    static void Apply(LatencyLevelEnum new_latency_level);
    static void Apply(unsigned char new_latency_level) { Apply(static_cast<LatencyLevelEnum>(new_latency_level)); }
    static unsigned int Get_Max_Ahead(LatencyLevelEnum latency_level);
    static const char* Get_Latency_Message(LatencyLevelEnum latency_level);
    static LatencyLevelEnum From_Response_Time(unsigned int response_time);
};
