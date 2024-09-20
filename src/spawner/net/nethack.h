/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NETHACK.H
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
#pragma once

#include <winsock2.h>

struct ListAddress
{
    static ListAddress Array[8];

    u_short Port;
    u_long  Ip;
};

class NetHack
{
public:
    static bool PortHack;

    static
    int WINAPI SendTo(
        int sockfd,
        char* buf,
        size_t len,
        int flags,
        sockaddr_in* dest_addr,
        int addrlen
    );

    static
    int WINAPI RecvFrom(
        int sockfd,
        char* buf,
        size_t len,
        int flags,
        sockaddr_in* src_addr,
        int* addrlen
    );
};

class Tunnel
{
public:
    static u_short Id;
    static u_long  Ip;
    static u_short Port;

    static
    int WINAPI SendTo(
        int sockfd,
        char* buf,
        size_t len,
        int flags,
        sockaddr_in* dest_addr,
        int addrlen
    );

    static
    int WINAPI RecvFrom(
        int sockfd,
        char* buf,
        size_t len,
        int flags,
        sockaddr_in* src_addr,
        int* addrlen
    );
};
