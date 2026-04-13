/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_CONFIG.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Sidebar and battle UI configuration implementation.
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

#include "sidebar_config.h"

#include "ccfile.h"
#include "ccini.h"
#include "debughandler.h"

#include <cstring>


/**
 *  Global battle UI configuration instance.
 */
BattleUIConfig UIConfig;


/**
 *  Reads the battle UI configuration from UI.INI.
 *
 *  @author: ZivDero
 */
bool Read_UI_INI()
{
    CCFileClass file("UI.INI");
    CCINIClass ini;

    if (!file.Is_Available()) {
        DEV_DEBUG_WARNING("UI.INI not found, using default sidebar config.\n");
        return false;
    }

    ini.Load(file, false);

    /**
     *  Read sidebar view type.
     */
    char buffer[64];
    ini.Get_String("Sidebar", "ViewType", "Classic", buffer, sizeof(buffer));

    if (_stricmp(buffer, "Tabbed") == 0) {
        UIConfig.Sidebar.ViewType = SIDEBAR_TABBED;
    } else {
        UIConfig.Sidebar.ViewType = SIDEBAR_CLASSIC;
    }

    return true;
}
