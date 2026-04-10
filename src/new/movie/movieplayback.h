#pragma once

#include "always.h"

#include "tibsun_defines.h"

#include <string>


std::string Normalize_Movie_Basename(const char *name);

bool MoviePlayback_Is_Available(const char *basename);
bool MoviePlayback_Play(const char *basename, ThemeType theme, bool clear_before, bool stretch_allowed, bool clear_after);
