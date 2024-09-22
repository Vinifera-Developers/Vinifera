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
#include <memory>

class Spawner
{
public:
    static bool Enabled;
    static bool Active;

private:
    static std::unique_ptr<SpawnerConfig> Config;

public:
    static SpawnerConfig* Get_Config()
    {
        return Config.get();
    }

    static void Init();
    static bool Start_Game();

    static void Init_UI();
    static void Prepare_Screen();

private:
    static bool Start_New_Scenario(const char* scenario_name);
    static bool Load_Saved_Game(const char* save_game_name);

    static void Spawner_Init_Network();
    static void Load_Sides_Stuff();
};
