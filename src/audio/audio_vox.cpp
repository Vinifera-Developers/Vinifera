/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  EVA speech/voice audio management.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_vox.h"

#include "asserthandler.h"
#include "audio_debug.h"
#include "audio_sample.h"
#include "ccini.h"
#include "house.h"
#include "housetype.h"
#include "options.h"
#include "scenario.h"
#include "side.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "vox.h"

#include <algorithm>
#include <thread>


/**
 *  Global list of all registered speech (vox) entries.
 */
static DynamicVectorClass<AudioVoxClass *> Voxs;


/**
 *  AudioVoxClass statics.
 */
bool AudioVoxClass::IsSpeechAllowed = true;
int AudioVoxClass::DefaultPriority = AudioManagerClass::AudioPriority_To_Priority(AUDIO_PRIORITY_NORMAL);
float AudioVoxClass::DefaultDelay = 0.2f; // We add a fake delay so the speech is not played on top of the building placement sound effect etc.
float AudioVoxClass::DefaultFrequencyShift = 1.0f;
float AudioVoxClass::DefaultVolume = AUDIO_VOLUME_MAX;
float AudioVoxClass::DefaultMinVolume = AUDIO_VOLUME_MIN;
float AudioVoxClass::DefaultMaxVolume = AUDIO_VOLUME_MAX;


/**
 *  The sound handle to the current speech being played. This is
 *  used to query if a speech is currently playing.
 */
static AudioHandleID SpeechHandle = INVALID_AUDIO_HANDLE_ID;


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
    Name(name)
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
    const char *name = Name.c_str();

    if (!ini.Is_Present(name)) {
        return;
    }

    Sound = ini.Get_String(name, "Sound", Sound);
    DescriptionText = ini.Get_String(name, "Text", DescriptionText);

    if (ini.Is_Present(name, "Priority")) {
        Priority = ini.Get_Int(name, "Priority", Get_Priority());
    }
    if (ini.Is_Present(name, "Volume")) {
        Volume = std::clamp<float>(ini.Get_Float(name, "Volume", Get_Volume()), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }
    //if (ini.Is_Present(name, "Delay")) {
    //    Delay = std::clamp<float>(ini.Get_Float(name, "Delay", Get_Delay()), 0.0f, 5.0f);
    //}
    if (ini.Is_Present(name, "Delay")) {
        FrequencyShift = std::clamp<float>(ini.Get_Float(name, "FShift", Get_FrequencyShift()), -5.0f, 5.0f);
    }

    if (ini.Is_Present(name, "MinVolume")) {
        MinVolume = std::clamp<float>(ini.Get_Float(name, "MinVolume", Get_MinVolume()), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }
    if (ini.Is_Present(name, "MaxVolume")) {
        MaxVolume = std::clamp<float>(ini.Get_Float(name, "MaxVolume", Get_MaxVolume()), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }

    // Read the per-side filenames
    SideSounds.resize(Sides.Count());
    for (int i = 0; i < Sides.Count(); i++) {
        SideSounds[i] = ini.Get_String(name, Sides[i]->IniName.c_str(), SideSounds[i]);
    }
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

    /**
     *  Load the global default values for speech entries.
     */
    if (ini.Is_Present(DEFAULTS)) {
        DefaultPriority = ini.Get_Int(DEFAULTS, "Priority", DefaultPriority);
        DefaultDelay = ini.Get_Float(DEFAULTS, "Delay", DefaultDelay);
        DefaultVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "Volume", DefaultVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
        DefaultMinVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "MinVolume", DefaultMinVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
        DefaultMaxVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "MaxVolume", DefaultMaxVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }

    if (ini.Is_Present(SOUNDLIST)) {

        int counter = ini.Entry_Count(SOUNDLIST);

        for (int index = 0; index < counter; ++index) {

            if (ini.Get_String(SOUNDLIST, ini.Get_Entry(SOUNDLIST, index), "", buffer, sizeof(buffer)-1)) {
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
        if (!voxptr->Get_Sound_Name().empty()) {
            name = voxptr->Get_Sound_Name();
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
            AudioManagerClass::Priority_To_AudioPriority(voxptr->Get_Priority()),
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
        std::scoped_lock lock(VoxScanMutex);

        /**
         *  Speech file was found, now play it.
         */
        VoxType vox = SpeakQueue;
        float vol = std::clamp(voxptr->Get_Volume(), voxptr->Get_MinVolume(), voxptr->Get_MaxVolume());
        float pitch = voxptr->Get_FrequencyShift();
        float delay = voxptr->Get_Delay();
        AudioPriorityType priority = AudioManagerClass::Priority_To_AudioPriority(voxptr->Get_Priority());
        AudioSoundType type = voxptr->Type;
        AudioControlType control = voxptr->Control;

        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::AI - About to call AudioManager.Play with \"%s\".\n", filename.c_str());
        handle = AudioManager.Request_Play(filename, AUDIO_GROUP_SPEECH, vol, pitch, 0.0f, priority, 0.0f, delay);
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
    ini.Put_Int(DEFAULTS, "Priority", DefaultPriority);
    ini.Put_Float(DEFAULTS, "Delay", DefaultDelay);
    ini.Put_Float(DEFAULTS, "FrequencyShift", DefaultFrequencyShift);
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


std::string const& AudioVoxClass::Get_Sound_Name() const
{
    if (Scen != nullptr && Scen->SpeechSide != SIDE_NONE) {
        if (!SideSounds[Scen->SpeechSide].empty()) {
            return SideSounds[Scen->SpeechSide];
        }
    }
    return Sound;
}
