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

#include "spawnerconfig.h"

#include <ccini.h>

void SpawnerConfig::Read_INI(CCINIClass& spawn_ini)
{
    static char const* const SETTINGS = "Settings";

    { // Game Mode Options
        Bases          = spawn_ini.Get_Bool(SETTINGS, "Bases", Bases);
        Credits        = spawn_ini.Get_Int(SETTINGS, "Credits", Credits);
        BridgeDestroy  = spawn_ini.Get_Bool(SETTINGS, "BridgeDestroy", BridgeDestroy);
        Crates         = spawn_ini.Get_Bool(SETTINGS, "Crates", Crates);
        ShortGame      = spawn_ini.Get_Bool(SETTINGS, "ShortGame", ShortGame);
        BuildOffAlly   = spawn_ini.Get_Bool(SETTINGS, "BuildOffAlly", BuildOffAlly);
        GameSpeed      = spawn_ini.Get_Int(SETTINGS, "GameSpeed", GameSpeed);
        MultiEngineer  = spawn_ini.Get_Bool(SETTINGS, "MultiEngineer", MultiEngineer);
        UnitCount      = spawn_ini.Get_Int(SETTINGS, "UnitCount", UnitCount);
        AIPlayers      = spawn_ini.Get_Int(SETTINGS, "AIPlayers", AIPlayers);
        AIDifficulty   = spawn_ini.Get_Int(SETTINGS, "AIDifficulty", AIDifficulty);
        AlliesAllowed  = spawn_ini.Get_Bool(SETTINGS, "AlliesAllowed", AlliesAllowed);
        HarvesterTruce = spawn_ini.Get_Bool(SETTINGS, "HarvesterTruce", HarvesterTruce);
        FogOfWar       = spawn_ini.Get_Bool(SETTINGS, "FogOfWar", FogOfWar);
        MCVRedeploy    = spawn_ini.Get_Bool(SETTINGS, "MCVRedeploy", MCVRedeploy);
        spawn_ini.Get_String(SETTINGS, "UIGameMode", UIGameMode, UIGameMode, sizeof(UIGameMode));
    }

    // SaveGame Options
    LoadSaveGame     = spawn_ini.Get_Bool(SETTINGS, "LoadSaveGame", LoadSaveGame);
    /* SavedGameDir */ spawn_ini.Get_String(SETTINGS, "SavedGameDir", SavedGameDir, SavedGameDir, sizeof(SavedGameDir));
    /* SaveGameName */ spawn_ini.Get_String(SETTINGS, "SaveGameName", SaveGameName, SaveGameName, sizeof(SaveGameName));

    { // Scenario Options
        Seed             = spawn_ini.Get_Int(SETTINGS, "Seed", Seed);
        TechLevel        = spawn_ini.Get_Int(SETTINGS, "TechLevel", TechLevel);
        IsCampaign       = spawn_ini.Get_Bool(SETTINGS, "IsSinglePlayer", IsCampaign);
        Tournament       = spawn_ini.Get_Int(SETTINGS, "Tournament", Tournament);
        WOLGameID        = spawn_ini.Get_Int(SETTINGS, "GameID", WOLGameID);
        /* ScenarioName */ spawn_ini.Get_String(SETTINGS, "Scenario", ScenarioName, ScenarioName, sizeof(ScenarioName));
        /* MapHash      */ spawn_ini.Get_String(SETTINGS, "MapHash", MapHash, MapHash, sizeof(MapHash));
        spawn_ini.Get_String(SETTINGS, "UIMapName", UIMapName, UIMapName, sizeof(UIMapName));
    }

    { // Network Options
        Protocol         = spawn_ini.Get_Int(SETTINGS, "Protocol", Protocol);
        FrameSendRate    = spawn_ini.Get_Int(SETTINGS, "FrameSendRate", FrameSendRate);
        ReconnectTimeout = spawn_ini.Get_Int(SETTINGS, "ReconnectTimeout", ReconnectTimeout);
        ConnTimeout      = spawn_ini.Get_Int(SETTINGS, "ConnTimeout", ConnTimeout);
        MaxAhead         = spawn_ini.Get_Int(SETTINGS, "MaxAhead", MaxAhead);
        PreCalcMaxAhead  = spawn_ini.Get_Int(SETTINGS, "PreCalcMaxAhead", PreCalcMaxAhead);
        MaxLatencyLevel  = (byte)spawn_ini.Get_Int(SETTINGS, "MaxLatencyLevel", (int)MaxLatencyLevel);
    }

    { // Tunnel Options
        TunnelId   = spawn_ini.Get_Int(SETTINGS, "Port", TunnelId);
        ListenPort = spawn_ini.Get_Int(SETTINGS, "Port", ListenPort);

        static char const* const TUNNEL = "Tunnel";
        TunnelPort = spawn_ini.Get_Int(TUNNEL, "Port", TunnelPort);
    }

    // Players Options
    for (char i = 0; i < (char)std::size(Players); ++i)
    {
        (&Players[i])->Read_INI(spawn_ini, i);
        (&Houses[i])->Read_INI(spawn_ini, i);
    }

    // Extended Options
    Firestorm                = spawn_ini.Get_Bool(SETTINGS, "Firestorm", Firestorm);
    QuickMatch               = spawn_ini.Get_Bool(SETTINGS, "QuickMatch", QuickMatch);
    SkipScoreScreen          = spawn_ini.Get_Bool(SETTINGS, "SkipScoreScreen", SkipScoreScreen);
    WriteStatistics          = spawn_ini.Get_Bool(SETTINGS, "WriteStatistics", WriteStatistics);
    AINamesByDifficulty      = spawn_ini.Get_Bool(SETTINGS, "AINamesByDifficulty", AINamesByDifficulty);
    ContinueWithoutHumans    = spawn_ini.Get_Bool(SETTINGS, "ContinueWithoutHumans", ContinueWithoutHumans);
    DefeatedBecomesObserver  = spawn_ini.Get_Bool(SETTINGS, "DefeatedBecomesObserver", DefeatedBecomesObserver);
}

constexpr char* PlayerSectionArray[8] = {
    "Settings",
    "Other1",
    "Other2",
    "Other3",
    "Other4",
    "Other5",
    "Other6",
    "Other7"
};

constexpr char* MultiTagArray[8] = {
    "Multi1",
    "Multi2",
    "Multi3",
    "Multi4",
    "Multi5",
    "Multi6",
    "Multi7",
    "Multi8"
};

constexpr char* AlliancesSectionArray[8] = {
    "Multi1_Alliances",
    "Multi2_Alliances",
    "Multi3_Alliances",
    "Multi4_Alliances",
    "Multi5_Alliances",
    "Multi6_Alliances",
    "Multi7_Alliances",
    "Multi8_Alliances"
};

constexpr char* AlliancesTagArray[8] = {
    "HouseAllyOne",
    "HouseAllyTwo",
    "HouseAllyThree",
    "HouseAllyFour",
    "HouseAllyFive",
    "HouseAllySix",
    "HouseAllySeven",
    "HouseAllyEight"
};

void SpawnerConfig::PlayerConfig::Read_INI(CCINIClass& spawn_ini, int index)
{
    if (index >= 8)
        return;

    const char* SECTION = PlayerSectionArray[index];
    const char* MULTI_TAG = MultiTagArray[index];

    if (spawn_ini.Is_Present(SECTION))
    {
        this->IsHuman = true;
        this->Difficulty = -1;

        spawn_ini.Get_String(SECTION, "Name", this->Name, this->Name, sizeof(this->Name));

        this->Color       = spawn_ini.Get_Int(SECTION, "Color", this->Color);
        this->House       = spawn_ini.Get_Int(SECTION, "Side", this->House);

        spawn_ini.Get_String(SECTION, "Ip", this->Ip, this->Ip, sizeof(this->Ip));
        this->Port       = spawn_ini.Get_Int(SECTION, "Port", this->Port);
    }
    else if (!IsHuman)
    {
        this->Color       = spawn_ini.Get_Int("HouseColors", MULTI_TAG, this->Color);
        this->House       = spawn_ini.Get_Int("HouseCountries", MULTI_TAG, this->House);
        this->Difficulty  = spawn_ini.Get_Int("HouseHandicaps", MULTI_TAG, this->Difficulty);
    }
}

void SpawnerConfig::HouseConfig::Read_INI(CCINIClass& spawn_ini, int index)
{
    if (index >= 8)
        return;

    const char* ALLIANCES = AlliancesSectionArray[index];
    const char* MULTI_TAG = MultiTagArray[index];

    this->IsSpectator     = spawn_ini.Get_Bool("IsSpectator", MULTI_TAG, this->IsSpectator);
    this->SpawnLocation   = spawn_ini.Get_Int("SpawnLocations", MULTI_TAG, SpawnLocation);

    if (spawn_ini.Is_Present(ALLIANCES))
    {
        for(int i = 0; i < 8; i++)
            this->Alliances[i] = spawn_ini.Get_Int(ALLIANCES, AlliancesTagArray[i], this->Alliances[i]);
    }
}
