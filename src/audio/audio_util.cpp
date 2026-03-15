/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_UTIL.CPP
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

#include "audio_util.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "audio_theme.h"
#include "audio_vox.h"
#include "audio_voc.h"
#include "theme.h"
#include "addon.h"
#include "dsaudio.h"
#include "wwaud.h"
#include "wstring.h"
#include "ramfile.h"
#include "ccfile.h"
#include "ccini.h"
#include "critsection.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <algorithm>
#include <iostream>


bool Theme_Fill_In_All()
{
    CCFileClass theme_file("THEME.INI");
    CCINIClass theme_ini;

    if (!theme_file.Is_Available()) {
        DEBUG_WARNING("Failed to find THEME.INI!\n");
        return false;
    }
    if (!theme_ini.Load(theme_file, false)) {
        DEBUG_WARNING("Failed to load THEME.INI!\n");
        return false;
    }
    if (!ViniferaTheme.Fill_In_All(theme_ini)) {
        DEBUG_WARNING("Fill_In_All(THEME.INI) returned false!\n");
        return false;
    }

    if (Addon_Installed(ADDON_FIRESTORM)) {
        theme_file.Set_Name("THEME01.INI");
        theme_ini.Clear();

        if (!theme_ini.Load(theme_file, false)) {
            DEBUG_WARNING("Failed to load THEME01.INI!\n");
            return false;
        }
        if (!ViniferaTheme.Fill_In_All(theme_ini)) {
            DEBUG_WARNING("Fill_In_All(THEME01.INI) returned false!\n");
            return false;
        }
    }

    return true;
}


static std::string Audio_GetFileExtension(const std::string& filename)
{
    size_t dotPos = filename.rfind('.');
    if (dotPos != std::string::npos && dotPos + 1 < filename.size()) {
        return filename.substr(dotPos + 1);
    }
    return ""; // No extension
}

/**
 *  Determines if the given file is a Westwood AUD (IMA-ADPCM) audio file.
 */
bool Audio_IsAUDFile(const std::string & filename)
{
    // Very quick and simple filename check for now.
    std::string ext = Audio_GetFileExtension(filename.c_str());
    if (ext == "AUD") {
        return true;
    }

    return false;
}

/**
 *  Helper to convert bits per sample to Miniaudios ma_format enum
 */
ma_format Audio_GetMAFormatFromBPS(int bps)
{
    switch (bps) {
        case 8: return ma_format_u8;
        case 16: return ma_format_s16;
        case 32: return ma_format_f32; // common for float PCM
        default: return ma_format_s16;
    }
}
