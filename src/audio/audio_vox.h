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

    void Read_INI(CCINIClass const& ini);

    const std::string& Text() const { return DescriptionText; }
    SubtitleCategoryType Category() const { return SubtitleCategory; }

    static void One_Time();

    static bool Process(CCINIClass const& ini);
    static void Scan();
    static void ScanAsync();
    static void Wait_For_Scan();
    static void Preload();
    static void Clear();

    static void Speak(VoxType voice, bool now = false);
    static void Speak(std::string const& name, bool now = false);
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
     *  These are the defaults for all speeches loaded from the ini database.
     */
    static VoxPriorityType DefaultPriority;
    static float DefaultDelay;
    static float DefaultFrequencyShift;
    static float DefaultVolume;
    static float DefaultMinVolume;
    static float DefaultMaxVolume;

    VoxPriorityType Get_Priority() const { return Priority.value_or(DefaultPriority); }
    float Get_Delay() const { return Delay.value_or(DefaultDelay); }
    float Get_FrequencyShift() const { return FrequencyShift.value_or(DefaultFrequencyShift); }
    float Get_Volume() const { return Volume.value_or(DefaultVolume); }
    float Get_MinVolume() const { return MinVolume.value_or(DefaultMinVolume); }
    float Get_MaxVolume() const { return MaxVolume.value_or(DefaultMaxVolume); }

    std::string const& Get_Sound_Name() const;
    void Set_Vanilla_Defaults();

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
     *  Name of this speech event.
     */
    std::string Name;

    /**
     *  Player-facing subtitle text for this speech line. When non-empty and
     *  the player has subtitles enabled, this is drawn at the bottom of the
     *  tactical view while the VOX is playing.
     */
    std::string DescriptionText;

    /**
     *  Subtitle category for this speech line. Used by the SubtitleMode
     *  option (sun.ini) to filter which lines actually get displayed.
     *  Defaults to SYSTEM since the bulk of EVA lines are system alerts.
     */
    SubtitleCategoryType SubtitleCategory = SUBTITLE_CATEGORY_SCENARIO;

    /**
     *  The name override of this speech event.
     */
    std::string Sound;

    /**
     *  Per-side name overrides of this speech event.
     */
    std::vector<std::string> SideSounds;

    /**
     *  Priority used to order entries within a queue. Higher priorities drain
     *  first; equal priorities are FIFO. This is also forwarded to the audio
     *  engine for sample-slot arbitration.
     */
    std::optional<VoxPriorityType> Priority;

    /**
     *  The playback volume for this speech entry.
     */
    std::optional<float> Volume;

    /**
     *  The minimum and maximum volume bounds for this speech entry.
     */
    std::optional<float> MinVolume;
    std::optional<float> MaxVolume;

    /**
     *  Delay in seconds before this speech begins playback.
     */
    std::optional<float> Delay;

    /**
     *  Pitch multiplier applied to this speech during playback.
     */
    std::optional<float> FrequencyShift;

    /**
     *  The playback policy for this speech entry.
     */
    VoxControlType Control = VOX_CONTROL_STANDARD;
};

extern DynamicVectorClass<AudioVoxClass*> Voxs;

extern const char* EvaNames[VOX_COUNT];
