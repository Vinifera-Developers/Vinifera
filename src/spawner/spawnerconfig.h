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
        bool IsHuman;
        FixedString<20> Name;
        PlayerColorType Color;
        HousesType House;
        int Difficulty;
        FixedString<0x20> Ip;
        int Port;
        bool IsObserver;
        int SpawnLocation;
        int Alliances[8];

        PlayerConfig() :
            IsHuman {false}, Name {""}, Color {PCOLOR_NONE}, House {HOUSE_NONE}, Difficulty {-1}, Ip {"0.0.0.0"}, Port {-1},
            IsObserver {false}, SpawnLocation {-2}, Alliances {-1, -1, -1, -1, -1, -1, -1, -1}
        {}

        void Read_Player_INI(CCINIClass& spawn_ini, int index);
        void Read_House_INI(CCINIClass& spawn_ini, int index);
    };

public:
    /**
     *  Game Mode Options
     */
    bool Bases;
    int Credits;
    bool BridgeDestroy;
    bool Crates;
    bool ShortGame;
    bool BuildOffAlly;
    int GameSpeed;
    bool MultiEngineer;
    int UnitCount;
    int AIPlayers;
    int AIDifficulty;
    bool AlliesAllowed;
    bool HarvesterTruce;
    bool FogOfWar;
    bool MCVRedeploy;

    /**
     *  Savegame Options
     */
    bool LoadSaveGame;
    FixedString<60> SaveGameName;
    int AutoSaveInterval;
    int NextAutoSaveNumber;

    /**
     *  Scenario Options
     */
    int Seed;
    int TechLevel;
    bool IsCampaign;
    CampaignType CampaignID;
    DiffType CampaignDifficulty;
    DiffType CampaignCDifficulty;
    int Tournament;
    unsigned int WOLGameID;
    FixedString<260> ScenarioName;
    FixedString<0xff> MapHash;
    FixedString<44> UIMapName;
    bool PlayMoviesInMultiplayer;

    /**
     *  Network Options
     */
    int Protocol;
    int FrameSendRate;
    int ReconnectTimeout;
    int ConnTimeout;
    int MaxAhead;
    int PreCalcMaxAhead;
    LatencyLevelEnum MaxLatencyLevel;

    /**
     *  Tunnel Options
     */
    int TunnelId;
    FixedString<0x20> TunnelIp;
    int TunnelPort;
    int ListenPort;

    /**
     *  Player Options
     *  Sorted after Read_INI: humans first (by color), then AIs.
     */
    PlayerConfig Players[8];
    int HumanPlayers;
    int LocalPlayerIndex;

    /**
     *  Extended Options
     */
    bool Firestorm;
    bool QuickMatch;
    bool SkipScoreScreen;
    bool WriteStatistics;
    bool AINamesByDifficulty;
    bool CoachMode;
    bool AutoSurrender;
    bool AttackNeutralUnits;
    bool ScrapMetal;
    FixedString<PATH_MAX> CustomLoadScreen;
    TPoint2D<int> CustomLoadScreenPos;
    bool ContinueWithoutHumans;

    SpawnerConfig() :
        Bases {true},
        Credits {10000},
        BridgeDestroy {true},
        Crates {false},
        ShortGame {false},
        BuildOffAlly {false},
        GameSpeed {0},
        MultiEngineer {false},
        UnitCount {0},
        AIPlayers {0},
        AIDifficulty {1},
        AlliesAllowed {false},
        HarvesterTruce {false},
        FogOfWar {false},
        MCVRedeploy {true},

        LoadSaveGame {false},
        SaveGameName {""},
        AutoSaveInterval {1},
        NextAutoSaveNumber {0},

        Seed {0},
        TechLevel {10},
        IsCampaign {false},
        CampaignID {CAMPAIGN_NONE},
        CampaignDifficulty {DIFF_NORMAL},
        CampaignCDifficulty {DIFF_NORMAL},
        Tournament {0},
        WOLGameID {0xDEADBEEF},
        ScenarioName {"spawnmap.ini"},
        MapHash {""},
        UIMapName {""},
        PlayMoviesInMultiplayer {false},

        Protocol {2},
        FrameSendRate {4},
        ReconnectTimeout {2400},
        ConnTimeout {3600},
        MaxAhead {-1},
        PreCalcMaxAhead {0},
        MaxLatencyLevel {static_cast<LatencyLevelEnum>(0xFF)} ,

        TunnelId {0},
        TunnelIp {"0.0.0.0"},
        TunnelPort {0},
        ListenPort {1234} ,

        Players {PlayerConfig(), PlayerConfig(), PlayerConfig(), PlayerConfig(),
                 PlayerConfig(), PlayerConfig(), PlayerConfig(), PlayerConfig()},
        HumanPlayers(0),
        LocalPlayerIndex(0),

        Firestorm {true},
        QuickMatch {false},
        SkipScoreScreen {false},
        WriteStatistics {false},
        AINamesByDifficulty {false},
        CoachMode {false},
        AutoSurrender {true},
        AttackNeutralUnits {false},
        ScrapMetal {false},
        CustomLoadScreen {""},
        CustomLoadScreenPos {},
        ContinueWithoutHumans {false}
    {
    }

    void Read_INI(CCINIClass& spawn_ini);
};
