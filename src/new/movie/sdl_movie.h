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
void SDL_Movie_Shutdown();
