/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Variation of the UDP Winsock interface for CnCNet5.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once

#include "tibsun_defines.h"
#include "wspudp.h"


struct TunnelAddress {
    unsigned long IP;
    unsigned short Port;
};


/**
 *  CnCNet5UDPInterfaceClass
 *
 *  This class is a variation of the UDP Winsock interface to be used for
 *  accessing the CnCNet5 tunnels. It should not be enabled unless the client
 *  front end has also been activated.
 */
class CnCNet5UDPInterfaceClass : public UDPInterfaceClass
{
public:
    CnCNet5UDPInterfaceClass(unsigned short id, unsigned long ip, unsigned short port, bool port_hack = false);
    virtual ~CnCNet5UDPInterfaceClass() override = default;

    virtual LRESULT Message_Handler(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam) override;

private:
    int Send_To(SOCKET s, const char* buf, int len, int flags, sockaddr_in* dest_addr, int addrlen);
    int Receive_From(SOCKET s, char* buf, int len, int flags, sockaddr_in* src_addr, int* addrlen);

public:
    TunnelAddress AddressList[MAX_PLAYERS];

    unsigned short TunnelID;
    unsigned long TunnelIP;
    unsigned short TunnelPort;

    bool PortHack;
};
