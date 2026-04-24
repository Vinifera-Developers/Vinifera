/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Audio engine debug logging macros and utilities.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


typedef enum AudioDebugLogLevel {
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR
} AudioDebugLogLevel;


typedef enum AudioDebugLogType {
    TYPE_MANAGER,
    TYPE_INSTANCE,
    TYPE_SAMPLE,
    TYPE_AMBIENT,
    TYPE_THREAD,
    TYPE_DECODER,
    TYPE_IO,
    TYPE_VOC,
    TYPE_VOX,
    TYPE_THEME,

    // Special for anything else
    TYPE_HOOKS
} AudioDebugLogType;


/**
 *  Macro alias for the audio debug logging function.
 */
#ifndef NDEBUG
#define AUDIO_DEBUG_MSG(level, type, message, ...) Audio_Debug_Log(level, type, message, ##__VA_ARGS__)
void __cdecl Audio_Debug_Log(AudioDebugLogLevel level, AudioDebugLogType type, const char* message, ...);
#else
#define AUDIO_DEBUG_MSG(level, type, message, ...)
#endif