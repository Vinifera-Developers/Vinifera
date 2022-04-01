/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DSHOWVIDEO_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the DirectShow video player.
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
#include "dshowvideo_hooks.h"
#include "dshowvideo.h"
#include "tibsun_globals.h"
#include "dsurface.h"
#include "iomap.h"
#include "session.h"
#include "playmovie.h"
#include "vox.h"
#include "wstring.h"
#include "dsaudio.h"
#include "wwmouse.h"
#include "wwkeyboard.h"
#include "fatal.h"
#include "winutil.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <string>

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Global instance of the DirectShow video player.
 */
static DirectShowVideoPlayer VideoPlayer;


/**
 *  Play a DirectShow video.
 * 
 *  @author: CCHyper
 */
static bool Play_Movie_DirectShow(const char *name, ThemeType theme, bool clear_before = true, bool stretch_allowed = true, bool clear_after = true)
{
    WWKeyboard->Clear();
    WWMouse->Hide_Mouse();

    /**
     *  Only play fullscreen videos in campaign/single-player!
     */
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    if (clear_before) {
        HiddenSurface->Clear();
        GScreenClass::Blit(true, HiddenSurface);
        InvalidateRect(MainWindow, nullptr, false);
    }

    /**
     *  Open the new video using the DirectShow player.
     */
    VideoPlayer.Set_HWND(MainWindow);
    VideoPlayer.Open_File(name);

    if (!VideoPlayer.Has_Video()) {
        DEBUG_ERROR("Failed to create DirectShow instance for \"%s\"!\n", VideoPlayer.Get_Filename() ? VideoPlayer.Get_Filename() : name);
        return false;
    }

    DEBUG_INFO("Play_Movie \"%s\" with DirectShow!\n", VideoPlayer.Get_Filename());

    /**
     *  Full-screen videos can be skipped and scaled.
     */
    VideoPlayer.Set_Breakout(true);
    VideoPlayer.Set_Stretch(stretch_allowed);

    /**
     *  Setup the drawing window.
     */
    VideoPlayer.Set_Video_Window(MainWindow);

    /**
     *  Play the movie!
     */
    VideoPlayer.Play_Callback();

    if (clear_after) {
        HiddenSurface->Clear();
        GScreenClass::Blit(true, HiddenSurface);
        InvalidateRect(MainWindow, nullptr, false);
    }
    
    WWMouse->Show_Mouse();
    WWKeyboard->Clear();

    /**
     *  Reset the video player to prepare for next video playback.
     */
    VideoPlayer.Reset();

    Map.Flag_To_Redraw(2);
    
    return true;
}


/**
 *  Utility function for checking if a movie exists in any of the known locations.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Is_Movie_Available(const char *name)
{
    static char filename_buffer[32];
    std::strncpy(filename_buffer, name, sizeof(filename_buffer));
    
    /**
     *  Find the location of the file extension separator.
     */
    char *movie_name = std::strchr((char *)filename_buffer, '.');
    
    /**
     *  Unexpected filename format passed in?
     */
    if (!movie_name) {
        return false;
    }

    /**
     *  Insert a null-char where the "." was. This will give us the actual
     *  movie name without the extension, allowing us to rebuild them.
     */
    *movie_name = '\0';

    const char *upper_filename = strupr((char *)filename_buffer);

    /**
     *  Was a DirectShow video found?
     */
    if (DirectShowVideoPlayer::Is_File_Available(upper_filename)) {
        return true;
    }

    /**
     *  Finally check if the VQA is available.
     */
    return CCFileClass(name).Is_Available();
}


/**
 *  Intercept to the games Play_Movie which checks if the DirectShow video
 *  file is available, falling back to VQA if not.
 * 
 *  @author: CCHyper
 */
void Vinifera_Play_Movie(const char *name, ThemeType theme, bool clear_before, bool stretch_allowed, bool clear_after)
{
    static char filename_buffer[32];
    std::strncpy(filename_buffer, name, sizeof(filename_buffer));
    
    /**
     *  Find the location of the file extension separator.
     */
    char *movie_name = std::strchr((char *)filename_buffer, '.');
    
    /**
     *  Unexpected filename format passed in?
     */
    if (!movie_name) {
        DEBUG_ERROR("Invalid movie filename \"%s\"!\n", filename_buffer);
        return;
    }

    /**
     *  Insert a null-char where the "." was. This will give us the actual
     *  movie name without the extension, allowing us to rebuild them.
     */
    *movie_name = '\0';

    const char *upper_filename = strupr((char *)filename_buffer);

    /**
     *  Attempt to play the video using the DirectShow player.
     */
    if (Play_Movie_DirectShow(upper_filename, theme, clear_before, stretch_allowed, clear_after)) {
        return;
    } else {
        DEV_DEBUG_WARNING("Failed to play \"%s\" using DirectShow!\n", upper_filename);
    }

    char vqa_buffer[32-4];
    std::snprintf(vqa_buffer, sizeof(vqa_buffer), "%s.VQA", upper_filename);

    /**
     *  The movie did not exist as a DirectShow or failed to play, attempt to play the .VQA.
     */
    if (CCFileClass(vqa_buffer).Is_Available()) {
        DEBUG_INFO("Play_Movie \"%s\" as VQA!\n", upper_filename);

        /**
         *  Call the games VQA Play_Movie.
         */
        Play_Movie(vqa_buffer, theme, clear_before, stretch_allowed, clear_after);
        
    } else {
        DEBUG_ERROR("Failed to play movie \"%s\" as VQA!\n", upper_filename);
    }
}


/**
 *  Main function for patching the hooks.
 */
void DirectShowVideo_Hooks()
{
    /**
     *  Patch in the main Play_Movie interceptor.
     */
    Patch_Call(0x004E07AD, &Vinifera_Play_Movie);
    Patch_Call(0x004E07CC, &Vinifera_Play_Movie);
    Patch_Call(0x004E0840, &Vinifera_Play_Movie);
    Patch_Call(0x004E2865, &Vinifera_Play_Movie);
    Patch_Call(0x004E287F, &Vinifera_Play_Movie);
    Patch_Call(0x00563A1C, &Vinifera_Play_Movie);
    Patch_Call(0x0057FEDA, &Vinifera_Play_Movie);
    Patch_Call(0x0057FF3F, &Vinifera_Play_Movie);
    Patch_Call(0x005DB314, &Vinifera_Play_Movie);
    Patch_Call(0x005E35C8, &Vinifera_Play_Movie);
}
