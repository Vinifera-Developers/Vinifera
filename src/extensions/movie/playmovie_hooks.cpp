/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PLAYMOVIE_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks related to Play_Movie and related functions.
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
#include "playmovie_hooks.h"
#include "tibsun_globals.h"
#include "options.h"
#include "campaign.h"
#include "campaignext.h"
#include "scenario.h"
#include "vqa.h"
#include "movie.h"
#include "playmovie.h"
#include "cd.h"
#include "wstring.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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

    if (Scen->Scenario != 1) {
        return false;
    }

    char movie_filename[32];
    VQType intro_vq = VQ_NONE;

    /**
     *  Fetch the campaign disk id.
     */
    CampaignClass *campaign = Campaigns[campaign_id];
    DiskID cd_num = campaign->WhichCD;

    /**
     *  Check if the current campaign is an original GDI or NOD campaign.
     */
    Wstring scenario_name = Strip_Scenario_Path(campaign->Scenario);
    bool is_original_gdi = (cd_num == DISK_GDI && (Wstring(campaign->IniName) == "GDI1" || Wstring(campaign->IniName) == "GDI1A") && scenario_name.Compare_No_Case("GDI1A.MAP") == 0);
    bool is_original_nod = (cd_num == DISK_NOD && (Wstring(campaign->IniName) == "NOD1" || Wstring(campaign->IniName) == "NOD1A") && scenario_name.Compare_No_Case("NOD1A.MAP") == 0);

    /**
     *  #issue-762
     * 
     *  Fetch the campaign extension (if available) and get the custom intro movie.
     * 
     *  @author: CCHyper
     */
    CampaignClassExtension *campaignext = Extension::Fetch<CampaignClassExtension>(campaign);
    if (campaignext->IntroMovie[0] != '\0') {
        std::snprintf(movie_filename, sizeof(movie_filename), "%s.VQA", campaignext->IntroMovie);
        DEBUG_INFO("About to play \"%s\".\n", movie_filename);
        Play_Movie(movie_filename);

    /**
     *  If this is an original Tiberian Sun campaign, play the respective intro movie.
     */
    } else if (is_original_gdi || is_original_nod) {

        /**
         *  "The First Decade" and "Freeware TS" installations reshuffle
         *  the movie files due to all mix files being local now and a
         *  primitive "no-cd" added;
         *  
         *  MOVIES01.MIX -> INTRO.VQA (GDI) is now INTR0.VQA
         *  MOVIES02.MIX -> INTRO.VQA (NOD) is now INTR1.VQA
         * 
         *  Build the movie filename based on the current campaigns desired CD (see DiskID enum). 
         */
        std::snprintf(movie_filename, sizeof(movie_filename), "INTR%d.VQA", cd_num);

        /**
         *  Now play the movie if it is found, falling back to original behavior otherwise.
         */
        if (CCFileClass(movie_filename).Is_Available()) {
            DEBUG_INFO("About to play \"%s\".\n", movie_filename);
            Play_Movie(movie_filename);

        } else if (CCFileClass("INTRO.VQA").Is_Available()) {
            DEBUG_INFO("About to play \"INTRO.VQA\".\n");
            Play_Movie("INTRO.VQA");

        } else {
            DEBUG_WARNING("Failed to find Intro movie!\n");
            return false;
        }

    } else {
        DEBUG_WARNING("No campaign intro movie defined.\n");
    }

    return true;
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


/**
 *  Main function for patching the hooks.
 */
void PlayMovieExtension_Hooks()
{
    Patch_Jump(0x005DB2DE, &_Start_Scenario_Intro_Movie_Patch);
    Patch_Jump(0x004E2796, &_Select_Game_Intro_SneakPeak_Movies_Patch);

    /**
     *  #issue-287
     * 
     *  Main menu transition videos incorrectly scale up when "StretchMovies=true".
     *  Changes Change Play_Movie "stretch_allowed" arg to false.
     * 
     *  @author: CCHyper
     */
    Patch_Byte(0x0057FF34+1, 0); // TS_TITLE.VQA
    Patch_Byte(0x0057FECF+1, 0); // FS_TITLE.VQA

    Patch_Jump(0x00563795, &_Play_Movie_Scale_By_Ratio_Patch);
}
