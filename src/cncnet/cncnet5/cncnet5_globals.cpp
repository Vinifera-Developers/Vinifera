/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Global values and types used for the CnCNet5 system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "cncnet5_globals.h"


/**
 *  Has the CnCNet5 system been activated?
 */
bool CnCNet5::IsActive = false;

/**
 *  Is the tunnel system active (set when tunnel information has been provided)?
 */
bool CnCNet5::IsTunnelActive = false;

/**
 *  CnCNet5 UDP Tunnel info.
 */
CnCNet5::TunnelInfoStruct CnCNet5::TunnelInfo { -1, -1, -1, false };
