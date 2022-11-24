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
#include "house.h"
#include "housetype.h"
#include "side.h"
#include "infantrytype.h"
#include "unittype.h"
#include "aircrafttype.h"
#include "buildingtype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  This is a real kludge to work around the fact that in Tiberian Sun, each
 *  of the side asset mix files contain their own versions of cameo artwork
 *  for the opposing side.
 * 
 *  We call Call_Back() after each iteration to ensure the network connection
 *  does not drop between clients in multiplayer games.
 * 
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Reload_Cameo_For_Side()
{
    char buffer[32];

    DEBUG_INFO("Reloading TechnoType cameos.\n");

    Call_Back();

    for (int index = 0; index < TechnoTypes.Count(); ++index) {

        TechnoTypeClass *ttype = TechnoTypes[index];
        std::snprintf(buffer, sizeof(buffer), "%s.SHP", ttype->CameoFilename);

        const ShapeFileStruct *cameodata = MFCC::RetrieveT<const ShapeFileStruct>(buffer);

        if (cameodata == nullptr) continue;

        if (ttype->CameoData != nullptr && cameodata == ttype->CameoData) continue;

        DEBUG_INFO("  Reloaded cameo for %s \"%s\" (%s).\n", Name_From_RTTI(RTTIType(ttype->What_Am_I())), ttype->Name(), buffer);
        ttype->CameoData = cameodata;
    }

    DEBUG_INFO("Finished reloading TechnoType cameos.\n");

    Call_Back();

    return true;
}


/**
 *  Process the rules and scenario rules overrides.
 *
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Rules_Process(CCINIClass &ini)
{
    /**
     *  Initialise and process rules.
     */
    DEBUG_INFO("Initializeing Rules\n");
    Rule->Initialize(*RuleINI);

    Session.Loading_Callback(35);

    Call_Back();

    /**
     *  Read global variables from rules.
     */
    DEBUG_INFO("Calling Scen->Read_Global_INI(*RuleINI)...\n");
    Scen->Read_Global_INI(*RuleINI);

    Call_Back();

    /**
     *  Process rules scenario overrides.
     */
    DEBUG_INFO("Calling Rule->Addition() with scenario overrides.\n");
    Rule->Addition(ini);
    DEBUG_INFO("Finished Rule->Addition() with scenario overrides.\n");

    Session.Loading_Callback(45);

    /**
     *  Read in scenario house types.
     */
    if (Session.Type == GAME_NORMAL) {
        DEBUG_INFO("Calling HouseClass::Read_Scenario_INI()...\n");
        HouseClass::Read_Scenario_INI(ini);
    }

    Session.Loading_Callback(50);

    /**
     *  Read scenario basic.
     */
    DEBUG_INFO("Calling Scen->Read_INI()...\n");
    if (!Scen->Read_INI(ini)) {
        return false;
    }

    return true;
}


/**
 *  Initialise the player value which is later used to load the side assets.
 *
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Init_Side(CCINIClass &ini)
{
    HouseTypeClass *housetype = nullptr;

    /**
     *  If this is a campaign session, load the house from the "Player" value.
     */
    if (Session.Type == GAME_NORMAL) {

        char buffer[32];
        ini.Get_String("Basic", "Player", "GDI", buffer, sizeof(buffer));

#if 0
        /**
         *  Original game code.
         */
        bool is_gdi = strcmpi(buffer, "GDI") == 0;
        Scen->IsGDI = is_gdi;
        Scen->SpeechSide = is_gdi ? SIDE_GDI : SIDE_NOD;
#endif

        /**
         *  Fetch the houses side type and use this to decide which assets to load.
         */
        housetype = (HouseTypeClass *)HouseTypeClass::As_Pointer(buffer);

        Scen->IsGDI = (unsigned char)housetype->Side & 0xFF;
        Scen->SpeechSide = housetype->Side;

        ASSERT_FATAL_PRINT(housetype != nullptr, "Invalid \"Player\" value in [Basic] section!");

        /**
         *  Read speech side override.
         */
        Scen->SpeechSide = ini.Get_SideType("Basic", "SpeechSide", Scen->SpeechSide);

        ASSERT_FATAL_PRINT(Scen->SpeechSide != SIDE_NONE && Scen->SpeechSide < Sides.Count(), "Invalid \"SpeechSide\" value in [Basic] section!");

    } else {

#if 0
        /**
         *  Original game code.
         */
        Scen->IsGDI = Session.IsGDI;
        Scen->SpeechSide = Session.IsGDI ? SIDE_GDI : SIDE_NOD;
#endif

        /**
         *  Fetch the houses side type and use this to decide which assets to load.
         */
        housetype = (HouseTypeClass *)HouseTypeClass::As_Pointer(HousesType(PlayerPtr->Class->House));
        ASSERT_FATAL_PRINT(housetype != nullptr, "Invalid multiplayer house!");

        Scen->IsGDI = (unsigned char)housetype->Side & 0xFF;
        Scen->SpeechSide = housetype->Side;

    }

    return true;
}


/**
 *  Initialise the player side assets (sidebar, speech, etc).
 *
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Prep_For_Side()
{
    /**
     *  Fetch the houses side type and use this to decide which assets to load.
     */
    HouseTypeClass *housetype = (HouseTypeClass *)HouseTypeClass::As_Pointer(HousesType(Scen->IsGDI));

#ifndef NDEBUG
    DEV_DEBUG_INFO("About to prepare for...\n");
    DEV_DEBUG_INFO("  House \"%s\" (%d) with Side \"%s\" (%d)\n",
        housetype->Name(), housetype->Get_Heap_ID(),
        Sides[housetype->Side]->Name(), Sides[housetype->Side]->Get_Heap_ID());

    DEV_DEBUG_INFO("Side info:\n");
    for (int i = 0; i < Sides.Count(); ++i) {
        SideClass *side = Sides[i];
        DEV_DEBUG_INFO("  Side \"%s\" (%d), Houses.Count %d\n", side->IniName, i, side->Houses.Count());
        for (int i = 0; i < side->Houses.Count(); ++i) {
            DEV_DEBUG_INFO("    Houses %d = %s (%d)\n", i, HouseTypes[side->Houses[i]]->Name(), side->Houses[i]);
        }
    }
#endif

    DEBUG_INFO("Calling Prep_For_Side()...\n");
    if (!Prep_For_Side(housetype->Side)) {

        DEBUG_WARNING("Prep_For_Side(%d) failed! Trying with side 0...\n", housetype->Side);

        /**
         *  Try once again but with the Side 0 (GDI) assets.
         */
        if (!Prep_For_Side(SIDE_GDI)) {
            DEBUG_ERROR("Prep_For_Side() failed!\n");
            return false;
        }
    }

    DEBUG_INFO("Calling Prep_Speech_For_Side()...\n");
    if (!Prep_Speech_For_Side(Scen->SpeechSide)) {

        DEBUG_WARNING("Prep_Speech_For_Side(%d) failed! Trying with side 0...\n", Scen->SpeechSide);

        /**
         *  Try once again but with the Side 0 (GDI) assets.
         */
        if (!Prep_Speech_For_Side(SIDE_GDI)) {
            DEBUG_ERROR("Prep_Speech_For_Side() failed!\n");
            return false;
        }
    }

    return true;
}


/**
 *  #issue-218
 *
 *  This patch replaces a fairly large chunk of code in Read_Scenario_INI that
 *  read/initialised the player house, loaded the side assets, then processed the
 *  rules data and scenario overrides.
 * 
 *  Because we now abuse ScenarioClass::IsGDI and SessionClass::IsGDI to store
 *  the player house, we need to reorder this chunk of code to we are reading
 *  the house types before we load the side assets, this allows us to use the
 *  Side member of HouseTypeClass to pick the correct assets.
 * 
 *  There is an additional chunk of code that has been added to reload the object
 *  cameos a second time, this is because the loading of the side assets is now
 *  before the rules initialisation, and as a result, the rules data is being
 *  processed before the side assets are declared to the mix file system.
 */
DECLARE_PATCH(_Read_Scenario_INI_Init_Side_Patch)
{
    GET_REGISTER_STATIC(CCINIClass *, ini, ebp);

    /**
     *  The original code flow was;
     *    Read [Basic] -> Player=
     *    Prep_For_Side
     *    Process Rules.
     * 
     *  To support assets for new sides, be now need to move
     *  the rules processing to before the side and asset loading.
     */

    if (!Read_Scenario_INI_Rules_Process(*ini)) {
        goto return_false;
    }

    if (!Read_Scenario_INI_Init_Side(*ini)) {
        goto return_false;
    }

    if (!Read_Scenario_INI_Prep_For_Side()) {
        goto return_false;
    }

    if (!Read_Scenario_INI_Reload_Cameo_For_Side()) {
        goto return_false;
    }

    JMP(0x005DD956);

return_false:
    _asm{ xor al, al }
    JMP_REG(ecx, 0x005DD7AB);
}


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

    /**
     *  #issue-218
     * 
     *  Changes the default value of ScenarioClass 0x1D91 (IsGDI) from "1" to "0". This is
     *  because we now use it as a HouseType index, and need it to default to the first index.
     */
    Patch_Byte(0x005DAFD0+6, 0x00); // +6 skips the opcode.
    Patch_Jump(0x005DD6FB, &_Read_Scenario_INI_Init_Side_Patch);
}
