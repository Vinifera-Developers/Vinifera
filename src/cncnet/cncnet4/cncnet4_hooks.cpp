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
}
