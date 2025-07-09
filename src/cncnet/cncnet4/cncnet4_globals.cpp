/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET4_GLOBALS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         CnCNet4 global values.
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
#include "cncnet4_globals.h"
#include "cncnet4.h"


/**
 *  Is the CnCNet4 interface active?
 */
bool CnCNet4::IsEnabled = false;

/**
 *  The host name (Must be running a instance of the dedicated server).
 */
char CnCNet4::Host[256] = { "server.cncnet.org" };
unsigned CnCNet4::Port = 9001;

/**
 *  Clients connect to each other rather than the server?
 */
bool CnCNet4::Peer2Peer = false;

bool CnCNet4::IsDedicated = false;

/**
 *  Use the UDP interface instead of IPX?
 */
bool CnCNet4::UseUDP = true;

struct sockaddr_in CnCNet4::Server;
