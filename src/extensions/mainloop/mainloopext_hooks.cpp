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
#include "vinifera_defines.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "iomap.h"
#include "tactical.h"
#include "tacticalext.h"
#include "saveload.h"
#include "house.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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
     *  Perform a quick save operation.
     */
    if (Vinifera_PendingQuickSave) {

        int free_slot = 0;

        /**
         *  Scan for a free save slot.
         */
        for (int slot = 0; slot < QUICK_SAVE_MAX_SLOTS; ++slot) {
            char tmpbuf[16];
            std::snprintf(tmpbuf, sizeof(tmpbuf), "QUICK%03d.SAV", slot);
            if (!RawFileClass(tmpbuf).Is_Available()) {
                free_slot = slot;
                DEBUG_INFO("Found free quick save slot \"%d\".\n", free_slot);
                break;
            }
        }

        /**
         *  Build the save file description and filename.
         */
        char desc[128];
        std::snprintf(desc, sizeof(desc), "[Quick Save %d: %s]", free_slot + 1, Scen->Description);

        char fname[16];
        std::snprintf(fname, sizeof(fname), "QUICK%03d.SAV", free_slot);

        if (Save_Game(fname, desc)) {

            TacticalMapExtension->CaptionTextTimer.Stop();

            TacticalMap->Clear_Caption_Text();

            std::strncpy(TacticalMap->ScreenText, "Game Saved", sizeof(TacticalMap->ScreenText));

            TacticalMapExtension->IsCaptionTextSet = true;

            TacticalMapExtension->CaptionTextTimer = QUICK_SAVE_TEXT_TIMEOUT;
            TacticalMapExtension->CaptionTextTimer.Start();

        }

    /**
     *  Perform a quick load operation.
     */
    } else if (Vinifera_PendingQuickLoad) {

        Wstring fname;

        /**
         *  Find the most recent quick save file.
         */
        FILETIME recentDate = { 0, 0 };
        FILETIME curDate = { 0, 0 };

        WIN32_FIND_DATA block;
        HANDLE handle = FindFirstFile("QUICK*.SAV", &block);
        while (handle != INVALID_HANDLE_VALUE) {
            if ((block.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY)) == 0) {
                char const* name = &block.cAlternateFileName[0];
                if (*name == '\0') name = &block.cFileName[0];

                DEV_DEBUG_INFO("Found file '%s'.\n", name);

                curDate = block.ftCreationTime;

                if (CompareFileTime(&curDate, &recentDate) > 0) {
                    recentDate = curDate;
                    fname = name;
                }
            }
            if (FindNextFile(handle, &block) == 0) break;
        }

        if (!fname.Get_Length()) {
            DEBUG_INFO("No quick save files found!\n");
            return;
        }

        DEBUG_INFO("Found most recent quick save \"%s\"", fname.Peek_Buffer());

        if (Load_Game(fname.Peek_Buffer())) {

            TacticalMapExtension->CaptionTextTimer.Stop();

            TacticalMap->Clear_Caption_Text();

            std::strncpy(TacticalMap->ScreenText, "Game Loaded", sizeof(TacticalMap->ScreenText));

            TacticalMapExtension->IsCaptionTextSet = true;

            TacticalMapExtension->CaptionTextTimer = QUICK_SAVE_TEXT_TIMEOUT;
            TacticalMapExtension->CaptionTextTimer.Start();

        }

    }

    /**
     *  Reset the pending flags for the next loop.
     */
    Vinifera_PendingQuickSave = false;
    Vinifera_PendingQuickLoad = false;
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
}
