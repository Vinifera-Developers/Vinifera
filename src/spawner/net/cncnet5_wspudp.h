/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET_WSPUDP.H
 *
 *  @author        CCHyper
 *
 *  @brief         Variation of the UDP Winsock interface for CnCNet5.
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

#include "wspudp.h"
#include "tibsun_defines.h"


struct TunnelAddress
{
    unsigned long IP;
    unsigned short Port;
};


/**
 *  CnCNet5UDPInterfaceClass
 *  
 *  This class is a variation of the UDP Winsock interface to be used for
 *  accessing the CnCNet5 tunnels. It should not be enabled unless the client
 *  front end has also been activated.
 */
class CnCNet5UDPInterfaceClass : public UDPInterfaceClass
{
    public:
        CnCNet5UDPInterfaceClass(unsigned short id, unsigned long ip, unsigned short port, bool port_hack = false);
        virtual ~CnCNet5UDPInterfaceClass() override = default;

        virtual LRESULT Message_Handler(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam) override;

    private:
        int Send_To(SOCKET s, const char *buf, int len, int flags, sockaddr_in *dest_addr, int addrlen);
        int Receive_From(SOCKET s, char *buf, int len, int flags, sockaddr_in *src_addr, int *addrlen);

    public:
        TunnelAddress AddressList[MAX_PLAYERS];

        unsigned short TunnelID;
        unsigned long TunnelIP;
        unsigned short TunnelPort;

        bool PortHack;
};
