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
#include "command.h"
#include "uicontrol.h"
#include "rules.h"
#include "iomap.h"
#include "tactical.h"
#include "house.h"
#include "ccfile.h"
#include "addon.h"
#include "ccini.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "rulesext.h"
#include "voxelinit.h"


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
    /**
     *  Has we been flagged to reload the rules data?
     */
    if (Vinifera_Developer_IsToReloadRules) {

        /**
         *  Reinitalise the Rule instance to the defaults.
         */
        Rule->~RulesClass();
        new (Rule) RulesClass();

        /**
         *  Clear the current ini databases.
         */
        ArtINI.Clear();
        RuleINI->Clear();
        FSRuleINI.Clear();

        /**
         *  Reload RULES.INI and FIRESTRM.INI.
         */
        {
            CCFileClass rulefile("RULES.INI");
            RuleINI->Load(rulefile, false);
            ASSERT_FATAL(rulefile.Is_Available());

            if (Is_Addon_Available(ADDON_FIRESTORM) && Is_Addon_Enabled(ADDON_FIRESTORM)) {
                rulefile.Set_Name("FIRESTRM.INI");
                ASSERT_FATAL(rulefile.Is_Available());
                FSRuleINI.Load(rulefile, false);
            }
        }

        /**
         *  Reload ART.INI and ARTFS.INI.
         */
        {
            CCFileClass artfile("ART.INI");

            DEBUG_INFO("Loading ART.INI.\n");
            ArtINI.Load(artfile, false);
            ASSERT_FATAL(artfile.Is_Available());
            DEBUG_INFO("Finished loading ART.INI.\n");

            if (Is_Addon_Available(ADDON_FIRESTORM) && Is_Addon_Enabled(ADDON_FIRESTORM)) {
                DEBUG_INFO("Loading ARTFS.INI.\n");
                artfile.Set_Name("ARTFS.INI");
                ASSERT_FATAL(artfile.Is_Available());
                ArtINI.Load(artfile, false);
                DEBUG_INFO("Finished loading ARTFS.INI.\n");
            }
        }

        /**
         *  Process rule INIs.
         */
        DEBUG_INFO("Calling Rule->Process(*RuleINI).\n");
        Rule->Process(*RuleINI);
        DEBUG_INFO("Finished Rule->Process(*RuleINI).\n");

        DEBUG_INFO("Calling Rule->Addition(FSRuleINI).\n");
        Rule->Addition(FSRuleINI);
        DEBUG_INFO("Finished Rule->Addition(FSRuleINI).\n");

        /**
         *  Process scenario rule overrides.
         */
        {
            CCFileClass scenfile(Scen->ScenarioName);
            ASSERT_FATAL(scenfile.Is_Available());

            CCINIClass scenini;

            scenini.Load(scenfile, false);

            DEBUG_INFO("Calling Rule->Addition() with scenario overrides.\n");
            Rule->Addition(scenini);
            DEBUG_INFO("Finished Rule->Addition() with scenario overrides.\n");
        }

        /**
         *  Finally, reload miscellaneous classes.
         */
        {
            CCFileClass workingfile;
            CCINIClass workingini;

            DEBUG_INFO("Calling UIControls->Read_INI().\n");
            workingfile.Set_Name("UI.INI");
            workingini.Clear();
            workingini.Load(workingfile, false);
            UIControls->Read_INI(workingini);
            DEBUG_INFO("Finished UIControls->Read_INI().\n");
        }

        /**
         *  All done!
         */
        Vinifera_Developer_IsToReloadRules = false;
    }
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

#if false // demo day-night light cycle loop
    static float light_angle = 0;
    static CDTimerClass<SystemTimerClass> light_timer = 5;
    if (light_timer == 0) {
        light_timer = 5;
        light_angle += DEG_TO_RADF(3);
        if (light_angle >= DEG_TO_RADF(360)) light_angle -= DEG_TO_RADF(360);
        RulesClassExtension::Set_Voxel_Light_Angle(RuleExtension->VoxelLightAzimuth, light_angle, 6);
    }
#endif

    return ret;
}


void Process_Command_If_Allowed(CommandClass* command)
{
    if (!Scen->InputLock || (CommandClass::From_Type(COMMAND_OPTIONS) == command)) {
        command->Process();
    }
}

/**
 *  #issue-255
 *
 *  Fixes the user being able to do keyboard input and as such, affect
 *  game state while input is locked.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_Main_Loop_Check_Keyboard_Input_Allowed)
{
    GET_REGISTER_STATIC(CommandClass*, command, ecx);
    Process_Command_If_Allowed(command);
    JMP(0x00508EA8);
}

DECLARE_PATCH(_Keyboard_Process_Check_Keyboard_Input_Allowed)
{
    GET_REGISTER_STATIC(CommandClass*, command, ecx);
    Process_Command_If_Allowed(command);

    // Rebuild function epilogue, we destroyed one byte of it
    // by jumping to this hack
    _asm { pop esi }
    _asm { pop ebp }
    _asm { retn }
}

DECLARE_PATCH(_Sync_Delay_Check_Keyboard_Input_Allowed_Patch1)
{
    GET_REGISTER_STATIC(CommandClass*, command, eax);
    Process_Command_If_Allowed(command);
    JMP(0x00509659);
}

DECLARE_PATCH(_Sync_Delay_Check_Keyboard_Input_Allowed_Patch2)
{
    GET_REGISTER_STATIC(CommandClass*, command, ecx);
    Process_Command_If_Allowed(command);
    JMP(0x0050976C);
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

    /**
     *  #issue-255
     *
     *  Keyboard processing patches.
     *  Technically not all are directly in the main loop,
     *  but all of these are similar to the main loop patch so we include them all here.
     */
    Patch_Jump(0x00508E83, &_Main_Loop_Check_Keyboard_Input_Allowed);
    Patch_Jump(0x0050945C, &_Keyboard_Process_Check_Keyboard_Input_Allowed);
    Patch_Jump(0x00509632, &_Sync_Delay_Check_Keyboard_Input_Allowed_Patch1);
    Patch_Jump(0x00509747, &_Sync_Delay_Check_Keyboard_Input_Allowed_Patch2);
}
