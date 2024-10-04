/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PROTOCOLZERO.H
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
#pragma once

class ViniferaEventClass;


/**
 *  ProtocolZero
 *
 *  This class is contains methods and the state of Protocol 0.
 */
class ProtocolZero
{
private:
    static constexpr int SendResponseTimeInterval = 30;

public:
    static bool Enable;
    static bool GetRealMaxAhead;
    static unsigned char MaxLatencyLevel;
    static unsigned int WorstMaxAhead;

    static void Send_Response_Time();
    static void Handle_Response_Time(ViniferaEventClass* event);
};
