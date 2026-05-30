/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Various audio utility functions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"
#include "audio_util.h"
#include "audio_manager.h"
#include "tibsun_globals.h"
#include "audio_theme.h"
#include "addon.h"
#include "dsaudio.h"
#include "ramfile.h"
#include "ccfile.h"
#include "ccini.h"
#include "debughandler.h"
#include <algorithm>


/**
 *  Fills in theme data from INI files for all registered themes.
 *
 *  @author: CCHyper
 */
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
    if (!AudioTheme.Init_Themes(theme_ini)) {
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
        if (!AudioTheme.Init_Themes(theme_ini)) {
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
 *
 *  @author: CCHyper
 */
bool Audio_IsAUDFile(const std::string & filename)
{
    // Very quick and simple filename check for now.
    std::string ext = Audio_GetFileExtension(filename.c_str());
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

    if (ext == "AUD" || ext == "V00" || ext == "V01") {
        return true;
    }

    return false;
}


static std::string Audio_Normalize_Name(const std::string &name)
{
    if (name.empty()) {
        return {};
    }

    std::string normalized(name);
    size_t slash_pos = normalized.find_last_of("\\/");
    size_t dot_pos = normalized.rfind('.');

    if (dot_pos != std::string::npos && (slash_pos == std::string::npos || dot_pos > slash_pos)) {
        normalized.erase(dot_pos);
    }

    return normalized;
}


/**
 *  Plays a UI sound effect by name with specified priority and volume.
 *
 *  @author: ZivDero
 */
AudioInstanceHandle Audio_Play_UI_Sample(const std::string &name, int priority, int volume)
{
    if (!AudioManager.Is_Available() || name.empty()) {
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    std::string lookup_name = Audio_Normalize_Name(name);
    if (lookup_name.empty()) {
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    AudioFileType type = AUDIO_TYPE_NONE;
    std::string filename;
    if (!AudioManager.Get_File_Info(lookup_name, type, filename)) {
        DEBUG_WARNING("Audio_Play_UI_Sample - Failed to resolve \"{}\".\n", name);
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    AudioPriorityType audio_priority = AudioManagerClass::Priority_To_AudioPriority(priority);
    if (!AudioManager.Has_Been_Submitted(filename, AUDIO_GROUP_UI)) {
        if (!AudioManager.Submit_Sample(filename, type, AUDIO_GROUP_UI, audio_priority, AUDIO_CONTROL_NORMAL, AUDIO_SOUND_UI, AUDIO_MAX_CONCURRENT_LIMIT)) {
            DEBUG_WARNING("Audio_Play_UI_Sample - Failed to submit \"{}\".\n", filename);
            return INVALID_AUDIO_INSTANCE_HANDLE;
        }
    }

    float vol = std::clamp(AudioManagerClass::iVolume_To_fVolume(volume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    return AudioManager.Request_Play(filename, AUDIO_GROUP_UI, vol, 1.0f, 0.0f, audio_priority, AUDIO_MAX_CONCURRENT_LIMIT);
}


/**
 *  Plays a UI sound effect from a file with specified priority and volume.
 *
 *  @author: ZivDero
 */
AudioInstanceHandle Audio_Play_UI_File(const std::string &filename, AudioFileType type, int priority, int volume)
{
    if (!AudioManager.Is_Available() || filename.empty()) {
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    AudioPriorityType audio_priority = AudioManagerClass::Priority_To_AudioPriority(priority);
    if (!AudioManager.Has_Been_Submitted(filename, AUDIO_GROUP_UI)) {
        if (!AudioManager.Submit_Sample(filename, type, AUDIO_GROUP_UI, audio_priority, AUDIO_CONTROL_NORMAL, AUDIO_SOUND_UI, AUDIO_MAX_CONCURRENT_LIMIT)) {
            DEBUG_WARNING("Audio_Play_UI_File - Failed to submit \"{}\".\n", filename);
            return INVALID_AUDIO_INSTANCE_HANDLE;
        }
    }

    float vol = std::clamp(AudioManagerClass::iVolume_To_fVolume(volume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    return AudioManager.Request_Play(filename, AUDIO_GROUP_UI, vol, 1.0f, 0.0f, audio_priority, AUDIO_MAX_CONCURRENT_LIMIT);
}

/**
 *  Converts bits-per-sample to the corresponding miniaudio format.
 *
 *  @author: CCHyper
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
