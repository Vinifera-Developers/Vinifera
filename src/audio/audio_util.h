/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_UTIL.H
 *
 *  @author        CCHyper
 *
 *  @brief         Various audio utility functions.
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
#pragma once

#include "always.h"
#include "tibsun_defines.h"
#include "vinifera_globals.h"
#include "wstring.h"
#include "dsaudio.h"
#include "debughandler.h"

//#define MINIAUDIO_IMPLEMENTATION      // Not needed here as we just want header info!
#include <miniaudio/miniaudio.h>


/**
 *  'ViniferaTheme' should be used for all Theme interaction as the new audio engine replaces it.
 */
class AudioThemeClass;
extern AudioThemeClass AudioTheme;
#define ViniferaTheme AudioTheme


/**
 *  Utility functions
 */
bool Audio_IsAUDFile(const std::string & filename);

// Helper to convert bits per sample to ma_format
ma_format Audio_GetMAFormatFromBPS(int bps);
