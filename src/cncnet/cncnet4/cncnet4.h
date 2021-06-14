/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET4.H
 *
 *  @author        CCHyper
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
#pragma once

#include <winsock2.h>
#include <windows.h>


namespace CnCNet4 {

bool __stdcall Init();
void __stdcall Shutdown();

int __stdcall bind(SOCKET s, const struct sockaddr *name, int namelen);
SOCKET __stdcall socket(int af, int type, int protocol);
int __stdcall recvfrom(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);
int __stdcall sendto(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen);
int __stdcall getsockopt(SOCKET s, int level, int optname, char *optval, int *optlen);
int __stdcall setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen);
int __stdcall closesocket(SOCKET s);
int __stdcall getsockname(SOCKET s, struct sockaddr *name, int *namelen);

}; // namespace CnCNet4
