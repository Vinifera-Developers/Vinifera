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


/**
 *  Initializes the Spawner.
 *
 *  @author: ZivDero
 */
void Spawner::Init()
{
    Vinifera_SpawnerConfig = new SpawnerConfig;

    CCFileClass spawn_file("SPAWN.INI");
    CCINIClass spawn_ini;

    if (spawn_file.Is_Available()) {

        spawn_ini.Load(spawn_file, false);
        Vinifera_SpawnerConfig->Read_INI(spawn_ini);

    } else {
        DEBUG_FATAL("SPAWN.INI not found!\n");
    }
}


/**
 *  Starts the game.
 *
 *  @author: ZivDero
 */
bool Spawner::Start_Game()
{
    if (Vinifera_HasSpawned) {
        return false;
    }

    GameActive = true;

    Init_UI();
    Prepare_Screen();

    const bool result = Start_Scenario(Vinifera_SpawnerConfig->ScenarioName.data());
    Vinifera_HasSpawned = true;
    return result;
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
    if (scenario_name[0] == 0 && !Vinifera_SpawnerConfig->LoadSaveGame) {
        DEBUG_INFO("[Spawner] Failed to read scenario [%s]\n", scenario_name);
        MessageBox(MainWindow, Text_String(TXT_UNABLE_READ_SCENARIO), "Vinifera", MB_OK);

        return false;
    }

    /**
     *  Turn Firestorm on, if requested.
     */
    Disable_Addon(ADDON_ANY);
    if (Vinifera_SpawnerConfig->Firestorm) {
        Enable_Addon(ADDON_FIRESTORM);
        Set_Required_Addon(ADDON_FIRESTORM);
    }

    /**
     *  Configure session options.
     */
    strcpy_s(Session.ScenarioFileName, 0x200, scenario_name);
    Session.Options.ScenarioIndex = -1;
    Session.Options.Bases = Vinifera_SpawnerConfig->Bases;
    Session.Options.Credits = Vinifera_SpawnerConfig->Credits;
    Session.Options.BridgeDestruction = Vinifera_SpawnerConfig->BridgeDestroy;
    Session.Options.Goodies = Vinifera_SpawnerConfig->Crates;
    Session.Options.ShortGame = Vinifera_SpawnerConfig->ShortGame;
    SessionExtension->ExtOptions.IsBuildOffAlly = Vinifera_SpawnerConfig->BuildOffAlly;
    Session.Options.GameSpeed = Vinifera_SpawnerConfig->GameSpeed;
    Session.Options.CrapEngineers = Vinifera_SpawnerConfig->MultiEngineer;
    Session.Options.UnitCount = Vinifera_SpawnerConfig->UnitCount;
    Session.Options.AIPlayers = Vinifera_SpawnerConfig->AIPlayers;
    Session.Options.AIDifficulty = Vinifera_SpawnerConfig->AIDifficulty;
    Session.Options.AlliesAllowed = Vinifera_SpawnerConfig->AlliesAllowed;
    Session.Options.HarvesterTruce = Vinifera_SpawnerConfig->HarvesterTruce;
    // Session.Options.CaptureTheFlag
    Session.Options.FogOfWar = Vinifera_SpawnerConfig->FogOfWar;
    Session.Options.RedeployMCV = Vinifera_SpawnerConfig->MCVRedeploy;
    std::strcpy(Session.Options.ScenarioDescription, Vinifera_SpawnerConfig->UIMapName.c_str());
    const auto& local_player = Vinifera_SpawnerConfig->Players[Vinifera_SpawnerConfig->LocalPlayerIndex];
    Session.ColorIdx = local_player.Color;
    Session.NumPlayers = Vinifera_SpawnerConfig->HumanPlayers;

    Seed = Vinifera_SpawnerConfig->Seed;
    BuildLevel = Vinifera_SpawnerConfig->TechLevel;
    Options.GameSpeed = Vinifera_SpawnerConfig->GameSpeed;

    Vinifera_NextAutoSaveNumber = Vinifera_SpawnerConfig->NextAutoSaveNumber;

    /**
     *  Create the player node for the local player.
     */
    const auto nodename = new NodeNameType();
    Session.Players.Add(nodename);

    std::strcpy(nodename->Name, local_player.Name.c_str());
    nodename->Player.House = local_player.House;
    nodename->Player.Color = local_player.Color;
    nodename->Player.ProcessTime = -1;

    /**
     *  Set session type.
     */
    if (Vinifera_SpawnerConfig->IsCampaign) {
        Session.Type = GAME_NORMAL;
    } else if (Session.NumPlayers > 1) {
        Session.Type = GAME_INTERNET; // HACK: will be set to GAME_IPX later
    } else {
        Session.Type = GAME_SKIRMISH;
    }

    const bool load_save_game = Vinifera_SpawnerConfig->LoadSaveGame;
    const CampaignType campaign_id = Vinifera_SpawnerConfig->CampaignID;
    const bool play_movies_in_multiplayer = Vinifera_SpawnerConfig->PlayMoviesInMultiplayer;

    char save_game_name[decltype(Vinifera_SpawnerConfig->SaveGameName)::capacity()];
    std::snprintf(save_game_name, sizeof(save_game_name), "%s", Vinifera_SpawnerConfig->SaveGameName.c_str());

    Freeze_Config_Into_Extensions();

    Init_Random();

    /**
     *  Start the scenario.
     */
    if (Session.Type == GAME_NORMAL) {
        Session.Options.Goodies = true;
        Release_Config();
        if (load_save_game) {
            return Load_Game(save_game_name);
        } else {
            return ::Start_Scenario(scenario_name, true, campaign_id);
        }
    } else if (Session.Type == GAME_SKIRMISH) {
        Release_Config();
        if (load_save_game) {
            return Load_Game(save_game_name);
        } else {
            return ::Start_Scenario(scenario_name, true, CAMPAIGN_NONE);
        }
    } else {
        Init_Network();

        /**
         *  Preserve fresh network values from the current spawn.ini.
         *  Load_Game will overwrite SpawnerRuntime with saved state,
         *  but network parameters should always come from the current session.
         */
        const bool fresh_proto_enabled = SessionExtension->SpawnerRuntime.ProtocolZeroEnabled;
        const unsigned char fresh_max_latency = SessionExtension->SpawnerRuntime.ProtocolZeroMaxLatencyLevel;
        const int fresh_reconnect_timeout = SessionExtension->SpawnerRuntime.ReconnectTimeout;

        Release_Config();

        bool result = load_save_game
        ? Load_Game(save_game_name)
        : ::Start_Scenario(scenario_name, play_movies_in_multiplayer, CAMPAIGN_NONE);

        if (!result) {
            return false;
        }

        /**
         *  Restore fresh network values after load may have overwritten them with saved state.
         */
        if (load_save_game) {
            SessionExtension->SpawnerRuntime.ProtocolZeroEnabled = fresh_proto_enabled;
            SessionExtension->SpawnerRuntime.ProtocolZeroMaxLatencyLevel = fresh_max_latency;
            SessionExtension->SpawnerRuntime.ReconnectTimeout = fresh_reconnect_timeout;
            SessionExtension->Apply_Spawner_Runtime_State();
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


void Spawner::Freeze_Config_Into_Extensions()
{
    if (Vinifera_SpawnerConfig == nullptr) {
        return;
    }

    SessionExtension->Clear_Spawner_State();
    ScenExtension->Clear_Spawner_Overrides();

    SessionExtension->IsSpawnerSession = true;

    auto& runtime = SessionExtension->SpawnerRuntime;
    runtime.MultiplayerAutoSaveInterval = Vinifera_SpawnerConfig->AutoSaveInterval;
    runtime.QuickMatch = Vinifera_SpawnerConfig->QuickMatch;
    runtime.WriteStatistics = Vinifera_SpawnerConfig->WriteStatistics;
    runtime.AutoSurrender = Vinifera_SpawnerConfig->AutoSurrender;
    runtime.AttackNeutralUnits = Vinifera_SpawnerConfig->AttackNeutralUnits;
    runtime.CoachMode = Vinifera_SpawnerConfig->CoachMode;
    runtime.ContinueWithoutHumans = Vinifera_SpawnerConfig->ContinueWithoutHumans;
    runtime.ScrapMetal = Vinifera_SpawnerConfig->ScrapMetal;
    runtime.AINamesByDifficulty = Vinifera_SpawnerConfig->AINamesByDifficulty;
    runtime.ProtocolZeroEnabled = Vinifera_SpawnerConfig->Protocol == 0;
    runtime.ProtocolZeroMaxLatencyLevel = static_cast<unsigned char>(std::clamp(Vinifera_SpawnerConfig->MaxLatencyLevel, LATENCY_LEVEL_1, LATENCY_LEVEL_MAX));
    runtime.ReconnectTimeout = Vinifera_SpawnerConfig->ReconnectTimeout;
    runtime.Tournament = Vinifera_SpawnerConfig->Tournament;
    runtime.GameID = Vinifera_SpawnerConfig->WOLGameID;

    ScenExtension->HasSpawnerScenarioOverrides = true;
    ScenExtension->CampaignDifficultyOverride = Vinifera_SpawnerConfig->CampaignDifficulty;
    ScenExtension->CampaignCDifficultyOverride = Vinifera_SpawnerConfig->CampaignCDifficulty;
    ScenExtension->SkipScoreScreenOverride = Vinifera_SpawnerConfig->SkipScoreScreen;
    std::snprintf(ScenExtension->StatsUIMapName, sizeof(ScenExtension->StatsUIMapName), "%s", Vinifera_SpawnerConfig->UIMapName.c_str());
    std::snprintf(ScenExtension->StatsMapHash, sizeof(ScenExtension->StatsMapHash), "%s", Vinifera_SpawnerConfig->MapHash.c_str());

    if (!Vinifera_SpawnerConfig->CustomLoadScreen.empty()) {
        ScenExtension->HasCustomLoadScreen = true;
        std::snprintf(ScenExtension->CustomLoadScreen, sizeof(ScenExtension->CustomLoadScreen), "%s", Vinifera_SpawnerConfig->CustomLoadScreen.c_str());
    }

    if (Vinifera_SpawnerConfig->CustomLoadScreenPos != Point2D(0, 0)) {
        ScenExtension->HasCustomLoadScreenPos = true;
        ScenExtension->CustomLoadScreenPos = Vinifera_SpawnerConfig->CustomLoadScreenPos;
    }

    const int total_slots = std::min(Vinifera_SpawnerConfig->HumanPlayers + Vinifera_SpawnerConfig->AIPlayers, MAX_PLAYERS);

    for (int slot_index = 0; slot_index < total_slots; ++slot_index) {
        const auto& player_config = Vinifera_SpawnerConfig->Players[slot_index];
        auto& slot_info = SessionExtension->SlotInfo[slot_index];

        slot_info.IsConfigured = true;
        slot_info.IsHuman = player_config.IsHuman;
        slot_info.Color = player_config.Color;
        slot_info.House = player_config.House;
        slot_info.Difficulty = player_config.Difficulty;
        slot_info.IsObserver = player_config.IsObserver;
        slot_info.SpawnLocation = player_config.SpawnLocation;

        for (int ally_index = 0; ally_index < std::size(slot_info.Alliances); ++ally_index) {
            slot_info.Alliances[ally_index] = player_config.Alliances[ally_index];
        }
    }

    SessionExtension->Apply_Spawner_Runtime_State();
}


void Spawner::Release_Config()
{
    delete Vinifera_SpawnerConfig;
    Vinifera_SpawnerConfig = nullptr;
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
    const unsigned short tunnel_id = htons(Vinifera_SpawnerConfig->TunnelId);
    const unsigned long tunnel_ip = inet_addr(Vinifera_SpawnerConfig->TunnelIp.c_str());
    const unsigned short tunnel_port = htons(Vinifera_SpawnerConfig->TunnelPort);

    /**
     *  Create the UDP interface.
     *  This needs to happen before we set up player nodes,
     *  because it contains player connection data.
     */
    const auto udp_interface = new CnCNet5UDPInterfaceClass(tunnel_id, tunnel_ip, tunnel_port, true);
    PacketTransport = udp_interface;

    WestwoodOnline_PortNumber = tunnel_port ? 0 : Vinifera_SpawnerConfig->ListenPort;

    /**
     *  Set up the player nodes for remote human players.
     */
    char remote_index = 1;
    for (int slot_index = 0; slot_index < Vinifera_SpawnerConfig->HumanPlayers; ++slot_index) {
        if (slot_index == Vinifera_SpawnerConfig->LocalPlayerIndex) continue;

        const auto& player = Vinifera_SpawnerConfig->Players[slot_index];

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
        if (port != Vinifera_SpawnerConfig->ListenPort) udp_interface->PortHack = false;

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
    if (SessionExtension->SpawnerRuntime.ProtocolZeroEnabled) {
        Session.FrameSendRate = 2;
        Session.PrecalcMaxAhead = Vinifera_SpawnerConfig->PreCalcMaxAhead;
    } else {
        Session.FrameSendRate = Vinifera_SpawnerConfig->FrameSendRate;
    }

    Session.MaxAhead = Vinifera_SpawnerConfig->MaxAhead == -1 ? Session.FrameSendRate * 6 : Vinifera_SpawnerConfig->MaxAhead;

    /**
     *  Miscellaneous network settings.
     */
    Session.MaxMaxAhead = 0;
    Session.CommProtocol = 2;
    Session.LatencyFudge = 0;
    Session.DesiredFrameRate = 60;
    WestwoodOnline_Tournament = Vinifera_SpawnerConfig->Tournament;
    WestwoodOnline_GameID = Vinifera_SpawnerConfig->WOLGameID;

    struct QueueAIMPTimings {
        int MIXFILE_RESEND_DELTA;
        int FRAMESYNC_DLG_TIME;
        int FRAMESYNC_TIMEOUT;
        int MIXFILE_TIMEOUT;
    };
    static QueueAIMPTimings(&Queue_AI_Multiplayer_Timings)[8] = *reinterpret_cast<QueueAIMPTimings(*)[8]>(0x00707F88);

    Queue_AI_Multiplayer_Timings[GAME_IPX].MIXFILE_TIMEOUT = Vinifera_SpawnerConfig->ReconnectTimeout;

    /**
     *  For Quick Match, make sure MPDebug is off so that players can't cheat with it.
     */
    if (Vinifera_SpawnerConfig->QuickMatch) {
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
