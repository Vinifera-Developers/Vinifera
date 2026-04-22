/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Global values and types used for the CnCNet5 system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


namespace CnCNet5
{

typedef struct TunnelInfoStruct
{
    unsigned long ID;
    unsigned long IP;
    unsigned short Port;
    bool PortHack;

    bool Is_Valid() const { return !(ID == -1 || IP == -1 || Port == -1); }

} TunnelInfoStruct;


/**
 *  Has the CnCNet5 system been activated?
 */
extern bool IsActive;

/**
 *  Is the tunnel system active (set when tunnel information has been provided)?
 */
extern bool IsTunnelActive;

/**
 *  CnCNet5 UDP Tunnel info.
 */
extern TunnelInfoStruct TunnelInfo;

};
