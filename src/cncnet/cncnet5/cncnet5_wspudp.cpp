/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET_WSPUDP.CPP
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
#include "cncnet5_wspudp.h"
#include "debughandler.h"


/**
 *  CnCNet5UDPInterfaceClass constructor.
 * 
 *  @author: CCHyper
 */
CnCNet5UDPInterfaceClass::CnCNet5UDPInterfaceClass(unsigned short id, unsigned long ip, unsigned short port, bool port_hack) :
    UDPInterfaceClass(),
    IsEnabled(false),
    AddressList(),
    TunnelID(id),
    TunnelIP(ip),
    TunnelPort(port),
    PortHack(port_hack)
{
}


/**
 *  Message handler function for UDP Winsock related messages.
 * 
 *  @note: (based on UDPInterfaceClass::Message_Handler).
 * 
 *  @author: CCHyper
 */
LRESULT CnCNet5UDPInterfaceClass::Message_Handler(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam)
{
    /**
     *  If the CnCNet interface has not been enabled, just use the standard UDP interface.
     */
    if (!IsEnabled) {
        return UDPInterfaceClass::Message_Handler(hWnd, uMsg, wParam, lParam);
    }

    struct sockaddr_in addr;
    int rc;
    int addr_len;
    WinsockBufferType *packet;

    /**
     *  We only handle UDP events.
     */
    if (uMsg != WM_UDPASYNCEVENT) return 1;

    /**
     *  Handle UDP packet events
     */
    switch (WSAGETSELECTEVENT(lParam)) {

        /**
         *  Read event. Winsock has data it would like to give us.
         */
        case FD_READ:
            /**
             *  Clear any outstanding errors on the socket.
             */
            rc = WSAGETSELECTERROR(lParam);
            if (rc != 0) {
                Clear_Socket_Error(Socket);
                return 0;
            }

            /**
             *  Call the CnCNet tunnel Receive_From function to get the outstanding packet.
             */
            addr_len = sizeof(addr);
            rc = CnCNet5UDPInterfaceClass::Receive_From(Socket, (char*)ReceiveBuffer, sizeof(ReceiveBuffer), 0, (PSOCKADDR_IN)&addr, &addr_len);
            if (rc == SOCKET_ERROR) {
                DEBUG_WARNING("CnCNet5: Send_To returned %d!\n", WSAGetLastError());
                Clear_Socket_Error(Socket);
                return 0;
            }

            /**
             *  (CnCNet) Now, we need to map addr ip/port to index by reversing the search!
             */
            for (int i = 0; i < std::size(AddressList); i++) {

                /**
                 *  Compare ip.
                 */
                if (addr.sin_addr.s_addr == AddressList[i].IP) {

                    /**
                     *  Compare port.
                     */
                    if (!PortHack && addr.sin_port != AddressList[i].Port) {
                        continue;
                    }

                    /**
                     *  Found it, set this index to source addr.
                     */
                    addr.sin_addr.s_addr = i + 1;
                    addr.sin_port = 0;
                    break;
                }
            }

            /**
             *  "rc" is the number of bytes received from Winsock.
             */
            if (rc != 0) {

                /**
                 *  Make sure this packet didn't come from us. If it did then throw it away.
                 */
                for (int i = 0; i < Local_Addresses_Count(); ++i) {
                    if (!std::memcmp(Get_Local_Address(i), &addr.sin_addr.s_addr, 4) ) return 0;
                }

                /**
                 *  Create a new buffer and store this packet in it.
                 */
                packet = Get_New_In_Buffer();
                packet->BufferLen = rc;
                std::memcpy(packet->PacketData.Buffer, ReceiveBuffer, rc);
                if (!Passes_CRC_Check(packet)) {
                    DEBUG_INFO("CnCNet5: Throwing away malformed packet!\n");
                    Delete_In_Buffer(packet);
                    return 0;
                }
                std::memset(packet->Address, 0, sizeof (packet->Address));
                std::memcpy(packet->Address+4, &addr.sin_addr.s_addr, 4);
                InBuffers.Add(packet);
            }
            return 0;


        /**
         *  Write event. We send ourselves this event when we have more data to send. This
         *  event will also occur automatically when a packet has finished being sent.
         */
        case FD_WRITE:
            /**
             *  Clear any outstanding erros on the socket.
             */
            rc = WSAGETSELECTERROR(lParam);
            if (rc != 0) {
                Clear_Socket_Error(Socket);
                return 0;
            }

            /**
             *  If there are no packets waiting to be sent then bail.
             */
            if (OutBuffers.Count() == 0) return 0;
            int packetnum = 0;

            /**
             *  Get a pointer to the packet.
             */
            packet = OutBuffers[packetnum];

            /**
             *  (CnCNet) pull index.
             */
            int i = addr.sin_addr.s_addr - 1;

            /**
             *  (CnCNet) validate index.
             */
            if (i >= std::size(AddressList) || i < 0) {
                return -1;
            }

            /**
             *  (CnCNet) Set up the address structure of the outgoing packet.
             */
            addr.sin_family = AF_INET;
            addr.sin_port = AddressList[i].Port;
            addr.sin_addr.s_addr = AddressList[i].IP;

            /**
             *  Send it.
             *  If we get a WSAWOULDBLOCK error it means that Winsock is unable to accept the packet
             *  at this time. In this case, we clear the socket error and just exit. Winsock will
             *  send us another WRITE message when it is ready to receive more data.
             */
            rc = CnCNet5UDPInterfaceClass::Send_To(Socket, (const char *)&packet->PacketData, packet->BufferLen, 0, (PSOCKADDR_IN)&addr, sizeof (addr));
            if (rc == SOCKET_ERROR){
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    Clear_Socket_Error(Socket);
                    return 0;
                }
            }

            /**
             *  Delete the sent packet.
             */
            OutBuffers.Delete(packetnum);
            Delete_Out_Buffer(packet);
            return 0;
    }

    return 0;
}


/**
 *  "sendto" for the CnCNet tunnel system.
 * 
 *  @author: CCHyper (based on implementation by Toni Spets).
 */
int CnCNet5UDPInterfaceClass::Send_To(SOCKET s, const char *buf, int len, int flags, sockaddr_in *dest_addr, int addrlen)
{
    char tempbuf[1024 + 4];
    unsigned short *buffrom = reinterpret_cast<unsigned short *>(&tempbuf[0]);
    unsigned short *bufto = reinterpret_cast<unsigned short *>(&tempbuf[2]);

    /**
     *  No processing if no tunnel.
     */
    if (TunnelPort == -1) {
        DEBUG_WARNING("CnCNet5: TunnelPort is invalid in Send_To!\n");
        return sendto(s, buf, len, flags, (const sockaddr *)dest_addr, addrlen);
    }

#ifndef NDEBUG
    //DEV_DEBUG_INFO("CnCNet5: sendto(s=%d, buf=%p, len=%d, flags=%08X, to=%p, addrlen=%d)\n", s, buf, len, flags, dest_addr, addrlen);
#endif

    /**
     *  Copy packet to our buffer.
     */
    std::memcpy(&tempbuf[4], buf, len);

    /**
     *  Pull dest port to header.
     */
    *buffrom = TunnelID;
    *bufto = dest_addr->sin_port;

    dest_addr->sin_port = TunnelPort;
    dest_addr->sin_addr.s_addr = TunnelIP;

    return sendto(s, tempbuf, len + 4, flags, (const sockaddr *)dest_addr, addrlen);
}


/**
 *  "recvfrom" for the CnCNet tunnel system.
 * 
 *  @author: CCHyper (based on implementation by Toni Spets).
 */
int CnCNet5UDPInterfaceClass::Receive_From(SOCKET s, char *buf, int len, int flags, sockaddr_in *src_addr, int *addrlen)
{
    char tempbuf[1024 + 4];
    unsigned short *buffrom = reinterpret_cast<unsigned short *>(&tempbuf[0]);
    unsigned short *bufto = reinterpret_cast<unsigned short *>(&tempbuf[2]);

    /**
     *  No processing if no tunnel.
     */
    if (TunnelPort == -1) {
        DEBUG_WARNING("CnCNet5: TunnelPort is invalid in Recieve_From!\n");
        return recvfrom(s, buf, len, flags, (sockaddr *)src_addr, addrlen);
    }

#ifndef NDEBUG
    //DEV_DEBUG_INFO("CnCNet5: recvfrom(s=%d, buf=%p, len=%d, flags=%08X, from=%p, addrlen=%p (%d))\n", s, buf, len, flags, src_addr, addrlen, *addrlen);
#endif

    /**
     *  Call recvfrom first to get the packet.
     */
    int ret = recvfrom(s, tempbuf, sizeof tempbuf, flags, (sockaddr *)src_addr, addrlen);

    /**
     *  No processing if returning error or less than 5 bytes of data.
     */
    if (ret < 5 || *bufto != TunnelID) {
        DEBUG_WARNING("CnCNet5: recvfrom returned invalid data!\n");
        return -1;
    }

    std::memcpy(buf, &tempbuf[4], ret - 4);

    src_addr->sin_port = *buffrom;
    src_addr->sin_addr.s_addr = 0;

    return ret - 4;
}
