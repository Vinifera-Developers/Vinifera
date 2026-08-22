/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Configuration of the multiplayer spawner.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "debughandler.h"
#include "spawnerconfig.h"
#include "ccini.h"


/**
 *  Reads spawner config from the INI.
 *
 *  @author: Belonit, ZivDero
 */
void SpawnerConfig::Read_INI(CCINIClass& spawn_ini)
{
    static char const* const SETTINGS = "Settings";
    static char const* const TUNNEL = "Tunnel";

    /**
     *  Game Mode Options
     */
    Bases = spawn_ini.Get_Bool(SETTINGS, "Bases", Bases);
    Credits = spawn_ini.Get_Int(SETTINGS, "Credits", Credits);
    BridgeDestroy = spawn_ini.Get_Bool(SETTINGS, "BridgeDestroy", BridgeDestroy);
    Crates = spawn_ini.Get_Bool(SETTINGS, "Crates", Crates);
    ShortGame = spawn_ini.Get_Bool(SETTINGS, "ShortGame", ShortGame);
    BuildOffAlly = spawn_ini.Get_Bool(SETTINGS, "BuildOffAlly", BuildOffAlly);
    GameSpeed = spawn_ini.Get_Int(SETTINGS, "GameSpeed", GameSpeed);
    MultiEngineer = spawn_ini.Get_Bool(SETTINGS, "MultiEngineer", MultiEngineer);
    UnitCount = spawn_ini.Get_Int(SETTINGS, "UnitCount", UnitCount);
    AIPlayers = spawn_ini.Get_Int(SETTINGS, "AIPlayers", AIPlayers);
    AIDifficulty = spawn_ini.Get_Int(SETTINGS, "AIDifficulty", AIDifficulty);
    AlliesAllowed = spawn_ini.Get_Bool(SETTINGS, "AlliesAllowed", AlliesAllowed);
    HarvesterTruce = spawn_ini.Get_Bool(SETTINGS, "HarvesterTruce", HarvesterTruce);
    FogOfWar = spawn_ini.Get_Bool(SETTINGS, "FogOfWar", FogOfWar);
    MCVRedeploy = spawn_ini.Get_Bool(SETTINGS, "MCVRedeploy", MCVRedeploy);

    /**
     *  Savegame Options
     */
    LoadSaveGame = spawn_ini.Get_Bool(SETTINGS, "LoadSaveGame", LoadSaveGame);
    SaveGameName = spawn_ini.Get_String(SETTINGS, "SaveGameName", std::string(SaveGameName));
    AutoSaveInterval = spawn_ini.Get_Int(SETTINGS, "AutoSaveGame", AutoSaveInterval);
    NextCampaignAutoSaveNumber = spawn_ini.Get_Int(SETTINGS, "NextSPAutoSaveId", NextCampaignAutoSaveNumber + 1) - 1; // Subtract 1 since our autosaves are 0-based internally
    NextSkirmishAutoSaveNumber = spawn_ini.Get_Int(SETTINGS, "NextSkirmishAutoSaveId", NextSkirmishAutoSaveNumber + 1) - 1; // Subtract 1 since our autosaves are 0-based internally

    /**
     *  Scenario Options
     */
    Seed = spawn_ini.Get_Int(SETTINGS, "Seed", Seed);
    TechLevel = spawn_ini.Get_Int(SETTINGS, "TechLevel", TechLevel);
    IsCampaign = spawn_ini.Get_Bool(SETTINGS, "IsSinglePlayer", IsCampaign);
    CampaignID = static_cast<CampaignType>(spawn_ini.Get_Int(SETTINGS, "CampaignID", CampaignID));
    CampaignDifficulty = static_cast<DiffType>(spawn_ini.Get_Int(SETTINGS, "DifficultyModeHuman", CampaignDifficulty));
    CampaignCDifficulty = static_cast<DiffType>(spawn_ini.Get_Int(SETTINGS, "DifficultyModeComputer", CampaignCDifficulty));
    Tournament = spawn_ini.Get_Int(SETTINGS, "Tournament", Tournament);
    WOLGameID = spawn_ini.Get_Int(SETTINGS, "GameID", WOLGameID);
    ScenarioName = spawn_ini.Get_String(SETTINGS, "Scenario", std::string(ScenarioName));
    MapHash = spawn_ini.Get_String(SETTINGS, "MapHash", std::string(MapHash));
    MapName = spawn_ini.Get_String(SETTINGS, "UIMapName", std::string(MapName));
    PlayMoviesInMultiplayer = spawn_ini.Get_Bool(SETTINGS, "PlayMoviesInMultiplayer", PlayMoviesInMultiplayer);
    IsHost = spawn_ini.Get_Bool(SETTINGS, "Host", IsHost);

    /**
     *  Network Options
     */
    Protocol = spawn_ini.Get_Int(SETTINGS, "Protocol", Protocol);
    FrameSendRate = spawn_ini.Get_Int(SETTINGS, "FrameSendRate", FrameSendRate);
    ReconnectTimeout = spawn_ini.Get_Int(SETTINGS, "ReconnectTimeout", ReconnectTimeout);
    ConnTimeout = spawn_ini.Get_Int(SETTINGS, "ConnTimeout", ConnTimeout);
    MaxAhead = spawn_ini.Get_Int(SETTINGS, "MaxAhead", MaxAhead);
    PreCalcMaxAhead = spawn_ini.Get_Int(SETTINGS, "PreCalcMaxAhead", PreCalcMaxAhead);
    MaxLatencyLevel = static_cast<LatencyLevelEnum>(spawn_ini.Get_Int(SETTINGS, "MaxLatencyLevel", MaxLatencyLevel));

    /**
     *  Tunnel Options
     */
    TunnelId = spawn_ini.Get_Int(SETTINGS, "Port", TunnelId);
    ListenPort = spawn_ini.Get_Int(SETTINGS, "Port", ListenPort);
    TunnelIp = spawn_ini.Get_String(TUNNEL, "Ip", std::string(TunnelIp));
    TunnelPort = spawn_ini.Get_Int(TUNNEL, "Port", TunnelPort);

    /**
     *  Player and House Options.
     *  Read player data into a temp array, then sort humans by color
     *  into the front of the final array, followed by AIs.
     *  House data is read per final slot index since it's already in game slot order.
     */
    PlayerConfig temp[std::size(Players)];
    for (int i = 0; i < std::size(temp); ++i) {
        temp[i].Read_Player_INI(spawn_ini, i);
        if (temp[i].IsHuman) {
            HumanPlayers++;
        }
    }

    bool claimed[std::size(Players)] = {};
    int slot = 0;

    for (int s = 0; s < HumanPlayers && slot < std::size(Players); ++s) {
        int best = -1;
        for (int i = 0; i < std::size(temp); ++i) {
            if (!temp[i].IsHuman || claimed[i]) continue;
            if (best == -1 || temp[i].Color < temp[best].Color) best = i;
        }
        if (best == -1) break;
        claimed[best] = true;
        Players[slot] = temp[best];
        if (best == 0) LocalPlayerIndex = slot;
        slot++;
    }

    for (int i = 0; i < std::size(temp) && slot < std::size(Players); ++i) {
        if (!temp[i].IsHuman) {
            Players[slot] = temp[i];
            slot++;
        }
    }

    for (int i = 0; i < std::size(Players); ++i) {
        Players[i].Read_House_INI(spawn_ini, i);
    }

    /**
     *  Extended Options
     */
    Firestorm = spawn_ini.Get_Bool(SETTINGS, "Firestorm", Firestorm);
    QuickMatch = spawn_ini.Get_Bool(SETTINGS, "QuickMatch", QuickMatch);
    SkipScoreScreen = spawn_ini.Get_Bool(SETTINGS, "SkipScoreScreen", SkipScoreScreen);
    WriteStatistics = spawn_ini.Get_Bool(SETTINGS, "WriteStatistics", WriteStatistics);
    AINamesByDifficulty = spawn_ini.Get_Bool(SETTINGS, "DifficultyBasedAINames", AINamesByDifficulty);
    CoachMode = spawn_ini.Get_Bool(SETTINGS, "CoachMode", CoachMode);
    AutoSurrender = spawn_ini.Get_Bool(SETTINGS, "AutoSurrender", AutoSurrender);
    AttackNeutralUnits = spawn_ini.Get_Bool(SETTINGS, "AttackNeutralUnits", AttackNeutralUnits);
    ScrapMetal = spawn_ini.Get_Bool(SETTINGS, "ScrapMetal", ScrapMetal);
    CustomLoadScreen = spawn_ini.Get_String(SETTINGS, "CustomLoadScreen", std::string(CustomLoadScreen));
    CustomLoadScreenPos = spawn_ini.Get_Point(SETTINGS, "CustomLoadScreenPos", CustomLoadScreenPos);
    ContinueWithoutHumans = spawn_ini.Get_Bool(SETTINGS, "ContinueWithoutHumans", ContinueWithoutHumans);
    DifficultyName = spawn_ini.Get_String(SETTINGS, "DifficultyName", std::string(DifficultyName));

    /**
     *  Environment Globals
     */
    char buffer[20];
    for (int i = 0; i < std::size(GlobalFlags); i++) {
        sprintf(buffer, "GlobalFlag%d", i);
        GlobalFlags[i] = spawn_ini.Get_Int("GlobalFlags", buffer, 0);
        if (GlobalFlags[i] > 0) {
            DEBUG_INFO("[Spawner] Read global {} as {} from {}\n", i, GlobalFlags[i], buffer);
        }
    }
}


static constexpr const char* PlayerSectionArray[8] = {
    "Settings",
    "Other1",
    "Other2",
    "Other3",
    "Other4",
    "Other5",
    "Other6",
    "Other7"
};


static constexpr const char* MultiTagArray[8] = {
    "Multi1",
    "Multi2",
    "Multi3",
    "Multi4",
    "Multi5",
    "Multi6",
    "Multi7",
    "Multi8"
};


static constexpr const char* AlliancesSectionArray[8] = {
    "Multi1_Alliances",
    "Multi2_Alliances",
    "Multi3_Alliances",
    "Multi4_Alliances",
    "Multi5_Alliances",
    "Multi6_Alliances",
    "Multi7_Alliances",
    "Multi8_Alliances"
};


static constexpr const char* AlliancesTagArray[8] = {
    "HouseAllyOne",
    "HouseAllyTwo",
    "HouseAllyThree",
    "HouseAllyFour",
    "HouseAllyFive",
    "HouseAllySix",
    "HouseAllySeven",
    "HouseAllyEight"
};


/**
 *  Reads player's config from the INI.
 *
 *  @author: Belonit, ZivDero
 */
void SpawnerConfig::PlayerConfig::Read_Player_INI(CCINIClass& spawn_ini, int index)
{
    if (index >= MAX_PLAYERS) return;

    const char* SECTION = PlayerSectionArray[index];
    const char* MULTI_TAG = MultiTagArray[index];

    if (spawn_ini.Is_Present(SECTION)) {
        IsHuman = true;
        Difficulty = -1;
        Name = spawn_ini.Get_String(SECTION, "Name", std::string(Name));
        Color = static_cast<PlayerColorType>(spawn_ini.Get_Int(SECTION, "Color", Color));
        House = static_cast<HousesType>(spawn_ini.Get_Int(SECTION, "Side", House));
        Ip = spawn_ini.Get_String(SECTION, "Ip", std::string(Ip));
        Port = spawn_ini.Get_Int(SECTION, "Port", Port);
    } else if (!IsHuman) {
        Color = static_cast<PlayerColorType>(spawn_ini.Get_Int("HouseColors", MULTI_TAG, Color));
        House = static_cast<HousesType>(spawn_ini.Get_Int("HouseCountries", MULTI_TAG, House));
        Difficulty = spawn_ini.Get_Int("HouseHandicaps", MULTI_TAG, Difficulty);
    }
}


/**
 *  Reads house's config from the INI.
 *
 *  @author: Belonit, ZivDero
 */
void SpawnerConfig::PlayerConfig::Read_House_INI(CCINIClass& spawn_ini, int index)
{
    if (index >= MAX_PLAYERS) return;

    const char* ALLIANCES = AlliancesSectionArray[index];
    const char* MULTI_TAG = MultiTagArray[index];

    IsObserver = spawn_ini.Get_Bool("IsSpectator", MULTI_TAG, IsObserver);
    SpawnLocation = spawn_ini.Get_Int("SpawnLocations", MULTI_TAG, SpawnLocation);

    /**
     *  Reset any weird values we might receive as input.
     */
    if (SpawnLocation < 0 || SpawnLocation > MAX_PLAYERS - 1) SpawnLocation = -1;

    if (spawn_ini.Is_Present(ALLIANCES)) {
        for (int i = 0; i < 8; i++) {
            Alliances[i] = spawn_ini.Get_Int(ALLIANCES, AlliancesTagArray[i], Alliances[i]);
        }
    }
}
