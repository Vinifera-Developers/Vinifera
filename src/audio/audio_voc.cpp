/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Sound effect (VOC) audio management.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_voc.h"

#include "asserthandler.h"
#include "audio_debug.h"
#include "audio_manager.h"
#include "audio_sample.h"
#include "ccini.h"
#include "debughandler.h"
#include "iomap.h"
#include "tactical.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "vinifera_util.h"

#include <algorithm>
#include <atomic>
#include <thread>


/**
 *  AudioVocClass statics.
 */
int AudioVocClass::DefaultLimit = 5;
int AudioVocClass::DefaultRange = 10;
AudioSoundType AudioVocClass::DefaultType = AUDIO_SOUND_SCREEN;
AudioControlType AudioVocClass::DefaultControl = AUDIO_CONTROL_NORMAL;
int AudioVocClass::DefaultPriority = AudioManagerClass::Priority_To_AudioPriority(10);
float AudioVocClass::DefaultVolume = AUDIO_VOLUME_MAX;
float AudioVocClass::DefaultMinVolume = AUDIO_VOLUME_MIN;
float AudioVocClass::DefaultMaxVolume = AUDIO_VOLUME_MAX;
static float AUDIO_VOLUME_CUTOFF_THRESHOLD = 0.05f;


/**
 *  Synchronisation primitives for the asynchronous voc scan thread.
 */
static std::mutex VocScanMutex;
static std::atomic<bool> IsVocScanComplete {false};


/**
 *  New vocs vector.
 */
DynamicVectorClass<AudioVocClass*> AudioVocs;


/**
 *  Calculates stereo panning and volume for a sound based on its world position relative to the tactical screen.
 *
 *  @author: tomsons26, CCHyper
 */
void AudioVocClass::Calculate_Pan_And_Volume(Coord const& coord, float& pan_result, float& volume_result) const
{
    if (!Available || coord == COORD_NONE) {
        return;
    }

    // Get center of the tactical screen in pixels.
    float tact_center_x = static_cast<float>(TacticalRect.Width) / 2.0f;
    float tact_center_y = static_cast<float>(TacticalRect.Height) / 2.0f;

    // Full screen width, used later for pan scaling.
    float tact_center_x_sq = tact_center_x + tact_center_x;

     // Convert logical range in map cells to pixel range.
    float range = CELL_PIXEL_W * Range;

    float volume = AUDIO_VOLUME_MIN;
    float pan = 0.0f;

    /**
     *  Adjust the volume of the sound.
     */
    Point2D pixel;
    //if (!TacticalMap->Coord_To_Pixel(coord, pixel)) {
    TacticalMap->Coord_To_Pixel(coord, pixel);

        // Compute offset from center of screen in pixels.
        float x_delta = static_cast<float>(pixel.X) - tact_center_x;
        float y_delta = static_cast<float>(pixel.Y) - tact_center_y;

        // Get unsigned distances from center.
        float abs_dist_x = static_cast<float>(std::abs(static_cast<int>(x_delta)));
        float abs_dist_y = static_cast<float>(std::abs(static_cast<int>(y_delta)));

        // If not a "local" sound, push back distance artificially.
        // This reduces volume for sounds near edge of screen or offscreen.
        if ((Type & AUDIO_SOUND_LOCAL) == 0) {
            abs_dist_x = abs_dist_x - tact_center_x;
            abs_dist_y = abs_dist_y - tact_center_y;
            abs_dist_x = std::max(abs_dist_x, 0.0f);
            abs_dist_y = std::max(abs_dist_y, 0.0f);
        }

        // Stretch Y distance to weight vertical positioning more heavily.
        float abs_dist_y_doubled = abs_dist_y + abs_dist_y;

        // If position is within range, compute linearly scaled volume.
        // Loudest at center, fades to 0 as it reaches outer edge of 'range'`'.
        if (abs_dist_x < range && abs_dist_y_doubled < range && range > 0.0f) {
            float subval = abs_dist_x > abs_dist_y_doubled ? abs_dist_x : abs_dist_y_doubled;
            volume = (range - subval) / range;
        }

        // If this is a global sound, enforce a minimum volume threshold.
        if ((Type & AUDIO_SOUND_GLOBAL) != 0) {
            volume = std::max(volume, MinVolume);
        }

    //}

    // If final volume is very low, cut it completely (optimization/mute).
    if (volume < 0.05) {
        volume = AUDIO_VOLUME_MIN;
    }

    /**
     * Calculate stereo panning based on X position.
     * 
     * - x_delta < 0 = left side of screen
     * - x_delta > 0 = right side of screen
     * - Pan is clamped to visible screen width * 2
     */
    if (x_delta < -tact_center_x_sq) {
        x_delta = -tact_center_x_sq;

    } else if (x_delta > tact_center_x_sq) {
        x_delta = tact_center_x_sq;
    }

    // Use 16K fixed-point scale for internal pan math.
    // Converts X offset into a pan value in [0 .. pan_scale]
    constexpr float pan_scale = 16384.0f;

    pan = (x_delta * (pan_scale / 2.0f) / tact_center_x_sq + (pan_scale / 2.0f));

    // Normalize to float range [-1.0 .. +1.0] for audio API.
    // Center is 0.0, left is -1.0, right is +1.0
    pan = (pan / (pan_scale / 2.0f)) - 1.0f;

    /**
     *  Finally, clamp the results within the expected ranges.
     */
    volume_result = std::clamp(volume, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    pan_result = std::clamp(pan, -1.0f, 1.0f);

    //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Play - V %f P %f\n", volume_result, pan_result);
}


/**
 *  Plays a sound effect with positional volume/panning adjustments and shroud visibility checks.
 *
 *  @author: CCHyper
 */
AudioHandleID AudioVocClass::Internal_Play(Coord const& coord, int variation, float volume, float fade_in_seconds) const
{
    /**
     *  Bail out early if the audio system is unavailable or sound is muted.
     */
    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return false;
    }

    /**
     *  Skip if the sound group volume is muted.
     */
    if (AudioManager.Get_Group_Volume(Group) <= AUDIO_VOLUME_MIN) {
        return false;
    }

    /**
     *  Ensure the voc has a valid audio file type assigned.
     */
    if (FileType == AUDIO_TYPE_NONE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Play - Voc has invalid file type!\n");
        return false;
    }

    std::string filename;

    /**
     *  If the RANDOM flag has been set, pick a random sound from the list.
     */
    if (Sounds.Count() > 0 && (Control & AUDIO_CONTROL_RANDOM) != 0) {
        std::string sound = Sounds[Sim_Random_Pick(0, Sounds.Count()-1)];
        filename = AudioManager.Build_Filename_From_Type(FileType, sound);
    } else {
        filename = AudioManager.Build_Filename_From_Type(FileType, Name);
    }

    /**
     *  Verify the resolved filename is valid before attempting playback.
     */
    if (filename.empty()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Play - Voc %s has a null filename!\n", Name.c_str());
        return false;
    }

    /**
     *  Resolve the voc index and prepare playback parameters.
     */
    VocType id = static_cast<VocType>(AudioVocs.ID(const_cast<AudioVocClass*>(this)));

    AudioPriorityType priority = AudioManagerClass::Priority_To_AudioPriority(Priority);
    AudioSoundType type = Type;
    AudioControlType control = Control;

    /**
     *  If we were given a coord, check to see if this sound is subject to certain rules.
     */
    if (coord != COORD_NONE) {

        Cell cell = coord.As_Cell();
        if (cell == CELL_NONE) {
            return false;
        }

        /**
         *  Can this sound only be played if the cell is revealed?
         */
        if ((Type & AUDIO_SOUND_SHROUDED) != 0) {
            if (Map[cell].IsVisible|| Map[cell].IsFogVisible) {
                return false;
            }

        /**
         *  Can this sound only be played if the cell is unrevealed?
         */
        } else if ((Type & AUDIO_SOUND_UNSHROUDED) != 0) {
            if (!Map[cell].IsVisible && !Map[cell].IsFogVisible) {
                return false;
            }
        }

    }

    float vol = volume;
    float pan = 0.0f;

    /**
     *  Apply a random volume shift if a range is defined.
     */
    float vshift = AUDIO_VOLUME_MIN;
    if (VolumeShift.X != 0 || VolumeShift.Y != 0) {
        vshift = Sim_Random_Pick_Float((static_cast<float>(VolumeShift.X) / static_cast<float>(AUDIO_VSHIFT_MAX)), (static_cast<float>(VolumeShift.Y) / static_cast<float>(AUDIO_VSHIFT_MAX)));
    }

    /**
     *  Voice and UI sounds are not subject to volume and panning adjustments.
     */
    if ((type & AUDIO_SOUND_VOICE) == 0 && (type & AUDIO_SOUND_UI) == 0) {

        /**
         *  Adjust the volume and panning of the sound depending on its
         *  location to the tactical screen.
         */
        if (coord != COORD_NONE) {
            Calculate_Pan_And_Volume(coord, pan, vol);
        }

    }

    vol = std::min(std::clamp((Volume * vol + vshift), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX), MaxVolume);

    /**
     *  If the volume drops below this level, just skip it. Otherwise the sounds
     *  will bleed into adjacent areas a little too far.
     */
    if (vol < AUDIO_VOLUME_CUTOFF_THRESHOLD) {
        return false;
    }

    /**
     *  Apply a random frequency (pitch) shift if a range is defined.
     */
    float fshift = 0.0f;
    if (FrequencyShift.X != 0 || FrequencyShift.Y != 0) {
        fshift = Sim_Random_Pick_Float((static_cast<float>(FrequencyShift.X) / static_cast<float>(AUDIO_FSHIFT_MAX)), (static_cast<float>(FrequencyShift.Y) / static_cast<float>(AUDIO_FSHIFT_MAX)));
    }

    float pitch = 1.0f + fshift;

    AudioHandleID handle = INVALID_AUDIO_HANDLE_ID;

    // As Voc instances are now preloaded in a new thread, we must use a mutex
    {
        std::scoped_lock lock(VocScanMutex);

        /**
         *  Submit the play request to the audio manager.
         */
        //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Play - About to call AudioManager.Play with \"%s\".\n", filename.c_str());
        const bool looping = (Control & AUDIO_CONTROL_LOOP) != 0;
        const int loop_limit = looping ? LoopLimit : 0;
        handle = AudioManager.Request_Play(filename, Group, vol, pitch, pan, priority, Limit, fade_in_seconds, 0.0f, true, looping, loop_limit);
        if (handle == INVALID_AUDIO_HANDLE_ID) {
            DEBUG_ERROR("Voc::Play - Failed to play \"%s\"!\n", Name.c_str());
            return handle;
        }
    }

    //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Play - Playing effect \"%s\".\n", Name.c_str());

    return handle;
}



/**
 *  Constructor for a sound effect entry, initialised with default values.
 *
 *  @author: CCHyper
 */
AudioVocClass::AudioVocClass(const char *name) :
    Name(name)
{
    string_to_upper(Name);

    AudioVocs.Add(this);
}


/**
 *  Destructor; removes this entry from the global voc list.
 *
 *  @author: CCHyper
 */
AudioVocClass::~AudioVocClass()
{
    AudioVocs.Delete(this);
}


/**
 *  Reads this sound effect's properties from the INI database.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Read_INI(CCINIClass &ini)
{
    if (!ini.Is_Present(Name.c_str())) {
        return;
    }

    char buffer[256];
    const char *name = Name.c_str();

    Sounds = ini.Get_Strings(name, "Sounds", Sounds);

    Limit = std::clamp(ini.Get_Int(name, "Limit", Limit), 0, AUDIO_MAX_CONCURRENT_LIMIT);
    LoopLimit = std::max(0, ini.Get_Int(name, "LoopLimit", LoopLimit));
    Range = std::max(0, ini.Get_Int(name, "Range", Range));
    Priority = ini.Get_Int(name, "Priority", Priority);
    Volume = std::clamp<float>(ini.Get_Float(name, "Volume", Volume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);

    MinVolume = std::clamp<float>(ini.Get_Float(name, "MinVolume", MinVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    //MaxVolume = std::clamp<float>(ini.Get_Float(name, "MaxVolume", MaxVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX); // Not to be loaded from the ini database.

    VolumeShift = ini.Get_Point(name, "VShift", VolumeShift);
    VolumeShift.X = std::clamp(VolumeShift.X, AUDIO_VSHIFT_MIN, AUDIO_VSHIFT_MAX);
    VolumeShift.Y = std::clamp(VolumeShift.Y, AUDIO_VSHIFT_MIN, AUDIO_VSHIFT_MAX);

    FrequencyShift = ini.Get_Point(name, "FShift", FrequencyShift);
    FrequencyShift.X = std::clamp(FrequencyShift.X, AUDIO_FSHIFT_MIN, AUDIO_FSHIFT_MAX);
    FrequencyShift.Y = std::clamp(FrequencyShift.Y, AUDIO_FSHIFT_MIN, AUDIO_FSHIFT_MAX);

    if (ini.Get_String(name, "Type", "", buffer, sizeof(buffer)) > 0) {

        static struct {
            std::string name;
            AudioSoundType type;
        } _sound_types[] = {
            // NOTE: Update this if you modify AudioSoundType!
            "NORMAL", AUDIO_SOUND_NORMAL,
            //"VIOLENT", ,
            //"MOVEMENT", ,
            //"QUIET", ,
            //"LOUD", ,
            "GLOBAL", AUDIO_SOUND_GLOBAL,
            //"SCREEN", ,
            "LOCAL", AUDIO_SOUND_LOCAL,
            //"PLAYER", ,
            //"ALLIES", ,
            //"ENEMIES", ,
            //"EVERYONE", ,
            //"GUN_SHY", ,
            //"NOISE_SHY", ,
            "UNSHROUDED", AUDIO_SOUND_UNSHROUDED,
            "SHROUDED", AUDIO_SOUND_SHROUDED,
            //"AMBIENT", ,
            //"VOICE", ,
            //"UI", ,
        };

        int flags = 0;
        const char * type = strtok(buffer, ",");

        while (type) {
            std::string tmp = type;
            string_to_upper(tmp);

            for (auto& sound_type : _sound_types) {
                if (sound_type.name == tmp) {
                    flags |= sound_type.type;
                }
            }

            type = strtok(nullptr, ",");
        }

        Type = static_cast<AudioSoundType>(flags);
    }

    if (ini.Get_String(name, "Control", "", buffer, sizeof(buffer)) > 0) {

        static struct {
            std::string name;
            AudioControlType control;
        } _control_types[] = {
            // NOTE: Update this if you modify AudioControlType!
            "NORMAL", AUDIO_CONTROL_NORMAL,
            "LOOP", AUDIO_CONTROL_LOOP,
            "RANDOM", AUDIO_CONTROL_RANDOM,
            //"SEQUENTIAL", AUDIO_CONTROL_SEQUENTIAL,
            //"ALL", AUDIO_CONTROL_ALL,
            "PREDELAY", AUDIO_CONTROL_PREDELAY,
            "QUEUE", AUDIO_CONTROL_QUEUE,
            //"QUEUED_INTERUPT", AUDIO_CONTROL_QUEUED_INTERUPT,
            //"INTERUPT", AUDIO_CONTROL_INTERUPT,
            //"ATTACK", AUDIO_CONTROL_ATTACK,
            //"DECAY", AUDIO_CONTROL_DECAY,
            "AMBIENT", AUDIO_CONTROL_AMBIENT,
        };

        int flags = 0;
        const char * type = strtok(buffer, ",");

        while (type) {
            std::string tmp = type;
            string_to_upper(tmp);

            for (auto& control_type : _control_types) {
                if (control_type.name == tmp) {
                    flags |= control_type.control;
                }
            }

            type = strtok(nullptr, ",");
        }

        Control = static_cast<AudioControlType>(flags);
    }
}


/**
 *  Checks whether this sound effect is eligible for playback.
 *
 *  @author: CCHyper
 */
bool AudioVocClass::Can_Play() const
{
    return AudioManager.Is_Available() && !Debug_Quiet && !Name.empty();
}


/**
 *  Plays this sound effect with the given volume and variation.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(float volume, int variation)
{
    return Internal_Play(COORD_NONE, variation, volume);
}


/**
 *  Plays this sound effect at the given volume.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(float volume)
{
    return Internal_Play(COORD_NONE, -1, volume);
}


/**
 *  Static helper to play a sound effect by VocType with a variation and volume.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(VocType voc, float volume, int variation)
{
    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        return AudioVocs[voc]->Internal_Play(COORD_NONE, variation, volume);
    }
    return INVALID_AUDIO_HANDLE_ID;
}


/**
 *  Static helper to play a sound effect by VocType at the given volume.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(VocType voc, float volume)
{
    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        return AudioVocs[voc]->Internal_Play(COORD_NONE, -1, volume);
    }
    return INVALID_AUDIO_HANDLE_ID;
}


/**
 *  Static helper to play a sound effect by VocType at the given world coordinate.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(VocType voc, Coord const &coord)
{
    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        return AudioVocs[voc]->Internal_Play(coord);
    }
    return INVALID_AUDIO_HANDLE_ID;
}


/**
 *  Scans all registered vocs for available audio files and resolves their file info.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Scan()
{
    for (int index = 0; index < AudioVocs.Count(); ++index) {

        AudioVocClass *vocptr = AudioVocs[index];

        /**
         *  Check if the audio file is available. As Voc's can have multiple sounds
         *  defined which can be picked at random (if the RANDOM flag is set), we also
         *  flag the audio engine to ignore any errors at this point.
         */

        if (!AudioManager.Is_File_Available(vocptr->Name)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Voc::Scan - File \"%s\" was not found in any supported formats!\n", vocptr->Name.c_str());
            continue;
        }

        vocptr->Available = true;

        /**
         *  Resolve the file type and full filename for this voc.
         */
        AudioManager.Get_File_Info(vocptr->Name, vocptr->FileType, vocptr->FileName);
    }

    // Call preload here to reduce patches.
    Preload();
}


/**
 *  Submits all available voc samples to the audio manager for preloading.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Preload()
{
    for (int index = 0; index < AudioVocs.Count(); ++index) {

        AudioVocClass *vocptr = AudioVocs[index];

        if (!vocptr->Available) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Voc::Preload - File \"%s\" was not found in any supported formats!\n", vocptr->Name.c_str());
            continue;
        }

        if (AudioManager.Has_Been_Submitted(vocptr->FileName, vocptr->Group)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Voc::Preload - File \"%s\" has already been submitted to the audio manager!\n", vocptr->Name.c_str());
            continue;
        }

        /**
         *  Submit this sample to the audio manager with its configured properties.
         */
        bool submitted = AudioManager.Submit_Sample(
            vocptr->FileName,
            vocptr->FileType,
            vocptr->Group,
            AudioManagerClass::Priority_To_AudioPriority(vocptr->Priority),
            vocptr->Control,
            vocptr->Type,
            vocptr->Limit);

        if (submitted) {
            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Preload - Submitted \"%s\" to audio manager.\n", vocptr->FileName.c_str());
        } else {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Preload - Failed to submit \"%s\" to audio manager!\n", vocptr->FileName.c_str());
        }
    }
}


/**
 *  Sound effects are the biggest overhead when scanning and preloading. As we won't need to
 *  play a sound effect until the main menu shows, we can defer this off to its own thread to
 *  improve loading times.
 * 
 *  @author: CCHyper
 */
void AudioVocClass::ScanAsync()
{
    std::thread([] {
        IsVocScanComplete.store(false);
        Scan(); // Also calls preload for us
        IsVocScanComplete.store(true);
    }).detach(); // Fire-and-forget
}


/**
 *  Processes the sound INI database, loading defaults and creating/updating voc entries.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Process(CCINIClass &ini)
{
    static char const * const DEFAULTS = "Defaults";
    static char const * const SOUNDLIST = "SoundList";

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process(enter): AudioVocs.Count = %d\n", AudioVocs.Count());

    //Clear();

    char buffer[32];

    if (ini.Is_Present(DEFAULTS)) {
        DefaultLimit = std::clamp(ini.Get_Int(DEFAULTS, "Limit", DefaultLimit), 0, AUDIO_MAX_CONCURRENT_LIMIT);
        DefaultRange = std::max(0, ini.Get_Int(DEFAULTS, "Range", DefaultRange));
        DefaultPriority = ini.Get_Int(DEFAULTS, "Priority", DefaultPriority);
        DefaultVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "Volume", DefaultVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
        DefaultMinVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "MinVolume", DefaultMinVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
        //DefaultMaxVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "MaxVolume", DefaultMaxVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX); // Not to be loaded from the ini database.
    }

    if (ini.Is_Present(SOUNDLIST)) {
        int count = ini.Entry_Count(SOUNDLIST);

        for (int index = 0; index < count; ++index) {
            if (ini.Get_String(SOUNDLIST, ini.Get_Entry(SOUNDLIST, index), "", buffer, sizeof(buffer)-1) > 0) {
                VocType voc = From_Name(buffer);

                AudioVocClass *vocptr = nullptr;
                if (voc == VOC_NONE) {
                    vocptr = new AudioVocClass(buffer);
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process: Creating new Voc %s, processing.\n", vocptr->Name.c_str());

                } else {
                    vocptr = AudioVocs[voc];
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process: Found existing Voc %s, updating.\n", vocptr->Name.c_str());
                }
                vocptr->Read_INI(ini);
            }
        }
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process(exit): AudioVocs.Count = %d\n", AudioVocs.Count());
}


/**
 *  Destroys all registered voc instances and frees associated memory.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Clear()
{
    while (AudioVocs.Count() > 0) {
        delete AudioVocs[0];
    }
}


/**
 *  Returns the VocType index for a given AudioVocClass pointer, or VOC_NONE if not found.
 *
 *  @author: CCHyper
 */
VocType AudioVocClass::VocType_From_Voc(AudioVocClass *voc)
{
    for (VocType index = VOC_FIRST; index < AudioVocs.Count(); ++index) {
        if (AudioVocs[index] == voc) {
            return index;
        }
    }

    return VOC_NONE;
}


/**
 *  Looks up a VocType by name, returning VOC_NONE if not found.
 *
 *  @author: CCHyper
 */
VocType AudioVocClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (name == nullptr || !strcasecmp(name, "<none>") || !strcasecmp(name, "none")) {
        return VOC_NONE;
    }

    if (name != nullptr) {
        for (VocType index = VOC_FIRST; index < AudioVocs.Count(); ++index) {
            AudioVocClass *vocptr = AudioVocs[index];
            if (!strcasecmp(vocptr->Name.c_str(), name)) {
                return index;
            }
        }
    }

    return VOC_NONE;
}


/**
 *  Looks up an AudioVocClass pointer by name, returning nullptr if not found.
 *
 *  @author: CCHyper
 */
AudioVocClass *AudioVocClass::Voc_From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (name == nullptr || !strcasecmp(name, "<none>") || !strcasecmp(name, "none")) {
        return nullptr;
    }

    if (name != nullptr) {
        for (VocType index = VOC_FIRST; index < AudioVocs.Count(); ++index) {
            AudioVocClass *vocptr = AudioVocs[index];
            if (vocptr->Name == name) {
                return vocptr;
            }
        }
    }

    return nullptr;
}


/**
 *  Returns the INI name string for a given VocType, or "<none>" if invalid.
 *
 *  @author: CCHyper
 */
const char *AudioVocClass::INI_Name_From(VocType type)
{
    return (type != VOC_NONE && type < AudioVocs.Count() ? AudioVocs[type]->Name.c_str() : "<none>");
}


/**
 *  Sets the global sound effect volume across all SFX, UI, and event groups.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Set_Volume(int volume)
{
    float volf = std::clamp(AudioManagerClass::iVolume_To_fVolume(volume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_SFX, volf);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_UI, volf);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_EVENT, volf);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_STREAMING, volf);
}
