/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CONQUEREXT_HOOKS.H
 *
 *  @author        ZivDero, tomsons26
 *
 *  @brief         Contains the hooks for conquer.cpp.
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
#include "conquerext_hooks.h"

#include "beacon.h"
#include "extension_globals.h"
#include "hooker.h"
#include "house.h"
#include "ipxmgr.h"
#include "mouse.h"
#include "msgloop.h"
#include "netdlg.h"
#include "netdlg2.h"
#include "objectext.h"
#include "progressscreen.h"
#include "rules.h"
#include "sessionext.h"
#include "voc.h"


/**
 *  Replacement for Unselect_All.
 *
 *  @author: tomsons26, ZivDero
 */
void _Unselect_All()
{
    while (CurrentObjects.Count()) {
        CurrentObjects[0]->Unselect();
    }

    BeaconManager.Unselect_All_Beacons();
}


/**
 *  Replacement for IPX_Call_Back.
 *
 *  @author: tomsons26, ZivDero
 */
void _IPX_Call_Back()
{
    Windows_Message_Handler();

    Ipx.Service();

    /*
    ** Read packets only if the game is "closed", so we don't steal global
    ** messages from the connection dialogs.
    */
    if (!Session.NetOpen) {
        while (Ipx.Get_Global_Message(&Session.GPacket, &Session.GPacketlen, &Session.GAddress, &Session.GProductID)) {

            if (Session.GProductID == IPXGlobalConnClass::COMMAND_AND_CONQUER2) {

                switch (Session.GPacket.Command) {

                case NET_PROPOSE_KICK: {
                    Store_Global_Packet_In_Vector(Session.GPacket, Session.GAddress);
                    break;
                }

                /*
                **  If this is another player signing off, remove the connection &
                **  mark that player's house as non-human, so the computer will take
                **  it over.
                */
                case NET_SIGN_OFF: {
                    for (int i = 0; i < Ipx.Num_Connections(); i++) {

                        int id = Ipx.Connection_ID(i);

                        if (Session.GAddress == *Ipx.Connection_Address(id)) {
                            Destroy_Connection(id, 0);
                        }
                    }
                    break;
                }

                /*
                **  Process a message from another user.
                */
                case NET_MESSAGE: {
                    bool msg_ok = false;
                    ExtGlobalPacketType& packet = reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket);

                    /*
                    ** If NetProtect is set, make sure this message came from within
                    ** this game.
                    */
                    if (!Session.NetProtect) {
                        msg_ok = true;
                    } else {
                        if (Session.GPacket.Message.NameCRC == Compute_Name_CRC(Session.GameName)) {
                            msg_ok = true;
                        } else {
                            msg_ok = false;
                        }
                    }

                    if (msg_ok) {
                        char name[32];
                        std::snprintf(name, std::size(name), "%s [%s]", packet.Name, packet.Message.Scope);
                        if (!Session.Messages.Concat_Message(name, packet.Message.Color, packet.Message.Buf, static_cast<int>(Rule->MessageDelay * TICKS_PER_MINUTE))) {
                            Session.Messages.Add_Message(name, packet.Message.Color, packet.Message.Buf, Session.Scheme_From_Color_ID(packet.Message.Color), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, static_cast<int>(Rule->MessageDelay * TICKS_PER_MINUTE));

                            Sound_Effect(Rule->IncomingMessage);
                        }

                        /*
                        **  Tell the map to do a partial update (just to force the messages
                        **  to redraw).
                        */
                        Map.Flag_To_Redraw(GS_REDRAW_ALL);

                        /*
                        **  Save this message in our last-message buffer
                        */
                        strcpy(Session.LastMessage, packet.Message.Buf);
                    }
                    break;
                }

                case NET_PROGRESS_REPORT: {
                    for (int i = 0; i < Session.Players.Count(); i++) {
                        if (Session.Players[i]->Address == Session.GAddress) {
                            DEBUG_INFO("Received progress message - %d%% from %s\n", Session.GPacket.Progress.Percent, Session.Players[i]->Name);
                            Progress.Set_Progress_Percent(i, Session.GPacket.Progress.Percent);
                            break;
                        }
                    }
                    break;
                }

                case NET_REQ_SCENARIO:
                    break;

                case NET_READY_TO_GO:
                    break;

                case EXT_NET_BEACON_PLACE:
                    BeaconManager.Place_Beacon(static_cast<HousesType>(reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).PlaceBeacon.House), reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).PlaceBeacon.Position, reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).PlaceBeacon.Number);
                    break;

                case EXT_NET_BEACON_DELETE:
                    BeaconManager.Delete_Beacon(static_cast<HousesType>(reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).DeleteBeacon.House), reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).DeleteBeacon.Number);
                    break;

                case EXT_NET_BEACON_TEXT:
                    BeaconManager.Set_Beacon_Text(reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).BeaconText.Text, static_cast<HousesType>(reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).BeaconText.House), reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).BeaconText.Number);
                    break;

                default: {
                    Process_Global_Packet(&Session.GPacket, &Session.GAddress);
                    break;
                }
                }
            }

            Windows_Message_Handler();
            Ipx.Service();
        }
    }
}


/**
 *  Main function for patching the hooks.
 */
void ConquerExtension_Hooks()
{
    Patch_Jump(0x00463180, &_Unselect_All);
    Patch_Jump(0x00462DC0, &_IPX_Call_Back);
}
