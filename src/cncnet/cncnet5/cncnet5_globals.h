/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET_GLOBALS.H
 *
 *  @author        CCHyper
 *
 *  @brief         Global values and types used for the CnCNet5 system. 
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

#include <always.h>


namespace CnCNet5
{

typedef struct TunnelInfoStruct
{
    unsigned long ID;
    unsigned long IP;
    unsigned short Port;
    bool PortHack;

    bool Is_Valid() const { return !(ID == -1 || IP == -1 || Port == -1); }

} TunnelInfoStruct;


/**
 *  Has the CnCNet5 system been activated?
 */
extern bool IsActive;

/**
 *  Is the tunnel system active (set when tunnel information has been provided)?
 */
extern bool IsTunnelActive;

/**
 *  CnCNet5 UDP Tunnel info.
 */
extern TunnelInfoStruct TunnelInfo;

};
