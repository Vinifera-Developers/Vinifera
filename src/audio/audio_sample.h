/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Audio sample class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_defines.h"

#include <algorithm>


struct ma_sound;
struct ma_decoder;
typedef ma_sound ma_sound_group;


/**
 *
 */
class AudioSampleClass
{
    friend class AudioManagerClass;

public:
    AudioSampleClass() = default;
    ~AudioSampleClass() = default;

    std::string Get_FileName() const { return FileName; }

    AudioGroupType Get_Group() const { return Group; }
    AudioPriorityType Get_Priority() const { return Priority; }
    AudioControlType Get_Control() const { return Control; }
    AudioSoundType Get_Type() const { return Type; }
    int Get_Limit() const { return ConcurrentLimit; }

    void Set_Group(AudioGroupType group) { Group = group; }
    void Set_Priority(AudioPriorityType priority) { Priority = priority; }
    void Set_Control(AudioControlType control) { Control = control; }
    void Set_Type(AudioSoundType type) { Type = type; }
    void Set_Limit(int limit)
    {
        ConcurrentLimit = std::clamp(limit, 0, AUDIO_MAX_CONCURRENT_LIMIT);
    }

    bool Is_Available() const;

protected:
    /**
     *  Original source filename.
     */
    std::string FileName;

    /**
     *  The filetype/ext of this file.
     */
    AudioFileType FileType = AUDIO_TYPE_NONE;

    /**
     *  Sound group classification.
     */
    AudioGroupType Group = AUDIO_GROUP_NONE;

    /**
     *  Playback priority.
     */
    AudioPriorityType Priority = AUDIO_PRIORITY_NORMAL;

    /**
     *  Control flags or rules.
     */
    AudioControlType Control = AUDIO_CONTROL_NORMAL;

    /**
     *  Sound type (sample/stream/etc).
     */
    AudioSoundType Type = AUDIO_SOUND_NORMAL;

    /**
     *  Max simultaneous plays of this sample at one time.
     */
    int ConcurrentLimit = AUDIO_MAX_CONCURRENT_LIMIT / 4;

public:
    // Disable copy semantics
    AudioSampleClass(const AudioSampleClass&) = delete;
    AudioSampleClass& operator=(const AudioSampleClass&) = delete;
};
