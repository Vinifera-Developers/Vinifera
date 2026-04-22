/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  CnCNet4 global values.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "cncnet4_globals.h"

#include "cncnet4.h"


/**
 *  Is the CnCNet4 interface active?
 */
bool CnCNet4::IsEnabled = false;

/**
 *  The host name (Must be running a instance of the dedicated server).
 */
char CnCNet4::Host[256] = { "server.cncnet.org" };
unsigned CnCNet4::Port = 9001;

/**
 *  Clients connect to each other rather than the server?
 */
bool CnCNet4::Peer2Peer = false;

bool CnCNet4::IsDedicated = false;

/**
 *  Use the UDP interface instead of IPX?
 */
bool CnCNet4::UseUDP = true;

struct sockaddr_in CnCNet4::Server;
