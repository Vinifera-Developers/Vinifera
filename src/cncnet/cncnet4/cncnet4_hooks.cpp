/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET4_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the CnCNet4 system.
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
#include "cncnet4_hooks.h"
#include "cncnet4_globals.h"
#include "cncnet4.h"
#include "tibsun_globals.h"
#include "session.h"
#include "wspudp.h"
#include "wspipx.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


static int __stdcall bind_intercept(SOCKET s, const struct sockaddr *name, int namelen)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::bind(s, name, namelen);
    } else {
        return ::bind(s, name, namelen);
    }
}

static int __stdcall closesocket_intercept(SOCKET s)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::closesocket(s);
    } else {
        return ::closesocket(s);
    }
}

static int __stdcall getsockname_intercept(SOCKET s, struct sockaddr *name, int *namelen)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::getsockname(s, name, namelen);
    } else {
        return ::getsockname(s, name, namelen);
    }
}

static int __stdcall getsockopt_intercept(SOCKET s, int level, int optname, char *optval, int *optlen)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::getsockopt(s, level, optname, optval, optlen);
    } else {
        return ::getsockopt(s, level, optname, optval, optlen);
    }
}

static int __stdcall recvfrom_intercept(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::recvfrom(s, buf, len, flags, from, fromlen);
    } else {
        return ::recvfrom(s, buf, len, flags, from, fromlen);
    }
}

static int __stdcall sendto_intercept(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::sendto(s, buf, len, flags, to, tolen);
    } else {
        return ::sendto(s, buf, len, flags, to, tolen);
    }
}

static int __stdcall setsockopt_intercept(SOCKET s, int level, int optname, const char *optval, int optlen)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::setsockopt(s, level, optname, optval, optlen);
    } else {
        return ::setsockopt(s, level, optname, optval, optlen);
    }
}

static SOCKET __stdcall socket_intercept(int af, int type, int protocol)
{
    if (CnCNet4::IsEnabled) {
        return CnCNet4::socket(af, type, protocol);
    } else {
        return ::socket(af, type, protocol);
    }
}


/**
 *  #issue-504
 * 
 *  Create the CnCNet4 UDP interface or standard UDP interface depending
 *  on if the CnCNet4 system has been enabled.
 * 
 *  @author: CCHyper
 */
static void CnCNet_Create_PacketTransport()
{
    bool created = false;

    if (CnCNet4::IsEnabled && CnCNet4::UseUDP) {
        PacketTransport = new UDPInterfaceClass();
        if (PacketTransport) {
            DEBUG_INFO("UDP PacketTransport for CnCNet4.\n");
        }

        created = (PacketTransport != nullptr);
    }

    if (!created) {
        PacketTransport = new IPXInterfaceClass();
        if (PacketTransport) {
            DEBUG_INFO("IPX PacketTransport created.\n");
        }
    }

    if (!PacketTransport) {
        DEBUG_ERROR("Failed to create PacketTransport!\n");
    }
}


/**
 *  #issue-504
 * 
 *  This patch replaces the call to the IPXInterfaceClass constructor when
 *  setting up the PacketTransport for network multiplayer games with
 *  conditional code that creates the UDPInterfaceClass when enabled.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Select_Game_Network_Create_PacketTransport_Patch)
{
    Session.Type = GAME_IPX;
    Session.CommProtocol = 2; // COMM_PROTOCOL_MULTI_E_COMP

    if (!PacketTransport) {
        CnCNet_Create_PacketTransport();
    }

    JMP(0x004E2698);
}


/**
 *  Main function for patching the hooks.
 */
void CnCNet4_Hooks()
{
    Patch_Jump(0x006B4D54, &bind_intercept);
    Patch_Jump(0x006B4D4E, &closesocket_intercept);
    //Patch_Jump(0x, &getsockname_intercept);
    Patch_Jump(0x006B4D48, &getsockopt_intercept);
    Patch_Jump(0x006B4D66, &recvfrom_intercept);
    Patch_Jump(0x006B4D6C, &sendto_intercept);
    Patch_Jump(0x006B4D60, &setsockopt_intercept);
    Patch_Jump(0x006B4D5A, &socket_intercept);

    Patch_Jump(0x004E2656, &_Select_Game_Network_Create_PacketTransport_Patch);
}
