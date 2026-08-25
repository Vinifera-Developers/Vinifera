/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Sound effect (VOC) audio management.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_manager.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"

#include <vector>


class AudioVocClass;
class CCINIClass;
class AudioEventClass;


class AudioVocClass
{
    friend class AudioEventClass;
    friend class AudioVocHandle;

    void Calculate_Pan_And_Volume(Coord const& coord, float& pan_result, float& volume_result) const;
    AudioInstanceHandle Start_File(const std::string& filename, Coord const& coord, float volume, float fade_in_seconds, bool looping, int loop_limit, float delay_seconds = 0.0f, AudioGroupType group = AUDIO_GROUP_SFX) const;

    std::vector<std::string> Build_Filename_Pool(int variation) const;
    float Random_Delay_Seconds() const;
    size_t Advance_Sequential_Index() const;
    void Submit_Sounds(AudioGroupType group) const;

public:
    AudioVocClass(std::string name);
    ~AudioVocClass();

    void Read_INI(CCINIClass& ini);
    bool Can_Play() const;
    bool Is_Available() const { return Available; }

    int Play(float volume, int variation);
    int Play(float volume);

    static int Play(VocType voc, float volume = 1.0f, int = 0);
    static int Play(VocType voc, float volume = 1.0f);
    static int Play(VocType voc, Coord const& coord);
    static int Voice_Play(VocType voc, float volume = 1.0f);

    static void Scan();
    static void ScanAsync();
    static void Wait_For_Scan();
    static void Preload();
    static void Process(CCINIClass& ini);
    static void Clear();

    static VocType VocType_From_Voc(AudioVocClass* voc);
    static VocType From_Name(const char* name);
    static AudioVocClass* Voc_From_Name(const char* name);
    static const char* INI_Name_From(VocType type);

    static void Set_Volume(int volume);

private:
    /**
     *  These are the defaults for all sounds loaded from the ini database.
     */
    static int DefaultLimit;
    static int DefaultRange;
    static int DefaultPriority;
    static float DefaultVolume;
    static float DefaultMinVolume;

    int Get_Limit() const { return Limit.value_or(DefaultLimit); }
    int Get_Range() const { return Range.value_or(DefaultRange); }
    int Get_Priority() const { return Priority.value_or(DefaultPriority); }
    float Get_Volume() const { return Volume.value_or(DefaultVolume); }
    float Get_MinVolume() const { return MinVolume.value_or(DefaultMinVolume); }

    /**
     *  Name of the sound event (up to 31 characters).
     */
    std::string Name;

    /**
     *  The file type of this sound.
     */
    AudioFileType FileType = AUDIO_TYPE_AUD;

    /**
     *  Full filename of the sound effect.
     */
    std::string FileName;

    /**
     *  Is the sound available?
     */
    bool Available = false;

    /**
     *  Up to 32 sound files can be associated with an audio event. Do not specify
     *  path or extension of the file, just the name of the file. The file name must
     *  not be more that 31 characters, and do not use spaces in the filename!
     */
    DynamicVectorClass<std::string> Sounds;

    /**
     *  Priority and Limit are the most important attributes of them all. While
     *  possibly hundreds of audio events want to trigger every frame, only a few
     *  will be chosen. It is vitally important to ensure that important events
     *  do not get dropped. The audio engine uses the priority of the audio event
     *  when choosing which events to drop. So make priorities are set correctly
     *  for all events.
     */
    std::optional<int> Priority;

    /**
     *  Limit specifies the maximum number of instances of an audio event type
     *  that can be played SIMULTANEOUSLY. Limit of one, along with IMMEDIATE
     *  control, can be used to achieve monaural sounds.
     */
    std::optional<int> Limit;

    /**
     *  Optional cap on the total number of plays for loop-enabled vocs.
     *  0 means unlimited looping when Control includes LOOP.
     */
    int LoopLimit = 0;

    /**
     *  Optional playback delay range in milliseconds. Without PREDELAY this
     *  delay is used between repeated/ordered event samples; with PREDELAY the
     *  first event sample waits before starting.
     */
    Point2D Delay = {0, 0};

    /**
     *  Volume level playback for audio event. We assume that all sounds are
     *  normalized. Use this attribute to set the mixing levels for audio.
     *  The value specified is the percentage of full volume. e.g. 0.25 means
     *  playback at one quarter of full volume.
     */
    std::optional<float> Volume;

    /**
     *  The minimum volume for the GLOBAL Type event (ignored for all other types).
     */
    std::optional<float> MinVolume;

    /**
     *  The VShift attribute allows the playback volume to be randomly varied.
     *  Values specify a delta range as a percentage of full volume, e.g.:
     *
     *     Volume = 80
     *     VShift = -5, 5     ; vary "Volume" between 75 and 85.
     */
    Point2D VolumeShift {0, 0};

    /**
     *  The FShift attribute alows the playback frequency to be randomly changed
     *  in order to give some variance to repetitive sounds. The engine uses a
     *  percentage value to represent the frequency of a sound. 100 percent
     *  means normal frequency. 50 percent means half the frequency. 200 percent
     *  means twice the frequency, and so on.
     *
     *  The minDelta and maxDelta values describe a range relative to 100 percent
     *  frequency. For example:
     *
     *     FShift = -5, 5      ; vary the frequency between (100 - 5) and (100 + 5)
     *     FShift = 5, 10      ; vary the frequency between 105% and 110% of original
     *     FShift = -50, 0     ; vary the frequency between 50% and 100% of original
     */
    Point2D FrequencyShift {0, 0};

    /**
     *  Specifies the audible range of a sound in game cells.
     */
    std::optional<int> Range;

    /**
     *  Type information allows the game engine to modify event behaviour.
     */
    AudioSoundType Type = AUDIO_SOUND_SCREEN;

    /**
     *  The control attributes can be used in any combination to achieve desired
     *  playback effects. The default control behaviour is to play the first sound
     *  in the sound list just once.
     */
    AudioControlType Control = AUDIO_CONTROL_NORMAL;

    /**
     *  Persistent index used by AUDIO_CONTROL_SEQUENTIAL to advance through
     *  the Sounds list across plays / body cycles. Mutable so const Play()
     *  paths can advance it.
     */
    mutable size_t SequentialIndex = 0;
};

extern DynamicVectorClass<AudioVocClass*> AudioVocs;
