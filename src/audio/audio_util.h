/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Various audio utility functions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_defines.h"
#include "miniaudio.h"


class AudioThemeClass;
extern AudioThemeClass AudioTheme;


/**
 *  Utility functions
 */
bool Audio_IsAUDFile(const std::string & filename);
AudioInstanceHandle Audio_Play_UI_Sample(const std::string &name, int priority, int volume, AudioGroupType group = AUDIO_GROUP_UI);
AudioInstanceHandle Audio_Play_UI_File(const std::string &filename, AudioFileType type, int priority, int volume, AudioGroupType group = AUDIO_GROUP_UI);

ma_format Audio_GetMAFormatFromBPS(int bps);
