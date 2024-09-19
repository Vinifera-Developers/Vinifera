/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "spawner.h"
#include "nethack.h"
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
#include "debughandler.h"
#include "extension_globals.h"
#include "gscreen.h"
#include "language.h"
#include "mouse.h"
#include "ownrdraw.h"
#include "tab.h"
#include "WinUser.h"
#include "sessionext.h"
#include "tibsun_functions.h"

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
	//	scen_name += sizeof("RA2->") - 1;

	//if (strstr(scen_name, "PlayMovies->"))
	//{
	//	scen_name += sizeof("PlayMovies->") - 1;
	//	char* context = nullptr;
	//	char* movieName = strtok_s(scen_name, Main::readDelims, &context);
	//	for (; movieName; movieName = strtok_s(nullptr, Main::readDelims, &context))
	//		Game::PlayMovie(movieName);

	//	return false;
	//}

	Load_Sides_Stuff();

	bool result = Config->LoadSaveGame
		? Load_Saved_Game(Config->SaveGameName)
		: Start_New_Scenario(scen_name);

	//if (Main::GetConfig()->DumpTypes)
	//	DumperTypes::Dump();

	Prepare_Screen();

	return result;
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

bool Spawner::Start_New_Scenario(const char* scenario_name)
{
	if (scenario_name[0] == 0)
	{
		DEBUG_INFO("[Spawner] Failed Read Scenario [%s]\n", scenario_name);
		MessageBox(MainWindow, Text_String(TXT_UNABLE_READ_SCENARIO), "Vinifera", MB_OK);

		return false;
	}

	Addon_4071C0(ADDON_ANY);

	if (Config->Firestorm)
		Addon_407190(ADDON_FIRESTORM);

	Set_Required_Addon(Config->Firestorm ? ADDON_FIRESTORM : ADDON_NONE);

	strcpy_s(Session.ScenarioFileName, 0x200, scenario_name);
	Session.Read_Scenario_Descriptions();

	{ // Set Options
		//Session.Options.ScenarioIndex
		Session.Options.Bases						= Config->Bases;
		Session.Options.Credits						= Config->Credits;
		Session.Options.BridgeDestruction			= Config->BridgeDestroy;
		Session.Options.Goodies						= Config->Crates;
		Session.Options.ShortGame					= Config->ShortGame;
		SessionExtension->ExtOptions.IsBuildOffAlly = Config->BuildOffAlly;
		Session.Options.GameSpeed					= Config->GameSpeed;
		Session.Options.CrapEngineers				= Config->MultiEngineer;
		Session.Options.UnitCount					= Config->UnitCount;
		Session.Options.AIPlayers					= Config->AIPlayers;
		Session.Options.AIDifficulty				= Config->AIDifficulty;
		// Session.Options.AISlots
		Session.Options.AlliesAllowed				= Config->AlliesAllowed;
		Session.Options.HarvesterTruce				= Config->HarvesterTruce;
		// Session.Options.CaptureTheFlag
		Session.Options.FogOfWar					= Config->FogOfWar;
		Session.Options.RedeployMCV					= Config->MCVRedeploy;
		std::strcpy(Session.Options.ScenarioDescription, Config->UIMapName);

		Seed = Config->Seed;
		BuildLevel = Config->TechLevel;
		Session.ColorIdx = (PlayerColorType)Config->Players[0].Color;
		Options.GameSpeed = Config->GameSpeed;
	}

	//{ // Added AI Players
	//	const auto pAISlots = &Session.Options.AISlots;
	//	for (char slotIndex = 0; slotIndex < (char)std::size(pAISlots->Allies); ++slotIndex)
	//	{
	//		const auto pPlayerConfig = &Config->Players[slotIndex];
	//		if (pPlayerConfig->IsHuman)
	//			continue;

	//		pAISlots->Difficulties[slotIndex] = pPlayerConfig->Difficulty;
	//		pAISlots->Countries[slotIndex]    = pPlayerConfig->House;
	//		pAISlots->Colors[slotIndex]       = pPlayerConfig->Color;
	//		pAISlots->Allies[slotIndex]       = -1;
	//	}
	//}

	{ // Added Human Players
		NetHack::PortHack = true;
		const char max_players = Config->IsCampaign ? 1 : (char)std::size(Config->Players);
		for (char player_index = 0; player_index < max_players; player_index++)
		{
			const auto player = &Config->Players[player_index];
			if (!player->IsHuman)
				continue;

			const auto nodename = new NodeNameType();
			Session.Players.Add(nodename);

			std::strcpy(nodename->Name, player->Name);
			nodename->Player.House = (HousesType)player->House;
			nodename->Player.Color = (PlayerColorType)player->Color;
			nodename->Player.ProcessTime = -1;

			if (player_index > 0)
			{
				nodename->Address.NodeAddress[0] = player_index;

				const auto ip = inet_addr(player->Ip);
				const auto port = htons((u_short)player->Port);
				ListAddress::Array[player_index - 1].Ip = ip;
				ListAddress::Array[player_index - 1].Port = port;
				if (port != (u_short)Config->ListenPort)
					NetHack::PortHack = false;
			}
		}

		Session.NumPlayers = Session.Players.Count();
	}

	{ // Set SessionType
		if (Config->IsCampaign)
			Session.Type = GAME_NORMAL;
		else if (Session.NumPlayers > 1)
			Session.Type = GAME_INTERNET; // HACK: will be set to GAME_IPX later
		else
			Session.Type = GAME_SKIRMISH;
	}

	Init_Random();

	// Start the scenario
	if (Session.Type == GAME_NORMAL)
	{
		Session.Options.Goodies = true;
		return Start_Scenario(scenario_name, true, CAMPAIGN_FIRST); // set the campaign number properly here?
	}
	else if (Session.Type == GAME_SKIRMISH)
	{
		return Start_Scenario(scenario_name, false, CAMPAIGN_NONE);
	}
	else
	{
		Init_Network();
		if (!Start_Scenario(scenario_name, false, CAMPAIGN_NONE))
			return false;

		Session.Type = GAME_IPX;
		Session.Create_Connections();

		return true;
	}
}

bool Spawner::Load_Saved_Game(const char* save_game_name)
{
	if (!save_game_name[0] || !LoadOptionsClass().Load_File(save_game_name))
	{
		DEBUG_INFO("[Spawner] Failed to Load Savegame [%s]\n", save_game_name);
		MessageBox(MainWindow, Text_String(TXT_ERROR_LOADING_GAME), "Vinifera", MB_OK);

		return false;
	}

	return true;
}

void Spawner::Init_Network()
{
	const auto spawner_config = GetConfig();

	Tunnel::Id = htons((u_short)spawner_config->TunnelId);
	Tunnel::Ip = inet_addr(spawner_config->TunnelIp);
	Tunnel::Port = htons((u_short)spawner_config->TunnelPort);

	PlanetWestwoodPortNumber = Tunnel::Port ? 0 : (u_short)spawner_config->ListenPort;

	PacketTransport = new UDPInterfaceClass();
	PacketTransport->Init();
	PacketTransport->Open_Socket(0);
	PacketTransport->Start_Listening();
	PacketTransport->Discard_In_Buffers();
	PacketTransport->Discard_Out_Buffers();
	Ipx->Set_Timing(60, -1, 600, true);

	PlanetWestwoodStartTime = time(nullptr);
	GameFSSKU = 0x1C00;
	GameSKU = 0x1D00;

	ProtocolZero::Enable = (spawner_config->Protocol == 0);
	if (ProtocolZero::Enable)
	{
		Session.FrameSendRate = 2;
		Session.PrecalcMaxAhead = spawner_config->PreCalcMaxAhead;
		ProtocolZero::MaxLatencyLevel = std::clamp(
			spawner_config->MaxLatencyLevel,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_1,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_MAX
		);
	}
	else
	{
		Session.FrameSendRate = spawner_config->FrameSendRate;
	}

	Session.MaxAhead = spawner_config->MaxAhead == -1
		? Session.FrameSendRate * 6
		: spawner_config->MaxAhead;

	Session.MaxMaxAhead      = 0;
	Session.CommProtocol     = 2;
	Session.LatencyFudge     = 0;
	Session.DesiredFrameRate = 60;
	TournamentGameType = (WOL::Tournament)spawner_config->Tournament;
	PlanetWestwoodGameID = spawner_config->WOLGameID;
	FrameSyncSettings[GAME_IPX].Timeout = spawner_config->ReconnectTimeout;

	if (spawner_config->QuickMatch)
	{
		Session.MPlayerDebug = false;
	}

	Init_Network();
}

void Spawner::Load_Sides_Stuff()
{
	//RulesClass* pRules = RulesClass::Instance;
	//CCINIClass* pINI = CCINIClass::INI_Rules;

	//pRules->Read_Countries(pINI);
	//pRules->Read_Sides(pINI);

	//for (auto const& pItem : *HouseTypeClass::Array)
	//	pItem->LoadFromINI(pINI);
}
