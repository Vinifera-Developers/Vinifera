/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_DEFINES.H
 *
 *  @author        CCHyper
 *
 *  @brief         Audio engine type definitions, enumerations, and constants.
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
#include <string>


/**
 *  Unique identifier type for tracking active audio playback instances.
 */
typedef uint32_t AudioHandleID; // Must be 32bit so it can be passed around in a register if required.
#define INVALID_AUDIO_HANDLE_ID 0xDEAFDEAF


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
     *  Continually play sound event. LoopLimit attribute can be used to
     *  specify how many times to loop the event.
     */
    //AUDIO_CONTROL_LOOP = 1 << 0,

    /**
     *  Randomly pick sound from sound list to play.
     */
    AUDIO_CONTROL_RANDOM = 1 << 1,

    /**
     *  TODO: x
     */
    //AUDIO_CONTROL_SEQUENTIAL = 1 << 2,

    /**
     *  Use all sounds in sound list.
     */
    //AUDIO_CONTROL_ALL = 1 << 3,

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
    //AUDIO_CONTROL_ATTACK = 1 << 8,

    /**
     *  Specifies that the last sound in the sound list gets played,
     *  regardless, at the end of the audio event. (see DECAY attribute
     *  also). This sound is called the decay sound.
     */
    //AUDIO_CONTROL_DECAY = 1 << 9,

    /**
     *  Marks this audio event as an ambient sound.
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
     *  Sound associated with violent or combat actions.
     */
    //AUDIO_SOUND_VIOLENT = 1 << 1,

    /**
     *  Sound associated with unit or object movement.
     */
    //AUDIO_SOUND_MOVEMENT = 1 << 2,

    /**
     *  Sound played at a reduced volume level.
     */
    //AUDIO_SOUND_QUIET = 1 << 3,

    /**
     *  Sound played at an elevated volume level.
     */
    //AUDIO_SOUND_LOUD = 1 << 4,

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
     *  Only audible by the player who triggerd this sound event.
     */
    //AUDIO_SOUND_PLAYER = 1 << 8,

    /**
     *  TODO: x
     */
    //AUDIO_SOUND_ALLIES = 1 << 9,

    /**
     *  TODO: x
     */
    //AUDIO_SOUND_ENEMIES = 1 << 10,

    /**
     *  TODO: x
     */
    //AUDIO_SOUND_EVERYONE = 1 << 11,

    /**
     *  Not audible if sounds with greater volume than us are playing.
     */
    //AUDIO_SOUND_GUN_SHY = 1 << 12,

    /**
     *  Not audible if other sounds are playing.
     */
    //AUDIO_SOUND_NOISE_SHY = 1 << 13,

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
