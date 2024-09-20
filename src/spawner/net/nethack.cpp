/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NETHACK.CPP
 *
 *  @author        Toni Spets
 *
 *  @brief         NetHack.
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
 *  @note          Copyright (c) 2012, 2013, 2014 Toni Spets <toni.spets@iki.fi>
 *
 *                 Permission to use, copy, modify, and distribute this
 *                 software for any purpose with or without fee is hereby
 *                 granted, provided that the above copyright notice and
 *                 this permission notice appear in all copies.
 *
 ******************************************************************************/

#include "nethack.h"

#include <windows.h>
#include <cstdint>
#include <winsock2.h>
#pragma comment(lib, "wsock32.lib")

#include "spawner.h"

ListAddress ListAddress::Array[8] = {};

bool NetHack::PortHack = true;

u_short Tunnel::Id = 0;
u_long  Tunnel::Ip = 0;
u_short Tunnel::Port = 0;


int WINAPI NetHack::SendTo(
    int sockfd,
    char* buf,
    size_t len,
    int flags,
    sockaddr_in* dest_addr,
    int addrlen
)
{
    const int index = dest_addr->sin_addr.s_addr - 1;

    // validate index
    if (index >= 8 || index < 0)
        return -1;

    const auto player = ListAddress::Array[index];
    sockaddr_in tempDest;
    tempDest.sin_family = AF_INET;
    tempDest.sin_port = player.Port;
    tempDest.sin_addr.S_un.S_addr = player.Ip;

    return Tunnel::SendTo(sockfd, buf, len, flags, &tempDest, addrlen);
}

int WINAPI NetHack::RecvFrom(
    int sockfd,
    char* buf,
    size_t len,
    int flags,
    sockaddr_in* src_addr,
    int* addrlen
)
{
    // call recvfrom first to get the packet
    int ret = Tunnel::RecvFrom(sockfd, buf, len, flags, src_addr, addrlen);

    // bail out if error
    if (ret == -1)
        return ret;

    // now, we need to map src_addr ip/port to index by reversing the search!
    for (char i = 0; i < (char)std::size(ListAddress::Array); ++i)
    {
        const auto player = ListAddress::Array[i];
        // compare ip
        if (src_addr->sin_addr.S_un.S_addr != player.Ip)
            continue;

        // compare port
        if (!NetHack::PortHack && src_addr->sin_port != player.Port)
            continue;

        // found it, set this index to source addr
        src_addr->sin_addr.s_addr = i + 1;
        src_addr->sin_port = 0;
        break;
    }

    return ret;
}

int WINAPI Tunnel::SendTo(
    int sockfd,
    char* buf,
    size_t len,
    int flags,
    sockaddr_in* dest_addr,
    int addrlen
)
{
    char TempBuf[1024 + 4];
    uint16_t* BufFrom = reinterpret_cast<uint16_t*>(TempBuf);
    uint16_t* BufTo = reinterpret_cast<uint16_t*>(TempBuf + 2);

    // no processing if no tunnel
    if (!Tunnel::Port)
        return sendto(sockfd, buf, len, flags, (struct sockaddr*)dest_addr, addrlen);

    // copy packet to our buffer
    memcpy(TempBuf + 4, buf, len);

    // pull dest port to header
    *BufFrom = Tunnel::Id;
    *BufTo = dest_addr->sin_port;

    dest_addr->sin_port = Tunnel::Port;
    dest_addr->sin_addr.S_un.S_addr = Tunnel::Ip;

    return sendto(sockfd, TempBuf, len + 4, flags, (struct sockaddr*)dest_addr, addrlen);
}

int WINAPI Tunnel::RecvFrom(
    int sockfd,
    char* buf,
    size_t len,
    int flags,
    sockaddr_in* src_addr,
    int* addrlen
)
{
    char TempBuf[1024 + 4];
    uint16_t* BufFrom = reinterpret_cast<uint16_t*>(TempBuf);
    uint16_t* BufTo = reinterpret_cast<uint16_t*>(TempBuf + 2);

    // no processing if no tunnel
    if (!Tunnel::Port)
        return recvfrom(sockfd, buf, len, flags, (struct sockaddr*)src_addr, addrlen);

    // call recvfrom first to get the packet
    int ret = recvfrom(sockfd, TempBuf, sizeof TempBuf, flags, (struct sockaddr*)src_addr, addrlen);

    // no processing if returning error or less than 5 bytes of data
    if (ret < 5 || *BufTo != Tunnel::Id)
        return -1;

    memcpy(buf, TempBuf + 4, ret - 4);

    src_addr->sin_port = *BufFrom;
    src_addr->sin_addr.s_addr = 0;

    return ret - 4;
}
