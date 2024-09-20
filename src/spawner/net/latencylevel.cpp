/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          LATENCYLEVEL.CPP
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

#include "latencylevel.h"

#include "protocolzero.h"
#include "debughandler.h"
#include "house.h"
#include "session.h"

LatencyLevelEnum LatencyLevel::CurentLatencyLevel = LATENCY_LEVEL_INITIAL;
unsigned char LatencyLevel::NewFrameSendRate = 3;

void LatencyLevel::Apply(LatencyLevelEnum new_latency_level)
{
    if (new_latency_level > LATENCY_LEVEL_MAX)
        new_latency_level = LATENCY_LEVEL_MAX;

    auto max_latency_level = static_cast<LatencyLevelEnum>(ProtocolZero::MaxLatencyLevel);
    if (new_latency_level > max_latency_level)
        new_latency_level = max_latency_level;

    if (new_latency_level <= CurentLatencyLevel)
        return;

    DEBUG_INFO("[Spawner] Player %ls, Loss mode (%d, %d) Frame = %d\n"
        , PlayerPtr->IniName
        , new_latency_level
        , CurentLatencyLevel
        , Frame
    );

    CurentLatencyLevel = new_latency_level;
    NewFrameSendRate = static_cast<unsigned char>(new_latency_level);
    Session.PrecalcDesiredFrameRate = 60;
    Session.PrecalcMaxAhead = Get_MaxAhead(new_latency_level);
    Session.Messages.Add_Message(nullptr, 0, Get_Latency_Message(new_latency_level), COLORSCHEME_WHITE, TPF_USE_GRAD_PAL | TPF_FULLSHADOW | TPF_6PT_GRAD, 270);
}

int LatencyLevel::Get_MaxAhead(LatencyLevelEnum latencyLevel)
{
    const int maxAhead[] =
    {
        /* 0 */ 1

        /* 1 */ ,4
        /* 2 */ ,6
        /* 3 */ ,12
        /* 4 */ ,16
        /* 5 */ ,20
        /* 6 */ ,24
        /* 7 */ ,28
        /* 8 */ ,32
        /* 9 */ ,36
    };

    return maxAhead[(int)latencyLevel];
}

const char* LatencyLevel::Get_Latency_Message(LatencyLevelEnum latencyLevel)
{
    const char* message[] =
    {
        /* 0 */ "CnCNet: Latency mode set to: 0 - Initial" // Players should never see this, if they do, then it's a bug

        /* 1 */ ,"CnCNet: Latency mode set to: 1 - Best"
        /* 2 */ ,"CnCNet: Latency mode set to: 2 - Super"
        /* 3 */ ,"CnCNet: Latency mode set to: 3 - Excellent"
        /* 4 */ ,"CnCNet: Latency mode set to: 4 - Very Good"
        /* 5 */ ,"CnCNet: Latency mode set to: 5 - Good"
        /* 6 */ ,"CnCNet: Latency mode set to: 6 - Good"
        /* 7 */ ,"CnCNet: Latency mode set to: 7 - Default"
        /* 8 */ ,"CnCNet: Latency mode set to: 8 - Default"
        /* 9 */ ,"CnCNet: Latency mode set to: 9 - Default"
    };

    return message[(int)latencyLevel];
}

LatencyLevelEnum LatencyLevel::From_Response_Time(unsigned char rspTime)
{
    for (char i = LATENCY_LEVEL_1; i < LATENCY_LEVEL_MAX; i++)
    {
        if (rspTime <= Get_MaxAhead(static_cast<LatencyLevelEnum>(i)))
            return static_cast<LatencyLevelEnum>(i);
    }

    return LATENCY_LEVEL_MAX;
}
