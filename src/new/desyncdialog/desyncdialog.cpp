/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Dialog shown to the players when a multiplayer game goes out of sync.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "desyncdialog.h"

#include "bsurface.h"
#include "conquerext_hooks.h"
#include "debughandler.h"
#include "dsurface.h"
#include "house.h"
#include "iomap.h"
#include "ipxmgr.h"
#include "loadoptions.h"
#include "netdlg.h"
#include "ownrdraw.h"
#include "resource.h"
#include "sessionext.h"
#include "spritecollection.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_defines.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include "windialog.h"

#include <algorithm>
#include <filesystem>
#include <windowsx.h>


DesyncDialogClass DesyncDialog;


namespace
{
    enum
    {
        HEARTBEAT_TIMER = 1,
        QUIT_ENABLE_TIMER = 2,
    };

    constexpr int HEARTBEAT_INTERVAL_MS = 1000;
    constexpr int HEARTBEAT_TIMEOUT_MS = 25000;
    constexpr int QUIT_ENABLE_DELAY_MS = 10000;
    constexpr int LOAD_COUNTDOWN_MS = 5000;
    constexpr int CHAT_BACKLOG_MAX = 50;

    constexpr char CHAT_EDIT_PLACEHOLDER[] = "Type here to chat...";

    /**
     *  Player list column positions, in pixels within the listbox client
     *  area (the same units the game's lobby player lists use).
     */
    constexpr int PLAYER_LIST_HOST_COL_X = 2;
    constexpr int PLAYER_LIST_NAME_COL_X = 20;
    constexpr int PLAYER_LIST_STATUS_COL_WIDTH = 56;

    /**
     *  The x-position of the status column. Computed from the listbox's
     *  actual size, because the dialog is not rescaled by the game and so
     *  its pixel size depends on the system's font metrics.
     */
    int Player_List_Status_Col_X(HWND list)
    {
        RECT rect {};
        GetClientRect(list, &rect);
        return rect.right - PLAYER_LIST_STATUS_COL_WIDTH;
    }
}


/**
 *  Shows the dialog and pumps it until a decision has been made.
 *
 *  Game logic is halted for the duration: this function does not return
 *  until the master has decided what to do (or we have decided to quit).
 *  The network is serviced the whole time, so chat, sign-offs and the
 *  master's decision still come through.
 *
 *  @author: ZivDero
 */
DesyncDialogOutcomeType DesyncDialogClass::Run()
{
    DEBUG_INFO("DesyncDialog: opening on frame {}.\n", Frame);

    TacticalActive = false;

    /**
     *  Suspending the session makes the game's dialog message handler
     *  service Call_Back() instead of recursively running the main loop.
     *  This is also what makes it safe to open the stock load dialog from
     *  inside our pump loop.
     */
    Session.Suspended++;

    Decision = 0;
    ContinueReceived = false;
    LoadCountdownActive = false;
    std::fill(std::begin(PlayerLeft), std::end(PlayerLeft), false);
    std::fill(std::begin(PlayerLeftName), std::end(PlayerLeftName), std::string());
    std::fill(std::begin(LastHeartbeatFrom), std::end(LastHeartbeatFrom), std::chrono::steady_clock::now());
    ChatBacklog.clear();

    Create_Dialog();

    DesyncDialogOutcomeType outcome;

    if (Window == nullptr) {

        /**
         *  If the dialog could not be created for whatever reason, fall back
         *  to the old behavior of continuing without the desynced players.
         */
        DEBUG_ERROR("DesyncDialog: failed to create the dialog!\n");
        outcome = DESYNC_OUTCOME_CONTINUE;

    } else while (true) {

        /**
         *  Service the network, the Windows message loop and incoming
         *  global packets (chat, sign-offs, heartbeats, load requests...).
         */
        Call_Back();

        Check_Heartbeat_Timeouts();

        /**
         *  Quitting is always allowed, even during the load countdown.
         */
        if (Decision == IDC_DESYNC_QUIT) {
            outcome = DESYNC_OUTCOME_QUIT;
            break;
        }

        /**
         *  A load may have been scheduled by the master (or by us, via the
         *  nested load dialog). Run the countdown and exit when it is over.
         */
        if (!LoadCountdownActive && PendingMultiplayerSaveLoadTime) {
            Start_Load_Countdown();
        }

        if (LoadCountdownActive) {

            Update_Countdown_Text();
            InvalidateRect(Window, nullptr, FALSE);

            if (std::chrono::steady_clock::now() >= *PendingMultiplayerSaveLoadTime) {
                outcome = DESYNC_OUTCOME_LOAD;
                break;
            }

        } else if (ContinueReceived || Decision == IDC_DESYNC_CONTINUE) {

            if (Decision == IDC_DESYNC_CONTINUE) {
                Send_Continue();
            }
            outcome = DESYNC_OUTCOME_CONTINUE;
            break;

        } else if (Decision == IDC_DESYNC_LOAD) {

            /**
             *  Open the stock load dialog so the master can pick a save.
             *  If they pick one, the multiplayer load hook broadcasts it
             *  to the other players and schedules the load for everyone.
             *
             *  Stop our heartbeat timer first: the load dialog runs its own
             *  message loop, and the multiplayer load can rebuild the player
             *  list, so we must not let the timer fire a heartbeat in the
             *  middle of that.
             */
            KillTimer(Window, HEARTBEAT_TIMER);
            EnableWindow(Window, FALSE);
            LoadOptionsClass().Load_Dialog();
            EnableWindow(Window, TRUE);
            SetFocus(GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST));
            SetTimer(Window, HEARTBEAT_TIMER, HEARTBEAT_INTERVAL_MS, nullptr);
        }

        Decision = 0;
        Sleep(10);
    }

    Destroy_Dialog();

    Session.Suspended--;
    TacticalActive = true;
    Map.Flag_To_Redraw(GS_REDRAW_ALL);

    DEBUG_INFO("DesyncDialog: closed with outcome {}.\n", static_cast<int>(outcome));
    return outcome;
}


/**
 *  Creates the dialog appropriate for the local player - the decision
 *  dialog for the game master, the wait dialog for everyone else.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Create_Dialog()
{
    IsHostDialog = Session.Am_I_Master();
    const int dialog_id = IsHostDialog ? IDD_DESYNC_HOST : IDD_DESYNC_WAIT;

    /**
     *  WSCreateDialog finds our template via the hooked Fetch_Resource, which
     *  falls back to the Vinifera DLL for resources the game's language DLL
     *  doesn't have, and registers the dialog with the message loop. It also
     *  runs the same Resize_Dialog pass as the stock in-game dialogs, keeping
     *  the dialog in the game's logical pixel coordinate system instead of
     *  letting monitor DPI decide its final size.
     */
    Window = WSCreateDialog(ProgramInstance, dialog_id, MainWindow, &Dialog_Proc, FALSE);
    if (Window == nullptr) {
        return;
    }

    /**
     *  The dialog's pixel size depends on the system's font metrics, not on
     *  the game's resolution, so at low resolutions it may not fit on the
     *  screen. If so, take the excess height out of the chat list and move
     *  everything below it up.
     */
    RECT dialog_rect;
    GetWindowRect(Window, &dialog_rect);

    RECT client_rect;
    GetClientRect(MainWindow, &client_rect);

    const int dialog_height = dialog_rect.bottom - dialog_rect.top;
    if (dialog_height > client_rect.bottom) {

        HWND chat = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
        RECT chat_rect;
        GetWindowRect(chat, &chat_rect);
        const int chat_height = chat_rect.bottom - chat_rect.top;

        /**
         *  Don't shrink the chat list to less than a third of its height.
         */
        const int delta = std::min<int>(dialog_height - client_rect.bottom, chat_height * 2 / 3);

        SetWindowPos(chat, nullptr, 0, 0, chat_rect.right - chat_rect.left, chat_height - delta, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        for (int id : { IDC_DESYNC_CHAT_EDIT, IDC_DESYNC_COUNTDOWN_TEXT, IDC_DESYNC_COUNTDOWN_BAR, IDC_DESYNC_LOAD, IDC_DESYNC_CONTINUE, IDC_DESYNC_QUIT }) {
            HWND control = GetDlgItem(Window, id);
            if (control != nullptr) {
                RECT rect;
                GetWindowRect(control, &rect);
                MapWindowPoints(HWND_DESKTOP, Window, reinterpret_cast<POINT*>(&rect), 1);
                SetWindowPos(control, nullptr, rect.left, rect.top - delta, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }

        SetWindowPos(Window, nullptr, 0, 0, dialog_rect.right - dialog_rect.left, dialog_height - delta, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    WinDialogClass::Center_Window(Window);

    /**
     *  Set up the player list columns: player name, host icon, status.
     *  The name column must be added first: the listbox automatically
     *  gives every new row a PRIMARY cell (which draws the row's string)
     *  in the first column that was added, and INVALID cells in the rest.
     *  This is also why the game's lobby adds its name column first.
     */
    HWND list = GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST);
    if (list != nullptr) {
        const int status_x = Player_List_Status_Col_X(list);
        SendMessage(list, OD_ADDCOLUMN, status_x - PLAYER_LIST_NAME_COL_X - 6, PLAYER_LIST_NAME_COL_X);
        SendMessage(list, OD_ADDCOLUMN, 0, PLAYER_LIST_HOST_COL_X);
        SendMessage(list, OD_ADDCOLUMN, 0, status_x);
    }
    Update_Player_List();

    if (IsHostDialog) {
        const bool can_load = SessionExtension->IsSpawnerSession && Any_Multiplayer_Save_Exists();
        EnableWindow(GetDlgItem(Window, IDC_DESYNC_LOAD), can_load && !LoadCountdownActive);
        EnableWindow(GetDlgItem(Window, IDC_DESYNC_CONTINUE), !LoadCountdownActive);
    } else {

        /**
         *  The Quit button starts out disabled (so the player doesn't
         *  instantly quit out of reflex) and is enabled after a delay.
         */
        SetTimer(Window, QUIT_ENABLE_TIMER, QUIT_ENABLE_DELAY_MS, nullptr);
    }

    SetTimer(Window, HEARTBEAT_TIMER, HEARTBEAT_INTERVAL_MS, nullptr);

    Refill_Chat_List();

    /**
     *  Show a hint in the chat edit box so the players can tell what it is.
     */
    HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
    if (edit != nullptr && GetFocus() != edit) {
        SetWindowText(edit, CHAT_EDIT_PLACEHOLDER);
        ChatPlaceholderActive = true;
    }

    if (LoadCountdownActive) {
        ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_TEXT), SW_SHOW);
        ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_BAR), SW_SHOW);
    }

    ShowWindow(Window, SW_SHOWNORMAL);
    UpdateWindow(Window);

    /**
     *  Focus the player list rather than the dialog itself: the dialog would
     *  pass focus on to its first tab stop, the chat edit box, dismissing
     *  the hint text in it right away.
     */
    SetForegroundWindow(Window);
    SetFocus(GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST));
}


/**
 *  Destroys the dialog.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Destroy_Dialog()
{
    if (Window != nullptr) {
        const int dialog_id = IsHostDialog ? IDD_DESYNC_HOST : IDD_DESYNC_WAIT;
        KillTimer(Window, HEARTBEAT_TIMER);
        KillTimer(Window, QUIT_ENABLE_TIMER);
        WSDestroyDialog(Window, dialog_id);
        Window = nullptr;
    }
}


/**
 *  If the game master has left and we have been promoted in their place,
 *  replace the wait dialog with the decision dialog.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Morph_To_Host_Dialog_If_Needed()
{
    if (!Is_Active() || IsHostDialog) {
        return;
    }

    /**
     *  If a load countdown is already running, the decision has been made;
     *  there is nothing left to decide.
     */
    if (LoadCountdownActive) {
        return;
    }

    if (!Session.Am_I_Master()) {
        return;
    }

    DEBUG_INFO("DesyncDialog: we are the new game master, switching to the decision dialog.\n");

    Destroy_Dialog();
    Create_Dialog();
}


/**
 *  Refills the player list with every player's name and status.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Update_Player_List()
{
    if (!Is_Active()) {
        return;
    }

    HWND list = GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST);
    if (list == nullptr) {
        return;
    }

    ListBox_ResetContent(list);

    const int status_x = Player_List_Status_Col_X(list);

    for (int i = 0; i < MAX_PLAYERS && i < Houses.Count(); i++) {

        HouseClass* house = Houses[i];
        if (house == nullptr) {
            continue;
        }

        /**
         *  Players that have left are kept in the list (marked "Quit") even
         *  though their house is no longer human after the AI took it over.
         */
        if (!house->IsHuman && !PlayerLeft[i]) {
            continue;
        }

        /**
         *  Use the name we captured when the player left, since the AI takeover
         *  has by now overwritten the house's name with the computer name.
         */
        const char* name = (PlayerLeft[i] && !PlayerLeftName[i].empty())
            ? PlayerLeftName[i].c_str() : house->IniName.c_str();

        const int row = ListBox_AddString(list, name);
        if (row < 0) {
            continue;
        }

        /**
         *  The game master gets the same host icon the lobby uses.
         */
        if (i == Session.MasterPlayerID) {
            OwnerDraw::CellData host_cell;
            host_cell.type = OwnerDraw::CellData::SURFACE;
            host_cell.string.Set("");
            host_cell.string2.Set("");
            host_cell.surf = SpriteCollection.Get_Image_Surface("wolhost.pcx");
            SendMessage(list, OD_SETCELL, MAKEWPARAM(PLAYER_LIST_HOST_COL_X, row), reinterpret_cast<LPARAM>(&host_cell));
        }

        const char* status;
        COLORREF color;
        if (PlayerLeft[i]) {
            status = "Quit";
            color = RGB(200, 0, 0);
        } else if (SessionExtension->Is_Out_of_Sync(i)) {
            status = "Desynced";
            color = RGB(200, 200, 0);
        } else {
            status = "OK";
            color = RGB(0, 200, 0);
        }

        OwnerDraw::CellData status_cell;
        status_cell.type = OwnerDraw::CellData::TEXT;
        status_cell.string.Set(status);
        status_cell.string2.Set("");
        status_cell.color = color;
        SendMessage(list, OD_SETCELL, MAKEWPARAM(status_x, row), reinterpret_cast<LPARAM>(&status_cell));
    }

    InvalidateRect(list, nullptr, FALSE);
}


/**
 *  Refills the chat list from the backlog after the dialog is (re-)created.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Refill_Chat_List()
{
    if (!Is_Active()) {
        return;
    }

    HWND list = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
    if (list == nullptr) {
        return;
    }

    ListBox_ResetContent(list);
    for (const auto& line : ChatBacklog) {
        ListBox_AddString(list, line.c_str());
    }
    ListBox_SetTopIndex(list, ListBox_GetCount(list) - 1);
}


/**
 *  Appends a line to the chat list (and the backlog).
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Append_Chat_Line(const char* line)
{
    ChatBacklog.emplace_back(line);
    if (ChatBacklog.size() > CHAT_BACKLOG_MAX) {
        ChatBacklog.erase(ChatBacklog.begin());
    }

    if (!Is_Active()) {
        return;
    }

    HWND list = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
    if (list == nullptr) {
        return;
    }

    ListBox_AddString(list, line);
    while (ListBox_GetCount(list) > CHAT_BACKLOG_MAX) {
        ListBox_DeleteString(list, 0);
    }
    ListBox_SetTopIndex(list, ListBox_GetCount(list) - 1);
}


/**
 *  Sends the message currently in the chat edit box to the other players.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Send_Chat()
{
    if (!Is_Active()) {
        return;
    }

    HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
    if (edit == nullptr) {
        return;
    }

    if (ChatPlaceholderActive) {
        return;
    }

    char buf[MAX_MESSAGE_LENGTH];
    GetWindowText(edit, buf, sizeof(buf));
    if (buf[0] == '\0') {
        return;
    }

    SetWindowText(edit, "");
    SetFocus(edit);

    Session.MessageAddress = IPXAddressClass(); // set to broadcast
    SessionExtension->IsChatToAllies = false;
    Vinifera_Send_Network_Chat(buf);
}


/**
 *  Manages the hint text in the chat edit box: the hint is shown while the
 *  box is empty and unfocused, and cleared when the player clicks into it.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::On_Chat_Edit_Focus(bool gained)
{
    if (!Is_Active()) {
        return;
    }

    HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
    if (edit == nullptr) {
        return;
    }

    if (gained && ChatPlaceholderActive) {
        SetWindowText(edit, "");
        ChatPlaceholderActive = false;
    } else if (!gained && GetWindowTextLength(edit) == 0) {
        SetWindowText(edit, CHAT_EDIT_PLACEHOLDER);
        ChatPlaceholderActive = true;
    }
}


/**
 *  Lets the other players know we are still alive while game logic is halted.
 *  This both keeps the connections (and any NAT mappings) warm and lets
 *  everyone detect players that have silently disappeared.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Send_Heartbeat()
{
    /**
     *  Guard against being called while the world is in an inconsistent state
     *  (e.g. a save load tearing down and rebuilding the player list under us).
     */
    if (PlayerPtr == nullptr || Session.Players.Count() == 0) {
        return;
    }

    ExtGlobalPacketType packet {};
    packet.Command = EXT_NET_DESYNC_HEARTBEAT;
    std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name));
    packet.Heartbeat.HouseID = static_cast<char>(PlayerPtr->HeapID);
    packet.Heartbeat.IsHost = Session.Am_I_Master();

    for (int i = 1; i < Session.Players.Count(); i++) {
        Ipx.Send_Global_Message(&packet, sizeof(packet), 0, &Session.Players[i]->Address);
    }
    Ipx.Service();
}


/**
 *  Broadcasts the master's decision to continue without the desynced players.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Send_Continue()
{
    DEBUG_INFO("DesyncDialog: broadcasting the decision to continue.\n");

    ExtGlobalPacketType packet {};
    packet.Command = EXT_NET_DESYNC_CONTINUE;
    std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name));

    for (int i = 1; i < Session.Players.Count(); i++) {
        Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[i]->Address);
        Ipx.Service();
    }
}


/**
 *  Drops players we have not heard from in a long while (e.g. their game has
 *  crashed without sending a sign-off). This keeps the dialog from waiting on
 *  a dead master forever, and removes dead players from the player list so a
 *  subsequent save load reconciles them to the AI cleanly.
 *
 *  Each client decides this on its own local timer, which is fine: this is the
 *  same model the engine itself uses for connection timeouts (Handle_Timeout
 *  kicks the most-behind player based on a local timer). And since game logic
 *  is halted while the dialog is open, the EVENT_REMOVEPLAYER queued by
 *  Destroy_Connection is scheduled against the same frozen frame on every
 *  client and executes deterministically once the game resumes.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Check_Heartbeat_Timeouts()
{
    const auto now = std::chrono::steady_clock::now();

    for (int i = Session.Players.Count() - 1; i >= 1; i--) {

        const int id = Session.Players[i]->Player.ID;
        if (id < 0 || id >= MAX_PLAYERS) {
            continue;
        }

        if (now - LastHeartbeatFrom[id] > std::chrono::milliseconds(HEARTBEAT_TIMEOUT_MS)) {
            DEBUG_INFO("DesyncDialog: no heartbeat from {} (house {}) for {} seconds, dropping them.\n", Session.Players[i]->Name, id, HEARTBEAT_TIMEOUT_MS / 1000);

            /**
             *  Capture the player's name before Destroy_Connection removes
             *  them from the player list and (possibly) turns their house
             *  over to the AI, which overwrites its name.
             */
            std::string name = Session.Players[i]->Name;

            /**
             *  A non-zero error makes Destroy_Connection remove the player
             *  via a queued EVENT_REMOVEPLAYER rather than an immediate AI
             *  takeover, and print a "connection lost" message.
             */
            Destroy_Connection(id, 1);
            SessionExtension->Update_Master_After_Player_Removal();
            Notify_Player_Left(id, name.c_str());
        }
    }
}


/**
 *  Starts the countdown to a scheduled multiplayer save load.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Start_Load_Countdown()
{
    DEBUG_INFO("DesyncDialog: starting the load countdown.\n");

    LoadCountdownActive = true;
    LastCountdownSecond = -1;

    if (!Is_Active()) {
        return;
    }

    Append_Chat_Line("Loading the saved game...");

    ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_TEXT), SW_SHOW);
    ShowWindow(GetDlgItem(Window, IDC_DESYNC_COUNTDOWN_BAR), SW_SHOW);
    Update_Countdown_Text();

    if (IsHostDialog) {
        EnableWindow(GetDlgItem(Window, IDC_DESYNC_LOAD), FALSE);
        EnableWindow(GetDlgItem(Window, IDC_DESYNC_CONTINUE), FALSE);
    }

    InvalidateRect(Window, nullptr, FALSE);
}


/**
 *  Updates the countdown label with the number of seconds remaining, but only
 *  when the whole-second value changes.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Update_Countdown_Text()
{
    if (!Is_Active() || !LoadCountdownActive || !PendingMultiplayerSaveLoadTime) {
        return;
    }

    using namespace std::chrono;
    int remaining_ms = static_cast<int>(duration_cast<milliseconds>(*PendingMultiplayerSaveLoadTime - steady_clock::now()).count());
    if (remaining_ms < 0) {
        remaining_ms = 0;
    }

    /**
     *  Round up so the label shows "5" for the first second, down to "1" for
     *  the last, and never a bare "0".
     */
    int seconds = (remaining_ms + 999) / 1000;
    if (seconds < 1) {
        seconds = 1;
    }

    if (seconds == LastCountdownSecond) {
        return;
    }
    LastCountdownSecond = seconds;

    char buf[64];
    std::snprintf(buf, std::size(buf), "Loading the saved game in %d second%s...", seconds, seconds == 1 ? "" : "s");
    SetDlgItemText(Window, IDC_DESYNC_COUNTDOWN_TEXT, buf);
}


/**
 *  Draws the load countdown progress bar, the same way the vanilla
 *  reconnection dialog draws its sync bars.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Draw_Countdown_Bar(HWND window)
{
    if (!LoadCountdownActive || !PendingMultiplayerSaveLoadTime) {
        return;
    }

    HWND bar = GetDlgItem(window, IDC_DESYNC_COUNTDOWN_BAR);
    if (bar == nullptr) {
        return;
    }

    /**
     *  Get the placeholder control's rectangle, relative to the client
     *  area of the game's window (which is what the game surfaces map to).
     */
    RECT winrect;
    GetWindowRect(bar, &winrect);

    RECT client {};
    GetClientRect(MainWindow, &client);
    ClientToScreen(MainWindow, reinterpret_cast<POINT*>(&client));

    Rect bar_rect;
    bar_rect.X = winrect.left - client.left;
    bar_rect.Y = winrect.top - client.top;
    bar_rect.Width = winrect.right - winrect.left;
    bar_rect.Height = winrect.bottom - winrect.top;

    using namespace std::chrono;
    int remaining = static_cast<int>(duration_cast<milliseconds>(*PendingMultiplayerSaveLoadTime - steady_clock::now()).count());
    remaining = std::clamp(remaining, 0, LOAD_COUNTDOWN_MS);

    /**
     *  The bar shrinks and goes from green to yellow to red as the
     *  countdown progresses.
     */
    unsigned color = DSurface::Build_Hicolor_Pixel(0, 200, 0);
    const int elapsed = LOAD_COUNTDOWN_MS - remaining;
    if (elapsed > LOAD_COUNTDOWN_MS * 2 / 5) {
        color = DSurface::Build_Hicolor_Pixel(200, 200, 0);
        if (elapsed > LOAD_COUNTDOWN_MS * 4 / 5) {
            color = DSurface::Build_Hicolor_Pixel(200, 0, 0);
        }
    }

    bar_rect.Width = std::max(6, bar_rect.Width * remaining / LOAD_COUNTDOWN_MS);

    AlternateSurface->Fill_Rect(AlternateSurface->Get_Rect(), bar_rect, color);
}


/**
 *  Appends a chat message to the dialog's chat list.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Notify_Chat(const char* name, const char* text)
{
    if (!Is_Active()) {
        return;
    }

    char buf[256];
    std::snprintf(buf, std::size(buf), "%s: %s", name, text);
    Append_Chat_Line(buf);
}


/**
 *  Called when a player has left the game (signed off or timed out)
 *  while the dialog was open.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Notify_Player_Left(int house_id, const char* name)
{
    if (!Is_Active()) {
        return;
    }

    if (house_id >= 0 && house_id < MAX_PLAYERS) {
        PlayerLeft[house_id] = true;
        if (name != nullptr && name[0] != '\0') {
            PlayerLeftName[house_id] = name;
        }
    }

    if (name != nullptr && name[0] != '\0') {
        char buf[128];
        std::snprintf(buf, std::size(buf), "%s has left the game.", name);
        Append_Chat_Line(buf);
    }

    Update_Player_List();

    /**
     *  If the master is the one who left, we may have been promoted.
     */
    Morph_To_Host_Dialog_If_Needed();
}


/**
 *  Called when the master has decided to continue the game without
 *  the desynced players.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Notify_Continue()
{
    if (!Is_Active()) {
        return;
    }

    DEBUG_INFO("DesyncDialog: the game master has chosen to continue.\n");
    ContinueReceived = true;
}


/**
 *  Called when a heartbeat has been received from another player.
 *
 *  @author: ZivDero
 */
void DesyncDialogClass::Notify_Heartbeat(int house_id, bool is_host)
{
    if (!Is_Active()) {
        return;
    }

    if (house_id < 0 || house_id >= MAX_PLAYERS) {
        return;
    }

    LastHeartbeatFrom[house_id] = std::chrono::steady_clock::now();

    /**
     *  Self-heal the master fields in case we missed the announcement,
     *  and move the host icon in the player list to the right row.
     */
    if (is_host && Session.MasterPlayerID == -1) {
        SessionExtension->Set_Master(house_id);
        Update_Player_List();
    }
}


/**
 *  Checks if any multiplayer save that is actually loadable in the current
 *  session exists, using the same validation as the load dialog.
 *
 *  @author: ZivDero
 */
bool DesyncDialogClass::Any_Multiplayer_Save_Exists()
{
    namespace fs = std::filesystem;

    try {
        for (const auto& entry : fs::directory_iterator(fs::path(Vinifera_SavedGamesDirectory))) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const std::string name = entry.path().filename().string();
            if (!name.starts_with("SAVEGAME_") || !name.ends_with(".NET")) {
                continue;
            }

            if (Vinifera_Is_Save_Loadable(entry.path().string())) {
                return true;
            }
        }
    } catch (const std::exception& e) {
        DEBUG_ERROR("DesyncDialog: failed to scan for multiplayer saves! Reason: {}\n", e.what());
    }

    return false;
}


/**
 *  The window procedure for both dialog variants, modeled after the vanilla
 *  reconnection dialog's Reconnect_Dialog_Proc.
 *
 *  @author: ZivDero
 */
BOOL CALLBACK DesyncDialogClass::Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {

        case WM_INITDIALOG:
            OwnerDraw::SubclassDlg(window, 0);
            break;

        case WM_DRAWITEM:
            OwnerDraw::DrawItem(reinterpret_cast<DRAWITEMSTRUCT*>(lparam));
            return TRUE;

        case WM_PAINT:
            OwnerDraw::DrawDialogBack(window);
            DesyncDialog.Draw_Countdown_Bar(window);
            ValidateRect(window, nullptr);
            break;

        case WM_MOVING:
            return On_WM_MOVING(window, wparam, lparam);

        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
            return reinterpret_cast<BOOL>(GetStockObject(BLACK_BRUSH));

        case WM_ERASEBKGND:
            return TRUE;

        case WM_TIMER:

            /**
             *  Heartbeats are sent from a timer rather than the pump loop,
             *  so that they keep flowing while the nested load dialog runs
             *  its own message loop.
             */
            if (wparam == HEARTBEAT_TIMER) {
                DesyncDialog.Send_Heartbeat();
            } else if (wparam == QUIT_ENABLE_TIMER) {
                EnableWindow(GetDlgItem(window, IDC_DESYNC_QUIT), TRUE);
                KillTimer(window, QUIT_ENABLE_TIMER);
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wparam)) {

                case IDC_DESYNC_LOAD:
                case IDC_DESYNC_CONTINUE:
                case IDC_DESYNC_QUIT:
                    DesyncDialog.Decision = LOWORD(wparam);
                    break;

                /**
                 *  Pressing Enter in the chat edit box arrives as IDOK,
                 *  since the dialog has no default push button.
                 */
                case IDOK:
                    DesyncDialog.Send_Chat();
                    break;

                case IDC_DESYNC_CHAT_EDIT:
                    if (HIWORD(wparam) == EN_SETFOCUS) {
                        DesyncDialog.On_Chat_Edit_Focus(true);
                    } else if (HIWORD(wparam) == EN_KILLFOCUS) {
                        DesyncDialog.On_Chat_Edit_Focus(false);
                    }
                    break;
            }
            break;
    }

    return FALSE;
}
