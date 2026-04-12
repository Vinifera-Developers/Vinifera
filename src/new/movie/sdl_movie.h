/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SDL helpers for movie playback.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "always.h"
#include "movieplayback_backend.h"
#include "rect.h"


bool SDL_Movie_Present_Frame(const MovieVideoFrame &frame, const Rect &destination_rect);
bool SDL_Movie_Repaint();
bool SDL_Movie_Queue_Audio(const void *data, int data_length, int sample_rate, int channels, MovieSampleFormat format);
bool SDL_Movie_Pause_Audio();
bool SDL_Movie_Resume_Audio();
void SDL_Movie_Flush_Audio();
int SDL_Movie_Get_Queued_Audio_Size();
bool SDL_Movie_Has_Audio();
void SDL_Movie_Shutdown();
