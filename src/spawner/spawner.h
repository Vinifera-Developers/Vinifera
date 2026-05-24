/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNER.H
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
    static void Apply_Scenario_Values();

private:
    static bool Start_Scenario(char* scenario_name);
    static bool Load_Game(const char* file_name);

    static int Spawner_Config_AI_Difficulty_To_Game_AI_Difficulty(int difficulty);

    static bool Init_Session(char* scenario_name);
    static void Init_Network();
    static bool Reconcile_Players();

    static void Init_UI();
    static void Prepare_Screen();

private:
    static bool HasSpawned;
    static std::unique_ptr<SpawnerConfig> Config;
};
