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

#include "hooker.h"
#include "movie.h"
#include "movieplayback.h"
#include "movieskip.h"
#include "sessionext.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "tspp.h"


DEFINE_HOOK(0x00563677, _Play_Movie_Intercept_Patch, 5)
{
    GET(const char *, name, ECX);
    GET(ThemeType, theme, EDX);
    GET_STACK(bool, clear_after, 0x80);
    GET_STACK(bool, stretch_allowed, 0x84);
    GET_STACK(bool, clear_before, 0x88);

    MovieSkip::Begin(name);

    if (MoviePlayback_Play(name, theme, clear_before, stretch_allowed, clear_after)) {
        MovieSkip::End();
        return 0x00563891;
    }

    return 0;
}


/**
 *  The original VQA playback path exits early for every session type except
 *  campaign. Allow it to continue in multiplayer when movie playback was
 *  enabled by the spawner.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x005636C6, _Play_Movie_Allow_Legacy_In_Multiplayer_Patch, 0)
{
    if (Session.Type == GAME_NORMAL || SessionExtension->ExtOptions.IsPlayMoviesInMultiplayer) {
        return 0x005636D2;
    }

    return 0x00563891;
}


DEFINE_HOOK(0x00563A37, _Play_Ingame_Movie_Create_Intercept_Patch, 5)
{
    GET(const char *, name, ECX);

    if (MoviePlayback_Play_Ingame(name)) {
        return 0x00563AF7;
    }

    return 0;
}


DEFINE_HOOK(0x00563B7E, _Play_Ingame_Movie_VQType_Create_Intercept_Patch, 5)
{
    auto& name = Make_Global<char[20]>(0x00806D74);

    if (MoviePlayback_Play_Ingame(name)) {
        return 0x00563C31;
    }

    return 0;
}


DEFINE_HOOK(0x005645D0, _Movie_Advance_Frame_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);
    GET(bool *, done, EDX);

    const MoviePlaybackIngameAdvanceResult result = MoviePlayback_Advance_Ingame(handle, *done);
    if (result != MOVIEPLAYBACK_INGAME_NOT_HANDLED) {
        R->AL(result == MOVIEPLAYBACK_INGAME_FRAME_ADVANCED ? 1 : 0);
        return 0x005645DB;
    }

    return 0;
}


DEFINE_HOOK(0x00564470, _Movie_Destroy_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);

    if (MoviePlayback_Destroy_Ingame(handle)) {
        return 0x005644A5;
    }

    return 0;
}


DEFINE_HOOK(0x00564610, _Movie_Pause_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);

    if (MoviePlayback_Pause_Ingame(handle)) {
        return 0x0056461F;
    }

    if (handle != nullptr && handle->VQA != nullptr) {
        R->ECX(handle->VQA);
        return 0x0056461A;
    }

    return 0x0056461F;
}


DEFINE_HOOK(0x00564620, _Movie_Resume_Modern_Ingame_Patch, 5)
{
    GET(VQHandle *, handle, ECX);

    if (MoviePlayback_Resume_Ingame(handle)) {
        return 0x0056462F;
    }

    if (handle != nullptr && handle->VQA != nullptr) {
        R->ECX(handle->VQA);
        return 0x0056462A;
    }

    return 0x0056462F;
}


void MoviePlayback_Hooks()
{

}
