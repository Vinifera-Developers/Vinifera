/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNERCONFIG.H
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
#pragma once

#include "abstractext.h"
#include "stringid.h"
#include "tibsun_defines.h"
#include "latencylevel.h"

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

    /**
     *  Savegame Options
     */
    bool LoadSaveGame = false;
    FixedString<60> SaveGameName {""};
    int AutoSaveInterval = 1;
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
    TPoint2D<int> CustomLoadScreenPos {};
    bool ContinueWithoutHumans = false;
    FixedString<64> DifficultyName {""};

    void Read_INI(CCINIClass& spawn_ini);
};
