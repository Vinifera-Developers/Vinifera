/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Dialog shown to the players when a multiplayer game goes out of sync.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "always.h"

#include "session.h"

#include <chrono>
#include <string>
#include <vector>


/**
 *  The decision the desync dialog was closed with.
 */
enum DesyncDialogOutcomeType
{
    DESYNC_OUTCOME_CONTINUE,    // Continue playing without the desynced players.
    DESYNC_OUTCOME_LOAD,        // A multiplayer save load has been scheduled; let it happen.
    DESYNC_OUTCOME_QUIT,        // The local player wants to exit the game.
};


/**
 *  Manages the modal "Synchronization Error" dialog that is shown when a
 *  multiplayer game goes out of sync. The game master gets a dialog with
 *  Load Game/Continue/Quit options; everyone else gets a dialog asking them
 *  to wait for the master's decision. Both variants have a chat box.
 *
 *  While the dialog is open, game logic is halted, but the network is
 *  serviced and connections are kept alive with periodic heartbeats.
 */
class DesyncDialogClass
{
public:
    DesyncDialogClass() = default;
    ~DesyncDialogClass() = default;

    /**
     *  Shows the dialog and pumps it until a decision has been made.
     */
    DesyncDialogOutcomeType Run();

    bool Is_Active() const { return Window != nullptr; }

    /**
     *  Notifications from the incoming global packet processor.
     *  All of these are no-ops while the dialog is not open.
     */
    void Notify_Chat(const char* name, const char* text);
    void Notify_Player_Left(int house_id, const char* name);
    void Notify_Continue();
    void Notify_Heartbeat(int house_id, bool is_host);

private:
    void Create_Dialog();
    void Destroy_Dialog();
    void Morph_To_Host_Dialog_If_Needed();
    void Update_Player_List();
    void Refill_Chat_List();
    void Append_Chat_Line(const char* line);
    void Send_Chat();
    void On_Chat_Edit_Focus(bool gained);
    void Send_Heartbeat();
    void Send_Continue();
    void Check_Heartbeat_Timeouts();
    void Start_Load_Countdown();
    void Update_Countdown_Text();
    void Draw_Countdown_Bar(HWND window);

    static bool Any_Multiplayer_Save_Exists();
    static BOOL CALLBACK Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

private:
    /**
     *  The dialog window, null while the dialog is not open.
     */
    HWND Window = nullptr;

    /**
     *  Is the currently open dialog the host (game master) variant?
     */
    bool IsHostDialog = false;

    /**
     *  Control ID of the button the player pressed, consumed by the pump loop.
     */
    int Decision = 0;

    /**
     *  Has the game master decided to continue without the desynced players?
     */
    bool ContinueReceived = false;

    /**
     *  Is the chat edit box currently showing its hint text?
     */
    bool ChatPlaceholderActive = false;

    /**
     *  Is the 5-second countdown to a scheduled multiplayer save load running?
     */
    bool LoadCountdownActive = false;

    /**
     *  The whole-second value last shown in the countdown text, so we only
     *  refresh the label when it actually changes.
     */
    int LastCountdownSecond = -1;

    /**
     *  Players that have left the game while the dialog was open, by house ID.
     */
    bool PlayerLeft[MAX_PLAYERS] = {};

    /**
     *  The names of the players that have left, captured before their houses
     *  were turned over to the AI (which overwrites the house name with the
     *  computer player name), so the dialog can keep showing the real name.
     */
    std::string PlayerLeftName[MAX_PLAYERS] = {};

    /**
     *  Heartbeat bookkeeping for detecting players that silently disappear.
     */
    std::chrono::steady_clock::time_point LastHeartbeatFrom[MAX_PLAYERS] = {};

    /**
     *  All chat lines shown so far, so the list can be refilled when the
     *  dialog is re-created (e.g. when a waiting player becomes the master).
     */
    std::vector<std::string> ChatBacklog;
};

extern DesyncDialogClass DesyncDialog;
