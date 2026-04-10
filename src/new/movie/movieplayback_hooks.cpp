/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MOVIEPLAYBACK_HOOKS.CPP
 *
 *  @brief         Hooks for pluggable movie playback.
 *
 ******************************************************************************/

#include "always.h"

#include "movieplayback_hooks.h"

#include "ccfile.h"
#include "debughandler.h"
#include "hooker.h"
#include "movieplayback.h"
#include "playmovie.h"
#include "syringe.h"
#include "vinifera_globals.h"

#include <cstdio>
#include <string>


static bool Vinifera_Play_Movie(const char *name, ThemeType theme, bool clear_before, bool stretch_allowed, bool clear_after)
{
    const std::string basename = Normalize_Movie_Basename(name);
    if (basename.empty()) {
        DEBUG_ERROR("Invalid movie filename \"%s\"!\n", name ? name : "<null>");
        return false;
    }

    if (!MoviePlayback_Is_Available(basename.c_str())) {
        return false;
    }

    return MoviePlayback_Play(basename.c_str(), theme, clear_before, stretch_allowed, clear_after);
}


DEFINE_HOOK(0x00563677, _Play_Movie_Intercept_Patch, 5)
{
    GET(const char *, name, ECX);
    GET(ThemeType, theme, EDX);
    GET_STACK(bool, clear_after, 0x80);
    GET_STACK(bool, stretch_allowed, 0x84);
    GET_STACK(bool, clear_before, 0x88);

    if (Vinifera_Play_Movie(name, theme, clear_before, stretch_allowed, clear_after)) {
        return 0x00563891;
    }

    return 0;
}


void MoviePlayback_Hooks()
{

}
