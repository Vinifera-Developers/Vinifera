/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MAINLOOPEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the intercepting Main_Loop().
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
#include "mainloopext_hooks.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "iomap.h"
#include "layer.h"
#include "logic.h"
#include "tactical.h"
#include "session.h"
#include "house.h"
#include "ccfile.h"
#include "ccini.h"
#include "saveload.h"
#include "addon.h"
#include "language.h"
#include "wstring.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Strip the the full filename of its path, returning only the filename.
 *
 *  @author: CCHyper
 */
static Wstring Strip_Scenario_Path(Wstring file_path)
{
    char path[_MAX_PATH];
    char fname[_MAX_FNAME];
    char fext[_MAX_EXT];

    /**
     *  Strip the drive and path (if present) from the filename.
     */
    _splitpath(file_path.Peek_Buffer(), nullptr, nullptr, fname, fext);
    _makepath(path, nullptr, nullptr, fname, fext);

    return path;
}


/**
 *  Fix the descriptions of the original campaign scenarios for the save name.
 *
 *  #NOTE: Currently only handles the EN release!
 * 
 *  @author: CCHyper
 */
static Wstring Fixup_Scenario_Description(Wstring &scen_name, Wstring &scen_desc)
{
   /**
     *  These are the CRC values for the unmodified mission ini files, TS2.03EN.
     */
    static const int Unmodified_MissionINI_CRC = 0xBB88240F;
    static const int Unmodified_Mission1INI_CRC = 0xB4C887B8;

    static int MissionINI_CRC = 0x0;
    static int Mission1INI_CRC = 0x0;

    bool mission_ini_unmodified = false;
    bool mission1_ini_unmodified = false;

    Wstring new_scen_desc;

    /**
     *
     */
    static bool _one_time = false;
    if (!_one_time) {

        char buffer[32];
        CCFileClass file;
        CCINIClass ini;

        /**
         *  x
         */
        file.Set_Name("MISSION.INI");

        ini.Load(file, false);

        MissionINI_CRC = ini.Get_Unique_ID();

        if (Scen->RequiredAddOn == ADDON_FIRESTORM && Addon_Enabled(ADDON_FIRESTORM)) {
            std::snprintf(buffer, sizeof(buffer), "MISSION%d.INI", Scen->RequiredAddOn);
            file.Set_Name(buffer);

            ini.Load(file, false);

            Mission1INI_CRC = ini.Get_Unique_ID();
        }

        _one_time = true;
    }

    DEV_DEBUG_INFO("MissionINI CRC = %lX\n", MissionINI_CRC);
    DEV_DEBUG_INFO("Mission1INI CRC = %lX\n", Mission1INI_CRC);

    /**
     *  Check to see if the ini files have been modified.
     */
    if (MissionINI_CRC == Unmodified_MissionINI_CRC) {
        DEBUG_INFO("MissionINI is unmodified (version 2.03).\n");
        mission_ini_unmodified = true;
    }
    if (Scen->RequiredAddOn == ADDON_FIRESTORM && Addon_Enabled(ADDON_FIRESTORM)) {
        if (Unmodified_Mission1INI_CRC == Unmodified_Mission1INI_CRC) {
            DEBUG_INFO("Mission1INI is unmodified (version 2.03).\n");
            mission1_ini_unmodified = true;
        }
    }

    /**
     *  We need to fix up the campaign mission names with prefixes as they are
     *  not consistent between the mission briefing and the scenario description.
     */

    /**
     *  Fix the Firestorm mission names.
     */
    if (Scen->RequiredAddOn == ADDON_FIRESTORM && Addon_Enabled(ADDON_FIRESTORM) && mission_ini_unmodified) {

        if (scen_name.Compare_No_Case("FSGDI01.MAP") == 0
            && (scen_desc.Contains_String("Recover the Tacitus")
                || scen_desc.Contains_String("Kodiak Down!"))) {

            new_scen_desc = "(Firestorm) GDI 01: Recover the Tacitus";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI02.MAP") == 0
            && scen_desc.Contains_String("Party Crashers")) {

            new_scen_desc = "(Firestorm) GDI 02: Party Crashers";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI03.MAP") == 0
            && (scen_desc.Contains_String("Quell the Civilian Riot")
                || scen_desc.Contains_String("The Tratos Riots"))) {

            new_scen_desc = "(Firestorm) GDI 03: Quell the Civilian Riot";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI04.MAP") == 0
            && scen_desc.Contains_String("In the Box")) {

            new_scen_desc = "(Firestorm) GDI 04: In the Box";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI05.MAP") == 0
            && scen_desc.Contains_String("Dogma Day Afternoon")) {

            new_scen_desc = "(Firestorm) GDI 05: Dogma Day Afternoon";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI06.MAP") == 0
            && scen_desc.Contains_String("Escape from CABAL")) {

            new_scen_desc = "(Firestorm) GDI 06: Escape from CABAL";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI07.MAP") == 0
            && scen_desc.Contains_String("The Cyborgs are Coming")) {

            new_scen_desc = "(Firestorm) GDI 07: The Cyborgs are Coming";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI08.MAP") == 0
            && scen_desc.Contains_String("Factory Recall")) {

            new_scen_desc = "(Firestorm) GDI 08: Factory Recall";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSGDI09.MAP") == 0
            && scen_desc.Contains_String("Core of the Problem")) {

            new_scen_desc = "(Firestorm) GDI 09: Core of the Problem";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD01.MAP") == 0
            && scen_desc.Contains_String("Operation Reboot")) {

            new_scen_desc = "(Firestorm) NOD 01: Operation Reboot";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD02.MAP") == 0
            && scen_desc.Contains_String("Seeds of Destruction")) {

            new_scen_desc = "(Firestorm) NOD 02: Seeds of Destruction";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD03.MAP") == 0
            && scen_desc.Contains_String("Tratos’ Final Act")) {

            new_scen_desc = "(Firestorm) NOD 03: Tratos's Final Act";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD04.MAP") == 0
            && scen_desc.Contains_String("Mutant Extermination")) {

            new_scen_desc = "(Firestorm) NOD 04: Mutant Extermination";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD05.MAP") == 0
            && scen_desc.Contains_String("Escape from CABAL")) {

            new_scen_desc = "(Firestorm) NOD 05: Escape from CABAL";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD06.MAP") == 0
            && scen_desc.Contains_String("The Needs of the Many!")) {

            new_scen_desc = "(Firestorm) NOD 06: The Needs of the Many!";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD07.MAP") == 0
            && scen_desc.Contains_String("Determined Retribution")) {

            new_scen_desc = "(Firestorm) NOD 07: Determined Retribution";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD08.MAP") == 0
            && scen_desc.Contains_String("Harvester Hunting")) {

            new_scen_desc = "(Firestorm) NOD 08: Harvester Hunting";
            goto return_label;

        } else if (scen_name.Compare_No_Case("FSNOD09.MAP") == 0
            && scen_desc.Contains_String("Core of the Problem")) {

            new_scen_desc = "(Firestorm) NOD 09: Core of the Problem";
            goto return_label;
        }

    }

    /**
     *  Fix the Tiberian Sun mission names.
     */
    if (mission1_ini_unmodified) {

        if (scen_name.Compare_No_Case("GDI1A.MAP") == 0
            && scen_desc.Contains_String("Reinforce Phoenix Base")) {

            new_scen_desc = "GDI 1: Reinforce Phoenix Base";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI2A.MAP") == 0
            && scen_desc.Contains_String("Secure The Region")) {

            new_scen_desc = "GDI 2: Secure the Region";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI3A.MAP") == 0
            && scen_desc.Contains_String("Secure Crash Site")) {

            new_scen_desc = "GDI 3A: Locate and Secure Crash Site";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI3B.MAP") == 0
            && (scen_desc.Contains_String("Capture Train Station")
                || scen_desc.Contains_String("Secure the Region"))) {

            new_scen_desc = "GDI 3B: Capture the Train Station";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI4A.MAP") == 0
            && scen_desc.Contains_String("Defend Crash Site")) {

            new_scen_desc = "GDI 4: Defend the Crash Site";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI5A.MAP") == 0
            && scen_desc.Contains_String("Destroy Radar Array")) {

            new_scen_desc = "GDI 5A: Destroy the Radar Array";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI5B.MAP") == 0
            && scen_desc.Contains_String("Rescue Tratos")) {

            new_scen_desc = "GDI 5B: Rescue Tratos (A)";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI5C.MAP") == 0
            && scen_desc.Contains_String("Rescue Tratos")) {

            new_scen_desc = "GDI 5C: Rescue Tratos (B)";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI6A.MAP") == 0
            && scen_desc.Contains_String("Destroy Vega's Dam")) {

            new_scen_desc = "GDI 6A: Destroy Vega's Dam";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI6B.MAP") == 0
            && scen_desc.Contains_String("Destroy Vega's Base")) {

            new_scen_desc = "GDI 6B: Destroy Vega's Base";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI7A.MAP") == 0
            && scen_desc.Contains_String("Capture Hammerfest Base")) {

            new_scen_desc = "GDI 7: Recapture Hammerfest Base";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI8A.MAP") == 0
            && (scen_desc.Contains_String("Retrieve Disrupter Crystals")
                || scen_desc.Contains_String("Retrieval of Disrupter Crystals"))) {

            new_scen_desc = "GDI 8: Retrieval of Disrupter Crystals";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI9A.MAP") == 0
            && (scen_desc.Contains_String("Rescue Prisoners")
                || scen_desc.Contains_String("Rescue the Prisoners"))) {

            new_scen_desc = "GDI 9A: Rescue the Prisoners";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI9B.MAP") == 0
            && (scen_desc.Contains_String("Destroy Chemical Supply")
                || scen_desc.Contains_String("Destroy Chemical Supply Station"))) {

            new_scen_desc = "GDI 9B: Destroy Chemical Supply Station";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI9C.MAP") == 0
            && (scen_desc.Contains_String("Mine Power Grid")
                || scen_desc.Contains_String("Mine the Power Grid"))) {

            new_scen_desc = "GDI 9C: Mine the Power Grid";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI9D.MAP") == 0
            && (scen_desc.Contains_String("Destroy Chemical Missile Plant")
                || scen_desc.Contains_String("Destroy the Chemical Missile Plant"))) {

            new_scen_desc = "GDI 9D: Destroy the Chemical Missile Plant";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI10A.MAP") == 0
            && scen_desc.Contains_String("Destroy Prototype Facility")) {

            new_scen_desc = "GDI 10A: Destroy the Prototype Facility (A)";          // ?? Locate and Destroy Prototype Manufacturing Facility
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI10B.MAP") == 0
            && scen_desc.Contains_String("Destroy Prototype Facility")) {

            new_scen_desc = "GDI 10B: Destroy the Prototype Facility (B)";          // ?? Locate and Destroy Prototype Manufacturing Facility
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI11A.MAP") == 0
            && scen_desc.Contains_String("Weather the Storm")) {

            new_scen_desc = "GDI 11: Weather the Storm";
            goto return_label;

        } else if (scen_name.Compare_No_Case("GDI12A.MAP") == 0
            && scen_desc.Contains_String("Final Conflict")) {

            new_scen_desc = "GDI 12: Final Conflict";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD1A.MAP") == 0
            && scen_desc.Contains_String("The Messiah Returns")) {

            new_scen_desc = "NOD 1: The Messiah Returns";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD2A.MAP") == 0
            && scen_desc.Contains_String("Retaliation")) {

            new_scen_desc = "NOD 2: Retaliation";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD3A.MAP") == 0
            && (scen_desc.Contains_String("Detroy Hassan's Temple")
                || scen_desc.Contains_String("Destroy Hassan's Temple and Capture Him"))) {

            new_scen_desc = "NOD 3A: Destroy Hassan's Temple";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD3B.MAP") == 0
            && scen_desc.Contains_String("Free Rebel Commander")) {

            new_scen_desc = "NOD 3B: Free the Rebel Nod Commander";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD4A.MAP") == 0
            && scen_desc.Contains_String("Eviction Notice")) {

            new_scen_desc = "NOD 4A: Eviction Notice";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD4B.MAP") == 0
            && scen_desc.Contains_String("Blackout")) {

            new_scen_desc = "NOD 4B: Blackout";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD5A.MAP") == 0
            && scen_desc.Contains_String("Salvage Operation")) {

            new_scen_desc = "NOD 5: Salvage Operation";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD6A.MAP") == 0
            && scen_desc.Contains_String("Sheep's Clothing")) {

            new_scen_desc = "NOD 6A: Sheep's Clothing";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD6B.MAP") == 0
            && (scen_desc.Contains_String("Capture Umagon")
                || scen_desc.Contains_String("Locate and Capture Umagon"))) {

            new_scen_desc = "NOD 6B: Locate and Capture Umagon (A)";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD6C.MAP") == 0
            && scen_desc.Contains_String("Capture Umagon")) {

            new_scen_desc = "NOD 6C: Locate and Capture Umagon (B)";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD7A.MAP") == 0
            && (scen_desc.Contains_String("Destroy GDI Research Facility")
                || scen_desc.Contains_String("Destroy the Research Facility"))) {

            new_scen_desc = "NOD 7A: Destroy the GDI Research Facility";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD7B.MAP") == 0
            && (scen_desc.Contains_String("Escort Bio-toxin Trucks")
                || scen_desc.Contains_String("Escort the Bio-toxin Trucks"))) {

            new_scen_desc = "NOD 7B: Escort the Bio-toxin Trucks";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD8A.MAP") == 0
            && (scen_desc.Contains_String("Villainess in Distress")
                || scen_desc.Contains_String("Villainess In Distress"))) {

            new_scen_desc = "NOD 8: Villainess in Distress";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD9A.MAP") == 0
            && (scen_desc.Contains_String("Establish Nod Presence")
                || scen_desc.Contains_String("Re-establish Nod Presence"))) {

            new_scen_desc = "NOD 9A: Re-establish Nod Presence";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD9B.MAP") == 0
            && (scen_desc.Contains_String("Protect Waste Convoys")
                || scen_desc.Contains_String("Protect the Waste Convoys"))) {

            new_scen_desc = "NOD 9B: Protect the Waste Convoys";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD10A.MAP") == 0
            && (scen_desc.Contains_String("Destroy Mammoth Mk.II Prototype")
                || scen_desc.Contains_String("Destroy the Mammoth MkII Prototype")
                || scen_desc.Contains_String("Destroy Prototype Facility"))) {

            new_scen_desc = "NOD 10A: Destroy the Mammoth Mk.II Prototype";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD11A.MAP") == 0
            && scen_desc.Contains_String("Capture Jake McNeil")) {

            new_scen_desc = "NOD 11: Capture Jake McNeil";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD12A.MAP") == 0
            && scen_desc.Contains_String("A New Beginning")) {

            new_scen_desc = "NOD 12A: A New Beginning";
            goto return_label;

        } else if (scen_name.Compare_No_Case("NOD12B.MAP") == 0
            && scen_desc.Contains_String("Illegal Data Transfer")) {

            new_scen_desc = "NOD 12B: Illegal Data Transfer";
            goto return_label;

        }
    }

    /**
     *  Fix up the demo/tutorial mission names.
     */
    if (scen_name.Compare_No_Case("TSDEMO.MAP") == 0
        && scen_desc.Contains_String("TS demo mission #1 : Initiation")) {

        new_scen_desc = "Tutorial 1: Initiation";
        goto return_label;

    } else if (scen_name.Compare_No_Case("TSDEMO2.MAP") == 0
        && scen_desc.Contains_String("TS demo mission #2 : Clean Sweep")) {

        new_scen_desc = "Tutorial 2: Clean Sweep";
        goto return_label;

    }

return_label:
    if (new_scen_desc.Get_Length() > 0) {
        DEBUG_WARNING("Scenario \"%s\" description has been corrected!\n", scen_name.Peek_Buffer());
        DEBUG_INFO("Old: \"%s\".\n", scen_desc.Peek_Buffer());
        DEBUG_INFO("New: \"%s\".\n", new_scen_desc.Peek_Buffer());
        return new_scen_desc;
    }

    return scen_desc;
}


/**
 *  #issue-148
 * 
 *  Saves the current mission so it can be used as a mission checkpoint.
 * 
 *  author: CCHyper
 */
static void Save_Mission_Start()
{
    char scenname[PATH_MAX+4];
    char savename[64];

    Wstring scen_name = Strip_Scenario_Path(Scen->ScenarioName);
    Wstring scen_desc = Scen->Description;

    /**
     *  Build the save file name.
     */
    std::strncpy(scenname, scen_name.Peek_Buffer(), PATH_MAX);
    scenname[PATH_MAX] = '\0';
    char *ext = std::strrchr(scenname, '.');
    if (ext) ext[0] = '\0';
    std::strcat(scenname, ".SAV");

    /**
     *  Fixup the scenario descriptions.
     */
    scen_desc = Fixup_Scenario_Description(scen_name, scen_desc);

    /**
     *  Format the mission description. We use square brackets to signify
     *  this is a auto save and not a user made save.
     */
    std::snprintf(savename, sizeof(savename), "[%s]", scen_desc.Peek_Buffer());

    /**
     *  Now save it!
     */
    Save_Game(scenname, savename);

    DEBUG_INFO("Mission \"%s\" (%s) saved.\n", scen_name.Peek_Buffer(), scen_desc);
}


/**
 *  #issue-148
 * 
 *  Patch to save the mission on the first frame, before the initial logic
 *  loop has been processed.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_Main_Loop_Initial_Auto_Save_Patch)
{
    /**
     *  Perform a save of the current mission if we are in a singleplayer game
     *  and before the first frame logic pass has been performed.
     */
    if (Vinifera_AutoMissionSaveEnabled) {
        if (!Vinifera_JustLoadedSaveFile) {
            if (Session.Type == GAME_NORMAL && Frame == 0) {
                Save_Mission_Start();
            }
        } else {
            Vinifera_JustLoadedSaveFile = false;
        }
    }

    //DEV_DEBUG_INFO("Before Logic.AI\n");

    Logic.AI();

    //DEV_DEBUG_INFO("After Logic.AI\n");

    JMP(0x005091A0);
}


/**
 *  This patch stops EVENT_OPTIONS from being created when frame step
 *  mode is enabled. This is because we need to handle it differently
 *  due to us not processing any event while in frame step mode.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Queue_Options_Frame_Step_Check_Patch)
{
    _asm { sub esp, 0x30 }

    if (Vinifera_Developer_FrameStep) {
        goto function_return;
    }

    if (!PlayerPtr->IsToWin && !PlayerPtr->IsToLose && !PlayerPtr->IsToDie) {
        goto create_event;
    }

function_return:
    JMP_REG(ecx, 0x005B1171);

create_event:
    _asm { mov ecx, PlayerPtr } // Second dereference required due to the global reference in TS++.
    _asm { mov eax, [ecx] }
    JMP_REG(ecx, 0x005B1116);
}


static void Before_Main_Loop()
{
}


static void After_Main_Loop()
{
}


/**
 *  Main loop for the frame step mode. This should only handle basic
 *  input, redraw the map and update the scroll position.
 * 
 *  @author: CCHyper
 */
static bool FrameStep_Main_Loop()
{
    //DEV_DEBUG_INFO("FrameStep_Main_Loop(enter)\n");

    if (GameActive) {

        Call_Back();

        /**
         *  Update the display, unless we're inside a dialog.
         */
        if (SpecialDialog == SDLG_NONE && GameInFocus) {

            Map.Flag_To_Redraw(2);

            KeyNumType input;
            int x;
            int y;

            Map.Input(input, x, y);
            if (input != KN_NONE) {
                Keyboard_Process(input);

                /**
                 *  Kludge to allow the options dialog to open.
                 */
                if (input == KN_ESC || input == KN_SPACE) {
                    SpecialDialog = SDLG_OPTIONS;
                }

            }

            Map.Render();
            TacticalMap->AI();
        }

    }

    Sleep(1);

    //DEV_DEBUG_INFO("FrameStep_Main_Loop(exit)\n");

    return !GameActive;
}


static bool Main_Loop_Intercept()
{
    bool ret = false;

    /**
     *  Frame step mode enabled but no frames to process, so just perform
     *  a basic redraw and update of the screen, no game logic.
     */
    if (Vinifera_Developer_FrameStep && !Vinifera_Developer_FrameStepCount) {

        ret = FrameStep_Main_Loop();

    /**
     *  This is basically the original main loop, but now encapsulated by
     *  the frame step logic to allow us to process the requested frames.
     */
    } else if ((Vinifera_Developer_FrameStep && Vinifera_Developer_FrameStepCount > 0)
           || (!Vinifera_Developer_FrameStep && !Vinifera_Developer_FrameStepCount)) {

        //DEV_DEBUG_INFO("Before Main_Loop()\n");

        Before_Main_Loop();

        /**
         *  The games main loop function.
         */
        ret = Main_Loop();

        After_Main_Loop();

        //DEV_DEBUG_INFO("After Main_Loop()\n");

        /**
         *  Decrement the frame step count.
         */
        if (Vinifera_Developer_FrameStep && Vinifera_Developer_FrameStepCount > 0) {
            --Vinifera_Developer_FrameStepCount;
        }

    }

    return ret;
}

/**
 *  Main function for patching the hooks.
 */
void MainLoop_Hooks()
{
    Patch_Call(0x00462A8E, &Main_Loop_Intercept);
    Patch_Call(0x00462A9C, &Main_Loop_Intercept);
    Patch_Call(0x005A0B85, &Main_Loop_Intercept);
    Patch_Jump(0x005B10F0, &_Queue_Options_Frame_Step_Check_Patch);
    Patch_Jump(0x00509196, &_Main_Loop_Initial_Auto_Save_Patch);
}
