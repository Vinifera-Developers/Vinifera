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
    static bool Is_Active() { return Config != nullptr; }

private:
    static bool Start_Scenario(char* scenario_name);
    static bool Load_Game(const char* file_name);

    static int Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty(int difficulty);
    static bool Validate_Config();

    static bool Init_Session(char* scenario_name);
    static bool Init_Network();

private:
    static bool HasSpawned;
    static std::unique_ptr<SpawnerConfig> Config;
};
