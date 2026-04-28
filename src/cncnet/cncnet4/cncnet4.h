/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  CnCNet4 replacements for low level networking API.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <winsock2.h>


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
