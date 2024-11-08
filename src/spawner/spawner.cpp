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
#include "protocolzero.h"
#include "latencylevel.h"

#include "options.h"
#include "house.h"
#include "ipxmgr.h"
#include "loadoptions.h"
#include "scenario.h"
#include <ctime>

#include "addon.h"
#include "wspudp.h"
#include "wwmouse.h"
#include "ccini.h"
#include "cncnet5_wspudp.h"
#include "debughandler.h"
#include "extension_globals.h"
#include "gscreen.h"
#include "housetype.h"
#include "language.h"
#include "housetypeext.h"
#include "mouse.h"
#include "ownrdraw.h"
#include "saveload.h"
#include "tab.h"
#include "WinUser.h"
#include "sessionext.h"
#include "tibsun_functions.h"
#include "rules.h"
#include "vinifera_globals.h"


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

    }
    else {
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
    if (Vinifera_SpawnerActive)
        return false;

    Vinifera_SpawnerActive = true;
    GameActive = true;

    Init_UI();

    const bool result = Start_Scenario(Vinifera_SpawnerConfig->ScenarioName);

    Prepare_Screen();

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
    if (scenario_name[0] == 0 && !Vinifera_SpawnerConfig->LoadSaveGame)
    {
        DEBUG_INFO("[Spawner] Failed to read scenario [%s]\n", scenario_name);
        MessageBox(MainWindow, Text_String(TXT_UNABLE_READ_SCENARIO), "Vinifera", MB_OK);

        return false;
    }

    /**
     *  Turn Firestorm on, if requested.
     */
    Disable_Addon(ADDON_ANY);
    if (Vinifera_SpawnerConfig->Firestorm)
    {
        Enable_Addon(ADDON_FIRESTORM);
        Set_Required_Addon(ADDON_FIRESTORM);
    }

    /**
     *  Configure session options.
     */
    strcpy_s(Session.ScenarioFileName, 0x200, scenario_name);
    Session.Options.ScenarioIndex               = -1;
    Session.Options.Bases                       = Vinifera_SpawnerConfig->Bases;
    Session.Options.Credits                     = Vinifera_SpawnerConfig->Credits;
    Session.Options.BridgeDestruction           = Vinifera_SpawnerConfig->BridgeDestroy;
    Session.Options.Goodies                     = Vinifera_SpawnerConfig->Crates;
    Session.Options.ShortGame                   = Vinifera_SpawnerConfig->ShortGame;
    SessionExtension->ExtOptions.IsBuildOffAlly = Vinifera_SpawnerConfig->BuildOffAlly;
    Session.Options.GameSpeed                   = Vinifera_SpawnerConfig->GameSpeed;
    Session.Options.CrapEngineers               = Vinifera_SpawnerConfig->MultiEngineer;
    Session.Options.UnitCount                   = Vinifera_SpawnerConfig->UnitCount;
    Session.Options.AIPlayers                   = Vinifera_SpawnerConfig->AIPlayers;
    Session.Options.AIDifficulty                = Vinifera_SpawnerConfig->AIDifficulty;
    Session.Options.AlliesAllowed               = Vinifera_SpawnerConfig->AlliesAllowed;
    Session.Options.HarvesterTruce              = Vinifera_SpawnerConfig->HarvesterTruce;
    // Session.Options.CaptureTheFlag
    Session.Options.FogOfWar                    = Vinifera_SpawnerConfig->FogOfWar;
    Session.Options.RedeployMCV                 = Vinifera_SpawnerConfig->MCVRedeploy;
    std::strcpy(Session.Options.ScenarioDescription, Vinifera_SpawnerConfig->UIMapName);
    Session.ColorIdx = static_cast<PlayerColorType>(Vinifera_SpawnerConfig->Players[0].Color);
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

    std::strcpy(nodename->Name, Vinifera_SpawnerConfig->Players[0].Name);
    nodename->Player.House = static_cast<HousesType>(Vinifera_SpawnerConfig->Players[0].House);
    nodename->Player.Color = static_cast<PlayerColorType>(Vinifera_SpawnerConfig->Players[0].Color);
    nodename->Player.ProcessTime = -1;
    nodename->Game.LastTime = 1;

    /**
     *  Set session type.
     */
    if (Vinifera_SpawnerConfig->IsCampaign)
        Session.Type = GAME_NORMAL;
    else if (Session.NumPlayers > 1)
        Session.Type = GAME_INTERNET; // HACK: will be set to GAME_IPX later
    else
        Session.Type = GAME_SKIRMISH;
    

    Init_Random();

    /**
     *  Start the scenario.
     */
    if (Session.Type == GAME_NORMAL)
    {
        Session.Options.Goodies = true;

        const bool result = Vinifera_SpawnerConfig->LoadSaveGame ?
            Load_Game(Vinifera_SpawnerConfig->SaveGameName) : ::Start_Scenario(scenario_name, Vinifera_SpawnerConfig->PlayMoviesInMultiplayer, static_cast<CampaignType>(Vinifera_SpawnerConfig->CampaignID));

        return result;
    }
    else if (Session.Type == GAME_SKIRMISH)
    {
        const bool result = Vinifera_SpawnerConfig->LoadSaveGame ?
            Load_Game(Vinifera_SpawnerConfig->SaveGameName) : ::Start_Scenario(scenario_name, false, CAMPAIGN_NONE);

        return result;
    }
    else
    {
        Init_Network();

        bool result = Vinifera_SpawnerConfig->LoadSaveGame ?
            Load_Game(Vinifera_SpawnerConfig->SaveGameName) : ::Start_Scenario(scenario_name, false, CAMPAIGN_NONE);

        if (!result)
            return false;

        Session.Type = GAME_IPX;

        if (Vinifera_SpawnerConfig->LoadSaveGame && !Reconcile_Players())
            return false;

        if (!Session.Create_Connections())
            return false;

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
    if (!file_name[0] || !::Load_Game(file_name))
    {
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
    const unsigned long tunnel_ip = inet_addr(Vinifera_SpawnerConfig->TunnelIp);
    const unsigned short tunnel_port = htons(Vinifera_SpawnerConfig->TunnelPort);

    /**
     *  Create the UDP interface.
     *  This needs to happen before we set up player nodes,
     *  because it contains player connection data.
     */
    const auto udp_interface = new CnCNet5UDPInterfaceClass(tunnel_id, tunnel_ip, tunnel_port, true);
    PacketTransport = udp_interface;

    PlanetWestwoodPortNumber = tunnel_port ? 0 : Vinifera_SpawnerConfig->ListenPort;

    /**
     *  Set up the player nodes.
     */
    const char max_players = std::size(Vinifera_SpawnerConfig->Players);
    for (char player_index = 1; player_index < max_players; player_index++)
    {
        const auto player = &Vinifera_SpawnerConfig->Players[player_index];
        if (!player->IsHuman)
            continue;

        const auto nodename = new NodeNameType();
        Session.Players.Add(nodename);

        std::strcpy(nodename->Name, player->Name);
        nodename->Player.House = static_cast<HousesType>(player->House);
        nodename->Player.Color = static_cast<PlayerColorType>(player->Color);
        nodename->Player.ProcessTime = -1;
        nodename->Game.LastTime = 1;

        std::memset(&nodename->Address, 0, sizeof(nodename->Address));
        std::memcpy(&nodename->Address.NetworkNumber, &player_index, sizeof(player_index));
        std::memcpy(&nodename->Address.NodeAddress, &player_index, sizeof(player_index));

        const auto ip = inet_addr(player->Ip);
        const auto port = htons(player->Port);
        udp_interface->AddressList[player_index - 1].IP = ip;
        udp_interface->AddressList[player_index - 1].Port = port;
        if (port != Vinifera_SpawnerConfig->ListenPort)
            udp_interface->PortHack = false;
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

    PlanetWestwoodStartTime = time(nullptr);
    GameFSSKU = 0x1C00;
    GameSKU = 0x1D00;

    /**
     *  Set up protocol stuff.
     */
    ProtocolZero::Enable = (Vinifera_SpawnerConfig->Protocol == 0);
    if (ProtocolZero::Enable)
    {
        Session.FrameSendRate = 2;
        Session.PrecalcMaxAhead = Vinifera_SpawnerConfig->PreCalcMaxAhead;
        ProtocolZero::MaxLatencyLevel = std::clamp(
            Vinifera_SpawnerConfig->MaxLatencyLevel,
            static_cast<unsigned char>(LATENCY_LEVEL_1),
            static_cast<unsigned char>(LATENCY_LEVEL_MAX));
    }
    else
    {
        Session.FrameSendRate = Vinifera_SpawnerConfig->FrameSendRate;
    }

    Session.MaxAhead = Vinifera_SpawnerConfig->MaxAhead == -1
        ? Session.FrameSendRate * 6
        : Vinifera_SpawnerConfig->MaxAhead;

    /**
     *  Miscellaneous network settings.
     */
    Session.MaxMaxAhead                 = 0;
    Session.CommProtocol                = 2;
    Session.LatencyFudge                = 0;
    Session.DesiredFrameRate            = 60;
    PlanetWestwoodTournament            = static_cast<WOL::Tournament>(Vinifera_SpawnerConfig->Tournament);
    PlanetWestwoodGameID                = Vinifera_SpawnerConfig->WOLGameID;
    FrameSyncSettings[GAME_IPX].Timeout = Vinifera_SpawnerConfig->ReconnectTimeout;

    /**
     *  For Quick Match, make sure MPDebug is off so that players can't cheat with it.
     */
    if (Vinifera_SpawnerConfig->QuickMatch)
    {
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
    if (Session.Players.Count() == 0)
        return true;

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

            if (!stricmp(Session.Players[i]->Name, housep->IniName)) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
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
            if (!stricmp(Session.Players[i]->Name, housep->IniName)) {
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

            static char buffer[HOUSE_NAME_MAX + 1];
            std::snprintf(buffer, sizeof(buffer), "%s (AI)", housep->IniName);
            std::strncpy(housep->IniName, buffer, sizeof(housep->IniName));
            //strcpy(housep->IniName, Fetch_String(TXT_COMPUTER));

            Session.NumPlayers--;
        }
    }

    /**
     *  If all went well, our Session.NumPlayers value should now equal the value
     *  from the saved game, minus any players we removed.
     */
    if (Session.NumPlayers == Session.Players.Count()) {
        return true;
    }
    else {
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
    OwnerDraw::Init_UI_Color_Stuff_58F060();

    if (!OwnerDraw::UIInitialized)
    {
        OwnerDraw::Init_Glow_Colors();
        OwnerDraw::Load_Graphics();
        OwnerDraw::UIInitialized = true;
    }
}


/**
 *  Prepares the screen.
 *
 *  @author: ZivDero
 */
void Spawner::Prepare_Screen()
{
    WWMouse->Hide_Mouse();

    HiddenSurface->Fill(TBLACK);
    GScreenClass::Blit(true, HiddenSurface);
    LogicSurface = HiddenSurface;

    WWMouse->Show_Mouse();

    Map.MouseClass::Set_Default_Mouse(MOUSE_NO_MOVE, false);
    Map.MouseClass::Revert_Mouse_Shape();

    Map.TabClass::Activate(1);
    Map.SidebarClass::Flag_To_Redraw();
}
