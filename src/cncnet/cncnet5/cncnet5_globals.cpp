/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET_GLOBALS.CPP
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
#include "cncnet5_globals.h"


/**
 *  Has the CnCNet5 system been activated?
 */
bool CnCNet5::IsActive = false;

/**
 *  Is the tunnel system active (set when tunnel information has been provided)?
 */
bool CnCNet5::IsTunnelActive = false;

/**
 *  CnCNet5 UDP Tunnel info.
 */
CnCNet5::TunnelInfoStruct CnCNet5::TunnelInfo { -1, -1, -1, false };
