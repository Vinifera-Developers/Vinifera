/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for conquer.cpp.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "conquerext_hooks.h"

#include "beacon.h"
#include "debughandler.h"
#include "desyncdialog.h"
#include "event.h"
#include "gamedlg.h"
#include "hooker.h"
#include "house.h"
#include "ipxmgr.h"
#include "loadoptions.h"
#include "mouse.h"
#include "movieskip.h"
#include "msgloop.h"
#include "netdlg.h"
#include "netdlg2.h"
#include "objectext.h"
#include "progressscreen.h"
#include "rules.h"
#include "sessionext.h"
#include "sounddlg.h"
#include "syringe.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "voc.h"

#include <chrono>

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
 *  Processes incoming global network packets. Extracted from _IPX_Call_Back so
 *  that the desync dialog's pump loop can share the same packet handling.
 *
 *  @author: tomsons26, ZivDero
 */
void Vinifera_Process_Incoming_Global_Packets()
{
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

                            /*
                            **  Capture the player's name before Destroy_Connection's
                            **  AI takeover overwrites the house's name with the
                            **  computer player name.
                            */
                            std::string name = (id >= 0 && id < Houses.Count() && Houses[id] != nullptr)
                                ? Houses[id]->IniName.c_str() : "";

                            Destroy_Connection(id, 0);
                            SessionExtension->Update_Master_After_Player_Removal();
                            DesyncDialog.Notify_Player_Left(id, name.c_str());
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

                        /*
                        **  Also echo the message into the desync dialog's chat box,
                        **  if it is open.
                        */
                        DesyncDialog.Notify_Chat(packet.Name, packet.Message.Buf);
                    }
                    break;
                }

                case NET_PROGRESS_REPORT: {
                    for (int i = 0; i < Session.Players.Count(); i++) {
                        if (Session.Players[i]->Address == Session.GAddress) {
                            DEBUG_INFO("Received progress message - {}% from {}\n", Session.GPacket.Progress.Percent, Session.Players[i]->Name);
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

                case EXT_NET_LOAD_GAME:
                    // Only accept this message from the game host.
                    // TODO IpxAddressClass comparison is somehow broken
                    // if (Session.HostAddress != Session.GAddress) {
                    //     const char* expected = Session.HostAddress.As_String();
                    //     const char* actual = Session.GAddress.As_String();
                    //     DEBUG_INFO("EXT_NET_LOAD_GAME received from someone else than the game host! Expected: {}, actual: {}\n", expected, actual);
                    //     break;
                    // }

                    // Only allow loading in spawner sessions.
                    if (!SessionExtension->IsSpawnerSession) {
                        DEBUG_INFO("EXT_NET_LOAD_GAME can only be executed in a spawner session!\n");
                        break;
                    }

                    // If we are already scheduled to load a game, bail.
                    if (PendingMultiplayerSaveLoadTime) {
                        DEBUG_INFO("Game loading already scheduled, ignoring EXT_NET_LOAD_GAME\n");
                        break;
                    }

                    DEBUG_INFO("EXT_NET_LOAD_GAME received from game host.\n");

                    /*
                    **  When the desync dialog is open it shows the countdown itself,
                    **  so suppress the in-game message in that case.
                    */
                    if (!DesyncDialog.Is_Active()) {
                        Session.Messages.Add_Message(nullptr, 0, "The game host wants to load a saved game. Loading in 5 seconds...", static_cast<ColorSchemeType>(4), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
                    }

                    {
                        int saveid = reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).SaveInfo.ID;
                        PendingMultiplayerSaveLoadSlot = saveid;
                        PendingMultiplayerSaveLoadTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
                    }
                    break;

                case EXT_NET_HOST_ANNOUNCE: {
                    int house_id = reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket).Heartbeat.HouseID;
                    DEBUG_INFO("EXT_NET_HOST_ANNOUNCE received: house {} is the game master.\n", house_id);
                    SessionExtension->Set_Master(house_id);
                    break;
                }

                case EXT_NET_DESYNC_HEARTBEAT: {
                    auto& packet = reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket);
                    DesyncDialog.Notify_Heartbeat(packet.Heartbeat.HouseID, packet.Heartbeat.IsHost != 0);
                    break;
                }

                case EXT_NET_DESYNC_CONTINUE:
                    DesyncDialog.Notify_Continue();
                    break;

                case EXT_NET_MOVIE_SKIP_VOTE:
                    MovieSkip::Receive_Vote(reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket), Session.GAddress);
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
 *  Replacement for IPX_Call_Back.
 *
 *  @author: tomsons26, ZivDero
 */
void _IPX_Call_Back()
{
    Windows_Message_Handler();

    Ipx.Service();

    Vinifera_Process_Incoming_Global_Packets();
}


/**
 *  Replacement for Special_Dialog.
 *
 *  @author: Rampastring, tomsons26, ZivDero
 */
void _Special_Dialog()
{
    static const int TXT_SURRENDER = 265;

    if (SpecialDialog != SDLG_NONE) {
        if (Session.Type == GAME_NORMAL || (!PlayerPtr->IsToLose && !PlayerPtr->IsToWin && !PlayerPtr->IsToDie) && (SpecialDialogFlag || PlayerPtr->IsDefeated)) {
            SpecialDialogFlag = true;
        }

        Pause_Scenario();


        while (SpecialDialog != SDLG_NONE) {
            switch (SpecialDialog) {
#if 0
				case SDLG_SPECIAL:
					Map.Help_Text(TXT_NONE);
					Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
					Special_Dialog();
					Map.Revert_Mouse_Shape();
					SpecialDialog = SDLG_NONE;
					break;
#endif

            case SDLG_OPTIONS:
                Game_Options_Dialog();
                if (SpecialDialog != SDLG_OPTIONS) {
                    break;
                }
                SpecialDialog = SDLG_NONE;
                break;

            case SDLG_SETTINGS:
                GameControlsClass().Dialog();
                if (SpecialDialog != SDLG_SETTINGS) {
                    break;
                }
                SpecialDialog = SDLG_OPTIONS;
                break;

            case SDLG_SOUND:
                SoundControlsClass().Dialog();
                SpecialDialog = SDLG_SETTINGS;
                break;

            case SDLG_KEYBOARD:
                Options.Hotkey_Dialog();
                SpecialDialog = SDLG_SETTINGS;
                break;

            case SDLG_ABORT:
                switch (Abort_Dialog()) {

                // cancel
                case 1:
                    Queue_Exit();
                    SpecialDialog = SDLG_NONE;
                    break;

                case 2:
                    break;

                // abort
                case 3:
                    if (Session.Type == GAME_NORMAL) {
                        PlayerRestarts = true;
                    } else {
                        OutList.Add(EventClass(PlayerPtr->HeapID, EVENT_DESTRUCT));
                        SpecialDialogFlag = false;
                    }
                    break;
                }
                SpecialDialog = SDLG_NONE;
                break;

            case SDLG_SURRENDER:
                if (!PlayerPtr->IsDefeated && !PlayerPtr->IsToWin && !PlayerPtr->IsToLose && !PlayerPtr->IsToDie && Surrender_Dialog(TXT_SURRENDER)) {
                    if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
                        PlayerPtr->Flag_To_Lose();
                    } else {
                        OutList.Add(EventClass(PlayerPtr->HeapID, EVENT_DESTRUCT));
                        SpecialDialogFlag = false;
                    }
                }
                SpecialDialog = SDLG_NONE;
                break;

            case SDLG_WOL_OPTIONS:
                DoFindPage();
                SpecialDialog = SDLG_NONE;
                break;

            case EXT_SDLG_LOAD:
                if (!LoadOptionsClass().Load_Dialog()) {
                    SpecialDialog = SDLG_OPTIONS;
                    break;
                } else {
                    // maybe we should return if Load_Dialog returns true? to avoid calling Resume_Scenario after loading a save
                    SpecialDialog = SDLG_NONE;
                    return;
                }

            default:
                break;
            }
        }

        Resume_Scenario();
    }
}


DEFINE_HOOK(0x004B6AC7, _Game_Options_Dialog_Proc_Load_Through_SpecialDialog_Patch, 0)
{
    GET(long*, retval, EBX);

    if (!Session.Singleplayer_Game() && !(SessionExtension->IsSpawnerSession && Session.Am_I_Master()))
    {
        Session.Messages.Add_Message(nullptr, 0, "Only the game host can initiate loading a game in multiplayer.", Fetch_Scheme_Index_By_Name("White"), TPF_USE_GRAD_PAL | TPF_FULLSHADOW | TPF_6PT_GRAD, static_cast<int>(Rule->MessageDelay * TICKS_PER_MINUTE / 2));
        SpecialDialog = SDLG_NONE;
    }
    else
    {
        SpecialDialog = (SpecialDialogType)EXT_SDLG_LOAD;
    }

    *retval = 1;       // Signal game options dialog to exit
    return 0x004B6956; // Abuse exit procedure of another case because it has just the code sequence we need
}


/**
 *  Main function for patching the hooks.
 */
void ConquerExtension_Hooks()
{
    Patch_Jump(0x00463180, &_Unselect_All);
    Patch_Jump(0x00462DC0, &_IPX_Call_Back);
    Patch_Jump(0x00462640, &_Special_Dialog);
}
