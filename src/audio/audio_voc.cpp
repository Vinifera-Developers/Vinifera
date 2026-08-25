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
#include "audio_event.h"
#include "audio_manager.h"
#include "audio_sample.h"
#include "ccini.h"
#include "debughandler.h"
#include "iomap.h"
#include "miscutil.h"
#include "tactical.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "vinifera_thread.h"
#include "vinifera_util.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>


/**
 *  AudioVocClass statics.
 */
int AudioVocClass::DefaultLimit = 5;
int AudioVocClass::DefaultRange = 10;
int AudioVocClass::DefaultPriority = AudioManagerClass::Priority_To_AudioPriority(10);
float AudioVocClass::DefaultVolume = AUDIO_VOLUME_MAX;
float AudioVocClass::DefaultMinVolume = AUDIO_VOLUME_MIN;
static float AUDIO_VOLUME_CUTOFF_THRESHOLD = 0.05f;


/**
 *  Synchronisation primitives for the asynchronous voc scan thread.
 */
static std::mutex VocScanMutex;
static std::atomic<bool> IsVocScanComplete {false};
static std::mutex VocScanThreadMutex;
static std::thread VocScanThread;


/**
 *  New vocs vector.
 */
DynamicVectorClass<AudioVocClass*> AudioVocs;

namespace {

/**
 *  Joins the owned asynchronous sound scan thread, if one is active.
 *
 *  @author: ZivDero
 */
void Join_Voc_Scan_Thread()
{
    std::scoped_lock lock(VocScanThreadMutex);
    if (VocScanThread.joinable()) {
        VocScanThread.join();
    }
}

float Parse_Delay_Seconds(const char* value, Point2D& delay)
{
    if (value == nullptr || *value == '\0') {
        delay = {0, 0};
        return 0.0f;
    }

    char buffer[64];
    std::strncpy(buffer, value, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* first = std::strtok(buffer, ",");
    char* second = std::strtok(nullptr, ",");

    const float min_seconds = first != nullptr ? static_cast<float>(std::atof(first)) : 0.0f;
    const float max_seconds = second != nullptr ? static_cast<float>(std::atof(second)) : min_seconds;

    const int min_ms = std::max(0, static_cast<int>(min_seconds * 1000.0f));
    const int max_ms = std::max(min_ms, static_cast<int>(max_seconds * 1000.0f));

    delay = {min_ms, max_ms};
    return static_cast<float>(min_ms) / 1000.0f;
}

AudioSoundType Parse_Sound_Type(char const * value)
{
    static struct {
        char const * Name;
        AudioSoundType Value;
    } _sound_types[] = {
        {"NORMAL", AUDIO_SOUND_NORMAL},
        {"GLOBAL", AUDIO_SOUND_GLOBAL},
        {"LOCAL", AUDIO_SOUND_LOCAL},
        {"UNSHROUDED", AUDIO_SOUND_UNSHROUDED},
        {"SHROUDED", AUDIO_SOUND_SHROUDED},
    };

    int flags = 0;
    for (auto& token : SplitView(value, ',')) {
        for (auto& sound_type : _sound_types) {
            std::string type = std::string(token);
            if (strcasecmp(type.c_str(), sound_type.Name) == 0) {
                flags |= sound_type.Value;
            }
        }
    }

    return static_cast<AudioSoundType>(flags);
}

AudioControlType Parse_Control_Type(char const* value)
{
    static struct {
        char const* Name;
        AudioControlType Value;
    } _control_types[] = {
        {"NORMAL", AUDIO_CONTROL_NORMAL},
        {"LOOP", AUDIO_CONTROL_LOOP},
        {"RANDOM", AUDIO_CONTROL_RANDOM},
        {"SEQUENTIAL", AUDIO_CONTROL_SEQUENTIAL},
        {"ALL", AUDIO_CONTROL_ALL},
        {"PREDELAY", AUDIO_CONTROL_PREDELAY},
        {"QUEUE", AUDIO_CONTROL_QUEUE},
        {"INTERRUPT", AUDIO_CONTROL_INTERRUPT},
        {"ATTACK", AUDIO_CONTROL_ATTACK},
        {"DECAY", AUDIO_CONTROL_DECAY}};

    int flags = 0;
    for (auto& token : SplitView(value, ',')) {
        for (auto& control_type : _control_types) {
            std::string type = std::string(token);
            if (strcasecmp(type.c_str(), control_type.Name) == 0) {
                flags |= control_type.Value;
            }
        }
    }

    return static_cast<AudioControlType>(flags);
}

} // namespace


/**
 *  Calculates stereo panning and positional volume attenuation for a sound
 *  based on its world position relative to the tactical screen. The incoming
 *  volume_result is scaled by the positional attenuation, so any caller-supplied
 *  volume is preserved rather than overwritten.
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
    float range = CELL_PIXEL_W * Get_Range();

    float volume = AUDIO_VOLUME_MIN;
    float pan = 0.0f;

    Point2D pixel;
    TacticalMap->Coord_To_Pixel(coord, pixel);

    // Compute offset from center of screen in pixels.
    float x_delta = static_cast<float>(pixel.X) - tact_center_x;
    float y_delta = static_cast<float>(pixel.Y) - tact_center_y;

    float abs_dist_x = static_cast<float>(std::abs(static_cast<int>(x_delta)));
    float abs_dist_y = static_cast<float>(std::abs(static_cast<int>(y_delta)));

    // Non-local sounds fade based on distance from the screen edge, not center.
    if ((Type & AUDIO_SOUND_LOCAL) == 0) {
        abs_dist_x = std::max(abs_dist_x - tact_center_x, 0.0f);
        abs_dist_y = std::max(abs_dist_y - tact_center_y, 0.0f);
    }

    // Stretch Y distance to weight vertical positioning more heavily.
    float abs_dist_y_doubled = abs_dist_y + abs_dist_y;

    // Linear volume falloff from center to the edge of the audible range.
    if (abs_dist_x < range && abs_dist_y_doubled < range && range > 0.0f) {
        float subval = abs_dist_x > abs_dist_y_doubled ? abs_dist_x : abs_dist_y_doubled;
        volume = (range - subval) / range;
    }

    // Global sounds enforce a floor so they remain audible everywhere.
    if ((Type & AUDIO_SOUND_GLOBAL) != 0) {
        volume = std::max(volume, Get_MinVolume());
    }

    if (volume < 0.05f) {
        volume = AUDIO_VOLUME_MIN;
    }

    // Clamp X offset to +/- 2x the screen half-width before converting to pan.
    x_delta = std::clamp(x_delta, -tact_center_x_sq, tact_center_x_sq);

    // Map x_delta in [-tact_center_x_sq, +tact_center_x_sq] to pan in [-1.0, +1.0].
    constexpr float pan_scale = 16384.0f;
    pan = (x_delta * (pan_scale / 2.0f) / tact_center_x_sq + (pan_scale / 2.0f));
    pan = (pan / (pan_scale / 2.0f)) - 1.0f;

    volume_result = std::clamp(volume * volume_result, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    pan_result = std::clamp(pan, -1.0f, 1.0f);
}


/**
 *  Resolves the full set of candidate filenames for this voc. The event
 *  system is responsible for picking which one(s) actually play based on
 *  Control flags. If a fixed variation index is provided, only that one
 *  filename is returned.
 */
std::vector<std::string> AudioVocClass::Build_Filename_Pool(int variation) const
{
    std::vector<std::string> filenames;

    auto add_sound = [&](const std::string& sound) {
        if (sound.empty()) {
            return;
        }
        std::string filename;
        AudioFileType filetype = AUDIO_TYPE_NONE;
        if (AudioManager.Get_File_Info(sound, filetype, filename, true)) {
            filenames.push_back(std::move(filename));
        }
    };

    if (variation >= 0 && variation < Sounds.Count()) {
        add_sound(Sounds[variation]);
        return filenames;
    }

    if (Sounds.Count() > 0) {
        for (int index = 0; index < Sounds.Count(); ++index) {
            add_sound(Sounds[index]);
        }
        return filenames;
    }

    add_sound(Name);
    return filenames;
}


/**
 *  Returns the current SEQUENTIAL index (clamped to the pool size) and
 *  advances it for the next caller. Wraps around modulo pool size.
 */
size_t AudioVocClass::Advance_Sequential_Index() const
{
    const int pool_size = std::max(1, Sounds.Count());
    const size_t current = SequentialIndex % static_cast<size_t>(pool_size);
    SequentialIndex = (current + 1) % static_cast<size_t>(pool_size);
    return current;
}


float AudioVocClass::Random_Delay_Seconds() const
{
    if (Delay.X <= 0 && Delay.Y <= 0) {
        return 0.0f;
    }

    const int min_delay = std::min(Delay.X, Delay.Y);
    const int max_delay = std::max(Delay.X, Delay.Y);
    return static_cast<float>(Sim_Random_Pick(min_delay, max_delay)) / 1000.0f;
}


AudioInstanceHandle AudioVocClass::Start_File(const std::string& filename, Coord const& coord, float volume, float fade_in_seconds, bool looping, int loop_limit, float delay_seconds, AudioGroupType group) const
{
    /**
     *  Bail out early if the audio system is unavailable or sound is muted.
     */
    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    /**
     *  Skip if the sound group volume is muted.
     */
    if (AudioManager.Get_Group_Volume(group) <= AUDIO_VOLUME_MIN) {
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    /**
     *  Verify the resolved filename is valid before attempting playback.
     */
    if (filename.empty()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Play - Voc %s has a null filename!\n", Name.c_str());
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    AudioPriorityType priority = AudioManagerClass::Priority_To_AudioPriority(Get_Priority());
    AudioSoundType type = Type;

    /**
     *  If we were given a coord, check to see if this sound is subject to certain rules.
     */
    if (coord != COORD_NONE) {

        Cell cell = coord.As_Cell();
        if (cell == CELL_NONE) {
            return INVALID_AUDIO_INSTANCE_HANDLE;
        }

        /**
         *  Can this sound only be played if the cell is revealed?
         */
        if ((Type & AUDIO_SOUND_SHROUDED) != 0) {
            if (Map[cell].IsVisible || Map[cell].IsFogVisible) {
                return INVALID_AUDIO_INSTANCE_HANDLE;
            }

        /**
         *  Can this sound only be played if the cell is unrevealed?
         */
        } else if ((Type & AUDIO_SOUND_UNSHROUDED) != 0) {
            if (!Map[cell].IsVisible && !Map[cell].IsFogVisible) {
                return INVALID_AUDIO_INSTANCE_HANDLE;
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

    vol = std::clamp((Get_Volume() * vol + vshift), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);

    /**
     *  If the volume drops below this level, just skip it. Otherwise the sounds
     *  will bleed into adjacent areas a little too far.
     */
    if (vol < AUDIO_VOLUME_CUTOFF_THRESHOLD) {
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    /**
     *  Apply a random frequency (pitch) shift if a range is defined.
     */
    float fshift = 0.0f;
    if (FrequencyShift.X != 0 || FrequencyShift.Y != 0) {
        fshift = Sim_Random_Pick_Float((static_cast<float>(FrequencyShift.X) / static_cast<float>(AUDIO_FSHIFT_MAX)), (static_cast<float>(FrequencyShift.Y) / static_cast<float>(AUDIO_FSHIFT_MAX)));
    }

    float pitch = 1.0f + fshift;

    AudioInstanceHandle handle = INVALID_AUDIO_INSTANCE_HANDLE;

    {
        std::scoped_lock lock(VocScanMutex);

        handle = AudioManager.Request_Play(filename, group, vol, pitch, pan, priority, Get_Limit(), fade_in_seconds, delay_seconds, true, looping, loop_limit, Control);
        if (handle == INVALID_AUDIO_INSTANCE_HANDLE) {
            DEBUG_ERROR("Voc::Play - Failed to play \"{}\"!\n", Name);
            return handle;
        }
    }

    return handle;
}


/**
 *  Constructor for a sound effect entry, initialised with default values.
 *
 *  @author: CCHyper
 */
AudioVocClass::AudioVocClass(std::string name) :
    Name(std::move(name))
{
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

    if (ini.Is_Present(name, "Limit")) {
        Limit = std::clamp(ini.Get_Int(name, "Limit", Get_Limit()), 0, AUDIO_MAX_CONCURRENT_LIMIT);
    }
    LoopLimit = std::max(0, ini.Get_Int(name, "LoopLimit", LoopLimit));
    if (ini.Is_Present(name, "Range")) {
        Range = std::max(0, ini.Get_Int(name, "Range", Get_Range()));
    }
    if (ini.Get_String(name, "Delay", "", buffer, sizeof(buffer)) > 0) {
        Parse_Delay_Seconds(buffer, Delay);
    }
    if (ini.Is_Present(name, "Priority")) {
        Priority = ini.Get_Int(name, "Priority", Get_Priority());
    }
    if (ini.Is_Present(name, "Volume")) {
        Volume = std::clamp<float>(ini.Get_Float(name, "Volume", Get_Volume()), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }

    if (ini.Is_Present(name, "MinVolume")) {
        MinVolume = std::clamp<float>(ini.Get_Float(name, "MinVolume", Get_MinVolume()), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }
    VolumeShift = ini.Get_Point(name, "VShift", VolumeShift);
    VolumeShift.X = std::clamp(VolumeShift.X, AUDIO_VSHIFT_MIN, AUDIO_VSHIFT_MAX);
    VolumeShift.Y = std::clamp(VolumeShift.Y, AUDIO_VSHIFT_MIN, AUDIO_VSHIFT_MAX);

    FrequencyShift = ini.Get_Point(name, "FShift", FrequencyShift);
    FrequencyShift.X = std::clamp(FrequencyShift.X, AUDIO_FSHIFT_MIN, AUDIO_FSHIFT_MAX);
    FrequencyShift.Y = std::clamp(FrequencyShift.Y, AUDIO_FSHIFT_MIN, AUDIO_FSHIFT_MAX);

    if (ini.Get_String(name, "Type", "", buffer, sizeof(buffer)) > 0) {
        Type = Parse_Sound_Type(buffer);
    }

    if (ini.Get_String(name, "Control", "", buffer, sizeof(buffer)) > 0) {
        Control = Parse_Control_Type(buffer);
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
 *  Vanilla-compat shims. These overloads exist solely so the patched
 *  vanilla call sites (Sound_Effect / Voice_Sound_Effect / Static_Sound)
 *  keep their original `int` return signature. Vanilla never reads the
 *  returned value, and Vinifera-side code that needs a long-lived handle
 *  uses AudioEventSystem::Start directly via AudioVocHandle. Always
 *  return -1.
 *
 *  The legacy variation argument is ignored, matching vanilla: the original
 *  VocClass::Play never read it and every call site passes the default 0.
 *  Honoring it as a forced Sounds index would pin multi-sound events to
 *  their first entry and override Control=RANDOM/SEQUENTIAL/ALL. Code that
 *  needs a fixed variation uses AudioEventSystem::Start directly.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(float volume, int)
{
    AudioEventSystem::Start(*this, COORD_NONE, -1, volume, 0.0f);
    return -1;
}


int AudioVocClass::Play(float volume)
{
    AudioEventSystem::Start(*this, COORD_NONE, -1, volume, 0.0f);
    return -1;
}


int AudioVocClass::Play(VocType voc, float volume, int)
{
    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        AudioEventSystem::Start(*AudioVocs[voc], COORD_NONE, -1, volume, 0.0f);
    }
    return -1;
}


int AudioVocClass::Play(VocType voc, float volume)
{
    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        AudioEventSystem::Start(*AudioVocs[voc], COORD_NONE, -1, volume, 0.0f);
    }
    return -1;
}


int AudioVocClass::Play(VocType voc, Coord const &coord)
{
    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        AudioEventSystem::Start(*AudioVocs[voc], coord, -1, 1.0f, 0.0f);
    }
    return -1;
}


/**
 *  Vanilla-compat shim for Voice_Sound_Effect. Unlike Sound_Effect, it is
 *  documented as not subject to Options.SoundVolume - vanilla used it for
 *  system feedback sounds that must be audible regardless of the sound
 *  effect slider (e.g. the voice volume slider feedback beep). Play it in
 *  the system group, which always stays at full volume, to preserve that
 *  contract.
 *
 *  @author: ZivDero
 */
int AudioVocClass::Voice_Play(VocType voc, float volume)
{
    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {

        /**
         *  Vocs are only submitted to the sound effect group up front, and
         *  samples are keyed by (filename, group), so submit this voc's files
         *  to the system group on demand.
         */
        AudioVocs[voc]->Submit_Sounds(AUDIO_GROUP_SYSTEM);

        AudioEventSystem::Start(*AudioVocs[voc], COORD_NONE, -1, volume, 0.0f, AUDIO_GROUP_SYSTEM);
    }
    return -1;
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

        vocptr->Available = false;
        vocptr->FileType = AUDIO_TYPE_NONE;
        vocptr->FileName.clear();

        /**
         *  Check if the audio file is available. As Voc's can have multiple sounds
         *  defined which can be picked at random (if the RANDOM flag is set), we also
         *  flag the audio engine to ignore any errors at this point.
         */

        AudioFileType filetype = AUDIO_TYPE_NONE;
        std::string filename;
        bool available = AudioManager.Get_File_Info(vocptr->Name, filetype, filename, true);

        if (!available) {
            for (int sound = 0; sound < vocptr->Sounds.Count(); ++sound) {
                if (AudioManager.Get_File_Info(vocptr->Sounds[sound], filetype, filename, true)) {
                    available = true;
                    break;
                }
            }
        }

        if (!available) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Voc::Scan - File \"%s\" was not found in any supported formats!\n", vocptr->Name.c_str());
            continue;
        }

        vocptr->Available = true;
        vocptr->FileType = filetype;
        vocptr->FileName = filename;
    }

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

        vocptr->Submit_Sounds(AUDIO_GROUP_SFX);
    }
}


/**
 *  Submits all of this voc's sound files to the audio manager under the given
 *  group. Files that have already been submitted to that group are skipped, so
 *  calling this repeatedly is cheap.
 *
 *  @author: CCHyper, ZivDero
 */
void AudioVocClass::Submit_Sounds(AudioGroupType group) const
{
    auto submit_sound = [&](const std::string& sound) {
        if (sound.empty()) {
            return;
        }
        std::string filename;
        AudioFileType filetype = AUDIO_TYPE_NONE;
        if (!AudioManager.Get_File_Info(sound, filetype, filename, true)) {
            return;
        }

        if (AudioManager.Has_Been_Submitted(filename, group)) {
            return;
        }

        bool submitted = AudioManager.Submit_Sample(
            filename,
            filetype,
            group,
            AudioManagerClass::Priority_To_AudioPriority(Get_Priority()),
            Control,
            Type,
            Get_Limit());

        if (submitted) {
            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Submit_Sounds - Submitted \"%s\" to audio manager.\n", filename.c_str());
        } else {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Submit_Sounds - Failed to submit \"%s\" to audio manager!\n", filename.c_str());
        }
    };

    submit_sound(Name);
    for (int sound = 0; sound < Sounds.Count(); ++sound) {
        submit_sound(Sounds[sound]);
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
    std::scoped_lock lock(VocScanThreadMutex);
    if (VocScanThread.joinable()) {
        VocScanThread.join();
    }
    IsVocScanComplete.store(false);

    VocScanThread = std::thread([] {
        Vinifera_Run_Thread([] {
            Scan();
            IsVocScanComplete.store(true);
        });
    });
}


/**
 *  Waits for any asynchronous sound effect scan to finish.
 *
 *  @author: ZivDero
 */
void AudioVocClass::Wait_For_Scan()
{
    Join_Voc_Scan_Thread();
}


/**
 *  Processes the sound INI database, loading defaults and creating/updating voc entries.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Process(CCINIClass &ini)
{
    Join_Voc_Scan_Thread();

    static char const * const DEFAULTS = "Defaults";
    static char const * const SOUNDLIST = "SoundList";

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process(enter): AudioVocs.Count = %d\n", AudioVocs.Count());

    if (ini.Is_Present(DEFAULTS)) {
        DefaultLimit = std::clamp(ini.Get_Int(DEFAULTS, "Limit", DefaultLimit), 0, AUDIO_MAX_CONCURRENT_LIMIT);
        DefaultRange = std::max(0, ini.Get_Int(DEFAULTS, "Range", DefaultRange));
        DefaultPriority = ini.Get_Int(DEFAULTS, "Priority", DefaultPriority);
        DefaultVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "Volume", DefaultVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
        DefaultMinVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "MinVolume", DefaultMinVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }

    if (ini.Is_Present(SOUNDLIST)) {
        int count = ini.Entry_Count(SOUNDLIST);

        for (int index = 0; index < count; ++index) {
            std::string name = ini.Get_String(SOUNDLIST, ini.Get_Entry(SOUNDLIST, index), "");
            if (!name.empty()) {
                VocType voc = From_Name(name.c_str());

                AudioVocClass *vocptr = nullptr;
                if (voc == VOC_NONE) {
                    vocptr = new AudioVocClass(name);
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
    Join_Voc_Scan_Thread();
    AudioEventSystem::Clear();

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

    for (VocType index = VOC_FIRST; index < AudioVocs.Count(); ++index) {
        AudioVocClass* vocptr = AudioVocs[index];
        if (strcasecmp(vocptr->Name.c_str(), name) == 0) {
            return index;
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

    for (VocType index = VOC_FIRST; index < AudioVocs.Count(); ++index) {
        AudioVocClass *vocptr = AudioVocs[index];
        if (strcasecmp(vocptr->Name.c_str(), name) == 0) {
            return vocptr;
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
    AudioManager.Set_Group_Volume(AUDIO_GROUP_STREAMING, volf);
}
