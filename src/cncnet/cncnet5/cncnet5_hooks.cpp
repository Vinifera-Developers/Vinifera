/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CNCNET5_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for implementing the CnCNet5 system.
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
 ******************************************************************************/
#include "cncnet5_hooks.h"
#include "cncnet5_globals.h"
#include "cncnet5_wspudp.h"
#include "wsproto.h"
#include "wspipx.h"
#include "wspudp.h"
#include "tibsun_globals.h"
#include "session.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "hooker.h"
#include "hooker_macros.h"


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
