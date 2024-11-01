/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNERCONFIG.CPP
 *
 *  @author        Belonit, ZivDero
 *
 *  @brief         Configuration of the multiplayer spawner.
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

    /**
     *  Savegame Options
     */
    LoadSaveGame       = spawn_ini.Get_Bool(SETTINGS, "LoadSaveGame", LoadSaveGame);
    /* SaveGameName */   spawn_ini.Get_String(SETTINGS, "SaveGameName", SaveGameName, SaveGameName, sizeof(SaveGameName));
    AutoSaveInterval   = spawn_ini.Get_Int(SETTINGS, "AutoSaveGame", AutoSaveInterval);
    NextAutoSaveNumber = spawn_ini.Get_Int(SETTINGS, "NextSPAutoSaveId", NextAutoSaveNumber + 1) - 1; // Subtract 1 since our autosaves are 0-based internally

    /**
     *  Scenario Options
     */
    Seed                    = spawn_ini.Get_Int(SETTINGS, "Seed", Seed);
    TechLevel               = spawn_ini.Get_Int(SETTINGS, "TechLevel", TechLevel);
    IsCampaign              = spawn_ini.Get_Bool(SETTINGS, "IsSinglePlayer", IsCampaign);
    CampaignID              = spawn_ini.Get_Int(SETTINGS, "CampaignID", CampaignID);
    CampaignDifficulty      = spawn_ini.Get_Int(SETTINGS, "DifficultyModeHuman", CampaignDifficulty);
    CampaignCDifficulty     = spawn_ini.Get_Int(SETTINGS, "DifficultyModeComputer", CampaignCDifficulty);
    Tournament              = spawn_ini.Get_Int(SETTINGS, "Tournament", Tournament);
    WOLGameID               = spawn_ini.Get_Int(SETTINGS, "GameID", WOLGameID);
    /* ScenarioName      */   spawn_ini.Get_String(SETTINGS, "Scenario", ScenarioName, ScenarioName, sizeof(ScenarioName));
    /* MapHash           */   spawn_ini.Get_String(SETTINGS, "MapHash", MapHash, MapHash, sizeof(MapHash));
    /* UIMapName         */   spawn_ini.Get_String(SETTINGS, "UIMapName", UIMapName, UIMapName, sizeof(UIMapName));
    PlayMoviesInMultiplayer = spawn_ini.Get_Bool(SETTINGS, "PlayMoviesInMultiplayer", PlayMoviesInMultiplayer);

    /**
     *  Network Options
     */
    Protocol         = spawn_ini.Get_Int(SETTINGS, "Protocol", Protocol);
    FrameSendRate    = spawn_ini.Get_Int(SETTINGS, "FrameSendRate", FrameSendRate);
    ReconnectTimeout = spawn_ini.Get_Int(SETTINGS, "ReconnectTimeout", ReconnectTimeout);
    ConnTimeout      = spawn_ini.Get_Int(SETTINGS, "ConnTimeout", ConnTimeout);
    MaxAhead         = spawn_ini.Get_Int(SETTINGS, "MaxAhead", MaxAhead);
    PreCalcMaxAhead  = spawn_ini.Get_Int(SETTINGS, "PreCalcMaxAhead", PreCalcMaxAhead);
    MaxLatencyLevel  = spawn_ini.Get_Int(SETTINGS, "MaxLatencyLevel", MaxLatencyLevel);

    /**
     *  Tunnel Options
     */
    TunnelId     = spawn_ini.Get_Int(SETTINGS, "Port", TunnelId);
    ListenPort   = spawn_ini.Get_Int(SETTINGS, "Port", ListenPort);
    /* TunnelIp */ spawn_ini.Get_String(TUNNEL, "Ip", TunnelIp, TunnelIp, sizeof(TunnelIp));
    TunnelPort   = spawn_ini.Get_Int(TUNNEL, "Port", TunnelPort);

    /**
     *  Player and House Options
     */
    for (int i = 0; i < std::size(Players); ++i)
    {
        Players[i].Read_INI(spawn_ini, i);
        if (Players[i].IsHuman)
            HumanPlayers++;

        Houses[i].Read_INI(spawn_ini, i);
    }

    /**
     *  Extended Options
     */
    Firestorm                = spawn_ini.Get_Bool(SETTINGS, "Firestorm", Firestorm);
    QuickMatch               = spawn_ini.Get_Bool(SETTINGS, "QuickMatch", QuickMatch);
    SkipScoreScreen          = spawn_ini.Get_Bool(SETTINGS, "SkipScoreScreen", SkipScoreScreen);
    WriteStatistics          = spawn_ini.Get_Bool(SETTINGS, "WriteStatistics", WriteStatistics);
    AINamesByDifficulty      = spawn_ini.Get_Bool(SETTINGS, "AINamesByDifficulty", AINamesByDifficulty);
    CoachMode                = spawn_ini.Get_Bool(SETTINGS, "CoachMode", CoachMode);
    AutoSurrender            = spawn_ini.Get_Bool(SETTINGS, "AutoSurrender", AutoSurrender);
    AttackNeutralUnits       = spawn_ini.Get_Bool(SETTINGS, "AttackNeutralUnits", AttackNeutralUnits);
    ScrapMetal               = spawn_ini.Get_Bool(SETTINGS, "ScrapMetal", ScrapMetal);
    /* CustomLoadScreen   */   spawn_ini.Get_String(SETTINGS, "CustomLoadScreen", CustomLoadScreen, sizeof(CustomLoadScreen));
    CustomLoadScreenPos      = spawn_ini.Get_Point(SETTINGS, "CustomLoadScreenPos", CustomLoadScreenPos);
    ContinueWithoutHumans    = spawn_ini.Get_Bool(SETTINGS, "ContinueWithoutHumans", ContinueWithoutHumans);
}


static constexpr char* PlayerSectionArray[8] = {
    "Settings",
    "Other1",
    "Other2",
    "Other3",
    "Other4",
    "Other5",
    "Other6",
    "Other7"
};


static constexpr char* MultiTagArray[8] = {
    "Multi1",
    "Multi2",
    "Multi3",
    "Multi4",
    "Multi5",
    "Multi6",
    "Multi7",
    "Multi8"
};


static constexpr char* AlliancesSectionArray[8] = {
    "Multi1_Alliances",
    "Multi2_Alliances",
    "Multi3_Alliances",
    "Multi4_Alliances",
    "Multi5_Alliances",
    "Multi6_Alliances",
    "Multi7_Alliances",
    "Multi8_Alliances"
};


static constexpr char* AlliancesTagArray[8] = {
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
void SpawnerConfig::PlayerConfig::Read_INI(CCINIClass& spawn_ini, int index)
{
    if (index >= MAX_PLAYERS)
        return;

    const char* SECTION = PlayerSectionArray[index];
    const char* MULTI_TAG = MultiTagArray[index];

    if (spawn_ini.Is_Present(SECTION))
    {
        IsHuman = true;
        Difficulty = -1;

        spawn_ini.Get_String(SECTION, "Name", Name, Name, sizeof(Name));

        Color       = spawn_ini.Get_Int(SECTION, "Color", Color);
        House       = spawn_ini.Get_Int(SECTION, "Side", House);

        spawn_ini.Get_String(SECTION, "Ip", Ip, Ip, sizeof(Ip));
        Port        = spawn_ini.Get_Int(SECTION, "Port", Port);
    }
    else if (!IsHuman)
    {
        Color       = spawn_ini.Get_Int("HouseColors", MULTI_TAG, Color);
        House       = spawn_ini.Get_Int("HouseCountries", MULTI_TAG, House);
        Difficulty  = spawn_ini.Get_Int("HouseHandicaps", MULTI_TAG, Difficulty);
    }
}


/**
 *  Reads house's config from the INI.
 *
 *  @author: Belonit, ZivDero
 */
void SpawnerConfig::HouseConfig::Read_INI(CCINIClass& spawn_ini, int index)
{
    if (index >= MAX_PLAYERS)
        return;

    const char* ALLIANCES = AlliancesSectionArray[index];
    const char* MULTI_TAG = MultiTagArray[index];

    IsObserver      = spawn_ini.Get_Bool("IsSpectator", MULTI_TAG, IsObserver);
    SpawnLocation   = spawn_ini.Get_Int("SpawnLocations", MULTI_TAG, SpawnLocation);

    /**
     *  The client might pass these to indicate that this is an observer.
     */
    if (SpawnLocation == -1 || SpawnLocation == 90)
    {
        IsObserver = true;
        SpawnLocation = -1;
    }

    /**
     *  Reset any weird values we might receive as input.
     */
    if (SpawnLocation < 0 || SpawnLocation > MAX_PLAYERS - 1)
        SpawnLocation = -1;

    if (spawn_ini.Is_Present(ALLIANCES))
    {
        for(int i = 0; i < 8; i++)
            Alliances[i] = spawn_ini.Get_Int(ALLIANCES, AlliancesTagArray[i], Alliances[i]);
    }
}
