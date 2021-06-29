/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INITEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains any hooks for the game init process.
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
#include "initext_hooks.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "special.h"
#include "playmovie.h"
#include "ccfile.h"
#include "newmenu.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


static bool CCFile_Is_Available(const char *filename)
{
    return CCFileClass(filename).Is_Available();
}


/**
 *  #issue-478
 * 
 *  
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Game_Skip_Startup_Movies_Patch)
{
    if (Vinifera_SkipStartupMovies) {
        DEBUG_INFO("Skipping startup movies.\n");
        goto skip_loading_screen;
    }

    if (Special.IsFromInstall) {
        DEBUG_GAME("Playing first time intro sequence.\n");
        Play_Movie("EVA.VQA", THEME_NONE, true, true, true);
    }

    if (!Vinifera_SkipWWLogoMovie) {
        DEBUG_GAME("Playing startup movies.\n");
        Play_Movie("WWLOGO.VQA", THEME_NONE, true, true, true);
    } else {
        DEBUG_INFO("Skipping startup movie.\n");
    }

    if (!NewMenuClass::Get()) {
        if (CCFile_Is_Available("FS_TITLE.VQA")) {
            Play_Movie("FS_TITLE.VQA", THEME_NONE, true, true, true);
        } else {
            Play_Movie("STARTUP.VQA", THEME_NONE, true, true, true);
        }
    }

loading_screen:
    _asm { or ebx, 0xFFFFFFFF }
    JMP(0x004E0848);

skip_loading_screen:
    JMP(0x004E084D);
}


/**
 *  Main function for patching the hooks.
 */
void GameInit_Hooks()
{
    Patch_Jump(0x004E0786, &_Init_Game_Skip_Startup_Movies_Patch);
}
