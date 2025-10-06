/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CONQUEREXT_HOOKS.H
 *
 *  @author        ZivDero
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
#include "fetchres.h"
#include "hooker.h"
#include "house.h"
#include "ipxmgr.h"
#include "language.h"
#include "mouse.h"
#include "msgloop.h"
#include "netdlg.h"
#include "netdlg2.h"
#include "nullmgr.h"
#include "objectext.h"
#include "progressscreen.h"
#include "rules.h"
#include "voc.h"

void _Unselect_All()
{
    while (CurrentObjects.Count()) {
        CurrentObjects[0]->Unselect();
    }

    BeaconManager.Unselect_All_Beacons();
}


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
                        if (!Session.Messages.Concat_Message(Session.GPacket.Name, Session.GPacket.Message.Color, Session.GPacket.Message.Buf, static_cast<int>(Rule->MessageDelay * TICKS_PER_MINUTE))) {
                            Session.Messages.Add_Message(Session.GPacket.Name, Session.GPacket.Message.Color, Session.GPacket.Message.Buf, Session.Scheme_From_Color_ID(Session.GPacket.Message.Color), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, static_cast<int>(Rule->MessageDelay * TICKS_PER_MINUTE));

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
                        strcpy(Session.LastMessage, Session.GPacket.Message.Buf);
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


void _Message_Input(KeyNumType& input)
{
    char txt[80 + MAX_MESSAGE_LENGTH + 32];

    char char_input = WWKeyboard->To_ASCII(input);
    if ((char_input == '\r' || char_input == '\b') && !Session.Messages.Is_Edit()) {
        BeaconClass* beacon = BeaconManager.Find_Selected_Beacon(PlayerPtr->HeapID);
        if (beacon != nullptr) {
            strcpy(txt, "Beacon Message:");

            Session.Messages.Add_Edit(static_cast<ColorSchemeType>(Session.ColorIdx), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, txt, 0, 10000);
            Session.Messages.EnableOverflow = false;

            BeaconManager.Set_Beacon_Text("_", HOUSE_NONE, -1, false);

            Map.Flag_To_Redraw();
        }
    }

    /*
    **  Check keyboard input for a request to send a message.
    **  The 'to' argument for Add_Edit is prefixed to the message buffer; the
    **  message buffer is big enough for the 'to' field plus MAX_MESSAGE_LENGTH.
    **  To send the message, calling Get_Edit_Buf retrieves the buffer minus the
    **  'to' portion.  At the other end, the buffer allocated to display the
    **  message must be MAX_MESSAGE_LENGTH plus the size of "From: xxx (house)".
    */
    if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH && input >= KN_F1 && input < KN_F1 + Session.MaxPlayers && !Session.Messages.Is_Edit()) {
        txt[0] = '\0';

        /*
        **  For a serial game, send a message on F1 or F4; set 'txt' to the
        **  "Message:" string & add an editable message to the list.
        */
        if (Session.Type == GAME_NULL_MODEM || Session.Type == GAME_MODEM) {
            if (input == KN_F1 || input == KN_F1 + Session.MaxPlayers - 1) {

                strcpy(txt, Fetch_String(TXT_MESSAGE)); // "Message:"

                Session.Messages.Add_Edit(static_cast<ColorSchemeType>(Session.ColorIdx), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, txt, 0, -1);
                Session.Messages.EnableOverflow = true;

                Map.Flag_To_Redraw();
            }
        } else if ((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && !Session.Messages.Is_Edit()) {

            /*
            **  For a network game:
            **  F1-F7 = "To <name> (house):" (only allowed if we're not in ObiWan mode)
            **  F8 = "To All:"
            */
            if (input == KN_F1 + Session.MaxPlayers - 1) {

                Session.MessageAddress = IPXAddressClass(); // set to broadcast
                strcpy(txt, Fetch_String(TXT_TO_ALL));      // "To All:"

                Session.Messages.Add_Edit(static_cast<ColorSchemeType>(Session.ColorIdx), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, txt, 0, -1);
                Session.Messages.EnableOverflow = true;

                Map.Flag_To_Redraw();

            } else if (input - KN_F1 < Ipx.Num_Connections() && !Session.ObiWan) {

                int id = Ipx.Connection_ID(input - KN_F1);
                Session.MessageAddress = *Ipx.Connection_Address(id);
                wsprintf(txt, Fetch_String(TXT_TO), Ipx.Connection_Name(id));

                Session.Messages.Add_Edit(static_cast<ColorSchemeType>(Session.ColorIdx), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, txt, 0, -1);
                Session.Messages.EnableOverflow = true;

                Map.Flag_To_Redraw();
            }
        }
    }

    /*
    **  Process message-system input; send the message out if RETURN is hit.
    */
    KeyNumType copy_input = input;
    int rc = Session.Messages.Input(input);

    if ((rc == 1 || rc == 2) && Session.Type != GAME_NORMAL) {

        /*
        **  If a single character has been added to an edit buffer, update the display.
        */
        if (rc == 1) {
            Map.Flag_To_Redraw();
        }

        /*
        **  If backspace was hit, redraw the map.  If the edit message was removed,
        **  the map must be force-drawn, since it won't be able to compute the
        **  cells to redraw; otherwise, let the map compute the cells to redraw,
        **  by not force-drawing it, but just setting the IsToRedraw bit.
        */
        else if (rc == 2) {
            if (copy_input == KN_ESC) {
                Map.Flag_To_Redraw(GS_REDRAW_ALL);
            } else {
                Map.Flag_To_Redraw();
            }
        }

        if (copy_input == KN_ESC) {
            BeaconManager.Set_Beacon_Text(nullptr, HOUSE_NONE, -1, true);
        } else {
            std::string buffer;
            char const* edit = Session.Messages.Get_Edit_Buf();
            if (edit != nullptr) {
                buffer += edit;
            }
            buffer += '_';
            BeaconManager.Set_Beacon_Text(buffer.c_str(), HOUSE_NONE, -1, false);
        }
    }

    /*
    **  Send a message
    */
    if (rc == 3 || rc == 4 /*&& Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH*/) {

        BeaconClass* beacon = BeaconManager.Find_Selected_Beacon(PlayerPtr->HeapID);
        if (beacon != nullptr) {
            if (copy_input == KN_ESC) {
                BeaconManager.Set_Beacon_Text(nullptr, HOUSE_NONE, -1, true);
            } else {
                std::string buffer;
                char const* edit = Session.Messages.Get_Edit_Buf();
                if (edit != nullptr) {
                    buffer += edit;
                    buffer.pop_back(); // MessageListClass appends a space to the end
                }
                BeaconManager.Set_Beacon_Text(buffer.c_str(), HOUSE_NONE, -1, false);
            }
        }

        /*
        **  Serial game: fill in a SerialPacketType & send it.
        **  (Note: The size of the SerialPacketType.Command must be the same as
        **  the EventClass.Type!)
        */
        else if (Session.Type == GAME_NULL_MODEM || Session.Type == GAME_MODEM) {
            SerialPacketType* serial_packet = reinterpret_cast<SerialPacketType*>(NullModem.BuildBuf);

            serial_packet->Command = SERIAL_MESSAGE;
            strcpy(serial_packet->Name, Session.Players[0]->Name);
            serial_packet->ID = Session.ColorIdx;

            if (rc == 3) {
                strcpy(serial_packet->Message.Message, Session.Messages.Get_Edit_Buf());
            } else {
                strcpy(serial_packet->Message.Message, Session.Messages.Get_Overflow_Buf());
                Session.Messages.Clear_Overflow_Buf();
            }

            /*
            ** Send the message, and store this message in our LastMessage
            ** buffer; the computer may send us a version of it later.
            */
            NullModem.Send_Message(NullModem.BuildBuf, sizeof(SerialPacketType), 1);

            strcpy(Session.LastMessage, serial_packet->Message.Message);
        } else if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {

            /*
            **  Network game: fill in a GlobalPacketType & send it.
            */
            Session.GPacket.Command = NET_MESSAGE;
            strcpy(Session.GPacket.Name, Session.Players[0]->Name);
            Session.GPacket.Message.Color = Session.ColorIdx;
            Session.GPacket.Message.NameCRC = Compute_Name_CRC(Session.GameName);

            if (rc == 3) {
                strcpy(Session.GPacket.Message.Buf, Session.Messages.Get_Edit_Buf());
            } else {
                strcpy(Session.GPacket.Message.Buf, Session.Messages.Get_Overflow_Buf());
                Session.Messages.Clear_Overflow_Buf();
            }

            /*
            **  If 'F4' was hit, MessageAddress will be a broadcast address; send
            **  the message to every player we have a connection with.
            */
            if (Session.MessageAddress.Is_Broadcast()) {
                for (int i = 0; i < Ipx.Num_Connections(); i++) {
                    Ipx.Send_Global_Message(&Session.GPacket, sizeof(GlobalPacketType), 1, Ipx.Connection_Address(Ipx.Connection_ID(i)));
                    Ipx.Service();
                }
            } else {

                /*
                **  Otherwise, MessageAddress contains the exact address to send to.
                **  Send to that address only.
                */
                Ipx.Send_Global_Message(&Session.GPacket, sizeof(GlobalPacketType), 1, &Session.MessageAddress);
                Ipx.Service();
            }

            /*
            **  Store this message in our LastMessage buffer; the computer may send
            **  us a version of it later.
            */
            strcpy(Session.LastMessage, Session.GPacket.Message.Buf);
        }

        /*
        **  Tell the map to completely update itself, since a message is now missing.
        */
        Map.Flag_To_Redraw(GS_REDRAW_ALL);
    }
}

void ConquerExtension_Hooks()
{
    Patch_Jump(0x00463180, &_Unselect_All);
    Patch_Jump(0x00462DC0, &_IPX_Call_Back);
    Patch_Jump(0x005098D0, &_Message_Input);
}