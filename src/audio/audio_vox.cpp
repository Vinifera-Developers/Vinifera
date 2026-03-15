/*******************************************************************************
/*                  O P E N  S O U R C E -- V I N I F E R A                   **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_VOX.CPP
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         EVA speech/voice audio management.
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

#include "audio_vox.h"
#include "tibsun_globals.h"
#include "options.h"
#include "vox.h"
#include "scenario.h"
#include "house.h"
#include "housetype.h"
#include "side.h"
#include "ccini.h"
#include "audio_sample.h"
#include "audio_util.h"
#include "audio_debug.h"
#include "vinifera_util.h"
#include "asserthandler.h"
#include <algorithm>
#include <thread>


/**
 *  Global list of all registered speech (vox) entries.
 */
static DynamicVectorClass<AudioVoxClass *> Voxs;


/**
 *  Controls whether EVA speech playback is globally enabled.
 */
bool AudioVoxClass::IsSpeechAllowed = true;


/**
 *  The sound handle to the current speech being played. This is
 *  used to query if a speech is currently playing.
 */
static AudioHandleID SpeechHandle = INVALID_AUDIO_HANDLE_ID;


/**
 *  These are the defaults for all speeches loaded from the ini database.
 */
static AudioSoundType DefaultType = AUDIO_SOUND_VOICE;
static AudioControlType DefaultControl = AUDIO_CONTROL_QUEUE;
static int DefaultPriority = AudioManager.AudioPriority_To_Priority(AUDIO_PRIORITY_NORMAL);
static float DefaultDelay = 0.2f; // We add a fake delay so the speech is not played on top of the building placement sound effect etc.
static int DefaultFrequencyShift = 1.0f;
static float DefaultVolume = AUDIO_VOLUME_MAX;
static float DefaultMinVolume = AUDIO_VOLUME_MIN;
static float DefaultMaxVolume = AUDIO_VOLUME_MAX;


/**
 *  Synchronisation primitives for the asynchronous vox scan thread.
 */
static std::mutex VoxScanMutex;
static std::atomic<bool> IsVoxScanComplete{false};


/**
 *  Constructor for a speech entry, initialised with default values.
 *
 *  @author: CCHyper
 */
AudioVoxClass::AudioVoxClass(std::string name) :
    FileType(AUDIO_TYPE_AUD),
    FileName(),
    Available(false),
    Name(name),
    Sound(),
    Priority(DefaultPriority),
    Volume(DefaultVolume),
    MinVolume(DefaultMinVolume),
    MaxVolume(DefaultMaxVolume),
    Delay(DefaultDelay),
    FrequencyShift(DefaultFrequencyShift),
    Type(DefaultType),
    Control(DefaultControl)
{
    string_to_upper(Name);

    Voxs.Add(this);
}


/**
 *  Destructor; removes this entry from the global vox list.
 *
 *  @author: CCHyper
 */
AudioVoxClass::~AudioVoxClass()
{
    Voxs.Delete(this);
}


/**
 *  Reads this speech entry's properties from the INI database.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Read_INI(CCINIClass &ini)
{
    char buffer[256];
    const char *name = Name.c_str();

    if (!ini.Is_Present(name)) {
        return;
    }

    ini.Get_String(name, "Sound", buffer, sizeof(buffer));
    Sound = buffer;

    ini.Get_String(name, "Text", buffer, sizeof(buffer));
    DescriptionText = buffer;

    Priority = ini.Get_Int(name, "Priority", DefaultPriority);
    DefaultDelay = ini.Get_Int(name, "Delay", DefaultDelay);
    Volume = ini.Get_Float_Clamp(name, "Volume", 0.0f, 1.0f, DefaultVolume);
    //Delay = ini.Get_Float_Clamp(name, "Delay", 0.0f, 5.0f, DefaultDelay); // Not to be read from the ini database.
    FrequencyShift = ini.Get_Float_Clamp(name, "FShift", -5.0f, 5.0f, FrequencyShift);

    // TODO, we should loop the HouseTypes and load GDI=, Nod= etc based of the names of the HouseTypes!
}


/**
 *  One-time initialisation; creates AudioVoxClass instances for all built-in speech entries.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::One_Time()
{
    for (VoxType vox = VOX_FIRST; vox < VOX_COUNT; ++vox) {
        AudioVoxClass *voxptr = new AudioVoxClass(Speech[vox]);
        ASSERT(voxptr != nullptr);
    }

#ifndef NDEBUG
    // To help find duplicate loading errors.
    ASSERT_FATAL_PRINT(Voxs.Count() == VOX_COUNT, "Voxs.Count == %d, VOX_COUNT == %d.", Voxs.Count(), VOX_COUNT);
#endif
}


/**
 *  Processes the speech INI database, loading defaults and creating/updating vox entries.
 *
 *  @author: CCHyper
 */
bool AudioVoxClass::Process(CCINIClass &ini)
{
    static char const * const DEFAULTS = "Defaults";
    static char const * const SOUNDLIST = "DialogList";

    char buffer[32];

    //Clear();

    /**
     *  Load the global default values for speech entries.
     */
    if (ini.Is_Present(DEFAULTS)) {
        DefaultPriority = ini.Get_Int(DEFAULTS, "Priority", DefaultPriority);
        DefaultDelay = ini.Get_Float(DEFAULTS, "Delay", DefaultDelay);
        DefaultVolume = ini.Get_Float_Clamp(DEFAULTS, "Volume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, DefaultVolume);
        DefaultMinVolume = ini.Get_Float_Clamp(DEFAULTS, "MinVolume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, DefaultMinVolume);
        //DefaultMaxVolume = ini.Get_Float_Clamp(DEFAULTS, "MaxVolume", AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX, DefaultMaxVolume); // Not to be loaded from the ini database.
    }

    if (ini.Is_Present(SOUNDLIST)) {

        int counter = ini.Entry_Count(SOUNDLIST);

        for (int index = 0; index < counter; ++index) {

            if (ini.Get_String(SOUNDLIST, ini.Get_Entry(SOUNDLIST, index), buffer, sizeof(buffer)-1)) {
                VoxType vox = From_Name(buffer);

                AudioVoxClass *voxptr = nullptr;
                if (vox == VOX_NONE) {
                    voxptr = new AudioVoxClass(buffer);
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::Process: Creating new Vox %s.\n", voxptr->Name.c_str());
                    Voxs.Add(voxptr);

                } else {
                    voxptr = Voxs[vox];
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::Process: Found exiting Vox %s.\n", voxptr->Name.c_str());
                }
                voxptr->Read_INI(ini);
            }

        }

    }

#ifndef NDEBUG
    // To help find duplicate loading errors.
    ASSERT_FATAL_PRINT(Voxs.Count() == VOX_COUNT, "Voxs.Count == %d, VOX_COUNT == %d.", Voxs.Count(), VOX_COUNT);
#endif

    return true;
}


/**
 *  Scans all registered vox entries for available audio files and resolves their file info.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Scan()
{
    for (int index = 0; index < Voxs.Count(); ++index) {

        AudioVoxClass *voxptr = Voxs[index];

        /**
         *  Use the sound name override if one has been specified.
         */
        std::string name = voxptr->Name;
        if (!voxptr->Sound.empty()) {
            name = voxptr->Sound;
        }

        if (!AudioManager.Is_File_Available(name)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Scan - File \"%s\" was not found in any supported formats!\n", name.c_str());
            continue;
        }

        voxptr->Available = true;

        /**
         *  Resolve the file type and full filename for this vox entry.
         */
        AudioManager.Get_File_Info(name, voxptr->FileType, voxptr->FileName);
    }

#if 0//#ifndef NDEBUG
    AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox dump...\n");
    for (int index = 0; index < Voxs.Count(); ++index) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "  %03d  %s\n", index, ((AudioVoxClass *)Voxs[index])->Name.c_str());
    }
#endif

    // Call preload here to reduce patches.
    Preload();
}


/**
 *  Submits all available vox samples to the audio manager for preloading.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Preload()
{
    for (int index = 0; index < Voxs.Count(); ++index) {

        AudioVoxClass *voxptr = Voxs[index];

        if (!voxptr->Available) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Preload - File \"%s\" was not found in any supported formats!\n", voxptr->Name.c_str());
            continue;
        }

        if (AudioManager.Has_Been_Submitted(voxptr->FileName, AUDIO_GROUP_SPEECH)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Vox::Preload - File \"%s\" has already been submitted to the audio manager!\n", voxptr->Name.c_str());
            continue;
        }

        /**
         *  Submit this speech sample to the audio manager with its configured properties.
         */
        bool submitted = AudioManager.Submit_Sample(
            voxptr->FileName,
            voxptr->FileType,
            AUDIO_GROUP_SPEECH,
            AudioManager.Priority_To_AudioPriority(voxptr->Priority),
            voxptr->Control,
            voxptr->Type,
            1);

        if (submitted) {
            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::Preload - Submitted \"%s\" to audio manager.\n", voxptr->FileName.c_str());
        } else {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::Preload - Failed to submit \"%s\" to audio manager!\n", voxptr->FileName.c_str());
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
void AudioVoxClass::ScanAsync()
{
    std::thread([] {
        IsVoxScanComplete.store(false);
        Scan(); // Also calls preload for us
        IsVoxScanComplete.store(true);
    }).detach(); // Fire-and-forget
}


/**
 *  Destroys all registered vox instances and frees associated memory.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Clear()
{
    while (Voxs.Count() > 0) {
        //int index = Voxs.Count()-1;
        delete Voxs[0];
        //Voxs.Delete(index);
    }

#ifndef NDEBUG
    // To help find duplicate loading errors.
    ASSERT_FATAL_PRINT(Voxs.Count() == 0, "Voxs.Count == %d.", Voxs.Count());
#endif
}


/**
 *  EVA speaks to the player.  
 * 
 *  @author: CCHyper
 */
void AudioVoxClass::Speak(VoxType voice, bool force)
{
    //ASSERT(voice != VOX_NONE);    // Removed, triggers when some superweapons enable for some reason...
    ASSERT(voice < VOX_COUNT);

    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return;
    }

    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_SPEECH) <= 0.0f) {
        return;
    }

    if (voice == VOX_NONE || voice == SpeakQueue || voice == CurrentVoice) {
        return;
    }

    if (SpeakQueue != VOX_NONE) {
        return;
    }

    if (!IsSpeechAllowed) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Speak - Speech is disabled!\n");
        return;
    }

    AudioVoxClass *voxptr = Voxs[voice];
    if (!voxptr) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::Speak - voxptr is null!\n");
        return;
    }

    SpeakQueue = voice;

    if (force) {
        SpeakTimer = 0;
        AI();
        return;
    }

    if (SpeakTimer.Expired()) {
        AI();
        return;
    }

    SpeakTimer = 60;
}


/**
 *  Handles starting the EVA voices.
 * 
 *  @author: CCHyper
 */
void AudioVoxClass::AI()
{
    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return;
    }

    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_SPEECH) <= 0.0f) {
        return;
    }

    if (!SpeakTimer.Expired()) {
        return;
    }

    if (CurrentVoice != VOX_NONE && AudioManager.Query_Is_Playing(SpeechHandle)) {
        CurrentVoice = VOX_NONE;
        return;
    }

    if (SpeakQueue != VOX_NONE && AudioManager.Query_Is_Playing(SpeechHandle)) {
        return;
    }

    if (SpeakQueue == VOX_NONE) {
        return;
    }

    CurrentVoice = VOX_NONE;

    AudioVoxClass *voxptr = Voxs[SpeakQueue];
    if (!voxptr) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::AI - voxptr is null!\n");
        return;
    }

    std::string name = voxptr->Name;
    std::string filename = voxptr->FileName;
    
    // Removed, fails to play NOD speech.
#if 1
    /// Urgh, this is a mega bug...
    /**
     *  Only allow EVA speeches to be played by the speech handler. This has been
     *  expanded compared to the original code so up to 99 sides are supported, based
     *  on the currently set SpeechSide of the mission.
     */
    if (name[3] == 'I') {

        bool is_valid = false;
        int match_index = -1;
        char buffer[4];

        std::string _name = name;
        std::string _filename = filename;

        SideType side = Scen->SpeechSide;
        if (side == SIDE_NONE) {
            side = PlayerPtr->Class->Side;
        }

        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::Speak - Remapping %s to side %d (%s).\n", _name.c_str(), side, SideClass::Name_From(side));

        std::snprintf(buffer, sizeof(buffer), "%02d-", side);
        std::string _buff = buffer;

        string_trim(_name, 0, 3);
        string_trim(_filename, 0, 3);

        _name = _buff + _name;
        _filename = _buff + _filename;

        if (CCFileClass(_filename.c_str()).Is_Available()) {

            name = _name;
            filename = _filename;

        } else {

            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Speak - Vox \"%s\" does not exist, restoring original name \"%s\"!\n", _name.c_str(), name.c_str());

        }

#if 0
        for (int index = 0; index < 99; ++index) {
            std::snprintf(buffer, sizeof(buffer), "%02d-", index);
            if (voxptr->Name.Contains(buffer)) {
                match_index = index;
                is_valid = true;
                break;
            }
        }

        if (!is_valid || match_index == -1) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Speak - Vox \"%s\" is not a valid speech file!\n", _name.c_str());
            return;
        }

        if (match_index != side) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Speak - Vox \"%s\" does not match the player side!\n", _name.c_str());
        }
#endif

    }

#endif

    AudioHandleID handle = INVALID_AUDIO_HANDLE_ID;

    // As Vox instances are now preloaded in a new thread, we must use a mutex
    {
        std::lock_guard<std::mutex> lock(VoxScanMutex);

        /**
         *  Speech file was found, now play it.
         */
        VoxType vox = SpeakQueue;
        float vol = std::clamp(voxptr->Volume, voxptr->MinVolume, voxptr->MaxVolume);
        float pitch = voxptr->FrequencyShift;
        float delay = voxptr->Delay;
        AudioPriorityType priority = AudioManager.Priority_To_AudioPriority(voxptr->Priority);
        AudioSoundType type = voxptr->Type;
        AudioControlType control = voxptr->Control;

        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::AI - About to call AudioManager.Play with \"%s\".\n", filename.c_str());
        handle = AudioManager.Request_Play(filename,
                                                        AUDIO_GROUP_SPEECH,
                                                        vol,
                                                        pitch,
                                                        0.0f,
                                                        priority,
                                                        0.0f,
                                                        delay);
    }

    if (handle == INVALID_AUDIO_HANDLE_ID) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::AI - Failed to play \"%s\"!\n", name.c_str());
        SpeakQueue = VOX_NONE;
        return;
    }

    if (AudioManager.Query_Is_Playing(SpeechHandle)) {
        AudioManager.Request_Stop(SpeechHandle, 0.5f); // A small fade out sounds better.
    }

    SpeechHandle = handle;
    CurrentVoice = SpeakQueue;

    SpeakQueue = VOX_NONE;
}


/**
 *  Forces the EVA voice to stop talking.
 * 
 *  @author: CCHyper
 */
void AudioVoxClass::Stop_Speaking()
{
    SpeakQueue = VOX_NONE;

    AudioManager.Request_Stop(SpeechHandle, 0.5f); // sounds better when it fades out.
}


/**
 *  Checks to see if the eva voice is still playing.
 * 
 *  @author: CCHyper
 */
bool AudioVoxClass::Is_Speaking()
{
    Speak_AI();

    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return false;
    }

    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_SPEECH) <= 0.0f) {
        return false;
    }

    return SpeakQueue != VOX_NONE && AudioManager.Query_Is_Playing(SpeechHandle);
}


/**
 *  Sets the global speech volume to that specified.
 * 
 *  @author: CCHyper
 */
void AudioVoxClass::Set_Speech_Volume(int vol)
{
    float volf = std::clamp(float(vol/255.0f), 0.0f, 1.0f);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_SPEECH, volf);
}


#ifndef NDEBUG
/**
 *  Writes out the default speech file.
 *
 *  @author: CCHyper
 */
bool AudioVoxClass::Write_Default_Speech_INI(CCINIClass &ini)
{
    static char const * const DEFAULTS = "Defaults";
    static char const * const SPEECHLIST = "SpeechList";

    char buffer[256];

    /**
     *  Clear out all existing base data from the ini file.
     */
    ini.Clear(DEFAULTS);
    ini.Clear(SPEECHLIST);

    ini.Clear();

    /**
     *  Save the default sound values.
     */
    //ini.Put_Int(DEFAULTS, "Priority", DefaultPriority);
    ini.Put_Float(DEFAULTS, "Delay", DefaultDelay);
    ini.Put_Float(DEFAULTS, "Volume", DefaultVolume);
    ini.Put_Float(DEFAULTS, "MinVolume", DefaultMinVolume);
    ini.Put_Float(DEFAULTS, "MaxVolume", DefaultMaxVolume);

    /**
     *  Write out each speech entry to the SpeechList section.
     */
    for (VoxType vox = VOX_FIRST; vox < std::size(Speech); ++vox) {

        const char * vox_name = Speech[vox];

        /**
         *  Format the entry index as the INI key.
         */
        char entrybuff[8];
        std::snprintf(entrybuff, sizeof(entrybuff), "%d", vox);

        ini.Put_String(SPEECHLIST, entrybuff, vox_name);

        /**
         *  Now write the keys for its section.
         */
        //ini.Put_Int(vox_name, "Priority", DefaultPriority);

    }

    return true;
}
#endif


/**
 *  Looks up a VoxType by name, returning VOX_NONE if not found.
 *
 *  @author: CCHyper
 */
VoxType AudioVoxClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (!strcasecmp(name, "<none>") || !strcasecmp(name, "none")) {
        return VOX_NONE;
    }

    if (name != nullptr) {
        for (VoxType index = VOX_FIRST; index < Voxs.Count(); ++index) {
            AudioVoxClass *vocptr = Voxs[index];
            if (vocptr->Name == name) {
                return index;
            }
        }
    }

    return VOX_NONE;
}


/**
 *  Returns the name string for a given VoxType, or "<none>" if invalid.
 *
 *  @author: CCHyper
 */
const char *AudioVoxClass::Name_From(VoxType type)
{
    return (type != VOX_NONE && type < Voxs.Count() ? (Voxs[type])->Name.c_str() : "<none>");
}


/**
 *  Enables or disables EVA speech playback globally.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Set_Speech_Allowed(bool set)
{
    IsSpeechAllowed = set;
}


/**
 *  Returns whether EVA speech playback is currently allowed.
 *
 *  @author: CCHyper
 */
bool AudioVoxClass::Is_Speech_Allowed()
{
    return IsSpeechAllowed;
}
