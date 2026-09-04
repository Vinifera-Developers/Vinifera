/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Multiplayer spawner class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "spawner.h"

#include "WinUser.h"
#include "addon.h"
#include "campaign.h"
#include "ccini.h"
#include "cncnet5_wspudp.h"
#include "debughandler.h"
#include "environmentext.h"
#include "extension_globals.h"
#include "gscreen.h"
#include "house.h"
#include "housetype.h"
#include "housetypeext.h"
#include "initext_hooks.h"
#include "ipxmgr.h"
#include "language.h"
#include "latencylevel.h"
#include "loadoptions.h"
#include "mouse.h"
#include "miscutil.h"
#include "netdlg.h"
#include "options.h"
#include "ownrdraw.h"
#include "protocolzero.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "scenarioext.h"
#include "sessionext.h"
#include "tab.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include "vinifera_savever.h"
#include "vinifera_util.h"
#include "wspudp.h"
#include "wwmouse.h"

#include <algorithm>
#include <climits>
#include <ctime>


namespace
{
    struct QueueAIMPTimings
    {
        int MIXFILE_RESEND_DELTA;
        int FRAMESYNC_DLG_TIME;
        int FRAMESYNC_TIMEOUT;
        int MIXFILE_TIMEOUT;
    };
}

bool Spawner::HasSpawned = false;
std::unique_ptr<SpawnerConfig> Spawner::Config;


/**
 *  Initializes the Spawner.
 *
 *  @author: ZivDero
 */
bool Spawner::Init()
{
    Config = std::make_unique<SpawnerConfig>();

    CCFileClass spawn_file("SPAWN.INI");
    CCINIClass spawn_ini;

    if (spawn_file.Is_Available()) {
        spawn_ini.Load(spawn_file, false);
        Config->Read_INI(spawn_ini);
        return true;
    }

    DEBUG_FATAL("SPAWN.INI not found!\n");
    return false;
}

/**
 *  Starts the game.
 *
 *  @author: ZivDero
 */
bool Spawner::Start_Game()
{
    if (HasSpawned) {
        return false;
    }

    GameActive = true;

    // Initialize MIX files for the side.
    // They are needed for mods that have side-specific OwnerDraw graphics
    // so the game can display potential WWMessageBoxes or other dialogs before
    // the side has been fully initialized through the scenario initialization routines.
    SideType side = static_cast<SideType>(Config->Players[0].House);
    if (side == SIDE_NONE) side = SIDE_GDI;
    if (!Vinifera_Load_Side_Mixfiles(side)) {
        DEBUG_ERROR("[Spawner] Start_Game: Failed to load side MIXes!\n");
        return false;
    }

    /**
     *  Initialize some OD global state so that dialogs work correctly.
     */
    OwnerDraw::Initialize();
    OwnerDraw::Init_Masks();
    OwnerDraw::Cache_Images();

    /**
     *  Clear the screen before Start_Scenario just in case so that it has
     *  a clean slate to work with.
     */
    HiddenSurface->Fill(TBLACK);
    Update_Visible_Surface();

    DEBUG_INFO("[Spawner] Start_Game: Starting scenario {}\n", Config->ScenarioName);
    const bool result = Start_Scenario(Config->ScenarioName.data());
    HasSpawned = true;

    if (!result) {
        DEBUG_ERROR("[Spawner] Start_Game: Start_Scenario returned false!\n");
        return result;
    }

    /**
     *  Tail of Select_Game: set up the game screen and
     *  render one frame for the caller to fade in. The unmatched final Hide
     *  returns the mouse hidden-by-one, which the caller (Main_Game) shows.
     */
    HiddenSurface->Fill(TBLACK);
    Update_Visible_Surface();
    LogicalSurface = HiddenSurface;

    Show_Mouse();

    Map.Override_Mouse_Shape(MOUSE_NO_MOVE);
    Map.Revert_Mouse_Shape();

    Map.Activate(1);
    Map.Flag_To_Redraw();

    Hide_Mouse();

    return result;
}

int Spawner::Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty(int difficulty)
{
    switch (difficulty) {
    case 0:
        return DIFF_EASY;
    case 1:
        return DIFF_NORMAL;
    case 2:
        return DIFF_HARD;
    case 3:
        return EXT_DIFF_VERY_EASY;
    case 4:
        return EXT_DIFF_BRUTALLY_EASY;
    case 5:
        return EXT_DIFF_EXTREMELY_EASY;
    case 6:
        return EXT_DIFF_ULTIMATELY_EASY;
    }

    DEBUG_FATAL("Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty: Unknown difficulty level {}", difficulty);
    return -1;
}


/**
 *  Validates config values that are later used as array and heap indices.
 *
 *  @author: ZivDero
 */
bool Spawner::Validate_Config()
{
    if (Config == nullptr || Config->HumanPlayers < 1 || Config->HumanPlayers > MAX_PLAYERS) {
        DEBUG_ERROR("[Spawner] Invalid human player count {}.\n", Config ? Config->HumanPlayers : -1);
        return false;
    }

    if (Config->LocalPlayerIndex < 0 || Config->LocalPlayerIndex >= Config->HumanPlayers) {
        DEBUG_ERROR("[Spawner] Invalid local player index {}.\n", Config->LocalPlayerIndex);
        return false;
    }

    if (Config->AIPlayers < 0 || Config->AIPlayers > MAX_PLAYERS - Config->HumanPlayers) {
        DEBUG_ERROR("[Spawner] Invalid AI player count {} for {} human players.\n", Config->AIPlayers, Config->HumanPlayers);
        return false;
    }

    if (Config->Protocol != 0 && (Config->FrameSendRate < 1 || Config->FrameSendRate > UCHAR_MAX)) {
        DEBUG_ERROR("[Spawner] Invalid frame send rate {}. Expected a value from 1 through 255.\n", Config->FrameSendRate);
        return false;
    }

    /**
     *  When resuming a saved game, the session state comes from the save
     *  itself and the client only provides a minimal config, so the slot
     *  data is neither present nor used.
     */
    if (Config->LoadSaveGame) {
        return true;
    }

    /**
     *  Campaign scenarios create their houses from the map rather than the
     *  multiplayer slot configuration.
     */
    if (Config->IsCampaign) {
        const int human_difficulty = static_cast<int>(Config->CampaignDifficulty);
        const int computer_difficulty = static_cast<int>(Config->CampaignCDifficulty);
        if (human_difficulty < DIFF_FIRST || human_difficulty >= EXT_DIFF_COUNT || computer_difficulty < DIFF_FIRST || computer_difficulty >= EXT_DIFF_COUNT) {
            DEBUG_ERROR("[Spawner] Invalid campaign difficulty values {}, {}.\n", human_difficulty, computer_difficulty);
            return false;
        }

        if (Config->CampaignID != CAMPAIGN_NONE && (Config->CampaignID < CAMPAIGN_FIRST || Config->CampaignID >= Campaigns.Count())) {
            DEBUG_ERROR("[Spawner] Invalid campaign ID {}.\n", static_cast<int>(Config->CampaignID));
            return false;
        }

        return true;
    }

    if (Config->AIDifficulty < DIFF_FIRST || Config->AIDifficulty >= DIFF_COUNT) {
        DEBUG_ERROR("[Spawner] Invalid global AI difficulty {}.\n", Config->AIDifficulty);
        return false;
    }

    const int total_slots = Config->HumanPlayers + Config->AIPlayers;
    std::vector<bool> colors_used(ColorSchemes.Count());

    for (int slot = 0; slot < total_slots; ++slot) {
        const auto& player = Config->Players[slot];
        const int color = static_cast<int>(player.Color);
        const int house = static_cast<int>(player.House);

        if (color < 0 || color >= static_cast<int>(colors_used.size())) {
            DEBUG_ERROR("[Spawner] Slot {} has invalid color index {}.\n", slot, color);
            return false;
        }

        /**
         *  Only human colors need to be unique; AI houses may share one.
         */
        if (player.IsHuman) {
            if (colors_used[color]) {
                DEBUG_ERROR("[Spawner] Human slot {} reuses color index {}.\n", slot, color);
                return false;
            }
            colors_used[color] = true;
        }

        if (house < 0 || house >= HouseTypes.Count()) {
            DEBUG_ERROR("[Spawner] Slot {} has invalid house index {}.\n", slot, house);
            return false;
        }

        if (player.IsHuman) {
            if (player.Name.empty()) {
                DEBUG_ERROR("[Spawner] Human slot {} has an empty name.\n", slot);
                return false;
            }

            if (player.Name.size() >= MPLAYER_NAME_MAX) {
                DEBUG_ERROR("[Spawner] Human slot {} has a name longer than {} characters.\n", slot, MPLAYER_NAME_MAX - 1);
                return false;
            }

            for (int other_slot = 0; other_slot < slot; ++other_slot) {
                const auto& other = Config->Players[other_slot];
                if (other.IsHuman && !_stricmp(player.Name.c_str(), other.Name.c_str())) {
                    DEBUG_ERROR("[Spawner] Human slots {} and {} use the same name.\n", other_slot, slot);
                    return false;
                }
            }
        } else {
            if (player.Difficulty < -1 || player.Difficulty > 6) {
                DEBUG_ERROR("[Spawner] AI slot {} has invalid difficulty {}.\n", slot, player.Difficulty);
                return false;
            }
        }

        for (int ally : player.Alliances) {
            if (ally < -1 || ally >= total_slots) {
                DEBUG_ERROR("[Spawner] Slot {} has invalid alliance target {}.\n", slot, ally);
                return false;
            }
        }
    }

    return true;
}

/**
 *  Configures session and extension state from the current spawn Config->
 *
 *  @author: ZivDero
 */
bool Spawner::Init_Session(char* scenario_name)
{
    if (!Validate_Config()) {
        return false;
    }

    const auto& local_player = Config->Players[Config->LocalPlayerIndex];

    strcpy_s(Session.ScenarioFileName, 0x200, scenario_name);
    Session.Options.ScenarioIndex = -1;
    Session.Options.Bases = Config->Bases;
    Session.Options.Credits = Config->Credits;
    Session.Options.BridgeDestruction = Config->BridgeDestroy;
    Session.Options.Goodies = Config->Crates;
    Session.Options.ShortGame = Config->ShortGame;
    SessionExtension->ExtOptions.IsBuildOffAlly = Config->BuildOffAlly;
    Session.Options.GameSpeed = Config->GameSpeed;
    Session.Options.CrapEngineers = Config->MultiEngineer;
    Session.Options.UnitCount = Config->UnitCount;
    Session.Options.AIPlayers = Config->AIPlayers;
    Session.Options.AIDifficulty = Config->AIDifficulty;
    Session.Options.AlliesAllowed = Config->AlliesAllowed;
    Session.Options.HarvesterTruce = Config->HarvesterTruce;
    Session.Options.FogOfWar = Config->FogOfWar;
    Session.Options.RedeployMCV = Config->MCVRedeploy;
    std::snprintf(Session.Options.ScenarioDescription, sizeof(Session.Options.ScenarioDescription), "%s", Config->MapName.c_str());
    Session.ColorIdx = local_player.Color;
    Session.NumPlayers = Config->HumanPlayers;

    Seed = Config->Seed;
    BuildLevel = Config->TechLevel;
    Options.GameSpeed = Config->GameSpeed;

    SessionExtension->Set_Next_Campaign_Autosave_Slot(Config->NextCampaignAutoSaveNumber);
    SessionExtension->Set_Next_Skirmish_Autosave_Slot(Config->NextSkirmishAutoSaveNumber);

    const auto nodename = new NodeNameType();
    Session.Players.Add(nodename);

    std::snprintf(nodename->Name, sizeof(nodename->Name), "%s", local_player.Name.c_str());
    nodename->Player.House = local_player.House;
    nodename->Player.Color = local_player.Color;
    nodename->Player.ProcessTime = -1;

    if (Config->IsCampaign) {
        Session.Type = GAME_NORMAL;
    } else if (Session.NumPlayers > 1) {
        Session.Type = GAME_INTERNET; // HACK: will be set to GAME_IPX later
    } else {
        Session.Type = GAME_SKIRMISH;
    }

    SessionExtension->IsSpawnerSession = true;
    SessionExtension->ExtOptions.MultiplayerAutoSaveInterval = Config->AutoSaveInterval;
    SessionExtension->ExtOptions.IsQuickMatch = Config->QuickMatch;
    SessionExtension->ExtOptions.IsWriteStatistics = Config->WriteStatistics;
    SessionExtension->ExtOptions.IsSkipScoreScreen = Config->SkipScoreScreen;
    SessionExtension->ExtOptions.IsAutoSurrender = Config->AutoSurrender;
    SessionExtension->ExtOptions.IsAttackNeutralUnits = Config->AttackNeutralUnits;
    SessionExtension->ExtOptions.IsCoachMode = Config->CoachMode;
    SessionExtension->ExtOptions.IsContinueWithoutHumans = Config->ContinueWithoutHumans;
    SessionExtension->ExtOptions.IsScrapMetal = Config->ScrapMetal;
    SessionExtension->ExtOptions.IsAINamesByDifficulty = Config->AINamesByDifficulty;
    SessionExtension->ExtOptions.IsPlayMoviesInMultiplayer = Config->PlayMoviesInMultiplayer;
    SessionExtension->MultiplayerSavesInitializedForThisSession = Config->LoadSaveGame;
    SessionExtension->IsOriginalHost = Config->IsHost;

    /**
     *  Until the host announces itself, there is no known game master. This makes
     *  our Am_I_Master replacement fall back to the "first human house" heuristic.
     */
    Session.MasterPlayerID = -1;
    Session.MasterPlayerName[0] = '\0';

    SessionExtension->SpawnerInfo.StatsMapName = Config->MapName.c_str();
    SessionExtension->SpawnerInfo.StatsMapHash = Config->MapHash.c_str();
    SessionExtension->SpawnerInfo.DifficultyName = Config->DifficultyName.c_str();
    if (!Config->CustomLoadScreen.empty()) {
        SessionExtension->SpawnerInfo.CustomLoadScreen = Config->CustomLoadScreen.c_str();
    }
    if (Config->CustomLoadScreenPos.X > 0 && Config->CustomLoadScreenPos.Y > 0) {
        SessionExtension->SpawnerInfo.CustomLoadScreenPos = Config->CustomLoadScreenPos;
    }

    /**
     *  NOTE: Scenario data gets cleared between this point and the scenario start, because the first step
     *  in reading a scenario is calling Clear_Scenario. Assume any scenario variables set here to get cleared.
     *  Only set up some fields that we need for initialization (like difficulty, which is read by the environment).
     */
    Scen->Difficulty = Config->CampaignDifficulty;
    Scen->CDifficulty = Config->CampaignCDifficulty;

    const int total_slots = std::min(Config->HumanPlayers + Config->AIPlayers, MAX_PLAYERS);

    for (int slot_index = 0; slot_index < total_slots; ++slot_index) {
        const auto& player_config = Config->Players[slot_index];
        auto& slot_info = SessionExtension->SlotInfo[slot_index];

        slot_info.IsConfigured = true;
        slot_info.IsHuman = player_config.IsHuman;
        slot_info.Color = player_config.Color;
        slot_info.House = player_config.House;

        if (!slot_info.IsHuman && player_config.Difficulty >= 0) {
            slot_info.Difficulty = Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty(player_config.Difficulty);

            if (slot_info.Difficulty < 0) {
                return false;
            }
        }

        slot_info.IsObserver = player_config.IsObserver;
        slot_info.SpawnLocation = player_config.SpawnLocation;

        for (int ally_index = 0; ally_index < std::size(slot_info.Alliances); ++ally_index) {
            slot_info.Alliances[ally_index] = player_config.Alliances[ally_index];
        }
    }

    /**
     *  (Re)initialize the environment so it can read our scenario difficulty options.
     */
    new (reinterpret_cast<ExtEnvironmentClass*>(&Environment)) ExtEnvironmentClass;

    /**
     *  Set the spawner's environment flags.
     */
    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        EnvironmentGlobals[i] = Config->GlobalFlags[i];
        if (Config->GlobalFlags[i] > 0) {
            DEBUG_INFO("[Spawner] Init_Session: Applied GlobalFlag {} as {}\n", i, EnvironmentGlobals[i]);
        }
    }

    return true;
}


/**
 *  Starts a new scenario.
 *
 *  @author: ZivDero
 */
bool Spawner::Start_Scenario(char* scenario_name)
{
    /**
     *  Can't read an unnamed file, bail.
     */
    if (scenario_name[0] == 0 && !Config->LoadSaveGame) {
        DEBUG_INFO("[Spawner] Failed to read scenario [{}]\n", scenario_name);
        MessageBox(MainWindow, Text_String(TXT_UNABLE_READ_SCENARIO), "Vinifera", MB_OK);

        return false;
    }

    /**
     *  Turn Firestorm on, if requested.
     */
    Disable_Addon(ADDON_ANY);
    if (Config->Firestorm) {
        Enable_Addon(ADDON_FIRESTORM);
        Set_Required_Addon(ADDON_FIRESTORM);
    }

    if (!Init_Session(scenario_name)) {
        DEBUG_ERROR("[Spawner] Init_Session returned false!\n");
        return false;
    }

    const bool load_save_game = Config->LoadSaveGame;
    const CampaignType campaign_id = Config->CampaignID;
    const bool play_movies_in_multiplayer = SessionExtension->ExtOptions.IsPlayMoviesInMultiplayer;

    char save_game_name[decltype(Config->SaveGameName)::capacity() + 1];
    std::snprintf(save_game_name, sizeof(save_game_name), "%s", Config->SaveGameName.c_str());

    Init_Random();

    /**
     *  Generate a unique ID for this playthrough.
     *  Inconsequential when loading a saved game, because the playthrough ID
     *  is loaded from saved games.
     */
    Vinifera_Generate_PlaythroughID();

    /**
     *  Start the scenario.
     */
    if (Session.Type == GAME_NORMAL) {
        Session.Options.Goodies = true;
        if (load_save_game) {
            return Load_Game(save_game_name);
        } else {
            return ::Start_Scenario(scenario_name, true, campaign_id);
        }
    } else if (Session.Type == GAME_SKIRMISH) {
        if (load_save_game) {
            return Load_Game(save_game_name);
        } else {
            return ::Start_Scenario(scenario_name, true, CAMPAIGN_NONE);
        }
    } else {
        if (!Init_Network()) {
            return false;
        }
        bool result = load_save_game ? Load_Game(save_game_name) : ::Start_Scenario(scenario_name, play_movies_in_multiplayer, CAMPAIGN_NONE);
        if (!result) {
            return false;
        }

        Session.Type = GAME_IPX;

        if (load_save_game && !SessionExtension->Reconcile_Players()) {
            return false;
        }

        if (!Session.Create_Connections()) {
            return false;
        }

        /**
         *  Let the other players know who the game host is, so that host
         *  privileges can be migrated if the host leaves the game.
         */
        if (SessionExtension->IsOriginalHost) {
            SessionExtension->Announce_Master();
        }

        return true;
    }
}


/**
 *  Loads a saved game.
 *
 *  @author: ZivDero
 */
bool Spawner::Load_Game(const char* file_name)
{
    char formatted_file_name[PATH_MAX];
    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory.c_str(), Filename_From_Path(file_name), nullptr);

    ViniferaSaveVersionInfo save_info;
    if (strlen(file_name) == 0 || !Vinifera_Is_Save_Loadable(formatted_file_name, &save_info)) {
        DEBUG_INFO("[Spawner] Failed to validate savegame [{}]\n", file_name);
        MessageBox(MainWindow, Text_String(TXT_ERROR_LOADING_GAME), "Vinifera", MB_OK);
        return false;
    }

    const GameEnum saved_type = static_cast<GameEnum>(save_info.Get_Game_Type());

    /**
     *  Loading restores the game type from the save itself, so only a
     *  multiplayer/single-player mismatch matters: a multiplayer save
     *  needs the network setup from the config, which a single-player
     *  session does not have.
     */
    const bool session_is_multiplayer = Session.Type == GAME_INTERNET || Session.Type == GAME_IPX;
    const bool save_is_multiplayer = saved_type == GAME_INTERNET || saved_type == GAME_IPX;

    if (session_is_multiplayer != save_is_multiplayer) {
        DEBUG_ERROR("[Spawner] Savegame [{}] has incompatible game type {}.\n", file_name, static_cast<int>(saved_type));
        MessageBox(MainWindow, Text_String(TXT_ERROR_LOADING_GAME), "Vinifera", MB_OK);
        return false;
    }

    // Using LoadOptionsClass().Load_File gives us a "Mission is loading. Please wait..." box.
    if (!LoadOptionsClass().Load_File(file_name)) {
        DEBUG_INFO("[Spawner] Failed to load savegame [{}]\n", file_name);
        MessageBox(MainWindow, Text_String(TXT_ERROR_LOADING_GAME), "Vinifera", MB_OK);

        return false;
    }

    if (Session.Type == GAME_INTERNET || Session.Type == GAME_IPX) {
        ProtocolZero::Reset();
    }

    Scen->IsSkipScore |= Config->SkipScoreScreen;

    return true;
}


/**
 *  Initializes everything necessary for an MP game.
 *
 *  @author: ZivDero
 */
bool Spawner::Init_Network()
{
    ProtocolZero::Reset();
    SessionExtension->ProtocolZeroEnabled = Config->Protocol == 0;
    SessionExtension->ProtocolZeroMaxLatencyLevel = static_cast<unsigned char>(std::clamp(Config->MaxLatencyLevel, LATENCY_LEVEL_1, LATENCY_LEVEL_MAX));
    SessionExtension->ConnTimeout = Config->ConnTimeout;

    const unsigned short tunnel_id = htons(Config->TunnelId);
    const unsigned long tunnel_ip = inet_addr(Config->TunnelIp.c_str());
    const unsigned short tunnel_port = htons(Config->TunnelPort);

    /**
     *  Create the UDP interface.
     *  This needs to happen before we set up player nodes,
     *  because it contains player connection data.
     */
    const auto udp_interface = new CnCNet5UDPInterfaceClass(tunnel_id, tunnel_ip, tunnel_port, true);
    PacketTransport = udp_interface;

    WestwoodOnline_PortNumber = tunnel_port ? 0 : Config->ListenPort;

    /**
     *  Set up the player nodes for remote human players.
     */
    char remote_index = 1;
    for (int slot_index = 0; slot_index < Config->HumanPlayers; ++slot_index) {
        if (slot_index == Config->LocalPlayerIndex) continue;

        const auto& player = Config->Players[slot_index];

        const auto nodename = new NodeNameType();
        Session.Players.Add(nodename);

        std::snprintf(nodename->Name, sizeof(nodename->Name), "%s", player.Name.c_str());
        nodename->Player.House = player.House;
        nodename->Player.Color = player.Color;
        nodename->Player.ProcessTime = -1;
        nodename->Game.LastTime = 1;

        std::memset(&nodename->Address, 0, sizeof(nodename->Address));
        std::memcpy(&nodename->Address.NetworkNumber, &remote_index, sizeof(remote_index));
        std::memcpy(&nodename->Address.NodeAddress, &remote_index, sizeof(remote_index));

        const auto ip = Config->TunnelPort != 0 ? INADDR_ANY : inet_addr(player.Ip.c_str());
        const auto port = htons(player.Port);
        udp_interface->AddressList[remote_index - 1].IP = ip;
        udp_interface->AddressList[remote_index - 1].Port = port;
        if (player.Port != Config->ListenPort) {
            udp_interface->PortHack = false;
        }

        remote_index++;
    }

    /**
     *  Now set up the rest of the network stuff.
     */
    if (!PacketTransport->Init()) {
        DEBUG_ERROR("[Spawner] Failed to initialize the UDP packet transport.\n");
        delete PacketTransport;
        PacketTransport = nullptr;
        return false;
    }

    if (!PacketTransport->Open_Socket(0)) {
        DEBUG_ERROR("[Spawner] Failed to open the UDP socket.\n");
        PacketTransport->Close();
        delete PacketTransport;
        PacketTransport = nullptr;
        return false;
    }

    if (!PacketTransport->Start_Listening()) {
        DEBUG_ERROR("[Spawner] Failed to start listening on the UDP socket.\n");
        PacketTransport->Close();
        delete PacketTransport;
        PacketTransport = nullptr;
        return false;
    }
    PacketTransport->Discard_In_Buffers();
    PacketTransport->Discard_Out_Buffers();
    Ipx.Set_Timing(60, -1, 600, true);

    WestwoodOnline_StartTime = time(nullptr);
    WestwoodOnline_GameSKU_FS = 0x1C00;
    WestwoodOnline_GameSKU_TS = 0x1D00;

    /**
     *  Set up protocol stuff.
     */
    if (SessionExtension->ProtocolZeroEnabled) {
        Session.FrameSendRate = 2;
        Session.PrecalcMaxAhead = Config->PreCalcMaxAhead;
    } else {
        Session.FrameSendRate = Config->FrameSendRate;
    }

    Session.MaxAhead = Config->MaxAhead == -1 ? Session.FrameSendRate * 6 : Config->MaxAhead;

    /**
     *  Miscellaneous network settings.
     */
    Session.MaxMaxAhead = 0;
    Session.CommProtocol = 2;
    Session.LatencyFudge = 0;
    Session.DesiredFrameRate = 60;
    WestwoodOnline_Tournament = Config->Tournament;
    WestwoodOnline_GameID = Config->WOLGameID;

    static QueueAIMPTimings(&Queue_AI_Multiplayer_Timings)[8] = *reinterpret_cast<QueueAIMPTimings(*)[8]>(0x00707F88);
    Queue_AI_Multiplayer_Timings[GAME_IPX].MIXFILE_TIMEOUT = Config->ReconnectTimeout;

    /**
     *  For Quick Match, make sure MPDebug is off so that players can't cheat with it.
     */
    if (Config->QuickMatch) {
        Session.MPlayerDebug = false;
    }

    ::Init_Network();
    return true;
}
