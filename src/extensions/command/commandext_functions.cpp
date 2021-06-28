/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMMANDEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended command class.
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
#include "commandext_functions.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "theme.h"
#include "rules.h"
#include "tacticalext.h"
#include "tactical.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Skips to the previous available music track allowed.
 * 
 *  @author: CCHyper
 */
bool Prev_Theme_Command()
{
    ThemeType theme = Theme.What_Is_Playing();

    /**
     *  Iterate backward from the current theme and find the next available
     *  music track we can play.
     */
    while (theme >= THEME_FIRST) {

        --theme;

        if (theme < THEME_FIRST) {
            theme = ThemeType(Theme.Max_Themes());
        }

        if (Theme.Is_Allowed(theme)) {
            break;
        }

    }

    /**
     *  Queue the track for playback. We need to stop the track first
     *  otherwise Queue_Song() will fade the track out.
     */
    Theme.Stop();
    Theme.Queue_Song(theme);

    /**
     *  Print the chosen music track name on the screen.
     */
    if (TacticalExtension) {

        TacticalExtension->InfoTextTimer.Stop();

        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), "Now Playing: %s", Theme.ThemeClass::Full_Name(theme));

        TacticalExtension->InfoTextBuffer = buffer;
        TacticalExtension->IsInfoTextSet = true;

        TacticalExtension->InfoTextPosition = InfoTextPosType::BOTTOM_LEFT;

        //TacticalExtension->InfoTextNotifySound = Rule->OptionsChanged;
        //TacticalExtension->InfoTextNotifySoundVolume = 0.5f;

        TacticalExtension->InfoTextTimer = SECONDS_TO_MILLISECONDS(4);
        TacticalExtension->InfoTextTimer.Start();
    }

    return true;
}


/**
 *  Skips to the next available music track allowed.
 * 
 *  @author: CCHyper
 */
bool Next_Theme_Command()
{
    ThemeType theme = Theme.What_Is_Playing();

    /**
     *  Iterate forward from the current theme and find the next available
     *  music track we can play.
     */
    while (theme < ThemeType(Theme.Max_Themes())) {

        ++theme;

        if (theme >= ThemeType(Theme.Max_Themes())) {
            theme = ThemeType(THEME_FIRST);
        }

        if (Theme.Is_Allowed(theme)) {
            break;
        }

    }

    /**
     *  Queue the track for playback. We need to stop the track first
     *  otherwise Queue_Song() will fade the track out.
     */
    Theme.Stop();
    Theme.Queue_Song(theme);

    /**
     *  Print the chosen music track name on the screen.
     */
    if (TacticalExtension) {

        TacticalExtension->InfoTextTimer.Stop();

        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), "Now Playing: %s", Theme.ThemeClass::Full_Name(theme));

        TacticalExtension->InfoTextBuffer = buffer;
        TacticalExtension->IsInfoTextSet = true;
        
        TacticalExtension->InfoTextPosition = InfoTextPosType::BOTTOM_LEFT;

        //TacticalExtension->InfoTextNotifySound = Rule->OptionsChanged;
        //TacticalExtension->InfoTextNotifySoundVolume = 0.5f;

        TacticalExtension->InfoTextTimer = SECONDS_TO_MILLISECONDS(4);
        TacticalExtension->InfoTextTimer.Start();
    }

    return true;
}
