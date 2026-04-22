/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  EVA speech/voice audio management.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_manager.h"
#include "tibsun_defines.h"


class CCINIClass;


/**
 *  A reimplementation of Vox interface for use with the new audio driver interface.
 */
class AudioVoxClass
{
public:
    AudioVoxClass(std::string name);
    ~AudioVoxClass();

    void Read_INI(CCINIClass& ini);

    static void One_Time();

    static bool Process(CCINIClass& ini);
    static void Scan();
    static void ScanAsync();
    static void Preload();
    static void Clear();

    static void Speak(VoxType voice, bool force = false);
    static void AI();
    static void Stop_Speaking();
    static bool Is_Speaking();
    static void Set_Speech_Volume(int vol);

#ifndef NDEBUG
    static bool Write_Default_Speech_INI(CCINIClass& ini);
#endif

    static VoxType From_Name(const char* name);
    static const char* Name_From(VoxType type);

    static void Set_Speech_Allowed(bool set);
    static bool Is_Speech_Allowed();

private:
    /**
     *  Controls whether EVA speech playback is globally enabled.
     */
    static bool IsSpeechAllowed;

    /**
     *  These are the defaults for all speeches loaded from the ini database.
     */
    static int DefaultPriority;
    static float DefaultDelay;
    static float DefaultFrequencyShift;
    static float DefaultVolume;
    static float DefaultMinVolume;
    static float DefaultMaxVolume;

    /**
         *  The file type of this speech.
         */
    AudioFileType FileType = AUDIO_TYPE_AUD;

    /**
     *  The resolved filename for this speech audio file.
     */
    std::string FileName;

    /**
     *  Is the speech available?
     */
    bool Available = false;

    /**
     *  Name of this speech event (up to 31 characters).
     */
    std::string Name;

    /**
     *  Text description for this speech line, for debugging purposes only.
     */
    std::string DescriptionText;

    /**
     *  The name override of this speech event (up to 31 characters).
     */
    std::string Sound;

    /**
     *  Priority and Limit are the most important attributes of them all. While
     *  possibly hundreds of audio events want to trigger every frame, only a few
     *  will be chosen. It is vitally important to ensure that important events
     *  do not get dropped. The audio engine uses the priority of the audio event
     *  when choosing which events to drop. So make priorities are set correctly
     *  for all events.
     */
    int Priority = DefaultPriority;

    /**
     *  The playback volume for this speech entry.
     */
    float Volume = DefaultVolume;

    /**
     *  The minimum and maximum volume bounds for this speech entry.
     */
    float MinVolume = DefaultMinVolume;
    float MaxVolume = DefaultMaxVolume;

    /**
     *  Delay in seconds before this speech begins playback.
     */
    float Delay = DefaultDelay;

    /**
     *  Pitch multiplier applied to this speech during playback.
     */
    float FrequencyShift = DefaultFrequencyShift;

    /**
     *  The sound type flags for this speech entry.
     */
    AudioSoundType Type = AUDIO_SOUND_VOICE;

    /**
     *  The playback control flags for this speech entry.
     */
    AudioControlType Control = AUDIO_CONTROL_QUEUE;
};
