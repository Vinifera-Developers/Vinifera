/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNER.CPP
 *
 *  @author        Belonit, ZivDero
 *
 *  @brief         Multiplayer spawner class.
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

#include "spawner.h"

#include "WinUser.h"
#include "addon.h"
#include "ccini.h"
#include "cncnet5_wspudp.h"
#include "debughandler.h"
#include "environmentext.h"
#include "extension_globals.h"
#include "gscreen.h"
#include "house.h"
#include "housetype.h"
#include "housetypeext.h"
#include "ipxmgr.h"
#include "language.h"
#include "latencylevel.h"
#include "loadoptions.h"
#include "mouse.h"
#include "netdlg.h"
#include "options.h"
#include "ownrdraw.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "scenarioext.h"
#include "sessionext.h"
#include "tab.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"
#include "wspudp.h"
#include "wwmouse.h"

#include <algorithm>
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

    Init_UI();
    Prepare_Screen();

    DEBUG_INFO("[Spawner] Start_Game: Starting scenario %s\n", Config->ScenarioName.c_str());
    const bool result = Start_Scenario(Config->ScenarioName.data());
    HasSpawned = true;

    if (!result) {
        DEBUG_ERROR("[Spawner] Start_Game: Start_Scenario returned false!\n");
    }

    return result;
}

int Spawner::Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty(int difficulty)
{
    // Temporarily uses DTA's original setup for now to avoid client changes as we figure out what to do

    switch (difficulty) {
    case 0:
        return DIFF_HARD;
    case 1:
        return DIFF_NORMAL;
    case 2:
        return DIFF_EASY;
    case 3:
        return EXT_DIFF_VERY_HARD;
    case 4:
        return EXT_DIFF_BRUTAL;
    case 5:
        return EXT_DIFF_EXTREME;
    case 6:
        return EXT_DIFF_ULTIMATE;
    }

    // switch (difficulty)
    // {
    // case 0:
    //     return EXT_DIFF_ULTIMATE;
    // case 1:
    //     return EXT_DIFF_EXTREME;
    // case 2:
    //     return EXT_DIFF_BRUTAL;
    // case 3:
    //     return EXT_DIFF_VERY_HARD;
    // case 4:
    //     return DIFF_HARD;
    // case 5:
    //     return DIFF_NORMAL;
    // case 6:
    //     return DIFF_EASY;
    // }
    
    DEBUG_FATAL("Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty: Unknown difficulty level %d", difficulty);
    return -1;
}

/**
 *  Configures session and extension state from the current spawn Config->
 *
 *  @author: ZivDero
 */
bool Spawner::Init_Session(char* scenario_name)
{
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
    std::strcpy(Session.Options.ScenarioDescription, Config->MapName.c_str());
    Session.ColorIdx = local_player.Color;
    Session.NumPlayers = Config->HumanPlayers;

    Seed = Config->Seed;
    BuildLevel = Config->TechLevel;
    Options.GameSpeed = Config->GameSpeed;

    SessionExtension->Set_Next_Campaign_Autosave_Slot(Config->NextCampaignAutoSaveNumber);
    SessionExtension->Set_Next_Skirmish_Autosave_Slot(Config->NextSkirmishAutoSaveNumber);

    const auto nodename = new NodeNameType;
    Session.Players.Add(nodename);

    std::strcpy(nodename->Name, local_player.Name.c_str());
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
    SessionExtension->ExtOptions.IsAutoSurrender = Config->AutoSurrender;
    SessionExtension->ExtOptions.IsAttackNeutralUnits = Config->AttackNeutralUnits;
    SessionExtension->ExtOptions.IsCoachMode = Config->CoachMode;
    SessionExtension->ExtOptions.IsContinueWithoutHumans = Config->ContinueWithoutHumans;
    SessionExtension->ExtOptions.IsScrapMetal = Config->ScrapMetal;
    SessionExtension->ExtOptions.IsAINamesByDifficulty = Config->AINamesByDifficulty;
    SessionExtension->ClearMultiplayerSavesOnSave = !Config->LoadSaveGame;

    /**
     *  NOTE: Scenario data gets cleared between this point and the scenario start, because the first step
     *  in reading a scenario is calling Clear_Scenario. Assume any scenario variables set here to get cleared.
     *  Only set up some fields that we need for initialization (like difficulty, which is read by the environment).
     */
    Apply_Scenario_Values();

    const int total_slots = std::min(Config->HumanPlayers + Config->AIPlayers, MAX_PLAYERS);

    for (int slot_index = 0; slot_index < total_slots; ++slot_index) {
        const auto& player_config = Config->Players[slot_index];
        auto& slot_info = SessionExtension->SlotInfo[slot_index];

        slot_info.IsConfigured = true;
        slot_info.IsHuman = player_config.IsHuman;
        slot_info.Color = player_config.Color;
        slot_info.House = player_config.House;

        if (!slot_info.IsHuman) {
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
            DEBUG_INFO("[Spawner] Init_Session: Applied GlobalFlag %d as %d\n", i, EnvironmentGlobals[i]);
        }
    }

    return true;
}


/**
 *  Writes spawner-related values to the scenario.
 *
 *  @author: Rampastring
 */
void Spawner::Apply_Scenario_Values()
{
    // TODO maybe the stats, difficulty name, etc. should belong to SessionExtension instead?
    Scen->Difficulty = Config->CampaignDifficulty;
    Scen->CDifficulty = Config->CampaignCDifficulty;
    std::snprintf(ScenExtension->StatsMapName, sizeof(ScenExtension->StatsMapName), "%s", Config->MapName.c_str());
    std::snprintf(ScenExtension->StatsMapHash, sizeof(ScenExtension->StatsMapHash), "%s", Config->MapHash.c_str());
    std::snprintf(ScenExtension->DifficultyName, sizeof(ScenExtension->DifficultyName), "%s", Config->DifficultyName.c_str());

    if (!Config->CustomLoadScreen.empty()) {
        ScenExtension->HasCustomLoadScreen = true;
        std::snprintf(ScenExtension->CustomLoadScreen, sizeof(ScenExtension->CustomLoadScreen), "%s", Config->CustomLoadScreen.c_str());
    }

    if (Config->CustomLoadScreenPos != Point2D(0, 0)) {
        ScenExtension->HasCustomLoadScreenPos = true;
        ScenExtension->CustomLoadScreenPos = Config->CustomLoadScreenPos;
    }
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
        DEBUG_INFO("[Spawner] Failed to read scenario [%s]\n", scenario_name);
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
    const bool play_movies_in_multiplayer = Config->PlayMoviesInMultiplayer;

    char save_game_name[decltype(Config->SaveGameName)::capacity()];
    std::snprintf(save_game_name, sizeof(save_game_name), "%s", Config->SaveGameName.c_str());

    Init_Random();

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
        Init_Network();
        bool result = load_save_game ? Load_Game(save_game_name) : ::Start_Scenario(scenario_name, play_movies_in_multiplayer, CAMPAIGN_NONE);
        if (!result) {
            return false;
        }

        Session.Type = GAME_IPX;

        if (load_save_game && !Reconcile_Players()) {
            return false;
        }

        if (!Session.Create_Connections()) {
            return false;
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
//    if (strlen(file_name) == 0 || !::Load_Game(file_name)) {
    if (strlen(file_name) == 0 || !LoadOptionsClass().Load_File(file_name)) { // using LoadOptionsClass().Load_File here gives us a "Mission is loading. Please wait..." box.
        DEBUG_INFO("[Spawner] Failed to load savegame [%s]\n", file_name);
        MessageBox(MainWindow, Text_String(TXT_ERROR_LOADING_GAME), "Vinifera", MB_OK);

        return false;
    }

    return true;
}


/**
 *  Initializes everything necessary for an MP game.
 *
 *  @author: ZivDero
 */
void Spawner::Init_Network()
{
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

        std::strcpy(nodename->Name, player.Name.c_str());
        nodename->Player.House = player.House;
        nodename->Player.Color = player.Color;
        nodename->Player.ProcessTime = -1;
        nodename->Game.LastTime = 1;

        std::memset(&nodename->Address, 0, sizeof(nodename->Address));
        std::memcpy(&nodename->Address.NetworkNumber, &remote_index, sizeof(remote_index));
        std::memcpy(&nodename->Address.NodeAddress, &remote_index, sizeof(remote_index));

        const auto ip = inet_addr(player.Ip.c_str());
        const auto port = htons(player.Port);
        udp_interface->AddressList[remote_index - 1].IP = ip;
        udp_interface->AddressList[remote_index - 1].Port = port;
        if (player.Port != Config->ListenPort) { // TODO: This used to compare the post-htons port in ts-patches, may be a bug?
            udp_interface->PortHack = false;
        }

        remote_index++;
    }

    /**
     *  Now set up the rest of the network stuff.
     */
    PacketTransport->Init();
    PacketTransport->Open_Socket(0);
    PacketTransport->Start_Listening();
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
bool Spawner::Reconcile_Players()
{
    int i;
    bool found;
    int house;
    HouseClass* housep;

    /**
     *  If there are no players, there's nothing to do.
     */
    if (Session.Players.Count() == 0) return true;

    /**
     *  Make sure every name we're connected to can be found in a House.
     */
    for (i = 0; i < Session.Players.Count(); i++) {
        found = false;
        for (house = 0; house < Session.Players.Count(); house++) {
            housep = Houses[house];
            if (!housep) {
                continue;
            }

            if (!stricmp(Session.Players[i]->Name, housep->IniName.c_str())) {
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
    for (house = 0; house < Session.Players.Count(); house++) {
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
        for (i = 0; i < Session.Players.Count(); i++) {
            if (!stricmp(Session.Players[i]->Name, housep->IniName.c_str())) {
                found = true;
                Session.Players[i]->Player.ID = static_cast<HousesType>(house);
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
    if (Session.NumPlayers == Session.Players.Count()) {
        return true;
    } else {
        return false;
    }
}


/**
 *  Initializes some things for OwnerDraw UI.
 *
 *  @author: ZivDero
 */
void Spawner::Init_UI()
{
    OwnerDraw::Initialize();
    OwnerDraw::Init_Masks();
    OwnerDraw::Cache_Images();
}


/**
 *  Prepares the screen.
 *
 *  @author: ZivDero
 */
void Spawner::Prepare_Screen()
{
    MouseCursor->Hide_Mouse();

    HiddenSurface->Fill(TBLACK);
    Update_Visible_Surface();
    LogicalSurface = HiddenSurface;

    MouseCursor->Show_Mouse();

    Map.MouseClass::Set_Default_Mouse(MOUSE_NO_MOVE, false);
    Map.MouseClass::Revert_Mouse_Shape();

    Map.TabClass::Activate(1);
    Map.SidebarClass::Flag_To_Redraw();
}
