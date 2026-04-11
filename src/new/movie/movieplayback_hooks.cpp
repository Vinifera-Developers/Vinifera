/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Hooks for movie playback.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
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


static bool Vinifera_Play_Ingame_Movie(const char *name)
{
    return MoviePlayback_Play_Ingame(name);
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


DEFINE_HOOK(0x00563AE7, _Play_Ingame_Movie_Create_Intercept_Patch, 5)
{
    GET(const char *, name, ECX);

    if (Vinifera_Play_Ingame_Movie(name)) {
        R->ESP(R->ESP() + 0x28);
        return 0x00563AF7;
    }

    return 0;
}


DEFINE_HOOK(0x00563C21, _Play_Ingame_Movie_VQType_Create_Intercept_Patch, 5)
{
    GET(const char *, name, ECX);

    if (Vinifera_Play_Ingame_Movie(name)) {
        R->ESP(R->ESP() + 0x28);
        return 0x00563C31;
    }

    return 0;
}


DEFINE_HOOK(0x005645D0, _Movie_Advance_Frame_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);
    GET(bool *, done_ptr, EDX);
    bool &done = *done_ptr;

    const MoviePlaybackIngameAdvanceResult result = MoviePlayback_Advance_Ingame(handle, done);
    if (result != MOVIEPLAYBACK_INGAME_NOT_HANDLED) {
        R->AL(result == MOVIEPLAYBACK_INGAME_FRAME_ADVANCED ? 1 : 0);
        return 0x005645DB;
    }

    return 0;
}


DEFINE_HOOK(0x00564473, _Movie_Destroy_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);

    if (MoviePlayback_Destroy_Ingame(handle)) {
        return 0x005644A4;
    }

    return 0;
}


DEFINE_HOOK(0x00564610, _Movie_Pause_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);

    if (MoviePlayback_Pause_Ingame(handle)) {
        return 0x0056461F;
    }

    return 0;
}


DEFINE_HOOK(0x00564620, _Movie_Resume_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);

    if (MoviePlayback_Resume_Ingame(handle)) {
        return 0x0056462F;
    }

    return 0;
}


void MoviePlayback_Hooks()
{

}
