/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  CnCNet4 global values.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <winsock2.h>


namespace CnCNet4 {

extern bool IsEnabled;

extern char Host[256];
extern unsigned Port;
extern bool Peer2Peer;
extern bool IsDedicated;
extern bool UseUDP;

extern struct sockaddr_in Server;

}; // namespace CnCNet4


int __stdcall bind_intercept(SOCKET s, const struct sockaddr *name, int namelen);
int __stdcall closesocket_intercept(SOCKET s);
int __stdcall getsockname_intercept(SOCKET s, struct sockaddr *name, int *namelen);
int __stdcall getsockopt_intercept(SOCKET s, int level, int optname, char *optval, int *optlen);
int __stdcall recvfrom_intercept(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);
int __stdcall sendto_intercept(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen);
int __stdcall setsockopt_intercept(SOCKET s, int level, int optname, const char *optval, int optlen);
SOCKET __stdcall socket_intercept(int af, int type, int protocol);
