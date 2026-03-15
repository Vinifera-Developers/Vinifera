/*******************************************************************************
/*                  O P E N  S O U R C E -- V I N I F E R A                   **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_NEWVOC.CPP
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         Sound effect (VOC) audio management.
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

#include "audio_voc.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "audio_sample.h"
#include "audio_manager.h"
#include "audio_debug.h"
#include "tactical.h"
#include "iomap.h"
#include "options.h"
#include "ccini.h"
#include "vinifera_util.h"
#include <thread>
#include <atomic>


/**
 *  These are the defaults for all sounds loaded from the ini database.
 */
static int DefaultLimit = 5;
static int DefaultRange = 10;
static AudioSoundType DefaultType = AUDIO_SOUND_SCREEN;
static AudioControlType DefaultControl = AUDIO_CONTROL_NORMAL;
static int DefaultPriority = AudioManager.Priority_To_AudioPriority(10);
static float DefaultVolume = AUDIO_VOLUME_MAX;
static float DefaultMinVolume = AUDIO_VOLUME_MIN;
static float DefaultMaxVolume = AUDIO_VOLUME_MAX;
static float AUDIO_VOLUME_CUTOFF_THRESHOLD = 0.05f;


/**
 *  The emblem dialog sound effect instance, allocated separately from the INI database.
 */
AudioVocClass *VocEmblem = nullptr;


/**
 *  Synchronisation primitives for the asynchronous voc scan thread.
 */
static std::mutex VocScanMutex;
static std::atomic<bool> IsVocScanComplete{false};


/**
 *  Calculates stereo panning and volume for a sound based on its world position relative to the tactical screen.
 *
 *  @author: tomsons26, CCHyper
 */
static void Voc_Calculate_Pan_And_Volume(AudioVocClass &voc, Coord &coord, float &pan_result, float &volume_result)
{
    if (!voc.Available || coord == COORD_NONE) {
        return;
    }

    // Get center of the tactical screen in pixels.
    float tact_center_x = (float)TacticalRect.Width / 2.0f;
    float tact_center_y = (float)TacticalRect.Height / 2.0f;

    // Full screen width, used later for pan scaling.
    float tact_center_x_sq = tact_center_x + tact_center_x;

     // Convert logical range in map cells to pixel range.
    float range = CELL_PIXEL_W * voc.Range;

    float volume = AUDIO_VOLUME_MIN;
    float pan = 0.0f;

    /**
     *  Adjust the volume of the sound.
     */
    Point2D pixel;
    //if (!TacticalMap->Coord_To_Pixel(coord, pixel)) {
    TacticalMap->Coord_To_Pixel(coord, pixel);

        // Compute offset from center of screen in pixels.
        float x_delta = (float)pixel.X - tact_center_x;
        float y_delta = (float)pixel.Y - tact_center_y;

        // Get unsigned distances from center.
        float abs_dist_x = (float)std::abs((int)x_delta);
        float abs_dist_y = (float)std::abs((int)y_delta);

        // If not a "local" sound, push back distance artificially.
        // This reduces volume for sounds near edge of screen or offscreen.
        if ((voc.Type & AUDIO_SOUND_LOCAL) == 0) {
            abs_dist_x = abs_dist_x - tact_center_x;
            abs_dist_y = abs_dist_y - tact_center_y;
            if (abs_dist_x < 0.0f) {
                abs_dist_x = 0.0f;
            }
            if (abs_dist_y < 0.0f) {
                abs_dist_y = 0.0f;
            }
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
        if ((voc.Type & AUDIO_SOUND_GLOBAL) != 0) {
            volume = std::max(volume, voc.MinVolume);
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
static bool Voc_Play(AudioVocClass &voc, Coord &coord = Coord(0,0,0), int variation = 0, float volume = 1.0f)
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
    if (AudioManager.Get_Group_Volume(voc.Group) <= AUDIO_VOLUME_MIN) {
        return false;
    }

    /**
     *  Ensure the voc has a valid audio file type assigned.
     */
    if (voc.FileType == AUDIO_TYPE_NONE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Play - Voc has invalid file type!\n");
        return false;
    }

#if 0
    /**
     *  Was the voc file found during initialisation?
     */
    if (!vocptr->Available) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Play - Voc file is unavailable!\n");
        return false;
    }
#endif

    std::string filename;

    /**
     *  If the RANDOM flag has been set, pick a random sound from the list.
     */
    if (voc.Sounds.Count()> 0 && (voc.Control & AUDIO_CONTROL_RANDOM) == 0) {
        std::string sound = voc.Sounds[Sim_Random_Pick(0, voc.Sounds.Count()-1)];
        filename = AudioManager.Build_Filename_From_Type(voc.FileType, sound);
    
    } else {
        filename = AudioManager.Build_Filename_From_Type(voc.FileType, voc.Name);
    }

    /**
     *  Verify the resolved filename is valid before attempting playback.
     */
    if (filename.empty()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Play - Voc %s has a null filename!\n", voc.Name.c_str());
        return false;
    }

    /**
     *  Resolve the voc index and prepare playback parameters.
     */
    VocType id = (VocType)Vocs.ID((VocClass*)&voc);

    AudioPriorityType priority = AudioManager.Priority_To_AudioPriority(voc.Priority);
    AudioSoundType type = voc.Type;
    AudioControlType control = voc.Control;

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
        if ((voc.Type & AUDIO_SOUND_SHROUDED) != 0) {
            if (Map[cell].IsVisible|| Map[cell].IsFogVisible) {
                return false;
            }

        /**
         *  Can this sound only be played if the cell is unrevealed?
         */
        } else if ((voc.Type & AUDIO_SOUND_UNSHROUDED) != 0) {
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
    if (voc.VolumeShift.X != 0 || voc.VolumeShift.Y != 0) {
        vshift = Sim_Random_Pick_Float((float(voc.VolumeShift.X) / float(AUDIO_VSHIFT_MAX)), (float(voc.VolumeShift.Y) / float(AUDIO_VSHIFT_MAX)));
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
            Voc_Calculate_Pan_And_Volume(voc, coord, pan, vol);
        }

    }

    vol = std::min(std::clamp((voc.Volume * vol + vshift), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX), voc.MaxVolume);

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
    if (voc.FrequencyShift.X != 0 || voc.FrequencyShift.Y != 0) {
        fshift = Sim_Random_Pick_Float((float(voc.FrequencyShift.X) / float(AUDIO_FSHIFT_MAX)), (float(voc.FrequencyShift.Y) / float(AUDIO_FSHIFT_MAX)));
    }

    float pitch = 1.0f + fshift;

    AudioHandleID handle = INVALID_AUDIO_HANDLE_ID;

    // As Voc instances are now preloaded in a new thread, we must use a mutex
    {
        std::lock_guard<std::mutex> lock(VocScanMutex);

        /**
         *  Submit the play request to the audio manager.
         */
        //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Play - About to call AudioManager.Play with \"%s\".\n", filename.c_str());
        handle = AudioManager.Request_Play(filename, voc.Group, vol, pitch, pan, priority, voc.Limit);
        if (handle == INVALID_AUDIO_HANDLE_ID) {
            DEBUG_ERROR("Voc::Play - Failed to play \"%s\"!\n", voc.Name.c_str());
            return false;
        }
    }

    //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Play - Playing effect \"%s\".\n", voc.Name.c_str());

    /**
     *  Store the handle to the sound we are playing.
     */
    voc.Handle = handle;

    return true;
}



/**
 *  Constructor for a sound effect entry, initialised with default values.
 *
 *  @author: CCHyper
 */
AudioVocClass::AudioVocClass(const char *name) :
    Handle(INVALID_AUDIO_HANDLE_ID),
    Name(name),
    Sounds(),
    FileType(AUDIO_TYPE_AUD),
    FileName(),
    Available(false),
    Priority(DefaultPriority),
    Limit(DefaultLimit),
    Range(DefaultRange),
    Type(DefaultType),
    Volume(DefaultVolume),
    MinVolume(DefaultMinVolume),
    MaxVolume(DefaultMaxVolume),
    VolumeShift(AUDIO_VSHIFT_MIN,AUDIO_VSHIFT_MIN),
    FrequencyShift(AUDIO_VSHIFT_MIN,AUDIO_VSHIFT_MIN),
    Control(AUDIO_CONTROL_NORMAL),
    Group(AUDIO_GROUP_SFX)
{
    string_to_upper(Name);

    Vocs.Add((VocClass *)this);
}


/**
 *  Destructor; removes this entry from the global voc list.
 *
 *  @author: CCHyper
 */
AudioVocClass::~AudioVocClass()
{
    Vocs.Delete((VocClass *)this);
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

    Priority = ini.Get_Int(name, "Priority", Priority);
    Volume = ini.Get_Float_Clamp(name, "Volume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, Volume);

    MinVolume = ini.Get_Float_Clamp(name, "MinVolume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, MinVolume);
    //MaxVolume = ini.Get_Float_Clamp(name, "MaxVolume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, MaxVolume); // Not to be loaded from the ini database.

    VolumeShift = ini.Get_Point(name, "VShift", VolumeShift);
    VolumeShift.X = std::clamp(VolumeShift.X, AUDIO_VSHIFT_MIN, AUDIO_VSHIFT_MAX);
    VolumeShift.Y = std::clamp(VolumeShift.Y, AUDIO_VSHIFT_MIN, AUDIO_VSHIFT_MAX);

    FrequencyShift = ini.Get_Point(name, "FShift", FrequencyShift);
    FrequencyShift.X = std::clamp(FrequencyShift.X, AUDIO_FSHIFT_MIN, AUDIO_FSHIFT_MAX);
    FrequencyShift.Y = std::clamp(FrequencyShift.Y, AUDIO_FSHIFT_MIN, AUDIO_FSHIFT_MAX);

    if (ini.Get_String(name, "Type", buffer, sizeof(buffer) - 1) > 0) {

        static struct _typestruct
        {
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
        std::string tmp;
        const char * type = strtok(buffer, ",");

        while (type) {

            tmp = type;
            string_to_upper(tmp);

            for (int i = 0; i < std::size(_sound_types); ++i) {
                if (_sound_types[i].name == tmp) {
                    flags |= _sound_types[i].type;
                }
            }

            type = strtok(nullptr, ",");
        }

        Type = AudioSoundType(flags);
    }

    if (ini.Get_String(name, "Control", buffer, sizeof(buffer)-1) > 0) {

        static struct _controlstruct
        {
            std::string name;
            AudioControlType control;

        } _control_types[] = {
            // NOTE: Update this if you modify AudioControlType!
            "NORMAL", AUDIO_CONTROL_NORMAL,
            //"LOOP", AUDIO_CONTROL_LOOP,
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

            for (int i = 0; i < std::size(_control_types); ++i) {
                if (_control_types[i].name == tmp) {
                    flags |= _control_types[i].control;
                }
            }

            type = strtok(nullptr, ",");
        }

        Control = AudioControlType(flags);
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
    Voc_Play(*this, Coord(0,0,0), variation, volume);

    /**
     *  New audio interface does not use sample handles in the same way, so just return INVALID_AUDIO_HANDLE.
     */
    return INVALID_AUDIO_HANDLE;
}


/**
 *  Plays this sound effect at the given volume.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(float volume)
{
    Voc_Play(*this, Coord(0,0,0), -1, volume);

    /**
     *  New audio interface does not use sample handles in the same way, so just return INVALID_AUDIO_HANDLE.
     */
    return INVALID_AUDIO_HANDLE;
}


/**
 *  Plays this sound effect at the given world coordinate with positional audio.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(Coord &coord)
{
    Voc_Play(*this, coord);

    /**
     *  New audio interface does not use sample handles, so just return INVALID_AUDIO_HANDLE.
     */
    return INVALID_AUDIO_HANDLE;
}


/**
 *  Static helper to play a sound effect by VocType with a variation and volume.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(VocType voc, int variation, float volume)
{
    Voc_Play((AudioVocClass &)*Vocs[voc], Coord(0,0,0), variation, volume);

    /**
     *  New audio interface does not use sample handles, so just return INVALID_AUDIO_HANDLE.
     */
    return INVALID_AUDIO_HANDLE;
}


/**
 *  Static helper to play a sound effect by VocType at the given volume.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(VocType voc, float volume)
{
    Voc_Play((AudioVocClass &)*Vocs[voc], Coord(0,0,0), -1, volume);

    /**
     *  New audio interface does not use sample handles, so just return INVALID_AUDIO_HANDLE.
     */
    return INVALID_AUDIO_HANDLE;
}


/**
 *  Static helper to play a sound effect by VocType at the given world coordinate.
 *
 *  @author: CCHyper
 */
int AudioVocClass::Play(VocType voc, Coord &coord)
{
    Voc_Play((AudioVocClass &)*Vocs[voc], coord);

    /**
     *  New audio interface does not use sample handles, so just return INVALID_AUDIO_HANDLE.
     */
    return INVALID_AUDIO_HANDLE;
}


/**
 *  Scans all registered vocs for available audio files and resolves their file info.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Scan()
{
    for (int index = 0; index < Vocs.Count(); ++index) {

        AudioVocClass *vocptr = (AudioVocClass *)Vocs[index];

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

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc: Loading isolated audio files...\n");

    /**
     *  Various sounds are loaded on the fly (dialog animation sound, etc), so we manually
     *  allocate these so they are a part of the new audio engine setup.
     */
    if (!VocEmblem) {
        VocEmblem = new AudioVocClass("EMBLEM");
        ASSERT(VocEmblem != nullptr);

        bool available = true;
        if (!AudioManager.Is_File_Available(VocEmblem->Name)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Voc::Scan - File \"%s\" was not found in any supported formats!\n", VocEmblem->Name.c_str());
            available = false;
        }
        VocEmblem->Available = available;
        VocEmblem->Type = AUDIO_SOUND_UI;
        VocEmblem->Control = AUDIO_CONTROL_INTERRUPT;
        VocEmblem->Group = AUDIO_GROUP_UI;
        AudioManager.Get_File_Info(VocEmblem->Name, VocEmblem->FileType, VocEmblem->FileName);
    }

#if 0//#ifndef NDEBUG
    AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Voc dump...\n");
    for (int index = 0; index < Voxs.Count(); ++index) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "  %03d  %s\n", index, ((AudioVoxClass*)Voxs[index])->Name.c_str());
    }
#endif

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
    for (int index = 0; index < Vocs.Count(); ++index) {

        AudioVocClass *vocptr = (AudioVocClass *)Vocs[index];

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
            AudioManager.Priority_To_AudioPriority(vocptr->Priority),
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

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process(enter): Vocs.Count = %d\n", Vocs.Count());

    //Clear();

    char buffer[32];

    if (ini.Is_Present(DEFAULTS)) {
        DefaultLimit = ini.Get_Int(DEFAULTS, "Limit", DefaultLimit);
        DefaultRange = ini.Get_Int(DEFAULTS, "Range", DefaultRange);
        DefaultType; // Not currently supported.
        DefaultControl; // Not currently supported.
        DefaultPriority = ini.Get_Int(DEFAULTS, "Priority", DefaultPriority);
        DefaultVolume = ini.Get_Float_Clamp(DEFAULTS, "Volume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, DefaultVolume);
        DefaultMinVolume = ini.Get_Float_Clamp(DEFAULTS, "MinVolume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, DefaultMinVolume);
        //DefaultMaxVolume = ini.Get_Float_Clamp(DEFAULTS, "MaxVolume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, DefaultMaxVolume); // Not to be loaded from the ini database.
    }

    if (ini.Is_Present(SOUNDLIST)) {

        int count = ini.Entry_Count(SOUNDLIST);

        for (int index = 0; index < count; ++index) {

            if (ini.Get_String(SOUNDLIST, ini.Get_Entry(SOUNDLIST, index), buffer, sizeof(buffer)-1) > 0) {

                VocType voc = From_Name(buffer);

                AudioVocClass *vocptr = nullptr;
                if (voc == VOC_NONE) {
                    vocptr = new AudioVocClass(buffer);
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process: Creating new Voc %s, processing.\n", vocptr->Name.c_str());
                    Vocs.Add((VocClass *)vocptr);

                } else {
                    vocptr = (AudioVocClass *)Vocs[voc];
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process: Found existing Voc %s, updating.\n", vocptr->Name.c_str());
                }
                vocptr->Read_INI(ini);

            }

        }

    }

#if 0//#ifndef NDEBUG
    AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Voc dump...\n");
    for (int index = 0; index < Vocs.Count(); ++index) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "  %03d  %s\n", index, ((AudioVocClass *)Vocs[index])->Name.c_str());
    }
#endif

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOC, "Voc::Process(exit): Vocs.Count = %d\n", Vocs.Count());
}


/**
 *  Destroys all registered voc instances and frees associated memory.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Clear()
{
    while (Vocs.Count() > 0) {
        //int index = Vocs.Count()-1;
        delete Vocs[0];
        //Vocs.Delete(index);
    }

    if (VocEmblem) {
        delete VocEmblem;
        VocEmblem = nullptr;
    }
}


/**
 *  Returns the VocType index for a given AudioVocClass pointer, or VOC_NONE if not found.
 *
 *  @author: CCHyper
 */
VocType AudioVocClass::VocType_From_Voc(AudioVocClass *voc)
{
    for (VocType index = VOC_FIRST; index < Vocs.Count(); ++index) {
        if ((AudioVocClass *)Vocs[index] == voc) {
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
        for (VocType index = VOC_FIRST; index < Vocs.Count(); ++index) {
            AudioVocClass *vocptr = (AudioVocClass *)Vocs[index];
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
        for (VocType index = VOC_FIRST; index < Vocs.Count(); ++index) {
            AudioVocClass *vocptr = (AudioVocClass *)Vocs[index];
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
    return (type != VOC_NONE && type < Vocs.Count() ? ((AudioVocClass *)Vocs[type])->Name.c_str() : "<none>");
}


bool AudioVocClass::Update_Position(Coord &coord)
{
    if (!IsSpecialEvent) {
        return false;
    }

    /**
     *  Skip if the sound group volume is muted.
     */
    if (AudioManager.Get_Group_Volume(Group) <= AUDIO_VOLUME_MIN) {
        return false;
    }

    if (coord == COORD_NONE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Update_Audio_Event - Invalid coord when updating \"%s\"!\n", Name.c_str());
        return false;
    }

    Cell cell = coord.As_Cell();
    if (cell == CELL_NONE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "Voc::Update_Audio_Event - Invalid cell when updating \"%s\"!\n", Name.c_str());
        return false;
    }

    float vol = AUDIO_VOLUME_MAX;
    float pitch = 1.0f;
    float pan = 0.0f;

    /**
     *  If the sound is no longer playing, restart it with updated parameters.
     */
    if (!AudioManager.Query_Is_Playing(Handle)) {

        std::string filename;

        /**
         *  Build the filename for this sound effect.
         */
        //if (voc.Sounds.Count() && (voc.Control & AUDIO_CONTROL_RANDOM) == 0) {
        //    std::string sound = voc.Sounds[Sim_Random_Pick(0, voc.Sounds.Count()-1)];
        //    filename = AudioManager.Build_Filename_From_Type(voc.FileType, sound);
        //
        //} else {
            filename = AudioManager.Build_Filename_From_Type(FileType, Name);
        //}

        //Init(filename);

        /**
         *  Apply a random volume shift if a range is defined.
         */
        float vshift = AUDIO_VOLUME_MIN;
        if (VolumeShift.X != 0 || VolumeShift.Y != 0) {
            vshift = Sim_Random_Pick_Float((float(VolumeShift.X) / float(AUDIO_VSHIFT_MAX)), (float(VolumeShift.Y) / float(AUDIO_VSHIFT_MAX)));
        }

        /**
         *  Apply a random frequency (pitch) shift if a range is defined.
         */
        float fshift = 0.0f;
        if (FrequencyShift.X != 0 || FrequencyShift.Y != 0) {
            fshift = Sim_Random_Pick_Float((float(FrequencyShift.X) / float(AUDIO_FSHIFT_MAX)), (float(FrequencyShift.Y) / float(AUDIO_FSHIFT_MAX)));
        }

        vol = std::min(std::clamp((Volume * vshift), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX), MaxVolume);
        pitch = 1.0f + fshift;
        
        AudioManager.Set_Volume(Handle, vol);
        AudioManager.Set_Pitch(Handle, pitch);

        AudioPriorityType priority = AudioManager.Priority_To_AudioPriority(Priority);

        // Special case where AUDIO_GROUP_EVENT is hardcoded so events play in a seperate group than sfx!
        AudioHandleID handle = AudioManager.Request_Play(filename, AUDIO_GROUP_EVENT, vol, pitch, pan, priority, Limit);

        if (handle == INVALID_AUDIO_HANDLE_ID) {
            DEBUG_ERROR("Voc::Update_Audio_Event - Failed to start \"%s\"!\n", Name.c_str());
            return false;
        }

        Handle = handle;

    }

    /**
     *  Adjust the volume and panning of the sound depending on its
     *  location to the tactical screen.
     */
    Voc_Calculate_Pan_And_Volume(*this, coord, pan, vol);

    /**
     *  If the volume drops below this level, just mute it. Otherwise the sounds
     *  will bleed into adjacent areas a little too far.
     */
    if (vol < AUDIO_VOLUME_CUTOFF_THRESHOLD) {
        AudioManager.Set_Volume(Handle, 0.0f);
        return true;
    }

    /**
     *  Can this sound only be heard if the cell is revealed?
     */
    if ((Type & AUDIO_SOUND_SHROUDED) != 0) {
        if (Map[cell].IsVisible|| Map[cell].IsFogVisible) {
            AudioManager.Set_Volume(Handle, 0.0f);
            return true;
        }

    /**
     *  Can this sound only be heard if the cell is unrevealed?
     */
    } else if ((Type & AUDIO_SOUND_UNSHROUDED) != 0) {
        if (!Map[cell].IsVisible && !Map[cell].IsFogVisible) {
            AudioManager.Set_Volume(Handle, 0.0f);
            return true;
        }
    }

    /**
     *  Update the audio event.
     */
    AudioManager.Set_Volume(Handle, pan);
    AudioManager.Set_Pan(Handle, pan);

    return true;
}


bool AudioVocClass::Stop()
{
    if (IsSpecialEvent) {
        // TODO, just leave it playing in silence for now?
        return AudioManager.Set_Volume(Handle, 0.0f);
    }

    return AudioManager.Request_Stop(Handle);
}


/**
 *  Sets the global sound effect volume across all SFX, UI, and event groups.
 *
 *  @author: CCHyper
 */
void AudioVocClass::Set_Volume(int volume)
{
    float volf = std::clamp(AudioManager.iVolume_To_fVolume(volume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_SFX, volf);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_UI, volf);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_EVENT, volf);
}
