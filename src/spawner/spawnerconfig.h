/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Configuration of the multiplayer spawner.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once

#include "abstractext.h"
#include "stringid.h"
#include "tibsun_defines.h"
#include "latencylevel.h"
#include "vinifera_globals.h"

class CCINIClass;


/**
 *  This class contains all the configuration for the spawner, usually read from SPAWN.INI.
 */
class SpawnerConfig
{
    /**
     *  Combined player and house configuration for a single game slot.
     *  After Read_INI, the array is sorted: humans first (by color), then AIs.
     */
    struct PlayerConfig {
        bool IsHuman = false;
        FixedString<20> Name {""};
        PlayerColorType Color = PCOLOR_NONE;
        HousesType House = HOUSE_NONE;
        int Difficulty = -1;
        FixedString<0x20> Ip {"0.0.0.0"};
        int Port = -1;
        bool IsObserver = false;
        int SpawnLocation = -2;
        int Alliances[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

        void Read_Player_INI(CCINIClass& spawn_ini, int index);
        void Read_House_INI(CCINIClass& spawn_ini, int index);
    };

public:
    /**
     *  Game Mode Options
     */
    bool Bases = true;
    int Credits = 10000;
    bool BridgeDestroy = true;
    bool Crates = false;
    bool ShortGame = false;
    bool BuildOffAlly = false;
    int GameSpeed = 0;
    bool MultiEngineer = false;
    int UnitCount = 0;
    int AIPlayers = 0;
    int AIDifficulty = 1;
    bool AlliesAllowed = false;
    bool HarvesterTruce = false;
    bool FogOfWar = false;
    bool MCVRedeploy = true;
    bool IsHost = false;

    /**
     *  Savegame Options
     */
    bool LoadSaveGame = false;
    FixedString<60> SaveGameName {""};
    int AutoSaveInterval = 10800;
    int NextCampaignAutoSaveNumber = 0;
    int NextSkirmishAutoSaveNumber = 0;

    /**
     *  Scenario Options
     */
    int Seed = 0;
    int TechLevel = 10;
    bool IsCampaign = false;
    CampaignType CampaignID = CAMPAIGN_NONE;
    DiffType CampaignDifficulty = DIFF_NORMAL;
    DiffType CampaignCDifficulty = DIFF_NORMAL;
    int Tournament = 0;
    unsigned int WOLGameID = 0xDEADBEEF;
    FixedString<260> ScenarioName {"spawnmap.ini"};
    FixedString<0xff> MapHash {""};
    FixedString<44> MapName {""};
    bool PlayMoviesInMultiplayer = false;

    /**
     *  Network Options
     */
    int Protocol = 2;
    int FrameSendRate = 4;
    int ReconnectTimeout = 2400;
    int ConnTimeout = 3600;
    int MaxAhead = -1;
    int PreCalcMaxAhead = 0;
    LatencyLevelEnum MaxLatencyLevel = static_cast<LatencyLevelEnum>(0xFF);

    /**
     *  Tunnel Options
     */
    int TunnelId = 0;
    FixedString<0x20> TunnelIp {"0.0.0.0"};
    int TunnelPort = 0;
    int ListenPort = 1234;

    /**
     *  Player Options
     *  Sorted after Read_INI: humans first (by color), then AIs.
     */
    PlayerConfig Players[8] {};
    int HumanPlayers = 0;
    int LocalPlayerIndex = 0;

    /**
     *  Extended Options
     */
    bool Firestorm = true;
    bool QuickMatch = false;
    bool SkipScoreScreen = false;
    bool WriteStatistics = false;
    bool AINamesByDifficulty = false;
    bool CoachMode = false;
    bool AutoSurrender = true;
    bool AttackNeutralUnits = false;
    bool ScrapMetal = false;
    FixedString<PATH_MAX> CustomLoadScreen {""};
    TPoint2D<int> CustomLoadScreenPos {0, 0};
    bool ContinueWithoutHumans = false;
    FixedString<64> DifficultyName {""};

    int GlobalFlags[MAX_ENVIRONMENT_GLOBALS];

    void Read_INI(CCINIClass& spawn_ini);
};
