/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for implementing the CnCNet5 system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "cncnet5_hooks.h"

#include "cncnet5_globals.h"
#include "cncnet5_wspudp.h"
#include "debughandler.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "session.h"
#include "tibsun_globals.h"
#include "wspipx.h"
#include "wsproto.h"
#include "wspudp.h"


/**
 *  #issue-69
 *
 *  Create the CnCNet5 UDP interface or standard UDP interface depending
 *  on if the CnCNet5 system has been enabled.
 *
 *  @author: CCHyper
 */
static void Create_PacketTransport()
{
    if (CnCNet5::IsActive && CnCNet5::TunnelInfo.Is_Valid()) {
        PacketTransport = new CnCNet5UDPInterfaceClass(
                                CnCNet5::TunnelInfo.ID,
                                CnCNet5::TunnelInfo.IP,
                                CnCNet5::TunnelInfo.Port,
                                CnCNet5::TunnelInfo.PortHack);
        if (!PacketTransport) {
            DEBUG_ERROR("Failed to create PacketTransport for CnCNet5!\n");
        }

    } else {
        PacketTransport = new UDPInterfaceClass();
        if (!PacketTransport) {
            DEBUG_ERROR("Failed to create PacketTransport!\n");
        }
    }
}


/**
 *  #issue-69
 * 
 *  This patch replaces the call to the UDPInterfaceClass constructor when
 *  setting up the PacketTransport for network multiplayer games with
 *  conditional code that creates the CnCNet5 interface is enabled.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Select_Game_Create_PacketTransport_Patch)
{
    Create_PacketTransport();

    Session.CommProtocol = COMM_PROTOCOL_SINGLE_NO_COMP;

    _asm { mov eax, [0x0074C8D8] } // PacketProtocol
    _asm { mov eax, [eax] }

    JMP_REG(edx, 0x004E2436);
}


/**
 *  Main function for patching the hooks.
 */
void CnCNet5_Hooks()
{
    Patch_Jump(0x004E2406, &_Select_Game_Create_PacketTransport_Patch);
}
