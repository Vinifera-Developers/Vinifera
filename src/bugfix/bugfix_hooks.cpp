/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUGFIX_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for all bug fixes.
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
#include "bugfix_hooks.h"
#include "bugfixes.h"

#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "wwmouse.h"
#include "campaign.h"
#include "scenario.h"
#include "playmovie.h"
#include "ccfile.h"
#include "cd.h"
#include "vqa.h"
#include "movie.h"
#include "dsurface.h"
#include "options.h"
#include "language.h"
#include "theme.h"
#include "dropship.h"
#include "msgbox.h"
#include "loadoptions.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-212
 * 
 *  Changes the default value of "IsScoreShuffle" to "true".
 * 
 *  @author: CCHyper
 */
static void _OptionsClass_Constructor_IsScoreShuffle_Default_Patch()
{
    Patch_Byte(0x005899F1+1, 0x50); // "cl" (zero) to "dl" (1)
}


/**
 *  #issue-8
 *  
 *  Fixes MultiMission "MaxPlayers" incorrectly loaded with "MinPlayers".
 * 
 *  @author: CCHyper
 */
static void _MultiMission_Constructor_MaxPlayers_Typo_Patch()
{
    static const char *TEXT_MAXPLAYERS = "MaxPlayers";
    Patch_Dword(0x005EF124+1, (uintptr_t)TEXT_MAXPLAYERS); // +1 skips "push" opcode
    Patch_Dword(0x005EF5E4+1, (uintptr_t)TEXT_MAXPLAYERS); // +1 skips "push" opcode
}


/**
 *  #issue-262
 * 
 *  In certain cases, the mouse might not be shown on the Dropship Loadout menu.
 *  This patch fixes that by showing the mouse regardless of its current state.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Start_Scenario_Dropship_Loadout_Show_Mouse_Patch)
{
    /**
     *  issue-284
     * 
     *  Play a background theme during the loadout menu.
     * 
     *  @author: CCHyper
     */
    if (!Theme.Still_Playing()) {

        /**
         *  If DSHPLOAD is defined in THEME.INI, play that, otherwise default
         *  to playing the TS Maps theme.
         */
        ThemeType theme = Theme.From_Name("DSHPLOAD");
        if (theme == THEME_NONE) {
            theme = Theme.From_Name("MAPS");
        }

        Theme.Play_Song(theme);
    }

    WWMouse->Release_Mouse();
    WWMouse->Show_Mouse();

    Dropship_Loadout();

    WWMouse->Hide_Mouse();
    WWMouse->Capture_Mouse();

    if (Theme.Still_Playing()) {
        Theme.Stop(true); // Smoothly fade out the track.
    }

    JMP(0x005DB3C0);
}

static void _Dropship_Loadout_Show_Mouse_Patch()
{
    Patch_Jump(0x005DB3BB, &_Start_Scenario_Dropship_Loadout_Show_Mouse_Patch);
}


/**
 *  Scale up the input rect to the desired width and height, while maintaining the aspect ratio.
 * 
 *  @author: CCHyper
 */
static bool Scale_Video_Rect(Rect &rect, int max_width, int max_height, bool maintain_ratio = false)
{
    /**
     *  No need to scale the rect if it is larger than the max width/height
     */
    bool smaller = rect.Width < max_width && rect.Height < max_height;
    if (!smaller) {
        return false;
    }

    /**
     *  This is a workaround for edge case issues with some versions
     *  of cnc-ddraw. This ensures the available draw area is actually
     *  the resolution the user defines, not what the cnc-ddraw forces
     *  the primary surface to.
     */
    int surface_width = std::clamp(HiddenSurface->Width, 0, Options.ScreenWidth);
    int surface_height = std::clamp(HiddenSurface->Height, 0, Options.ScreenHeight);

    if (maintain_ratio) {

        double dSurfaceWidth = surface_width;
        double dSurfaceHeight = surface_height;
        double dSurfaceAspectRatio = dSurfaceWidth / dSurfaceHeight;

        double dVideoWidth = rect.Width;
        double dVideoHeight = rect.Height;
        double dVideoAspectRatio = dVideoWidth / dVideoHeight;
    
        /**
         *  If the aspect ratios are the same then the screen rectangle
         *  will do, otherwise we need to calculate the new rectangle.
         */
        if (dVideoAspectRatio > dSurfaceAspectRatio) {
            int nNewHeight = (int)(surface_width/dVideoWidth*dVideoHeight);
            int nCenteringFactor = (surface_height - nNewHeight) / 2;
            rect.X = 0;
            rect.Y = nCenteringFactor;
            rect.Width = surface_width;
            rect.Height = nNewHeight;

        } else if (dVideoAspectRatio < dSurfaceAspectRatio) {
            int nNewWidth = (int)(surface_height/dVideoHeight*dVideoWidth);
            int nCenteringFactor = (surface_width - nNewWidth) / 2;
            rect.X = nCenteringFactor;
            rect.Y = 0;
            rect.Width = nNewWidth;
            rect.Height = surface_height;

        } else {
            rect.X = 0;
            rect.Y = 0;
            rect.Width = surface_width;
            rect.Height = surface_height;
        }

    } else {
        rect.X = 0;
        rect.Y = 0;
        rect.Width = surface_width;
        rect.Height = surface_height;
    }

    return true;
}


/**
 *  #issue-292
 * 
 *  Videos stretch to the whole screen size and ignore the video aspect ratio.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Play_Movie_Scale_By_Ratio_Patch)
{
    GET_REGISTER_STATIC(MovieClass *, this_ptr, esi);
    static Rect stretched_rect;

    /**
     *  Calculate the stretched rect for this video, maintaining the video ratio.
     */
    stretched_rect = this_ptr->VideoRect;
    if (Scale_Video_Rect(stretched_rect, HiddenSurface->Width, HiddenSurface->Height, true)) {

        /**
         *  Stretched rect calculated, assign it to the movie instance.
         */
        this_ptr->StretchRect = stretched_rect;

        DEBUG_INFO("Stretching movie - VideoRect: %d,%d -> StretchRect: %d,%d\n",
                this_ptr->VideoRect.Width, this_ptr->VideoRect.Height,
                this_ptr->StretchRect.Width, this_ptr->StretchRect.Height);

        /*DEBUG_GAME("Stretching movie %dx%d -> %dx%d\n",
            this_ptr->VideoRect.Width, this_ptr->VideoRect.Height, this_ptr->StretchRect.Width, this_ptr->StretchRect.Height);*/
    }

    JMP(0x00563805);
}

static void _Scale_Movies_By_Ratio_Patch()
{
    Patch_Jump(0x00563795, &_Play_Movie_Scale_By_Ratio_Patch);
}


/**
 *  #issue-187
 *  
 *  Fixes incorrect spelling of "Loser" on the multiplayer score screen debug output.
 * 
 *  @author: CCHyper
 */
static void _MultiScore_Tally_Score_Fix_Loser_Typo_Patch()
{
    static const char *TEXT_LOSER = "Loser";
    Patch_Dword(0x00568A05+1, (uintptr_t)TEXT_LOSER); // +1 skips "mov eax," opcode
}


/**
 *  #issue-287
 * 
 *  Main menu transition videos incorrectly scale up when "StretchMovies=true".
 * 
 *  @author: CCHyper
 */
static void _Dont_Stretch_Main_Menu_Video_Patch()
{
    /**
     *  Change Play_Movie "stretch_allowed" arg to false.
     */
    Patch_Byte(0x0057FF34+1, 0); // TS_TITLE.VQA
    Patch_Byte(0x0057FECF+1, 0); // FS_TITLE.VQA
}


/**
 *  #issue-269
 * 
 *  Adds a "Load Game" button to the dialog shown on mission lose.
 * 
 *  @author: CCHyper
 */
static bool _Save_Games_Available()
{
    return LoadOptionsClass().Read_Save_Files();
}

static bool _Do_Load_Dialog()
{
    return LoadOptionsClass().Load_Dialog();
}

DECLARE_PATCH(_Do_Lose_Create_Lose_WWMessageBox)
{
    static int ret;

    /**
     *  Show the message box.
     */
retry_dialog:
    ret = Vinifera_Do_WWMessageBox(Text_String(TXT_TO_REPLAY), Text_String(TXT_YES), Text_String(TXT_NO), "Load Game");
    switch (ret) {
        default:
        case 0: // User pressed "Yes"
            JMP(0x005DCE1A);

        case 1: // User pressed "No"
            JMP(0x005DCE56);

        case 2: // User pressed "Load Game"
        {
#ifdef RELEASE
            /**
             *  If no save games are available, notify the user and return back
             *  and reissue the main dialog.
             */
            if (!_Save_Games_Available()) {
                Vinifera_Do_WWMessageBox("No saved games available.", Text_String(TXT_OK));
                goto retry_dialog;
            }

            /**
             *  Show the load game dialog.
             */
            ret = _Do_Load_Dialog();
            if (ret) {
                Theme.Stop();
                JMP(0x005DCE48);
            }
#else
            /**
             *  We disable loading in non-release builds.
             */
            Vinifera_Do_WWMessageBox("No saved games available.", Text_String(TXT_OK));
#endif

            /**
             *  Reissue the dialog if the user pressed cancel on the load dialog.
             */
            goto retry_dialog;
        }
    };
}

static void _Show_Load_Game_On_Mission_Failure()
{
    Patch_Jump(0x005DCDFD, &_Do_Lose_Create_Lose_WWMessageBox);
}


/**
 *  #issue-95
 * 
 *  Patch for handling the campaign intro movies
 *  for "The First Decade" and "Freeware TS" installations.
 * 
 *  @author: CCHyper
 */
static bool Play_Intro_Movie(CampaignType campaign_id)
{
    /**
     *  Catch any cases where we might be starting a non-campaign scenario.
     */
    if (campaign_id == CAMPAIGN_NONE) {
        return false;
    }

    /**
     *  Only handle campaigns with either DISK_GDI (0) or DISK_NOD (1) set.
     */
    int cd_num = Campaigns[campaign_id]->WhichCD;
    if (cd_num >= 0 && cd_num < 2) {

        /**
         *  And make sure its only the first mission of this campaign.
         */
        if (Scen->Scenario == 1) {

            /**
             *  "The First Decade" and "Freeware TS" installations reshuffle
             *  the movie files due to all mix files being local now and a
             *  primitive "no-cd" added;
             *  
             *  MOVIES01.MIX -> INTRO.VQA (GDI) is now INTR0.VQA
             *  MOVIES02.MIX -> INTRO.VQA (NOD) is now INTR1.VQA
             * 
             *  Build the movies filename based on the current campaigns desired CD (see DiskID enum). 
             */
            char filename[12];
            std::snprintf(filename, sizeof(filename), "INTR%d.VQA", cd_num);

            /**
             *  Now play the movie if it is found, falling back to original behavior otherwise.
             */
            if (CCFileClass(filename).Is_Available()) {
                DEBUG_INFO("About to play %s.\n", filename);
                Play_Movie(filename);

            } else {
                DEBUG_INFO("About to play INTRO.VQA.\n");
                Play_Movie("INTRO.VQA");
            }

            return true;
        }

    }

    return false;
}

DECLARE_PATCH(_Start_Scenario_Intro_Movie_Patch)
{
    GET_REGISTER_STATIC(CampaignType, campaign_id, ebx);
    GET_REGISTER_STATIC(char *, name, ebp);

    Play_Intro_Movie(campaign_id);

read_scenario:
    //JMP(0x005DB319);

    /**
     *  The First Decade" and "Freeware TS" EXE's actually have patched code at
     *  the address 0x005DB319, so lets handle the debug log print ourself and
     *  jump back at a safe location.
     */
    DEBUG_GAME("Reading scenario: %s\n", name);
    JMP(0x005DB327);
}


/**
 *  #issue-95
 * 
 *  Patch for handling the campaign intro movies for "The First Decade"
 *  and "Freeware TS" installations when selecting "Intro / Sneak Peak" on
 *  the main menu.
 * 
 *  @author: CCHyper
 */
static void Play_Intro_SneakPeak_Movies()
{
    /**
     *  Backup the current volume.
     */
    //int disk = CD::RequiredCD;

    /**
     *  Find out what movies are available locally.
     */
    bool intro_available = CCFileClass("INTRO.VQA").Is_Available();
    bool intr0_available = CCFileClass("INTR0.VQA").Is_Available();
    bool sizzle_available = CCFileClass("SIZZLE1.VQA").Is_Available();

    bool movie_pair_available = (intro_available && sizzle_available) || (intr0_available && sizzle_available);

    /**
     *  If at least one of the movie pairs were found, we can go ahead and play
     *  them, otherwise set the required disk to GDI and request it if not present.
     */
    if (movie_pair_available || (CD::Set_Required_CD(DISK_GDI), CD().Is_Available(DISK_GDI))) {
        
        /**
         *  Play the intro movie (GDI).
         * 
         *  If the renamed intro is found play that, otherwise falling back to original behavior.
         */
        if (intr0_available) {
            DEBUG_INFO("About to play INTR0.VQA.\n");
            Play_Movie("INTR0.VQA");

            /**
             *  Also attempt to play the NOD intro, just because its a nice improvement.
             */
            DEBUG_INFO("About to play INTR1.VQA.\n");
            Play_Movie("INTR1.VQA");
    
        } else {
        
            DEBUG_INFO("About to play INTRO.VQA.\n");
            Play_Movie("INTRO.VQA");
        }

        /**
         *  Play the sizzle/showreel. This exists loosely on both disks, so we tell
         *  the VQA playback to not use the normal mix file handler.
         */
        VQA_Clear_Option(OPTION_USE_MIX_HANDLER);
        DEBUG_INFO("About to play SIZZLE1.VQA.\n");
        Play_Movie("SIZZLE1.VQA");
        VQA_Set_Option(OPTION_USE_MIX_HANDLER);

    } else {
        DEBUG_WARNING("Failed to find Intro and Sizzle movies!\n");
    }

    /**
     *  Restore the previous volume.
     */
    //CD::Set_Required_CD(disk);
    //CD().Force_Available(disk);
}

DECLARE_PATCH(_Select_Game_Intro_SneakPeak_Movies_Patch)
{
    Play_Intro_SneakPeak_Movies();

    JMP(0x004E288B);
}

static void _Intro_Movie_Patchies()
{
    Patch_Jump(0x005DB2DE, &_Start_Scenario_Intro_Movie_Patch);
    Patch_Jump(0x004E2796, &_Select_Game_Intro_SneakPeak_Movies_Patch);
}


/**
 *  #issue-244
 * 
 *  Changes the default value of "AllowHiResModes" to "true".
 * 
 *  @author: CCHyper
 */
static void _OptionsClass_Constructor_AllowHiResModes_Default_Patch()
{
    Patch_Byte(0x005899D6+1, 0x50); // "cl" (zero) to "dl" (1)
}


/**
 *  Main function for patching the hooks.
 */
void BugFix_Hooks()
{
    _OptionsClass_Constructor_AllowHiResModes_Default_Patch();
    _Intro_Movie_Patchies();
    _Show_Load_Game_On_Mission_Failure();
    _Dont_Stretch_Main_Menu_Video_Patch();
    _MultiScore_Tally_Score_Fix_Loser_Typo_Patch();
    _Scale_Movies_By_Ratio_Patch();
    _Dropship_Loadout_Show_Mouse_Patch();
    _MultiMission_Constructor_MaxPlayers_Typo_Patch();
    _OptionsClass_Constructor_IsScoreShuffle_Default_Patch();
}
