/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the intercepting Main_Loop().
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "mainloopext_hooks.h"

#include "addon.h"
#include "asserthandler.h"
#include "beacon.h"
#include "battleui.h"
#include "ccfile.h"
#include "ccini.h"
#include "command.h"
#include "conquerext_hooks.h"
#include "debughandler.h"
#include "desyncdialog.h"
#include "event.h"
#include "eventext.h"
#include "fatal.h"
#include "extension_globals.h"
#include "session.h"

#include "hooker.h"
#include "house.h"
#include "iomap.h"
#include "ipxmgr.h"
#include "msgbox.h"
#include "netdlg.h"
#include "nullmgr.h"
#include "optionsext.h"
#include "rules.h"
#include "rulesext.h"
#include "sdlsurface.h"
#include "sessionext.h"
#include "syringe.h"
#include "tactical.h"
#include "tacticalext.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "uicontrol.h"
#include "vinifera_globals.h"
#include "voxelinit.h"
#include "optionsext.h"
#include "saveload.h"
#include "spawner.h"
#include "textprint.h"


/**
 *  This patch handles options locally so opening the dialog does not wait
 *  for the event queue. It also stops EVENT_OPTIONS from being created when
 *  frame step mode is enabled, because no events are processed in that mode.
 *
 *  @author: CCHyper
 */
bool _Queue_Options()
{
    if (Vinifera_Developer_FrameStep) {
        return false;
    }

    if (PlayerPtr->IsToWin || PlayerPtr->IsToLose || PlayerPtr->IsToDie) {
        return false;
    }

    if (SpecialDialog != SDLG_NONE) {
        return false;
    }

    /**
     *  OPTIONS only sets the local SpecialDialog flag when it executes. Do it
     *  immediately instead of sending a no-op event through the event system.
     */
    EventClass(PlayerPtr->HeapID, EVENT_OPTIONS).Execute();
    return true;
}


static void Before_Main_Loop()
{
}


static void After_Main_Loop()
{
    /**
     *  Have we not yet sent our options? If not, do so now.
     */
    if (!Vinifera_PlayerOptionsSent) {
        OutList.Add(EventClassExt(PlayerPtr->HeapID, (EventType)EXT_EVENT_PLAYER_OPTIONS, OptionsExtension->IsPauseRepairs).As_Event());
        Vinifera_PlayerOptionsSent = true;
    }

    /**
     *  Has we been flagged to reload the rules data?
     */
    if (Vinifera_Developer_IsToReloadRules) {

        /**
         *  Reinitalise the Rule instance to the defaults.
         */
        Rule->~RulesClass();
        new (Rule) RulesClass();

        /**
         *  Clear the current ini databases.
         */
        ArtINI.Clear();
        RuleINI->Clear();
        FSRuleINI.Clear();

        /**
         *  Reload RULES.INI and FIRESTRM.INI.
         */
        {
            CCFileClass rulefile("RULES.INI");
            RuleINI->Load(rulefile, false);
            ASSERT_FATAL(rulefile.Is_Available());

            if (Addon_Installed(ADDON_FIRESTORM) && Addon_Enabled(ADDON_FIRESTORM)) {
                rulefile.Set_Name("FIRESTRM.INI");
                ASSERT_FATAL(rulefile.Is_Available());
                FSRuleINI.Load(rulefile, false);
            }
        }

        /**
         *  Reload ART.INI and ARTFS.INI.
         */
        {
            CCFileClass artfile("ART.INI");

            DEBUG_INFO("Loading ART.INI.\n");
            ArtINI.Load(artfile, false);
            ASSERT_FATAL(artfile.Is_Available());
            DEBUG_INFO("Finished loading ART.INI.\n");

            if (Addon_Installed(ADDON_FIRESTORM) && Addon_Enabled(ADDON_FIRESTORM)) {
                DEBUG_INFO("Loading ARTFS.INI.\n");
                artfile.Set_Name("ARTFS.INI");
                ASSERT_FATAL(artfile.Is_Available());
                ArtINI.Load(artfile, false);
                DEBUG_INFO("Finished loading ARTFS.INI.\n");
            }
        }

        /**
         *  Process rule INIs.
         */
        DEBUG_INFO("Calling Rule->Process(*RuleINI).\n");
        Rule->Process(*RuleINI);
        DEBUG_INFO("Finished Rule->Process(*RuleINI).\n");

        DEBUG_INFO("Calling Rule->Addition(FSRuleINI).\n");
        Rule->Addition(FSRuleINI);
        DEBUG_INFO("Finished Rule->Addition(FSRuleINI).\n");

        /**
         *  Process scenario rule overrides.
         */
        {
            CCFileClass scenfile(Scen->ScenarioName);
            ASSERT_FATAL(scenfile.Is_Available());

            CCINIClass scenini;

            scenini.Load(scenfile, false);

            DEBUG_INFO("Calling Rule->Addition() with scenario overrides.\n");
            Rule->Addition(scenini);
            DEBUG_INFO("Finished Rule->Addition() with scenario overrides.\n");
        }

        /**
         *  Finally, reload miscellaneous classes.
         */
        {
            DEBUG_INFO("Calling UIControls->Read_INI().\n");
            UIControls->Read_INI_File("UI.INI", true);
            UIControls->Read_INI_File("UIOVERRIDES.INI");
            BattleUI.Shift_Sidebar();
            Map.Flag_To_Redraw(GS_REDRAW_ALL);
            DEBUG_INFO("Finished UIControls->Read_INI().\n");
        }

        /**
         *  All done!
         */
        Vinifera_Developer_IsToReloadRules = false;
    }

    SessionExtension->Service_Autosave_After_Main_Loop();

    if (PendingMultiplayerSaveLoadTime && std::chrono::steady_clock::now() >= *PendingMultiplayerSaveLoadTime)
    {
        int slot = PendingMultiplayerSaveLoadSlot;
        PendingMultiplayerSaveLoadSlot = -1;
        PendingMultiplayerSaveLoadTime.reset();

        DEBUG_INFO("Loading multiplayer game while mid-session.\n");

        if (!SessionExtension->Load_Multiplayer_Save(slot)) {
            WWMessageBox().Process("Failed to load saved game! The game will now exit.", 0);
            Emergency_Exit(0);
            return;
        }
    }
}


/**
 *  Main loop for the frame step mode. This should only handle basic
 *  input, redraw the map and update the scroll position.
 * 
 *  @author: CCHyper
 */
static bool FrameStep_Main_Loop()
{
    //DEV_DEBUG_INFO("FrameStep_Main_Loop(enter)\n");

    if (GameActive) {

        Call_Back();

        /**
         *  Update the display, unless we're inside a dialog.
         */
        if (SpecialDialog == SDLG_NONE && GameInFocus) {

            Map.Flag_To_Redraw(2);

            KeyNumType input;
            int x;
            int y;

            Map.Input(input, x, y);
            if (input != KN_NONE) {
                Keyboard_Process(input);

                /**
                 *  Kludge to allow the options dialog to open.
                 */
                if (input == KN_ESC || input == KN_SPACE) {
                    SpecialDialog = SDLG_OPTIONS;
                }

            }

            Map.Render();
            TacticalMap->AI();
        }

    }

    Sleep(1);

    //DEV_DEBUG_INFO("FrameStep_Main_Loop(exit)\n");

    return !GameActive;
}


static bool Main_Loop_Intercept()
{
    bool ret = false;

    /**
     *  Frame step mode enabled but no frames to process, so just perform
     *  a basic redraw and update of the screen, no game logic.
     */
    if (Vinifera_Developer_FrameStep && !Vinifera_Developer_FrameStepCount) {

        ret = FrameStep_Main_Loop();

    /**
     *  This is basically the original main loop, but now encapsulated by
     *  the frame step logic to allow us to process the requested frames.
     */
    } else if ((Vinifera_Developer_FrameStep && Vinifera_Developer_FrameStepCount > 0)
           || (!Vinifera_Developer_FrameStep && !Vinifera_Developer_FrameStepCount)) {

        //DEV_DEBUG_INFO("Before Main_Loop()\n");

        Before_Main_Loop();

        /**
         *  The games main loop function.
         */
        ret = Main_Loop();

        After_Main_Loop();

        //DEV_DEBUG_INFO("After Main_Loop()\n");

        /**
         *  Decrement the frame step count.
         */
        if (Vinifera_Developer_FrameStep && Vinifera_Developer_FrameStepCount > 0) {
            --Vinifera_Developer_FrameStepCount;
        }

    }

#if false // demo day-night light cycle loop
    static float light_angle = 0;
    static CDTimerClass<SystemTimerClass> light_timer = 5;
    if (light_timer == 0) {
        light_timer = 5;
        light_angle += DEG_TO_RADF(3);
        if (light_angle >= DEG_TO_RADF(360)) light_angle -= DEG_TO_RADF(360);
        RulesClassExtension::Set_Voxel_Light_Angle(RuleExtension->VoxelLightAzimuth, light_angle, 6);
    }
#endif

    return ret;
}


void Process_Command_If_Allowed(CommandClass* command)
{
    if (!Scen->InputLock || CommandClass::From_Type(COMMAND_OPTIONS) == command) {
        command->Process();
    }
}

/**
 *  #issue-255
 *
 *  Keyboard processing patches.
 *  Technically not all are directly in the main loop,
 *  but all of these are similar to the main loop patch so we include them all here.
 */

/**
 *  #issue-255
 *
 *  Fixes the user being able to do keyboard input and as such, affect
 *  game state while input is locked.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00508E83, _Main_Loop_Check_Keyboard_Input_Allowed, 0)
{
    GET(CommandClass*, command, ECX);
    Process_Command_If_Allowed(command);
    return 0x00508EA8;
}

DEFINE_HOOK(0x0050945C, _Keyboard_Process_Check_Keyboard_Input_Allowed, 0)
{
    GET(CommandClass*, command, ECX);
    Process_Command_If_Allowed(command);
    return 0x00509461;
}

DEFINE_HOOK(0x00509632, _Sync_Delay_Check_Keyboard_Input_Allowed_Patch1, 0)
{
    GET(CommandClass*, command, EAX);
    Process_Command_If_Allowed(command);
    return 0x00509659;
}

DEFINE_HOOK(0x00509747, _Sync_Delay_Check_Keyboard_Input_Allowed_Patch2, 0)
{
    GET(CommandClass*, command, ECX);
    Process_Command_If_Allowed(command);
    return 0x0050976C;
}


/**
 *  Adds a new edit (message being typed).
 *
 *  @author: ZivDero
 */
static void New_Edit(char const* to, bool enable_overflow = true, int width = -1)
{
    /**
     *  Set the prefix (e.g. "From:").
     */
    char txt[80 + MAX_MESSAGE_LENGTH + 32];
    strcpy(txt, to);

    /**
     *  Create the edit.
     */
    Session.Messages.Add_Edit(static_cast<ColorSchemeType>(Session.ColorIdx), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, txt, 0, width);
    //Session.Messages.EnableOverflow = enable_overflow; // disabled because overflow is off in vanilla by default

    /**
     *  Flag the map to be redrawn so that the text shows up.
     */
    Map.Flag_To_Redraw();
}


/**
 *  Checks if we're beginning input.
 *
 *  @author: tomsons26, ZivDero
 */
static bool Begin_Message(KeyNumType input)
{
    /*
    **  Check if we've got an active message already.
    */
    if (Session.Messages.Is_Edit()) {
        return false;
    }

    /*
    **  Check if player is trying to type a beacon message.
    */
    if (input == OptionsExtension->KeyChatToAll1 || input == OptionsExtension->KeyChatToAll2 || input == OptionsExtension->KeyChatToAllies) {
        BeaconClass* beacon = BeaconManager.Find_Selected_Beacon(PlayerPtr->HeapID);
        if (beacon != nullptr) {
            New_Edit("Beacon Message: ", false, 10000);
            BeaconManager.Set_Beacon_Text("_", HOUSE_NONE, -1, false);
            TacticalMapExtension->IsEditingBeaconText = true;
            return true;
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
    if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
        if (input >= KN_F1 && input < KN_F7 || input == OptionsExtension->KeyChatToAll1 || input == OptionsExtension->KeyChatToAll2 || input == OptionsExtension->KeyChatToAllies) {

            /*
            **  Reset the chat to allies flag.
            */
            SessionExtension->IsChatToAllies = false;

            switch (Session.Type) {

                /*
                **  For a serial game, send a message on F1 or any of the chat keys; set "to" text to the
                **  "Message:" string & add an editable message to the list.
                */
            case GAME_NULL_MODEM:
            case GAME_MODEM:
                if (input == KN_F1 || input == OptionsExtension->KeyChatToAll1 || input == OptionsExtension->KeyChatToAll2 || input == OptionsExtension->KeyChatToAllies) {
                    New_Edit("Message: ");
                    return true;
                }
                break;

                /*
                **  For a network game:
                **  F1-F7 = "To <name> (house):" (only allowed if we're not in ObiWan mode)
                **  Backspace = "To Allies:" (only allowed if we're not in ObiWan mode)
                **  F8/Enter = "To All:"
                */
            case GAME_IPX:
            case GAME_INTERNET:
                if (input == OptionsExtension->KeyChatToAll1 || input == OptionsExtension->KeyChatToAll2) {
                    Session.MessageAddress = IPXAddressClass(); // set to broadcast
                    New_Edit("Send to all: ");
                    return true;
                }

                if (input == OptionsExtension->KeyChatToAllies && !Session.ObiWan) {
                    Session.MessageAddress = IPXAddressClass(); // set to broadcast
                    SessionExtension->IsChatToAllies = true;    // set to filter to allies only
                    New_Edit("Send to team: ");
                    return true;
                }

                if (input - KN_F1 < Ipx.Num_Connections() && !Session.ObiWan) {
                    char txt[80 + MAX_MESSAGE_LENGTH + 32];
                    int id = Ipx.Connection_ID(input - KN_F1);
                    Session.MessageAddress = *Ipx.Connection_Address(id);
                    std::strncpy(SessionExtension->MessageRecipientName, Ipx.Connection_Name(id), std::size(SessionExtension->MessageRecipientName));
                    std::sprintf(txt, "Send to %s: ", SessionExtension->MessageRecipientName);
                    New_Edit(txt);
                    return true;
                }
                break;

            default:
                break;
            }
        }
    }

    return false;
}


/**
 *  Replacement for Message_Input.
 *
 *  @author: tomsons26, ZivDero
 */
void _Message_Input(KeyNumType& input)
{
    /*
    **  Check if we should begin recording input right now.
    */
    if (Begin_Message(input)) {
        input = KN_NONE;
    }

    /*
    **  Process message-system input; send the message out if RETURN is hit.
    */
    KeyNumType copy_input = input;
    int rc = Session.Messages.Input(input);

    /*
    **  1 = caller should redraw the message list (no need to complete
    **      refresh, though)
    **  2 = caller should completely refresh the display.
    **  3 = caller should send the edit message.
    **      (sets 'input' to 0 if it processes it.)
    **  4 = caller should send the Overflow buffer
    */

    if (rc == 1 || rc == 2) {

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

        /*
        **  If we're typing a beacon message, save it into the beacon.
        **  If the player has hit the escape button, clear it instead.
        **  The fact that it's being typed is reflected by a blinking underscore.
        */
        if (TacticalMapExtension->IsEditingBeaconText) {
            BeaconManager.Input(copy_input, false);
        }
    }

    /*
    **  Send a message
    */
    if (rc == 3 || rc == 4) {

        /*
        **  If we're typing a beacon message, save it accordingly.
        */
        if (TacticalMapExtension->IsEditingBeaconText) {
            BeaconManager.Input(copy_input, true);
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

            /*
            **  Print the message for the sender player as well.
            */
            Session.Messages.Add_Message(serial_packet->Name, serial_packet->ID, serial_packet->Message.Message, static_cast<ColorSchemeType>(serial_packet->ID), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);

            /*
            **  Store this message in our LastMessage buffer; the computer may send
            **  us a version of it later.
            */
            strcpy(Session.LastMessage, serial_packet->Message.Message);
        } else if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {

            const char* text;
            if (rc == 3) {
                text = Session.Messages.Get_Edit_Buf();
            } else {
                text = Session.Messages.Get_Overflow_Buf();
            }

            Vinifera_Send_Network_Chat(text);

            if (rc == 4) {
                Session.Messages.Clear_Overflow_Buf();
            }
        }

        /*
        **  Tell the map to completely update itself, since a message is now missing.
        */
        Map.Flag_To_Redraw(GS_REDRAW_ALL);
    }
}


/**
 *  Sends a network chat message to the other players, using the current
 *  Session.MessageAddress and SessionExtension->IsChatToAllies settings,
 *  and echoes it into our own message list.
 *
 *  Extracted from _Message_Input so the desync dialog can reuse it.
 *
 *  @author: tomsons26, ZivDero
 */
void Vinifera_Send_Network_Chat(const char* text)
{
    /*
    **  Network game: fill in a GlobalPacketType & send it.
    */
    std::memset(&Session.GPacket, 0, sizeof(Session.GPacket));
    ExtGlobalPacketType& packet = reinterpret_cast<ExtGlobalPacketType&>(Session.GPacket);
    packet.Command = static_cast<ExtNetCommandType>(NET_MESSAGE);
    const char* sender_name = SessionExtension->ExtOptions.IsQuickMatch ? "Player" : Session.Players[0]->Name;
    std::snprintf(packet.Name, sizeof(packet.Name), "%s", sender_name);
    packet.Message.Color = Session.ColorIdx;
    packet.Message.NameCRC = Compute_Name_CRC(Session.GameName);

    /*
    **  Add a scope marker.
    */
    if (Session.MessageAddress.Is_Broadcast()) {
        if (SessionExtension->IsChatToAllies) {
            strcpy(packet.Message.Scope, "to team");
        } else {
            strcpy(packet.Message.Scope, "to all");
        }
    } else {
        std::snprintf(packet.Message.Scope, std::size(packet.Message.Scope), "to %s", SessionExtension->MessageRecipientName);
    }

    std::strncpy(packet.Message.Buf, text, std::size(packet.Message.Buf) - 1);
    packet.Message.Buf[std::size(packet.Message.Buf) - 1] = '\0';

    /*
    **  If the chat to all key was hit, MessageAddress will be a broadcast address; send
    **  the message to every player we have a connection with.
    */
    if (Session.MessageAddress.Is_Broadcast()) {
        for (int i = 0; i < Ipx.Num_Connections(); i++) {

            /*
            **  If this is a "to allies" message, check if the player is allied to the target house.
            */
            if (!SessionExtension->IsChatToAllies || PlayerPtr->Is_Ally(Session.Players[i + 1]->Player.ID)) {
                Ipx.Send_Global_Message(&Session.GPacket, sizeof(GlobalPacketType), 1, Ipx.Connection_Address(Ipx.Connection_ID(i)));
                Ipx.Service();
            }
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
    **  Print the message for the sender player as well.
    */
    char name[32];
    std::snprintf(name, std::size(name), "%s [%s]" , packet.Name, packet.Message.Scope);
    Session.Messages.Add_Message(name, packet.Message.Color, packet.Message.Buf, Session.Scheme_From_Color_ID(packet.Message.Color), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, static_cast<int>(Rule->MessageDelay * TICKS_PER_MINUTE));

    /*
    **  Echo our own message into the desync dialog's chat box, if it is open.
    */
    DesyncDialog.Notify_Chat(packet.Name, packet.Message.Buf);

    /*
    **  Tell the map to do an update.
    */
    Map.Flag_To_Redraw(GS_REDRAW_ALL);

    /*
    **  Store this message in our LastMessage buffer; the computer may send
    **  us a version of it later.
    */
    strcpy(Session.LastMessage, Session.GPacket.Message.Buf);
}


/**
 *  Main function for patching the hooks.
 */
void MainLoop_Hooks()
{
    Patch_Call(0x00462A8E, &Main_Loop_Intercept);
    Patch_Call(0x00462A9C, &Main_Loop_Intercept);
    Patch_Call(0x005A0B85, &Main_Loop_Intercept);
    Patch_Jump(0x005B10F0, &_Queue_Options);
    Patch_Jump(0x005098D0, &_Message_Input);
    Patch_Byte(0x00508A90 + 1, 0x1); // Patch Sleep(0xA) to Sleep(1) to prevent lag in multiplayer when game is not in focus
    Patch_Jump(0x00508DBD, 0x00508DCA); // Jump over GameInFocus check in main loop to render even when game is not in focus
                                        // to allow alt-tabbed players to see what is going on in multiplayer
}
