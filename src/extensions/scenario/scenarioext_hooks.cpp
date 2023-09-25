/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SCENARIOEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ScenarioClass.
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
#include "scenarioext_hooks.h"
#include "scenarioext_init.h"
#include "scenarioext.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "multiscore.h"
#include "scenario.h"
#include "session.h"
#include "rules.h"
#include "ccfile.h"
#include "ccini.h"
#include "addon.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Process additions to the Rules data from the input file.
 * 
 *  @author: CCHyper
 */
static bool Rule_Addition(const char *fname, bool with_digest = false)
{
    CCFileClass file(fname);
    if (!file.Is_Available()) {
        return false;
    }

    CCINIClass ini;
    if (!ini.Load(file, with_digest)) {
        return false;
    }

    DEBUG_INFO("Calling Rule->Addition() with \"%s\" overrides.\n", fname);

    Rule->Addition(ini);

    return true;
}


/**
 *  #issue-#671
 * 
 *  Add loading of MPLAYER.INI to override Rules data for multiplayer games.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Read_Scenario_INI_MPlayer_INI_Patch)
{
    if (Session.Type != GAME_NORMAL && Session.Type != GAME_WDT) {

        /**
         *  Process the multiplayer ini overrides.
         */
        Rule_Addition("MPLAYER.INI");
        if (Addon_Enabled(ADDON_FIRESTORM)) { 
            Rule_Addition("MPLAYERFS.INI");
        }

    }

    /**
     *  Update the progress screen bars.
     */
    Session.Loading_Callback(42);

    /**
     *  Stolen bytes/code.
     */
    Call_Back();

    JMP(0x005DD8DA);
}


/**
 *  #issue-522
 * 
 *  These patches make the multiplayer score screen to honour the value of
 *  "IsSkipScore" from ScenarioClass.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Do_Win_Skip_MPlayer_Score_Screen_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    ++Session.GamesPlayed;

    if (!Scen->IsSkipScore) {
        MultiScore::Presentation();
    }

    JMP(0x005DC9DF);
}

DECLARE_PATCH(_Do_Lose_Skip_MPlayer_Score_Screen_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    ++Session.GamesPlayed;

    if (!Scen->IsSkipScore) {
        MultiScore::Presentation();
    }

    JMP(0x005DCD9D);
}


/**
 *  Reimplements a part of scenario [Basic] section reading to support
 *  reading in new options added to the section.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_Read_Basic_Section_Read_New_Options_Patch)
{
    GET_REGISTER_STATIC(CCINIClass*, scenario_ini, edi);

    Read_Scenario_Read_Basic_Section_Options(*scenario_ini);

    JMP_REG(ecx, 0x005E04E8);
}


/**
 *  Main function for patching the hooks.
 */
void ScenarioClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    ScenarioClassExtension_Init();

    /**
     *  For compatibility with the TS Client we need to remove
     *  these two reimplementations as they conflict with the spawner.
     */
#if !defined(TS_CLIENT)
    /**
     *  Hooks in the new Assign_Houses() function.
     * 
     *  @author: CCHyper
     */
    Patch_Call(0x005E08E3, &ScenarioClassExtension::Assign_Houses);

    /**
     *  #issue-338
     * 
     *  Hooks in the new Create_Units() function.
     * 
     *  @author: CCHyper
     */
    Patch_Call(0x005DD320, &ScenarioClassExtension::Create_Units);
#endif

    Patch_Jump(0x005DC9D4, &_Do_Win_Skip_MPlayer_Score_Screen_Patch);
    Patch_Jump(0x005DCD92, &_Do_Lose_Skip_MPlayer_Score_Screen_Patch);
    Patch_Jump(0x005DD8D5, &_Read_Scenario_INI_MPlayer_INI_Patch);
    Patch_Jump(0x005E03EB, &_Read_Basic_Section_Read_New_Options_Patch);
}
