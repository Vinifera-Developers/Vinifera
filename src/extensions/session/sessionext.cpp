/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SessionClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sessionext.h"

#include "debughandler.h"
#include "house.h"
#include "ipxmgr.h"
#include "loadoptions.h"
#include "mouse.h"
#include "optionsext.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "textprint.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "txtlabel.h"
#include "vinifera_defines.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "wsproto.h"

#include <filesystem>
#include <format>

#define SAVING_GAME_MESSAGE_ID 0x564E4652

namespace
{
    void Print_Saving_Game_Message(bool is_autosave)
    {
        const int message_delay = Rule->MessageDelay * TICKS_PER_MINUTE;

        const char* text = is_autosave ? "Auto-saving..." : "Saving game...";

        Session.Messages.Add_Message(nullptr, SAVING_GAME_MESSAGE_ID, text, static_cast<ColorSchemeType>(4), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, message_delay);

        Map.Flag_To_Redraw(2);
        Map.Render();
    }

    void Print_Game_Saved_Message(bool is_autosave)
    {
        const char* text = is_autosave ? "Game auto-saved." : "Game saved.";

        TextLabelClass* label = Session.Messages.Get_Label(SAVING_GAME_MESSAGE_ID);
        if (label != nullptr) {

            // The message list has text label gadgets, and also an internal buffer for messages.
            // The gadget's text field is a direct pointer to the internal message buffer,
            // so for changing the message, it's enough for us to modify the text in the
            // message list's internal buffer.
            // The gadget's own text pointer should not be modified - message list message removal
            // functionality relies on the gadget's text pointer pointing somewhere to the internal buffer.
            for (int i = 0; i < MAX_NUM_MESSAGES; i++)
            {
                if (Session.Messages.MessageBuffers[i] == label->Text) {
                    strcpy(Session.Messages.MessageBuffers[i], text);
                }
            }
        }

        Map.Flag_To_Redraw(2);
        Map.Render();
    }

    void Print_Saving_Game_Failed_Message()
    {
        const int message_delay = Rule->MessageDelay * TICKS_PER_MINUTE;

        const char* text = "Saving game failed!";

        Session.Messages.Add_Message(nullptr, 0, text, Fetch_Scheme_Index_By_Name("Red"), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, message_delay);

        Map.Flag_To_Redraw(2);
        Map.Render();
    }
}


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const SessionClass *this_ptr) :
    GlobalExtensionClass(this_ptr)
{
    Init_Clear();
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::~SessionClassExtension()
{
}


/**
 *  Loads game options and autosave state from the stream.
 *
 *  @author: ZivDero
 */
HRESULT SessionClassExtension::Load(IStream *pStm)
{
    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr = pStm->Read(&ExtOptions, sizeof(ExtOptions), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Read(&AutoSave, sizeof(AutoSave), nullptr);
    return hr;
}


/**
 *  Saves game options and autosave state to the stream.
 *
 *  @author: ZivDero
 */
HRESULT SessionClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    static_cast<void>(fClearDirty);

    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr = pStm->Write(&ExtOptions, sizeof(ExtOptions), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Write(&AutoSave, sizeof(AutoSave), nullptr);
    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int SessionClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(ExtOptions.IsAutoDeployMCV);
    crc(ExtOptions.IsPrePlacedConYards);
    crc(ExtOptions.IsBuildOffAlly);
    crc(ExtOptions.MultiplayerAutoSaveInterval);
    crc(ExtOptions.IsQuickMatch);
    crc(ExtOptions.IsWriteStatistics);
    crc(ExtOptions.IsAutoSurrender);
    crc(ExtOptions.IsAttackNeutralUnits);
    crc(ExtOptions.IsCoachMode);
    crc(ExtOptions.IsContinueWithoutHumans);
    crc(ExtOptions.IsScrapMetal);
    crc(ExtOptions.IsAINamesByDifficulty);
    crc(ExtOptions.IsPlayMoviesInMultiplayer);
    crc(AutoSave.IsToSave);
    crc(AutoSave.NextAutoSaveFrame);
    crc(AutoSave.IsMultiplayerSaveSuppressed);
}


/**
 *  Resets all non-option session extension state for a new session.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Init_Clear()
{
    ExtOptions = ExtGameOptionsType();
    AutoSave = AutoSaveStateType();
    IsSpawnerSession = false;
    SpawnerInfo = SpawnerSessionInfoType();
    ProtocolZeroEnabled = false;
    ProtocolZeroMaxLatencyLevel = 0xFF;
    ConnTimeout = 0;

    for (auto& slot_info : SlotInfo) {
        slot_info = SpawnerSlotInfoType();
    }

    IsChatToAllies = false;
    std::memset(MessageRecipientName, '\0', sizeof(MessageRecipientName));

    Clear_Out_Of_Sync_Data();
}


/**
 *  Gets the autosave interval in frames for the current session, if any.
 *
 *  @author: ZivDero
 */
int SessionClassExtension::Get_Autosave_Interval() const
{
    if (Session.Singleplayer_Game() && OptionsExtension->AutoSaveCount > 0 && OptionsExtension->AutoSaveInterval > 0) {
        if (Session.Type == GAME_NORMAL || OptionsExtension->IsAutoSaveInSkirmish) {
            return OptionsExtension->AutoSaveInterval;
        }
    }

    if ((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && !AutoSave.IsMultiplayerSaveSuppressed && ExtOptions.MultiplayerAutoSaveInterval > 0) {
        return ExtOptions.MultiplayerAutoSaveInterval;
    }

    return 0;
}


/**
 *  Schedules the next periodic autosave based on the current frame and session state.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Schedule_Next_Autosave()
{
    const int interval = Get_Autosave_Interval();
    AutoSave.NextAutoSaveFrame = interval > 0 ? Frame + interval : -1;
}


/**
 *  Queues an autosave to run from the main-loop safe point.
 *
 *  @author: ZivDero, Rampastring
 */
void SessionClassExtension::Flag_To_Save(bool manual)
{
    AutoSave.IsToSave = true;

    if (manual && !Session.Singleplayer_Game()) {
        AutoSave.IsNextMultiplayerSaveManual = manual;
    }
}


/**
 *  Suppresses multiplayer autosaves for the remainder of the current session.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Disable_Multiplayer_Saves()
{
    AutoSave.IsMultiplayerSaveSuppressed = true;
    Schedule_Next_Autosave();
}


/**
 *  Sets the next campaign autosave slot using the internal 0-based slot numbering.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Set_Next_Campaign_Autosave_Slot(int slot)
{
    AutoSave.NextCampaignAutoSaveSlot = slot >= 0 ? slot : 0;
}


/**
 *  Sets the next skirmish autosave slot using the internal 0-based slot numbering.
 *
 *  @author: Rampastring
 */
void SessionClassExtension::Set_Next_Skirmish_Autosave_Slot(int slot)
{
    AutoSave.NextSkirmishAutoSaveSlot = slot >= 0 ? slot : 0;
}


/**
 *  Restores autosave state after loading a savegame.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Restore_Autosave_After_Load()
{
    AutoSave.IsToSave = false;

    if (Get_Autosave_Interval() <= 0) {
        AutoSave.NextAutoSaveFrame = -1;
        return;
    }

    Schedule_Next_Autosave();
}


/**
 *  Services autosaves from the post-main-loop safe point.
 *
 *  @author: ZivDero, Rampastring
 */
void SessionClassExtension::Service_Autosave_After_Main_Loop()
{
    if (Frame == AutoSave.NextAutoSaveFrame) {
        Flag_To_Save(false);
    }

    if (!AutoSave.IsToSave) {
        return;
    }

    if (Session.Singleplayer_Game() && OptionsExtension->AutoSaveCount > 0) {
        Print_Saving_Game_Message(true);
        AutoSave.IsToSave = false;
        Schedule_Next_Autosave();

        // Fetch the file name and description for this auto-save. Do this before incrementing the slot.
        std::string filename = Autosave_File_Name();
        std::string description = Autosave_Description();

        // Increment the auto-save slot. Do this before saving so the "next slot" is properly saved to the saved game
        // - so if the user loads slot #3, the next auto-save slot after loading will be slot #4.
        if (Session.Type == GAME_NORMAL) {
            AutoSave.NextCampaignAutoSaveSlot = (AutoSave.NextCampaignAutoSaveSlot + 1) % OptionsExtension->AutoSaveCount;
        } else {
            AutoSave.NextSkirmishAutoSaveSlot = (AutoSave.NextSkirmishAutoSaveSlot + 1) % OptionsExtension->AutoSaveCount;
        }

        // Actually save the game.
        bool success = Save_Game(filename.c_str(), description.c_str());

        if (success) {
            Print_Game_Saved_Message(true);
        } else {
            Print_Saving_Game_Failed_Message();
        }

        return;
    }
    
    if ((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && !AutoSave.IsMultiplayerSaveSuppressed && !PendingMultiplayerSaveLoadTime) {
        Print_Saving_Game_Message(!AutoSave.IsNextMultiplayerSaveManual);
        AutoSave.IsToSave = false;
        Schedule_Next_Autosave();
        Init_Multiplayer_Saves_For_Session();

        // Actually save the game.
        bool success = Save_Game(Autosave_File_Name().c_str(), Autosave_Description().c_str());

        if (success) {
            Print_Game_Saved_Message(!AutoSave.IsNextMultiplayerSaveManual);
        } else {
            Print_Saving_Game_Failed_Message();
        }

        AutoSave.IsNextMultiplayerSaveManual = false;
        return;
    }

    AutoSave.IsToSave = false;
    Schedule_Next_Autosave();
}


/**
 *  Returns the file name of a multiplayer save for a specific save slot.
 *
 *  @author: Rampastring
 */
std::string SessionClassExtension::Multiplayer_Save_File_Name_From_Index(int index)
{
    // SAVEGAME_000.NET
    return std::format("SAVEGAME_{:03}.NET", index);
}


/**
 *  Attempts to load a multiplayer game from the specified multiplayer save slot.
 *  Returns true if succeeded, false otherwise.
 *
 *  @author: Rampastring
 */
bool SessionClassExtension::Load_Multiplayer_Save(int slot)
{
    namespace fs = std::filesystem;

    std::string filename = Multiplayer_Save_File_Name_From_Index(slot);

    fs::path saved_games_directory(Vinifera_SavedGamesDirectory);
    fs::path fullpath = saved_games_directory / filename;

    if (!fs::exists(fullpath)) {
        DEBUG_ERROR("SessionClassExtension::Load_Multiplayer_Save: No save exists for slot {}!\n", slot);
        return false;
    }

    // Discard old networking data to prevent pre-load packets from interfering with state after load.
    PacketTransport->Discard_In_Buffers();
    PacketTransport->Discard_Out_Buffers();
    PacketTransport->Stop_Listening();

    bool result = LoadOptionsClass().Load_File(fullpath.string().c_str()); // using LoadOptionsClass().Load_File here gives us a "Mission is loading. Please wait..." box.
    if (!result) {
        DEBUG_ERROR("SessionClassExtension::Load_Multiplayer_Save: Load_Game failed!\n", slot);
        return false;
    }

    DEBUG_INFO("SessionClassExtension::Load_Multiplayer_Save: Save loaded. Restarting networking. Frame: {}\n", Frame);

    // Set the load game flag. It allows the event queue logic to re-perform the
    // post-scenario-load "handshake" and set initial networking delay properly.
    // The queue logic automatically clears the flag afterwards.
    Session.LoadGame = true;

    // If anyone was out of sync before we loaded the save, mark them as not out of sync anymore.
    Clear_Out_Of_Sync_Data();

    // Discard possible async leftovers.
    PacketTransport->Discard_In_Buffers();
    PacketTransport->Discard_Out_Buffers();
    PacketTransport->Start_Listening();

    for (int i = 0; i < Session.Players.Count(); i++)
    {
        DEBUG_INFO("Load_Multiplayer_Save: Deleting connection #{}\n", (int)Session.Players[i]->Player.ID);
        Ipx.Delete_Connection(Session.Players[i]->Player.ID);
    }

    if (!Reconcile_Players()) {
        DEBUG_ERROR("SessionClassExtension::Load_Multiplayer_Save: Reconcile_Players failed!\n", slot);
        return false;
    }

    if (!This()->Create_Connections()) {
        DEBUG_ERROR("SessionClassExtension::Load_Multiplayer_Save: Create_Connections failed!\n", slot);
        return false;
    }

    /**
     *  Re-announce the game master, since players may have dropped out
     *  during the reload, and the master fields are not saved.
     */
    if (Session.Am_I_Master()) {
        Announce_Master();
    } else {
        Update_Master_After_Player_Removal();
    }

    return true;
}


/**
 *  One-time initialization of multiplayer saved games for the current session.
 *  Clears existing multiplayer saves and copies spawn.ini
 *  to the saved games directory so it is available to the client.
 *
 *  @author: Rampastring
 */
void SessionClassExtension::Init_Multiplayer_Saves_For_Session()
{
    if (MultiplayerSavesInitializedForThisSession) {
        return;
    }

    // Delete any potential saves from previous multiplayer sessions.
    namespace fs = std::filesystem;

    fs::path saved_games_directory(Vinifera_SavedGamesDirectory);

    try {
        for (int i = 0; i < 1000; ++i) {

            std::string filename = Multiplayer_Save_File_Name_From_Index(i);

            fs::path fullpath = saved_games_directory / filename;

            std::error_code ec;
            fs::remove(fullpath, ec);

            // Optional:
            // log ec if desired
        }

        // Copy spawn.ini from main game directory and place it as spawnSG.ini to the saved games subdirectory.
        // It is read by the CnCNet Client when loading saved multiplayer games.
        fs::path spawn_ini = "spawn.ini";

        if (fs::exists(spawn_ini)) {
            fs::path spawn_sg_ini = saved_games_directory / fs::path("spawnSG.ini");
            fs::copy_file(spawn_ini, spawn_sg_ini, fs::copy_options::overwrite_existing);
        }
    } catch (const std::exception& e) {
        DEBUG_ERROR("Failed to copy spawn.ini and clear previous multiplayer saves! Reason: {}\n", e.what());
    }

    MultiplayerSavesInitializedForThisSession = true;
}


/**
 *  Returns the file name of the next multiplayer save file.
 *
 *  @author: Rampastring
 */
std::string SessionClassExtension::Multiplayer_Save_File_Name() const
{
    namespace fs = std::filesystem;

    fs::path saved_games_directory(Vinifera_SavedGamesDirectory);

    for (int i = 0; i < 1000; ++i) {

        std::string filename = Multiplayer_Save_File_Name_From_Index(i);

        fs::path fullpath = saved_games_directory / filename;

        if (!fs::exists(fullpath)) {
            return filename;
        }
    }

    return Multiplayer_Save_File_Name_From_Index(999);
}


std::string SessionClassExtension::Autosave_File_Name() const
{
    switch (Session.Type) {
    case GAME_IPX:
    case GAME_INTERNET:
        return Multiplayer_Save_File_Name();
    case GAME_NORMAL:
        return std::format("AUTOSAVE{}.SAV", AutoSave.NextCampaignAutoSaveSlot + 1);
    case GAME_SKIRMISH:
        return std::format("SKIRMISH_AUTOSAVE{}.SAV", AutoSave.NextSkirmishAutoSaveSlot + 1);
    default:
        return "";
    }
}


std::string SessionClassExtension::Autosave_Description() const
{
    switch (Session.Type) {
    case GAME_IPX:
    case GAME_INTERNET:
        return AutoSave.IsNextMultiplayerSaveManual ? "Multiplayer Game (Manual Save)" : "Multiplayer Game (Auto-Save)";
    case GAME_NORMAL:
        return std::format("Mission Auto-Save (Slot {})", AutoSave.NextCampaignAutoSaveSlot + 1);
    case GAME_SKIRMISH:
        return std::format("Skirmish Auto-Save (Slot {})", AutoSave.NextSkirmishAutoSaveSlot + 1);
    default:
        return "";
    }
}


/**
 *  Reconciles loaded data with the "Players" vector.
 *
 *  This function is for supporting loading a saved multiplayer game.
 *  When the game is loaded, we have to figure out which house goes with
 *  which entry in the Players vector. We also have to figure out if
 *  everyone who was originally in the game is still with us, and if not,
 *  turn their stuff over to the computer.
 */
bool SessionClassExtension::Reconcile_Players()
{
    int i;
    bool found;
    int house;
    HouseClass* housep;

    /**
     *  If there are no players, there's nothing to do.
     */
    if (This()->Players.Count() == 0) return true;

    /**
     *  Make sure every name we're connected to can be found in a House.
     */
    for (i = 0; i < This()->Players.Count(); i++) {
        found = false;
        for (house = 0; house < This()->Players.Count(); house++) {
            housep = Houses[house];
            if (!housep) {
                continue;
            }

            if (!stricmp(This()->Players[i]->Name, housep->IniName.c_str())) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    /**
     *  Loop through all Houses; if we find a human-owned house that we're
     *  not connected to, turn it over to the computer.
     */
    for (house = 0; house < This()->Players.Count(); house++) {
        housep = Houses[house];
        if (!housep) {
            continue;
        }

        /**
         *  Skip this house if it wasn't human to start with.
         */
        if (!housep->IsHuman) {
            continue;
        }

        /**
         *  Try to find this name in the Players vector; if it's found, set
         *  its ID to this house.
         */
        found = false;
        for (i = 0; i < This()->Players.Count(); i++) {
            if (!stricmp(This()->Players[i]->Name, housep->IniName.c_str())) {
                found = true;
                This()->Players[i]->Player.ID = static_cast<HousesType>(house);
                break;
            }
        }

        /**
         *  If this name wasn't found, remove it
         */
        if (!found) {

            /**
             *  Turn the player's house over to the computer's AI
             */
            housep->IsHuman = false;
            housep->IsStarted = true;
            housep->IQ = Rule->MaxIQ;
            housep->IniName = std::string(housep->IniName) + " (AI)";

            Session.NumPlayers--;
        }
    }

    /**
     *  If all went well, our Session.NumPlayers value should now equal the value
     *  from the saved game, minus any players we removed.
     */
    if (This()->NumPlayers == This()->Players.Count()) {
        return true;
    } else {
        return false;
    }
}


bool SessionClassExtension::Is_Out_of_Sync(int id)
{
    ASSERT_FATAL(id > -1 && id < MAX_PLAYERS);
    return IsOutOfSync[id];
}


void SessionClassExtension::Clear_Out_Of_Sync_Data()
{
    std::fill(std::begin(IsOutOfSync), std::end(IsOutOfSync), false);
    OutOfSyncFrame = -1;
}


void SessionClassExtension::Mark_Player_As_Out_of_Sync(int id)
{
    ASSERT_FATAL(id > -1 && id < MAX_PLAYERS);
    IsOutOfSync[id] = true;
    if (OutOfSyncFrame < 0) OutOfSyncFrame = Frame;
}


/**
 *  Records the given house as the game's master (host) in the vanilla
 *  MasterPlayerID/MasterPlayerName fields, which our replacement of
 *  SessionClass::Am_I_Master reads.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Set_Master(int house_id)
{
    if (house_id < 0 || house_id >= Houses.Count() || Houses[house_id] == nullptr) {
        DEBUG_ERROR("Set_Master: invalid house ID {}!\n", house_id);
        return;
    }

    Session.MasterPlayerID = house_id;
    std::strncpy(Session.MasterPlayerName, Houses[house_id]->IniName.c_str(), std::size(Session.MasterPlayerName) - 1);
    Session.MasterPlayerName[std::size(Session.MasterPlayerName) - 1] = '\0';

    DEBUG_INFO("Set_Master: game master is now {} (house {})\n", Session.MasterPlayerName, house_id);
}


/**
 *  Called on the game host to record itself as the game's master and
 *  let the other players know about it.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Announce_Master()
{
    Set_Master(PlayerPtr->HeapID);

    ExtGlobalPacketType packet {};
    packet.Command = EXT_NET_HOST_ANNOUNCE;
    std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name));
    packet.Heartbeat.HouseID = static_cast<char>(PlayerPtr->HeapID);
    packet.Heartbeat.IsHost = 1;

    for (int i = 1; i < Session.Players.Count(); i++) {
        Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[i]->Address);
        Ipx.Service();
    }
}


/**
 *  Recomputes the game's master after a human player has left the game.
 *  If the master is gone, the remaining player with the lowest house ID
 *  is promoted. This is deterministic, so all clients agree on the new
 *  master without any negotiation.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Update_Master_After_Player_Removal()
{
    if (Session.Singleplayer_Game()) {
        return;
    }

    /**
     *  If the current master is still in the game, there is nothing to do.
     */
    if (Session.MasterPlayerID != -1) {
        for (int i = 0; i < Session.Players.Count(); i++) {
            if (Session.Players[i]->Player.ID == Session.MasterPlayerID) {
                return;
            }
        }
    }

    /**
     *  Otherwise, promote the remaining player with the lowest house ID.
     */
    int new_master = -1;
    for (int i = 0; i < Session.Players.Count(); i++) {
        const int id = Session.Players[i]->Player.ID;
        if (new_master == -1 || id < new_master) {
            new_master = id;
        }
    }

    if (new_master != -1 && new_master != Session.MasterPlayerID) {
        Set_Master(new_master);
    }
}


/**
 *  Is statistics collection enabled?
 *
 *  @author: ZivDero
 */
bool SessionClassExtension::Are_Statistics_Enabled() const
{
    return This()->Type == GAME_INTERNET || (This()->Type == GAME_IPX && ExtOptions.IsWriteStatistics);
}


/**
 *  Is extra statistics collection enabled?
 *
 *  @author: ZivDero
 */
bool SessionClassExtension::Are_Extra_Statistics_Enabled() const
{
    return This()->Type == GAME_IPX && ExtOptions.IsWriteStatistics;
}
