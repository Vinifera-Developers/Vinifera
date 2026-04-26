/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Audio engine type definitions, enumerations, and constants.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <string>


/**
 *  Audio handle types — two deliberately distinct strong typedefs.
 *
 *  AudioInstanceHandle identifies one playing sample owned by AudioManager.
 *  AudioEventHandle identifies one logical voc event owned by AudioEventSystem.
 *
 *  VOC playback (anything driven by AudioVocClass / SOUND.INI) returns
 *  AudioEventHandle and is operated on through AudioEventSystem. Music,
 *  EVA speech, and VQA streaming use AudioManager directly and therefore
 *  return AudioInstanceHandle. The two types are not interchangeable —
 *  a compile error here is the intended outcome, since an event may swap
 *  the underlying AudioInstanceHandle it drives over its lifetime
 *  (e.g. across body cycles, attack/decay tails, looped retriggers).
 */


/**
 *  Low-level, miniaudio-backed handle for one playing sample inside
 *  AudioManagerClass. Must be 32-bit so it can be passed around in a
 *  register if required.
 */
struct AudioInstanceHandle
{
    uint32_t ID = 0;

    constexpr AudioInstanceHandle() = default;
    constexpr explicit AudioInstanceHandle(uint32_t id) : ID(id) {}

    constexpr bool Is_Valid() const { return ID != 0; }
    constexpr bool operator==(AudioInstanceHandle other) const { return ID == other.ID; }
    constexpr bool operator!=(AudioInstanceHandle other) const { return ID != other.ID; }
};

inline constexpr AudioInstanceHandle INVALID_AUDIO_INSTANCE_HANDLE{};


/**
 *  Opaque public handle for one logical audio event owned by AudioEventSystem.
 *  This is what voc playback returns to game code. Callers must operate on
 *  this handle and route through the AudioEventSystem APIs — never pass it
 *  to AudioManager directly.
 */
struct AudioEventHandle
{
    uint32_t ID = 0;

    constexpr AudioEventHandle() = default;
    constexpr explicit AudioEventHandle(uint32_t id) : ID(id) {}

    constexpr bool Is_Valid() const { return ID != 0; }
    constexpr bool operator==(AudioEventHandle other) const { return ID == other.ID; }
    constexpr bool operator!=(AudioEventHandle other) const { return ID != other.ID; }
};

inline constexpr AudioEventHandle INVALID_AUDIO_EVENT_HANDLE{};


namespace std
{
    template <>
    struct hash<AudioInstanceHandle>
    {
        std::size_t operator()(AudioInstanceHandle h) const noexcept
        {
            return std::hash<uint32_t>()(h.ID);
        }
    };

    template <>
    struct hash<AudioEventHandle>
    {
        std::size_t operator()(AudioEventHandle h) const noexcept
        {
            return std::hash<uint32_t>()(h.ID);
        }
    };
}


/**
 *  Audio frequency shift, volume shift, and playback limit constants.
 */
constexpr int AUDIO_FSHIFT_MIN = -100;
constexpr int AUDIO_FSHIFT_MAX = 100;
constexpr int AUDIO_VSHIFT_MIN = 0;
constexpr int AUDIO_VSHIFT_MAX = 100;

constexpr float AUDIO_VOLUME_MIN = 0.0f;
constexpr float AUDIO_VOLUME_MAX = 1.0f;

constexpr int AUDIO_MAX_CONCURRENT_LIMIT = 32;


/**
 *  Classification of audio into logical groups for independent volume and playback control.
 */
typedef enum AudioGroupType
{
    AUDIO_GROUP_MUSIC,            // Background music.
    AUDIO_GROUP_AMBIENT,          // Ambient music/sound (ie, Ion storm).
    AUDIO_GROUP_SPEECH,           // EVA speech and map selection voiceover.
    AUDIO_GROUP_SFX,              // In-game sound effects.
    AUDIO_GROUP_UI,               // UI and menu sounds.
    AUDIO_GROUP_EVENT,            // Audio events and ambient sounds.
    AUDIO_GROUP_STREAMING,        // Streaming audio (for VQ audio streams etc)

    AUDIO_GROUP_COUNT,
    AUDIO_GROUP_NONE = -1
} AudioGroupType;


/**
 *  Supported audio file formats.
 */
typedef enum AudioFileType
{
    AUDIO_TYPE_AUD,
    AUDIO_TYPE_OGG,
    AUDIO_TYPE_MP3,
    AUDIO_TYPE_WAV,
    AUDIO_TYPE_FLAC,

    AUDIO_TYPE_STREAM,          // Special case!

    AUDIO_TYPE_COUNT,
    AUDIO_TYPE_NONE = -1
} AudioFileType;


/**
 *  Lifecycle states of an active audio playback instance.
 */
typedef enum AudioHandleState {
    AUDIO_STATE_NEW,
    AUDIO_STATE_READY,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_PAUSED,
    AUDIO_STATE_FINISHED,
    AUDIO_STATE_FADING_IN,
    AUDIO_STATE_FADING_OUT,
} AudioHandleState;


/**
 *  Priority levels for audio playback, used to resolve concurrency conflicts.
 */
typedef enum AudioPriorityType
{
    AUDIO_PRIORITY_LOWEST = 0,
    AUDIO_PRIORITY_LOW = 1,
    AUDIO_PRIORITY_NORMAL = 2,
    AUDIO_PRIORITY_HIGH = 3,
    AUDIO_PRIORITY_CRITICAL = 4,
} AudioPriorityType;


/**
 *  Bitfield flags controlling audio event playback behavior.
 */
typedef enum AudioControlType
{
    /**
     *  No special controls applied.
     */
    AUDIO_CONTROL_NORMAL = 0,

    /**
     *  Continually play sound event unless a finite LoopLimit caps total plays.
     */
    AUDIO_CONTROL_LOOP = 1 << 0,

    /**
     *  Randomly pick sound from sound list to play.
     */
    AUDIO_CONTROL_RANDOM = 1 << 1,

    /**
     *  Step through Sounds one entry per play (or per loop cycle), advancing
     *  a persistent index on the voc. Combined with LOOP, each cycle plays
     *  the next sample, wrapping around.
     */
    AUDIO_CONTROL_SEQUENTIAL = 1 << 2,

    /**
     *  Use all sounds in sound list.
     */
    AUDIO_CONTROL_ALL = 1 << 3,

    /**
     *  Normally if the DELAY attribute is specified the delay happens
     *  when the sound finishes playing. PREDELAY forces the delay to 
     *  happen at the start of the sound. Random predelays help avoid
     *  'phasing' when multiple instances of the same audio event happen
     *  at the same time.
     */
    AUDIO_CONTROL_PREDELAY = 1 << 4,

    /**
     *  TODO: x
     * 
     *  Reserved for eva speech!
     */
     AUDIO_CONTROL_QUEUE = 1 << 5,

     /**
      *  TODO: x
      * 
      *  Reserved for eva speech!
      */
     AUDIO_CONTROL_QUEUED_INTERRUPT = 1 << 6,

    /**
     *  This new instances of this type sound event have priority over
     *  already playing instances.
     */
    AUDIO_CONTROL_INTERRUPT = 1 << 7,

    /**
     *  Specifies that the first sound in the sound list gets played,
     *  regardless, at the start of the audio event (see ATTACK attribute
     *  also). This sound is called the attack sound.
     */
    AUDIO_CONTROL_ATTACK = 1 << 8,

    /**
     *  Specifies that the last sound in the sound list gets played,
     *  regardless, at the end of the audio event. (see DECAY attribute
     *  also). This sound is called the decay sound.
     */
    AUDIO_CONTROL_DECAY = 1 << 9,

    /**
     *  Marks this audio event as ambient/classified for compatibility.
     *  This flag does not enable looping by itself.
     */
    AUDIO_CONTROL_AMBIENT = 1 << 10,

} AudioControlType;


typedef enum AudioSoundType
{
    /**
     *  The default type.
     */
    AUDIO_SOUND_NORMAL = 1 << 0,

    /**
     *  Positional audio event is always audible regardless of where in
     *  the world it is. Instead of fading to silence when out of range
     *  like normal events, global events do not fade below MINVOLUME.
     */
    AUDIO_SOUND_GLOBAL = 1 << 5,

    /**
     *  Audio event fades out only when it moves off the edge of the screen.
     */
    AUDIO_SOUND_SCREEN = 1 << 6,

    /**
     *  Only audible are its point of origin in the game world.
     */
    AUDIO_SOUND_LOCAL = 1 << 7,

    /**
     *  Not audible when not covered by shroud.
     */
    AUDIO_SOUND_UNSHROUDED = 1 << 14,

    /**
     *  Not audible when shrouded.
     * 
     *  If this flag is present, the sound will not play if its
     *  position is inside the shroud or fog of war. 
     */
    AUDIO_SOUND_SHROUDED = 1 << 15,

    /**
     *  TODO: Ambient background sound, not affected by INTERRUPT.
     */
    //AUDIO_SOUND_AMBIENT = 1 << 16,

    /**
     *  
     * 
     *  Reserved for use by eva speech!
     */
    AUDIO_SOUND_VOICE = 1 << 18,

    /**
     *
     *
     *  Reserved for use ui sound effects!
     */
     AUDIO_SOUND_UI = 1 << 19,

} AudioSoundType;


/**
 *  Composite key for looking up audio samples by filename and group.
 */
typedef struct AudioSampleKey
{
    std::string Filename;
    AudioGroupType Group;

    bool operator==(const AudioSampleKey& other) const
    {
        return Filename == other.Filename && Group == other.Group;
    }
} AudioSampleKey;

// Custom hasher for the unordered_map
namespace std
{
    template <>
    struct hash<AudioSampleKey>
    {
        std::size_t operator()(const AudioSampleKey& k) const
        {
            return hash<std::string>()(k.Filename) ^ (hash<int>()(static_cast<int>(k.Group)) << 1);
        }
    };
}
