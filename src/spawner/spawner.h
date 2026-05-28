/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Multiplayer spawner class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once


#include "spawnerconfig.h"
#include "vinifera_globals.h"


/**
 *  This class contains all logic for spawning players in-game (usually via the client).
 */
class Spawner
{
public:
    Spawner() = delete;

    static bool Init();
    static bool Start_Game();

private:
    static bool Start_Scenario(char* scenario_name);
    static bool Load_Game(const char* file_name);

    static int Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty(int difficulty);

    static bool Init_Session(char* scenario_name);
    static void Init_Network();
    static bool Reconcile_Players();

private:
    static bool HasSpawned;
    static std::unique_ptr<SpawnerConfig> Config;
};
