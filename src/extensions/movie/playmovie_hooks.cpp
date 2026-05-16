/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks related to Play_Movie and related functions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "playmovie_hooks.h"

#include "campaign.h"
#include "campaignext.h"
#include "cd.h"
#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "movie.h"
#include "movie/movieplayback.h"
#include "playmovie.h"
#include "scenario.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "vqa.h"


/**
 *  #issue-292
 *
 *  Videos stretch to the whole screen size and ignore the video aspect ratio.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00563795, _Play_Movie_Scale_By_Ratio_Patch, 0)
{
    GET(VQHandle*, this_ptr, ESI);

    /**
     *  Calculate the stretched rect for this video, maintaining the video ratio.
     */
    Rect stretched_rect = this_ptr->InitialRect;
    if (Scale_Video_Rect(stretched_rect, HiddenSurface->Width, HiddenSurface->Height, true)) {

        /**
         *  Stretched rect calculated, assign it to the movie instance.
         */
        this_ptr->StretchRect = stretched_rect;

        DEBUG_INFO("Stretching movie - InitialRect: %d,%d -> StretchRect: %d,%d\n", this_ptr->InitialRect.Width, this_ptr->InitialRect.Height, this_ptr->StretchRect.Width, this_ptr->StretchRect.Height);
    }

    return 0x00563805;
}


bool Vinifera_Is_Movie_Available(const char* name)
{
    const std::string basename = Normalize_Movie_Basename(name);
    if (basename.empty()) {
        return false;
    }

    if (MoviePlayback_Is_Available(basename.c_str())) {
        return true;
    }

    return CCFileClass(name).Is_Available();
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
    bool intro_available = Vinifera_Is_Movie_Available("INTRO.VQA");
    bool intr0_available = Vinifera_Is_Movie_Available("INTR0.VQA");
    bool sizzle_available = Vinifera_Is_Movie_Available("SIZZLE1.VQA");

    bool movie_pair_available = (intro_available && sizzle_available) || (intr0_available && sizzle_available);

    /**
     *  If at least one of the movie pairs were found, we can go ahead and play
     *  them, otherwise set the required disk to GDI and request it if not present.
     */
    if (movie_pair_available || (CD::SetRequiredDisk(DISK_GDI), CD().ForceAvailable(DISK_GDI))) {
        
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


DEFINE_HOOK(0x004E2796, _Select_Game_Intro_SneakPeak_Movies_Patch, 0)
{
    Play_Intro_SneakPeak_Movies();

    return 0x004E288B;
}


/**
 *  Main function for patching the hooks.
 */
void PlayMovieExtension_Hooks()
{

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

    // Allow playing radar movies outside of campaign
    Patch_Jump(0x00563A81, 0x00563A89);
    Patch_Jump(0x00563BBA, 0x00563BC2);
}
