/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET4.CPP
 *
 *  @author        CCHyper (Based on work by Toni Spets)
 *
 *  @brief         CnCNet4 replacements for low level networking API.
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
#include "cncnet4.h"
#include "cncnet4_net.h"
#include "cncnet4_globals.h"
#include "rawfile.h"
#include "ini.h"
#include "debughandler.h"
#include "asserthandler.h"

#include <windows.h>
#include <cstdio>
#include <cstdio>
#include <ctime>
#include <wsipx.h>


/**
 *  Initialises the CnCNet4 system.
 */
bool __stdcall CnCNet4::Init()
{
    RawFileClass file("SUN.INI");
    INIClass ini;

    /**
     *  Load the CnCNet4 settings.
     */
    ini.Load(file);
    if (ini.Is_Present("CnCNet4")) {
        CnCNet4::IsEnabled = ini.Get_Bool("CnCNet4", "Enabled", CnCNet4::IsEnabled);
        ini.Get_String("CnCNet4", "Host", CnCNet4::Host, sizeof(CnCNet4::Host));
        CnCNet4::Peer2Peer = ini.Get_Bool("CnCNet4", "P2P", CnCNet4::Peer2Peer);
        CnCNet4::UseUDP = ini.Get_Bool("CnCNet4", "UDP", CnCNet4::UseUDP);
        CnCNet4::Port = ini.Get_Int("CnCNet4", "Port", CnCNet4::Port);
    }

    if (!CnCNet4::IsEnabled) {

        /**
         *  Nothing went wrong, but the CnCNet4 interface is not enabled.
         */
        return true;
    }

    /**
     *  Check to make sure the CnCNet4 DLL is not present in the directory.
     */
    if (RawFileClass("wsock32.dll").Is_Available()) {
        MessageBox(nullptr, "File conflict detected!\nPlease remove WSOCK32.DLL from the game directory!", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }
    if (RawFileClass("cncnet.dll").Is_Available()) {
        MessageBox(nullptr, "File conflict detected!\nPlease remove CNCNET.DLL from the game directory!", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    int s = net_init();
    net_opt_reuse();

    DEBUG_INFO("CnCNet4: Initialising...\n");

    if (CnCNet4::Port < 1024 || CnCNet4::Port > 65534) {
        CnCNet4::Port = 9001;
    }

    DEBUG_INFO("CnCNet4: Broadcasting to \"%s:%d\".\n", CnCNet4::Host, CnCNet4::Port);

    CnCNet4::IsDedicated = true;

    if (!net_address(&CnCNet4::Server, CnCNet4::Host, CnCNet4::Port)) {
        return false;
    }

    if (CnCNet4::Peer2Peer) {

        /**
         *  Test P2P.
         */
        bool pass = false;

        fd_set rfds;
        struct timeval tv;
        struct sockaddr_in to;
        struct sockaddr_in from;
        int start = time(nullptr);

        net_write_int8(CMD_TESTP2P);
        net_write_int32(start);

        while (time(nullptr) < start + 5) {

            net_send_noflush(&to);

            FD_ZERO(&rfds);
            FD_SET(s, &rfds);
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            if (select(s + 1, &rfds, NULL, NULL, &tv) > -1) {
                if (FD_ISSET(s, &rfds)) {
                    net_recv(&from);

                    if (net_read_int8() == CMD_TESTP2P && net_read_int32() == start) {
                        pass = true;
                        break;
                    }
                }
            }
        }

        /**
         *  Enable P2P if passed.
         */
        if (pass) {
            DEBUG_INFO("CnCNet4: Peer-to-peer test passed.\n");
            DEBUG_INFO("CnCNet4: Peer-to-peer is enabled.\n");
            net_bind("0.0.0.0", 8054);

        } else {
            DEBUG_WARNING("CnCNet4: Peer-to-peer test failed!\n");
            //return false;
        }

    }

    return true;
}



/**
 *  Shutdown the CnCNet4 system.
 */
void __stdcall CnCNet4::Shutdown()
{
    net_free();
}


SOCKET __stdcall CnCNet4::socket(int af, int type, int protocol)
{
#ifndef NDEBUG
    DEV_DEBUG_INFO("CnCNet4: socket(af=%08X, type=%08X, protocol=%08X)\n", af, type, protocol);
#endif

    if (af == AF_IPX) {
        return net_socket;
    }

    return ::socket(af, type, protocol);
}


int __stdcall CnCNet4::bind(SOCKET s, const struct sockaddr *name, int namelen)
{
#ifndef NDEBUG
    DEV_DEBUG_INFO("CnCNet4: bind(s=%d, name=%p, namelen=%d)\n", s, name, namelen);
#endif

    if (s == net_socket) {
        return 0;
    }

    return ::bind(s, name, namelen);
}


int __stdcall CnCNet4::recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
#ifndef NDEBUG
    //DEV_DEBUG_INFO("CnCNet4: recvfrom(s=%d, buf=%p, len=%d, flags=%08X, from=%p, fromlen=%p (%d))\n", s, buf, len, flags, from, fromlen, *fromlen);
#endif

    if (s == net_socket) {
        int ret;
        struct sockaddr_in from_in;

        ret = net_recv(&from_in);

        if (ret > 0) {

            if (CnCNet4::IsDedicated) {

                if (from_in.sin_addr.s_addr == CnCNet4::Server.sin_addr.s_addr && from_in.sin_port == CnCNet4::Server.sin_port) {
                    uint8_t cmd = net_read_int8();

                    /**
                     *  Handle keepalive packets from server, very special case.
                     */
                    if (cmd == CMD_PING) {
                        net_write_int8(CMD_PING);
                        net_write_int32(net_read_int32());
                        net_send(&from_in);

                        /**
                         *  #FIXME: returning 0 means disconnected
                         */
                        return 0;
                    }

                    /**
                     *  P2p flag.
                     */
                    from_in.sin_zero[0] = cmd;

                    from_in.sin_addr.s_addr = net_read_int32();
                    from_in.sin_port = net_read_int16();

                } else if (CnCNet4::Peer2Peer) {
                    /**
                     *  P2p flag for direct packets.
                     */
                    from_in.sin_zero[0] = 1;

                } else {
                    /**
                     *  Discard p2p packets if not in p2p mode.
                     *  #FIXME: returning 0 means disconnected
                     */
                    return 0;
                }

                /**
                 *  Force p2p port.
                 */
                if (from_in.sin_zero[0]) {
                    from_in.sin_port = htons(8054);
                }

                in2ipx(&from_in, (struct sockaddr_ipx *)from);

            } else {
                in2ipx(&from_in, (struct sockaddr_ipx *)from);
            }

            ret = net_read_data((void *)buf, len);
        }

        return ret;
    }

    return ::recvfrom(s, buf, len, flags, from, fromlen);
}


int __stdcall CnCNet4::sendto(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
#ifndef NDEBUG
    //DEV_DEBUG_INFO("CnCNet4: sendto(s=%d, buf=%p, len=%d, flags=%08X, to=%p, tolen=%d)\n", s, buf, len, flags, to, tolen);
#endif

    if (to->sa_family == AF_IPX) {
        struct sockaddr_in to_in;

        if (CnCNet4::IsDedicated) {

            if (is_ipx_broadcast((struct sockaddr_ipx *)to)) {
                net_write_int8(CnCNet4::Peer2Peer ? 1 : 0);
                net_write_int32(0xFFFFFFFF);
                net_write_int16(0xFFFF);
                net_write_data((void *)buf, len);
                net_send(&CnCNet4::Server);

            } else {

                ipx2in((struct sockaddr_ipx *)to, &to_in);

                /**
                 *  Use p2p only if both clients are in p2p mode.
                 */
                if (to_in.sin_zero[0] && CnCNet4::Peer2Peer) {
                    net_write_data((void *)buf, len);
                    net_send(&to_in);

                } else {
                    net_write_int8(CnCNet4::Peer2Peer ? 1 : 0);
                    net_write_int32(to_in.sin_addr.s_addr);
                    net_write_int16(to_in.sin_port);
                    net_write_data((void *)buf, len);
                    net_send(&CnCNet4::Server);
                }
            }

            return len;
        }

        ipx2in((struct sockaddr_ipx *)to, &to_in);
        net_write_data((void *)buf, len);

        /**
         *  Check if it's a broadcast.
         */
        if (is_ipx_broadcast((struct sockaddr_ipx *)to)) {
            net_send(&CnCNet4::Server);
            return len;

        } else {
            return net_send(&to_in);
        }
    }

    return ::sendto(s, buf, len, flags, to, tolen);
}


int __stdcall CnCNet4::getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen)
{
#ifndef NDEBUG
    DEV_DEBUG_INFO("CnCNet4: getsockopt(s=%d, level=%08X, optname=%08X, optval=%p, optlen=%p (%d))\n", s, level, optname, optval, optlen, *optlen);
#endif

    if (level == 0x3E8) {
        *optval = 1;
        *optlen = 1;
        return 0;
    }

    if (level == 0xFFFF) {
        *optval = 1;
        *optlen = 1;
        return 0;
    }

    return ::getsockopt(s, level, optname, optval, optlen);
}


int __stdcall CnCNet4::setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen)
{
#ifndef NDEBUG
    DEV_DEBUG_INFO("CnCNet4: setsockopt(s=%d, level=%08X, optname=%08X, optval=%p, optlen=%d)\n", s, level, optname, optval, optlen);
#endif

    if (level == 0x3E8) {
        return 0;
    }
    if (level == 0xFFFF) {
        return 0;
    }

    return ::setsockopt(s, level, optname, optval, optlen);
}


int __stdcall CnCNet4::closesocket(SOCKET s)
{
#ifndef NDEBUG
    DEV_DEBUG_INFO("CnCNet4: closesocket(s=%d)\n", s);
#endif

    if (s == net_socket) {
        if (CnCNet4::IsDedicated) {
            net_write_int8(CMD_DISCONNECT);
            net_send(&CnCNet4::Server);
        }
        return 0;
    }

    return ::closesocket(s);
}


int __stdcall CnCNet4::getsockname(SOCKET s, struct sockaddr *name, int *namelen)
{
#ifndef NDEBUG
    DEV_DEBUG_INFO("CnCNet4: getsockname(s=%d, name=%p, namelen=%p (%d)\n", s, name, namelen, *namelen);
#endif

    if (s == net_socket) {
        struct sockaddr_in name_in;
        char hostname[256];
        struct hostent *he;

        gethostname(hostname, 256);
        he = ::gethostbyname(hostname);

        DEBUG_INFO("getsockname: local hostname: %s\n", hostname);

        if (he) {
            DEBUG_INFO("getsockname: local ip: %s\n", inet_ntoa(*(struct in_addr *)(he->h_addr_list[0])));
            name_in.sin_addr = *(struct in_addr *)(he->h_addr_list[0]);
            in2ipx(&name_in, (struct sockaddr_ipx *)name);
        }
    }

    return ::getsockname(s, name, namelen);;
}
