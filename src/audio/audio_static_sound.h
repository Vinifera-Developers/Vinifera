/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Tracked positional sound list for PLAY_SOUND_AT / STOP_SOUNDS_AT
 *  triggers.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "tibsun_defines.h"


/**
 *  Plays a sound at a coordinate as a tracked ambient so it can be stopped
 *  later by Stop_Tracked_Static_Sounds_At(coord). Intended ONLY for the
 *  PLAY_SOUND_AT trigger path - do not use for fire-and-forget effects (those
 *  should continue to call Static_Sound() directly).
 *  Returns true if playback successfully started.
 */
bool Play_Tracked_Static_Sound(VocType voc, Coord const& coord);

/**
 *  Stops every tracked static sound whose stored coord equals `coord`.
 *  Returns the number of sounds stopped.
 */
int Stop_Tracked_Static_Sounds_At(Coord const& coord, float fade_out_seconds = 0.25f);

/**
 *  Periodic maintenance: updates camera-relative volume/pan for each live
 *  entry and prunes entries whose sound has finished.
 *  Called from AudioManagerClass::Sound_Callback().
 */
void Tracked_Static_Sounds_AI();

/**
 *  Stops all entries and clears the list. Called on scenario teardown.
 */
void Clear_Tracked_Static_Sounds();
