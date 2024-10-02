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
#include "mouse.h"
#include "ownrdraw.h"
#include "saveload.h"
#include "tab.h"
#include "WinUser.h"
#include "sessionext.h"
#include "tibsun_functions.h"
#include "rules.h"

bool Spawner::Enabled = false;
bool Spawner::Active = false;
std::unique_ptr<SpawnerConfig> Spawner::Config = nullptr;

void Spawner::Init()
{
    Config = std::make_unique<SpawnerConfig>();

    CCFileClass spawn_file("SPAWN.INI");
    CCINIClass spawn_ini;

    if (spawn_file.Is_Available()) {

        spawn_ini.Load(spawn_file, false);
        Config->Read_INI(spawn_ini);

    }
    else {
        DEBUG_FATAL("SPAWN.INI not found!\n");
    }
}

bool Spawner::Start_Game()
{
    if (Active)
        return false;

    Active = true;
    GameActive = true;
    Init_UI();

    char* scen_name = Config->ScenarioName;

    //if (strstr(scen_name, "RA2->"))
    //    scen_name += sizeof("RA2->") - 1;

    //if (strstr(scen_name, "PlayMovies->"))
    //{
    //    scen_name += sizeof("PlayMovies->") - 1;
    //    char* context = nullptr;
    //    char* movieName = strtok_s(scen_name, Main::readDelims, &context);
    //    for (; movieName; movieName = strtok_s(nullptr, Main::readDelims, &context))
    //        Game::PlayMovie(movieName);

    //    return false;
    //}

    Read_Houses_And_Sides();

    bool result = Start_New_Scenario(scen_name);

    Prepare_Screen();

    return result;
}


bool Spawner::Start_New_Scenario(const char* scenario_name)
{
    if (scenario_name[0] == 0)
    {
        DEBUG_INFO("[Spawner] Failed to read scenario [%s]\n", scenario_name);
        MessageBox(MainWindow, Text_String(TXT_UNABLE_READ_SCENARIO), "Vinifera", MB_OK);

        return false;
    }

    Set_Required_Addon(Config->Firestorm ? ADDON_FIRESTORM : ADDON_NONE);

    strcpy_s(Session.ScenarioFileName, 0x200, scenario_name);

    // Set Options
    Session.Options.ScenarioIndex               = -1;
    Session.Options.Bases                       = Config->Bases;
    Session.Options.Credits                     = Config->Credits;
    Session.Options.BridgeDestruction           = Config->BridgeDestroy;
    Session.Options.Goodies                     = Config->Crates;
    Session.Options.ShortGame                   = Config->ShortGame;
    SessionExtension->ExtOptions.IsBuildOffAlly = Config->BuildOffAlly;
    Session.Options.GameSpeed                   = Config->GameSpeed;
    Session.Options.CrapEngineers               = Config->MultiEngineer;
    Session.Options.UnitCount                   = Config->UnitCount;
    Session.Options.AIPlayers                   = Config->AIPlayers;
    Session.Options.AIDifficulty                = Config->AIDifficulty;
    Session.Options.AlliesAllowed               = Config->AlliesAllowed;
    Session.Options.HarvesterTruce              = Config->HarvesterTruce;
    // Session.Options.CaptureTheFlag
    Session.Options.FogOfWar                    = Config->FogOfWar;
    Session.Options.RedeployMCV                 = Config->MCVRedeploy;
    std::strcpy(Session.Options.ScenarioDescription, Config->UIMapName);

    Seed = Config->Seed;
    BuildLevel = Config->TechLevel;
    Session.ColorIdx = (PlayerColorType)Config->Players[0].Color;
    Options.GameSpeed = Config->GameSpeed;
    

    // Inverted for now as the sidebar hack until we reimplement loading
    //Session.IsGDI = HouseTypes[Config->Players[0].House]->Get_Heap_ID();
    Session.IsGDI = HouseTypes[Config->Players[0].House]->Side != SIDE_NOD;
    DEBUG_INFO("[Spawner] Session.IsGDI = %d\n", Session.IsGDI);

    // Configure Human Players
    const int max_players = Config->IsCampaign ? 1 : std::size(Config->Players);
    for (int player_index = 0; player_index < max_players; player_index++)
    {
        const auto player = &Config->Players[player_index];
        if (!player->IsHuman)
            continue;

        const auto nodename = new NodeNameType();
        Session.Players.Add(nodename);

        std::strcpy(nodename->Name, player->Name);
        nodename->Player.House          = (HousesType)player->House;
        nodename->Player.Color          = (PlayerColorType)player->Color;
        nodename->Player.ProcessTime    = -1;
        nodename->Game.LastTime         = 1;
    }

    Session.NumPlayers = Session.Players.Count();
    

    // Set GameType
    if (Config->IsCampaign)
        Session.Type = GAME_NORMAL;
    else if (Session.NumPlayers > 1)
        Session.Type = GAME_INTERNET; // HACK: will be set to GAME_IPX later
    else
        Session.Type = GAME_SKIRMISH;
    

    Init_Random();

    // Start the scenario
    if (Session.Type == GAME_NORMAL)
    {
        Session.Options.Goodies = true;
        if (!Start_Scenario(scenario_name, false, static_cast<CampaignType>(Config->CampaignID)))
            return false;

        if (Config->LoadSaveGame && !Load_Saved_Game(Config->SaveGameName))
            return false;

        return true;
    }
    else if (Session.Type == GAME_SKIRMISH)
    {
        if (!Start_Scenario(scenario_name, false, CAMPAIGN_NONE))
            return false;

        if (Config->LoadSaveGame && !Load_Saved_Game(Config->SaveGameName))
            return false;

        return true;
    }
    else
    {
        Spawner_Init_Network();
        if (!Start_Scenario(scenario_name, false, CAMPAIGN_NONE))
            return false;

        if (Config->LoadSaveGame && !Load_Saved_Game(Config->SaveGameName))
            return false;

        Session.Type = GAME_IPX;
        Session.Create_Connections();

        return true;
    }
}

bool Spawner::Load_Saved_Game(const char* save_game_name)
{
    if (!save_game_name[0] || !Load_Game(save_game_name))
    {
        DEBUG_INFO("[Spawner] Failed to Load Savegame [%s]\n", save_game_name);
        MessageBox(MainWindow, Text_String(TXT_ERROR_LOADING_GAME), "Vinifera", MB_OK);

        return false;
    }

    return true;
}

void Spawner::Spawner_Init_Network()
{
    unsigned short id = htons(Config->TunnelId);
    unsigned long ip = inet_addr(Config->TunnelIp);
    unsigned short port = htons(Config->TunnelPort);

    const auto udp_interface = new CnCNet5UDPInterfaceClass(id, ip, port, true);
    PacketTransport = udp_interface;

    PlanetWestwoodPortNumber = port ? 0 : Config->ListenPort;

    const char max_players = std::size(Config->Players);

    for (char player_index = 1; player_index < max_players; player_index++)
    {
        const auto player = &Config->Players[player_index];
        if (!player->IsHuman)
            continue;

        auto& nodename = *Session.Players[player_index];

        std::memset(&nodename.Address, 0, sizeof(nodename.Address));
        std::memcpy(&nodename.Address.NetworkNumber, &player_index, sizeof(player_index));
        std::memcpy(&nodename.Address.NodeAddress, &player_index, sizeof(player_index));

        const auto ip = inet_addr(player->Ip);
        const auto port = htons(player->Port);
        udp_interface->AddressList[player_index - 1].IP = ip;
        udp_interface->AddressList[player_index - 1].Port = port;
        if (port != Config->ListenPort)
            udp_interface->PortHack = false;
    }

    PacketTransport->Init();
    PacketTransport->Open_Socket(0);
    PacketTransport->Start_Listening();
    PacketTransport->Discard_In_Buffers();
    PacketTransport->Discard_Out_Buffers();
    Ipx.Set_Timing(60, -1, 600, true);

    PlanetWestwoodStartTime = time(nullptr);
    GameFSSKU = 0x1C00;
    GameSKU = 0x1D00;

    ProtocolZero::Enable = (Config->Protocol == 0);
    if (ProtocolZero::Enable)
    {
        Session.FrameSendRate = 2;
        Session.PrecalcMaxAhead = Config->PreCalcMaxAhead;
        ProtocolZero::MaxLatencyLevel = std::clamp(
            Config->MaxLatencyLevel,
            (byte)LATENCY_LEVEL_1,
            (byte)LATENCY_LEVEL_MAX
        );
    }
    else
    {
        Session.FrameSendRate = Config->FrameSendRate;
    }

    Session.MaxAhead = Config->MaxAhead == -1
        ? Session.FrameSendRate * 6
        : Config->MaxAhead;

    Session.MaxMaxAhead                 = 0;
    Session.CommProtocol                = 2;
    Session.LatencyFudge                = 0;
    Session.DesiredFrameRate            = 60;
    TournamentGameType                  = static_cast<WOL::Tournament>(Config->Tournament);
    PlanetWestwoodGameID                = Config->WOLGameID;
    FrameSyncSettings[GAME_IPX].Timeout = Config->ReconnectTimeout;

    if (Config->QuickMatch)
    {
        Session.MPlayerDebug = false;
    }

    Init_Network();
}


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


void Spawner::Read_Houses_And_Sides()
{
    Rule->Houses(*RuleINI);
    Rule->Sides(*RuleINI);

    for (int i = 0; i < Houses.Count(); i++)
        Houses[i]->Read_INI(*RuleINI);
}
