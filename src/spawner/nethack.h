/*
* Copyright (c) 2012, 2013, 2014 Toni Spets <toni.spets@iki.fi>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

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
