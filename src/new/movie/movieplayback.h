/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Backend-agnostic movie playback service.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "always.h"

#include "tibsun_defines.h"

#include <cstdint>
#include <string>


struct VQHandle;


/**
 *  Return values from MoviePlayback_Advance_Ingame indicating whether
 *  the handle is managed by this system and whether a frame was produced.
 */
enum MoviePlaybackIngameAdvanceResult
{
    MOVIEPLAYBACK_INGAME_NOT_HANDLED = -1,
    MOVIEPLAYBACK_INGAME_NO_FRAME = 0,
    MOVIEPLAYBACK_INGAME_FRAME_ADVANCED = 1,
};


std::string Normalize_Movie_Basename(const char *basename);

bool MoviePlayback_Is_Available(const char *basename);
bool MoviePlayback_Play(const char *basename, ThemeType theme, bool clear_before, bool stretch_allowed, bool clear_after);
bool MoviePlayback_Play_Ingame(const char *basename);
MoviePlaybackIngameAdvanceResult MoviePlayback_Advance_Ingame(VQHandle *handle, bool &done);
bool MoviePlayback_Destroy_Ingame(VQHandle *handle);
bool MoviePlayback_Pause_Ingame(VQHandle *handle);
bool MoviePlayback_Resume_Ingame(VQHandle *handle);
void MoviePlayback_Update_Networking();