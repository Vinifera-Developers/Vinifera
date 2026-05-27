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
#include "mouse.h"
#include "optionsext.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "textprint.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"

#include <filesystem>
#include <format>


namespace
{
    void Print_Saving_Game_Message()
    {
        const int message_delay = Rule->MessageDelay * TICKS_PER_MINUTE;
        Session.Messages.Add_Message(nullptr, 0, "Saving game...", static_cast<ColorSchemeType>(4), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, message_delay);

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
 *  @author: ZivDero
 */
void SessionClassExtension::Flag_To_Save()
{
    AutoSave.IsToSave = true;
}


/**
 *  Suppresses multiplayer autosaves for the remainder of the current session.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Disable_Multiplayer_Autosaves()
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
 *  @author: ZivDero
 */
void SessionClassExtension::Service_Autosave_After_Main_Loop()
{
    if (Frame == AutoSave.NextAutoSaveFrame) {
        Flag_To_Save();
    }

    if (!AutoSave.IsToSave) {
        return;
    }

    Print_Saving_Game_Message();

    if (Session.Singleplayer_Game() && OptionsExtension->AutoSaveCount > 0) {
        AutoSave.IsToSave = false;
        Schedule_Next_Autosave();

        Pause_Scenario();
        Call_Back();
        Save_Game(Autosave_File_Name().c_str(), Autosave_Description().c_str());
        Resume_Scenario();

        if (Session.Type == GAME_NORMAL) {
            AutoSave.NextCampaignAutoSaveSlot = (AutoSave.NextCampaignAutoSaveSlot + 1) % OptionsExtension->AutoSaveCount;
        } else {
            AutoSave.NextSkirmishAutoSaveSlot = (AutoSave.NextSkirmishAutoSaveSlot + 1) % OptionsExtension->AutoSaveCount;
        }

        return;
    }
    
    if ((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && !AutoSave.IsMultiplayerSaveSuppressed) {
        AutoSave.IsToSave = false;
        Schedule_Next_Autosave();
        Init_Multiplayer_Saves_For_Session();
        Save_Game(Autosave_File_Name().c_str(), Autosave_Description().c_str());
        return;
    }

    AutoSave.IsToSave = false;
    Schedule_Next_Autosave();
}


std::string SessionClassExtension::Multiplayer_Save_File_Name_From_Index(int index)
{
    return std::format("SAVEGAME_{:03}.NET", index);
}


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


std::string SessionClassExtension::Multiplayer_Save_File_Name() const
{
    namespace fs = std::filesystem;

    fs::path saved_games_directory(Vinifera_SavedGamesDirectory);

    for (int i = 0; i < 1000; ++i) {

        // SAVEGAME_000.NET
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
        return "Multiplayer Game";
    case GAME_NORMAL:
        return std::format("Mission Auto-Save (Slot {})", AutoSave.NextCampaignAutoSaveSlot + 1);
    case GAME_SKIRMISH:
        return std::format("Skirmish Auto-Save (Slot {})", AutoSave.NextSkirmishAutoSaveSlot + 1);
    default:
        return "";
    }
}
