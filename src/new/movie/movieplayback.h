/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Declarations for the backend-agnostic movie playback service.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "always.h"

#include "tibsun_defines.h"

#include <string>


std::string Normalize_Movie_Basename(const char *name);

bool MoviePlayback_Is_Available(const char *basename);
bool MoviePlayback_Play(const char *basename, ThemeType theme, bool clear_before, bool stretch_allowed, bool clear_after);
