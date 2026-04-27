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
#include "extension_globals.h"
#include "house.h"
#include "housetype.h"
#include "options.h"
#include "scenario.h"
#include "side.h"
#include "tacticalext.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "vox.h"

#include <algorithm>
#include <cstring>
#include <deque>
#include <thread>


/**
 *  Global list of all registered speech (vox) entries.
 */
DynamicVectorClass<AudioVoxClass *> Voxs;


/**
 *  AudioVoxClass statics.
 */
VoxPriorityType AudioVoxClass::DefaultPriority = VOX_PRIORITY_NORMAL;
float AudioVoxClass::DefaultDelay = 0.2f; // We add a fake delay so the speech is not played on top of the building placement sound effect etc.
float AudioVoxClass::DefaultFrequencyShift = 1.0f;
float AudioVoxClass::DefaultVolume = AUDIO_VOLUME_MAX;
float AudioVoxClass::DefaultMinVolume = AUDIO_VOLUME_MIN;
float AudioVoxClass::DefaultMaxVolume = AUDIO_VOLUME_MAX;


/**
 *  The sound handle to the current speech being played. This is
 *  used to query if a speech is currently playing.
 */
static AudioInstanceHandle SpeechHandle = INVALID_AUDIO_INSTANCE_HANDLE;


/**
 *  Vinifera-owned speech queue state. The vanilla TS SpeakQueue/CurrentVoice
 *  globals (declared in TSpp/src/vox.h) are no longer used.
 */
namespace {

struct VoxQueueEntry
{
    VoxType voice;
    VoxPriorityType priority;
};

std::deque<VoxQueueEntry> NormalQueue;
std::deque<VoxQueueEntry> InterruptQueue;
VoxType ActiveVoice = VOX_NONE;


/**
 *  Insert into a queue keeping it sorted by priority desc, FIFO within priority.
 */
void Insert_Sorted(std::deque<VoxQueueEntry>& q, VoxQueueEntry entry)
{
    auto it = std::find_if(q.begin(), q.end(),
        [&](const VoxQueueEntry& x) { return x.priority < entry.priority; });
    q.insert(it, entry);
}


/**
 *  Returns true if the voice is already pending in either queue.
 */
bool Is_Queued(VoxType voice)
{
    auto match = [&](const VoxQueueEntry& x) { return x.voice == voice; };
    return std::any_of(NormalQueue.begin(), NormalQueue.end(), match)
        || std::any_of(InterruptQueue.begin(), InterruptQueue.end(), match);
}


/**
 *  Map our queue priority enum to the audio engine's priority enum so the
 *  sample-slot arbitration in the audio thread stays in sync.
 */
AudioPriorityType To_Audio_Priority(VoxPriorityType p)
{
    switch (p) {
    case VOX_PRIORITY_LOW:       return AUDIO_PRIORITY_LOW;
    case VOX_PRIORITY_NORMAL:    return AUDIO_PRIORITY_NORMAL;
    case VOX_PRIORITY_IMPORTANT: return AUDIO_PRIORITY_HIGH;
    case VOX_PRIORITY_CRITICAL:  return AUDIO_PRIORITY_CRITICAL;
    }
    return AUDIO_PRIORITY_NORMAL;
}


/**
 *  Parse a priority name (case-insensitive). Returns the fallback if no match.
 */
VoxPriorityType Priority_From_Name(const char* name, VoxPriorityType fallback)
{
    if (name == nullptr || name[0] == '\0') return fallback;
    if (stricmp(name, "LOW") == 0)       return VOX_PRIORITY_LOW;
    if (stricmp(name, "NORMAL") == 0)    return VOX_PRIORITY_NORMAL;
    if (stricmp(name, "IMPORTANT") == 0) return VOX_PRIORITY_IMPORTANT;
    if (stricmp(name, "CRITICAL") == 0)  return VOX_PRIORITY_CRITICAL;
    return fallback;
}


const char* Name_From_Priority(VoxPriorityType p)
{
    switch (p) {
    case VOX_PRIORITY_LOW:       return "LOW";
    case VOX_PRIORITY_NORMAL:    return "NORMAL";
    case VOX_PRIORITY_IMPORTANT: return "IMPORTANT";
    case VOX_PRIORITY_CRITICAL:  return "CRITICAL";
    }
    return "NORMAL";
}


/**
 *  Parse a control name (case-insensitive). Returns the fallback if no match.
 */
VoxControlType Control_From_Name(const char* name, VoxControlType fallback)
{
    if (!name || !*name) return fallback;
    if (stricmp(name, "QUEUE") == 0)             return VOX_CONTROL_QUEUE;
    if (stricmp(name, "STANDARD") == 0)          return VOX_CONTROL_STANDARD;
    if (stricmp(name, "INTERRUPT") == 0)         return VOX_CONTROL_INTERRUPT;
    if (stricmp(name, "QUEUED_INTERRUPT") == 0)  return VOX_CONTROL_QUEUED_INTERRUPT;
    return fallback;
}

} // namespace


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
    Name(std::move(name))
{
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
void AudioVoxClass::Read_INI(CCINIClass const& ini)
{
    const char *name = Name.c_str();

    if (!ini.Is_Present(name)) {
        return;
    }

    Sound = ini.Get_String(name, "Sound", Sound);
    DescriptionText = ini.Get_String(name, "Text", DescriptionText);

    char catbuf[32];
    if (ini.Get_String(name, "Category", "", catbuf, sizeof(catbuf)) > 0) {
        if (stricmp(catbuf, "Story") == 0) {
            SubtitleCategory = SUBTITLE_CATEGORY_STORY;
        } else if (stricmp(catbuf, "System") == 0) {
            SubtitleCategory = SUBTITLE_CATEGORY_SYSTEM;
        }
    }

    if (ini.Is_Present(name, "Priority")) {
        char prioritybuf[32];
        if (ini.Get_String(name, "Priority", "", prioritybuf, sizeof(prioritybuf)) > 0) {
            Priority = Priority_From_Name(prioritybuf, Get_Priority());
        }
    }
    if (ini.Is_Present(name, "Volume")) {
        Volume = std::clamp<float>(ini.Get_Float(name, "Volume", Get_Volume()), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }
    if (ini.Is_Present(name, "Delay")) {
        Delay = std::clamp<float>(ini.Get_Float(name, "Delay", Get_Delay()), 0.0f, 5.0f);
    }
    if (ini.Is_Present(name, "FShift")) {
        FrequencyShift = std::clamp<float>(ini.Get_Float(name, "FShift", Get_FrequencyShift()), 0.1f, 2.0f);
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

    char buffer[64];
    if (ini.Get_String(name, "Control", "", buffer, sizeof(buffer)) > 0) {
        Control = Control_From_Name(buffer, Control);
    }
}


/**
 *  One-time initialisation; creates AudioVoxClass instances for all built-in speech entries.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::One_Time()
{
    for (VoxType vox = VOX_FIRST; vox < std::size(EvaNames); ++vox) {
        AudioVoxClass *voxptr = new AudioVoxClass(EvaNames[vox]);
        voxptr->Sound = Speech[vox];
        if (strstr(EvaNames[vox], "EVA_") != nullptr) {
            voxptr->SubtitleCategory = SUBTITLE_CATEGORY_SYSTEM;
        }
    }
}


/**
 *  Processes the speech INI database, loading defaults and creating/updating vox entries.
 *
 *  @author: CCHyper
 */
bool AudioVoxClass::Process(CCINIClass const& ini)
{
    static char const * const DEFAULTS = "Defaults";
    static char const * const DIALOGLIST = "DialogList";

    /**
     *  Load the global default values for speech entries.
     */
    if (ini.Is_Present(DEFAULTS)) {
        char prioritybuf[32];
        if (ini.Get_String(DEFAULTS, "Priority", "", prioritybuf, sizeof(prioritybuf)) > 0) {
            DefaultPriority = Priority_From_Name(prioritybuf, DefaultPriority);
        }
        DefaultDelay = ini.Get_Float(DEFAULTS, "Delay", DefaultDelay);
        DefaultFrequencyShift = std::clamp<float>(ini.Get_Float(DEFAULTS, "FShift", DefaultFrequencyShift), 0.1f, 2.0f);
        DefaultVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "Volume", DefaultVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
        DefaultMinVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "MinVolume", DefaultMinVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
        DefaultMaxVolume = std::clamp<float>(ini.Get_Float(DEFAULTS, "MaxVolume", DefaultMaxVolume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }

    if (ini.Is_Present(DIALOGLIST)) {
        int count = ini.Entry_Count(DIALOGLIST);

        for (int index = 0; index < count; ++index) {
            std::string name = ini.Get_String(DIALOGLIST, ini.Get_Entry(DIALOGLIST, index), "");
            if (!name.empty()) {
                VoxType vox = From_Name(name.c_str());

                AudioVoxClass *voxptr = nullptr;
                if (vox == VOX_NONE) {
                    voxptr = new AudioVoxClass(name);
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::Process: Creating new Vox %s.\n", voxptr->Name.c_str());
                } else {
                    voxptr = Voxs[vox];
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::Process: Found existing Vox %s.\n", voxptr->Name.c_str());
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
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Preload - File \"%s\" has already been submitted to the audio manager!\n", voxptr->Name.c_str());
            continue;
        }

        /**
         *  Submit this speech sample to the audio manager. Speech is always voice-typed
         *  and the queue/interrupt logic is owned by AudioVoxClass, not the audio engine,
         *  so we hardcode AUDIO_SOUND_VOICE / AUDIO_CONTROL_NORMAL here.
         */
        bool submitted = AudioManager.Submit_Sample(
            voxptr->FileName,
            voxptr->FileType,
            AUDIO_GROUP_SPEECH,
            To_Audio_Priority(voxptr->Get_Priority()),
            AUDIO_CONTROL_NORMAL,
            AUDIO_SOUND_VOICE,
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
        Scan();
        IsVoxScanComplete.store(true);
    }).detach();
}


/**
 *  Destroys all registered vox instances and frees associated memory.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Clear()
{
    while (Voxs.Count() > 0) {
        delete Voxs[0];
    }
}


/**
 *  EVA speaks to the player.
 *
 *  @author: CCHyper, ZivDero
 */
void AudioVoxClass::Speak(VoxType voice, bool now)
{
    ASSERT(voice < Voxs.Count());

    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return;
    }

    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_SPEECH) <= 0.0f) {
        return;
    }

    if (voice == VOX_NONE || voice == ActiveVoice || Is_Queued(voice)) {
        return;
    }

    if (!SpeechEnabled) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Speak - Speech is disabled!\n");
        return;
    }

    AudioVoxClass *voxptr = Voxs[voice];
    if (!voxptr) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::Speak - voxptr is null!\n");
        return;
    }

    const VoxControlType ctrl = now ? VOX_CONTROL_INTERRUPT : voxptr->Control;
    const VoxQueueEntry entry { voice, voxptr->Get_Priority() };

    switch (ctrl) {
    case VOX_CONTROL_STANDARD:
        if (ActiveVoice != VOX_NONE || !NormalQueue.empty() || !InterruptQueue.empty()) {
            return;
        }
        InterruptQueue.push_front(entry);
        SpeakTimer = 0;
        AI();
        return;

    case VOX_CONTROL_INTERRUPT:
        if (AudioManager.Query_Is_Active(SpeechHandle)) {
            AudioManager.Request_Stop(SpeechHandle, 0.25f);
        }
        ActiveVoice = VOX_NONE;
        NormalQueue.clear();
        InterruptQueue.clear();
        InterruptQueue.push_back(entry);
        SpeakTimer = 0;
        AI();
        return;

    case VOX_CONTROL_QUEUED_INTERRUPT:
        Insert_Sorted(InterruptQueue, entry);
        break;

    case VOX_CONTROL_QUEUE:
    default:
        Insert_Sorted(NormalQueue, entry);
        break;
    }

    if (SpeakTimer.Expired()) {
        AI();
        return;
    }

    SpeakTimer = 60;
}


/**
 *  EVA speaks to the player.
 *
 *  @author: ZivDero
 */
void AudioVoxClass::Speak(std::string const& name, bool now)
{
    VoxType voice = From_Name(name.c_str());
    if (voice >= VOX_FIRST && voice < Voxs.Count()) {
        Speak(voice, now);
    } 
}


/**
 *  Handles starting the EVA voices.
 *
 *  @author: CCHyper, ZivDero
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

    if (ActiveVoice != VOX_NONE) {
        if (AudioManager.Query_Is_Active(SpeechHandle)) {
            return;
        }

        ActiveVoice = VOX_NONE;
        if (TacticalMapExtension) {
            TacticalMapExtension->Clear_Subtitle();
        }
    }

    if (InterruptQueue.empty() && NormalQueue.empty()) {
        return;
    }

    /**
     *  Pop the next entry: interrupt queue drains before the normal queue.
     */
    std::deque<VoxQueueEntry>& source = !InterruptQueue.empty() ? InterruptQueue : NormalQueue;
    VoxQueueEntry next = source.front();
    source.pop_front();

    AudioVoxClass *voxptr = Voxs[next.voice];
    if (!voxptr) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::AI - voxptr is null!\n");
        return;
    }

    std::string name = voxptr->Name;
    std::string filename = voxptr->FileName;

    AudioInstanceHandle handle = INVALID_AUDIO_INSTANCE_HANDLE;

    {
        std::scoped_lock lock(VoxScanMutex);

        /**
         *  Speech file was found, now play it.
         */
        float vol = std::clamp(voxptr->Get_Volume(), voxptr->Get_MinVolume(), voxptr->Get_MaxVolume());
        float pitch = voxptr->Get_FrequencyShift();
        float delay = voxptr->Get_Delay();
        AudioPriorityType priority = To_Audio_Priority(voxptr->Get_Priority());

        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::AI - About to call AudioManager.Play with \"%s\".\n", filename.c_str());
        handle = AudioManager.Request_Play(filename, AUDIO_GROUP_SPEECH, vol, pitch, 0.0f, priority, 1, 0.0f, delay);
    }

    if (handle == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::AI - Failed to play \"%s\"!\n", name.c_str());
        return;
    }

    if (AudioManager.Query_Is_Active(SpeechHandle)) {
        AudioManager.Request_Stop(SpeechHandle, 0.5f);
    }

    SpeechHandle = handle;
    ActiveVoice = next.voice;

    if (TacticalMapExtension) {
        if (!voxptr->DescriptionText.empty()) {
            TacticalMapExtension->Set_Subtitle(voxptr->DescriptionText.c_str(), voxptr->SubtitleCategory);
        } else {
            TacticalMapExtension->Clear_Subtitle();
        }
    }
}


/**
 *  Forces the EVA voice to stop talking.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Stop_Speaking()
{
    NormalQueue.clear();
    InterruptQueue.clear();

    AudioManager.Request_Stop(SpeechHandle, 0.5f);

    if (TacticalMapExtension) {
        TacticalMapExtension->Clear_Subtitle();
    }
}


/**
 *  Checks to see if the eva voice is still playing.
 *
 *  @author: CCHyper
 */
bool AudioVoxClass::Is_Speaking()
{
    AI();

    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return false;
    }

    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_SPEECH) <= 0.0f) {
        return false;
    }

    return !NormalQueue.empty() || !InterruptQueue.empty() || (ActiveVoice != VOX_NONE && AudioManager.Query_Is_Active(SpeechHandle));
}


/**
 *  Sets the global speech volume to that specified.
 * 
 *  @author: CCHyper
 */
void AudioVoxClass::Set_Speech_Volume(int vol)
{
    float volf = std::clamp(static_cast<float>(vol) / 255.0f, 0.0f, 1.0f);
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
    ini.Put_String(DEFAULTS, "Priority", Name_From_Priority(DefaultPriority));
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

    if (name == nullptr || !strcasecmp(name, "<none>") || !strcasecmp(name, "none")) {
        return VOX_NONE;
    }

    for (VoxType index = VOX_FIRST; index < Voxs.Count(); ++index) {
        AudioVoxClass *vocptr = Voxs[index];
        if (strcasecmp(vocptr->Name.c_str(), name) == 0) {
            return index;
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
    SpeechEnabled = set;
}


/**
 *  Returns whether EVA speech playback is currently allowed.
 *
 *  @author: CCHyper
 */
bool AudioVoxClass::Is_Speech_Allowed()
{
    return SpeechEnabled;
}


/**
 *  Fetch the side-aligned sound filename for this vox.
 *
 *  @author: ZivDero
 */
std::string const& AudioVoxClass::Get_Sound_Name() const
{
    if (Scen != nullptr && Scen->SpeechSide >= SIDE_FIRST && Scen->SpeechSide < SideSounds.size()) {
        if (!SideSounds[Scen->SpeechSide].empty()) {
            return SideSounds[Scen->SpeechSide];
        }
    }
    return Sound;
}


/**
 *  New EVA names
 *
 *  @author: Crimsonum
 */
const char* EvaNames[VOX_COUNT] = {
    "EVA_MissionAccomplished",
    "EVA_MissionFailed",
    "EVA_UnableToComply",
    "EVA_ConstructionComplete",
    "EVA_UnitReady",
    "EVA_NewConstructionOptions",
    "EVA_CannotDeployHere",
    "EVA_GDIStructureDestroyed",
    "EVA_InsufficientFunds",
    "EVA_BattleControlOffline",
    "EVA_ReinforcementsHaveArrived",
    "EVA_Canceled",
    "EVA_Building",
    "EVA_LowPower",
    "EVA_BaseUnderAttack",
    "EVA_PrimaryBuildingSet",
    "EVA_UnitLost",
    "EVA_SelectTarget",
    "EVA_SilosNeeded",
    "EVA_OnHold",
    "EVA_Repairing",
    "EVA_Training",
    "EVA_UnitArmorUpgraded",
    "EVA_UnitFirepowerUpgraded",
    "EVA_UnitSpeedUpgraded",
    "EVA_UnitRepaired",
    "EVA_StructureSold",
    "EVA_HarvesterUnderAttack",
    "EVA_CloakedUnitDetected",
    "EVA_SubterraneanUnitDetected",
    "EVA_20MinutesRemaining",
    "EVA_10MinutesRemaining",
    "EVA_5MinutesRemaining",
    "EVA_4MinutesRemaining",
    "EVA_3MinutesRemaining",
    "EVA_2MinutesRemaining",
    "EVA_1MinuteRemaining",
    "EVA_UnitSold",
    "EVA_BuildingCaptured",
    "EVA_EstablishBattlefieldControl",
    "EVA_IonStormApproaching",
    "EVA_MeteorStormApproaching",
    "EVA_NewTerrainDiscovered",
    "EVA_MissileLaunchDetected",
    "EVA_ChemicalMissileReady",
    "EVA_ClusterMissileReady",
    "EVA_IonCannonReady",
    "EVA_EMPulseCannonReady",
    "EVA_FirestormDefenseReady",
    "EVA_FirestormDefenseOffline",
    "EVA_PrimaryObjectiveAchieved",
    "EVA_SecondaryObjectiveAchieved",
    "EVA_TertiaryObjectiveAchieved",
    "EVA_QuaternaryObjectiveAchieved",
    "EVA_CriticalUnitLost",
    "EVA_CriticalStructureLost",
    "EVA_MutantSuppliesFound",
    "EVA_CommandosEnroute",
    "EVA_BuildingInfiltrated",
    "EVA_TimerStarted",
    "EVA_TimerStopped",
    "EVA_BridgeRepaired",
    "EVA_BaseDefensesOffline",
    "EVA_BuildingOffline",
    "EVA_BuildingOnline",
    "EVA_PlayerHasResigned",
    "EVA_PlayerWasDefeated",
    "EVA_YouAreVictorious",
    "EVA_YouHaveLost",
    "EVA_YouHaveResigned",
    "EVA_MutantCommandosAvailable",
    "EVA_AllianceFormed",
    "EVA_AllianceBroken",
    "EVA_OurAllyIsUnderAttack",
    "EVA_Tutorial1",
    "EVA_Tutorial2",
    "EVA_Tutorial3",
    "EVA_Tutorial4",
    "EVA_Tutorial5",
    "EVA_EvaTaunt01",
    "EVA_EvaTaunt02",
    "EVA_EvaTaunt03",
    "EVA_EvaTaunt04",
    "EVA_EvaTaunt05",
    "EVA_EvaTaunt06",
    "EVA_EvaTaunt07",
    "EVA_EvaTaunt08",
    "EVA_EvaTaunt09",
    "EVA_EvaTaunt10",
    "EVA_CabalTaunt01",
    "EVA_CabalTaunt02",
    "EVA_CabalTaunt03",
    "EVA_CabalTaunt04",
    "EVA_CabalTaunt05",
    "EVA_CabalTaunt06",
    "EVA_CabalTaunt07",
    "EVA_CabalTaunt08",
    "EVA_CabalTaunt09",
    "EVA_CabalTaunt10",
    "Mis_GDI01A_SoldierReinforcements",
    "Mis_GDI01A_SoldierItsNodSir",
    "Mis_GDI03A_SoldierLooksLikeTheyTore",
    "Mis_GDI04A_SoldierHereTheyCome",
    "Mis_GDI04A_SoldierIsThereAnyTechLeft",
    "Mis_GDI05A_SoldierTheseCritters",
    "Mis_GDI04A_SoldierCavalryHasArrived",
    "Mis_GDI12A_SoldierOldGDIBaseNear",
    "Mis_GDI11A_SoldierWelcomingCommittee",
    "Mis_NOD05A_SoldierETAOnMCV",
    "Mis_GDI05B_StarkTouchedDown",
    "Mis_GDI05B_StarkNodConverging",
    "Mis_GDI06A_PilotAirstrike",
    "Mis_GDI08A_PilotPickupService",
    "Mis_GDI08A_PilotCanMakeIt1",
    "Mis_GDI08A_PilotCanMakeIt2",
    "Mis_GDI08A_PilotDies",
    "Mis_GDI99A_SoldierSpiritHand",
    "Mis_GDI01A_EvaBuildRefinery",
    "Mis_GDI01A_EvaHarvester",
    "Mis_GDI01A_EvaBuildBarracks",
    "Mis_GDI01A_EvaDestroyAllNod",
    "Mis_GDI02A_EvaCivilianKilled",
    "Mis_GDI02A_EvaSAMsDestroyed",
    "Mis_GDI02A_EvaTransportsEnroute",
    "Mis_GDI02A_EvaCiviliansEvacuated",
    "Mis_GDI03A_EvaSiteSecure",
    "Mis_GDI03A_EvaTechCenterCaptured",
    "Mis_GDI03A_EvaBaseDestroyed",
    "Mis_GDI03B_EvaDestroyBridges",
    "Mis_GDI03B_EvaBridgesDestroyed",
    "Mis_GDI04A_EvaUfoUnderAttack",
    "Mis_GDI04A_EvaUfoDestroyed",
    "Mis_GDI05A_EvaArrayDestroyed",
    "Mis_GDI05A_EvaIncomingTransmission",
    "Mis_GDI05B_EvaTratosEvacuated",
    "Mis_GDI05B_EvaTransportDestroyed",
    "Mis_GDI05B_CabalIntrudersDetected",
    "Mis_GDI05B_CabalProbableObjective",
    "Mis_GDI05B_CabalKillAllPrisoners",
    "Mis_GDI05B_CabalAllForcesConverge",
    "Mis_GDI06A_EvaIonStormApproaching",
    "Mis_GDI06A_EvaIonStormAbating",
    "Mis_GDI06A_EvaAirPowerIneffective",
    "Mis_GDI06A_EvaDamDestroyed",
    "Mis_GDI06A_EvaDamSighted",
    "Mis_GDI06A_CabalRegulatorsOffline",
    "Mis_GDI06A_CabalDamFailing",
    "Mis_GDI06B_EvaSAMsDetected",
    "Mis_GDI06B_EvaSAMsObjective",
    "Mis_GDI06B_EvaSAMsDestroyed",
    "Mis_GDI06B_Eva090",
    "Mis_GDI06B_Eva092",
    "Mis_GDI06B_Eva094",
    "Mis_GDI06B_EvaCommandCenterDestroyed",
    "Mis_GDI06B_EvaInboundNuke",
    "Mis_GDI06B_CabalMainPowerOffline",
    "Mis_GDI06B_CabalMainPowerRestored",
    "Mis_GDI07A_EvaPerimeterDeactivated",
    "Mis_GDI07A_EvaReinforcementsInbound",
    "Mis_GDI08A_EvaBridgeRepaired",
    "Mis_GDI08A_EvaTrainReturningToBase",
    "Mis_GDI08A_EvaReinforcementsEnroute",
    "Mis_GDI08A_EvaCaptureTechCenter",
    "Mis_GDI08A_EvaTrainDisabled",
    "Mis_GDI08A_EvaCrystalsDestroyed",
    "Mis_GDI08A_EvaRogueIonStorm",
    "Mis_GDI08A_EvaPilotReturnToBase",
    "Mis_GDI09A_EvaForceDetected",
    "Mis_GDI09A_EvaMutantLost",
    "Mis_GDI09A_EvaAirstrikeReady",
    "Mis_GDI09A_EvaTransportsInbound",
    "Mis_GDI09B_EvaSupplyBaseDestroyed",
    "Mis_GDI09B_EvaMutantsToPowerGrid",
    "Mis_GDI09C_EvaC4Planted",
    "Mis_GDI09C_EvaGhostStalkerTerminated",
    "Mis_GDI09D_EvaMissileComplexDestroyed",
    "Mis_GDI09D_EvaTiberiumMissileInbound",
    "Mis_GDI09D_EvaStormDisablesFighters",
    "Mis_GDI10A_EvaMutantsDetected",
    "Mis_GDI10A_EvaFighterProdFacLocated",
    "Mis_GDI10A_EvaFighterProdFacLDestroyed",
    "Mis_GDI10B_Eva224",
    "Mis_GDI10B_Eva226",
    "Mis_GDI10B_Eva228",
    "Mis_GDI11A_EvaKodiakUnderAttack",
    "Mis_GDI11A_EvaStormAbating",
    "Mis_GDI11A_EvaKodiakDestroyed",
    "Mis_GDI11A_EvaKodiakCritical",
    "Mis_GDI11A_EvaEyeOfTheStorm",
    "Mis_GDI11A_EvaLifeformDetected",
    "Mis_GDI11A_EvaReenteringStorm",
    "Mis_GDI12A_EvaClearZoneForMCV",
    "Mis_GDI12A_EvaPhiladelphiaInRange",
    "Mis_GDI12A_EvaLifeformDetected",
    "Mis_GDI12A_EvaTiberiumMissileLaunched",
    "Mis_GDI12A_EvaICBMsDestroyed",
    "Mis_GDI12A_EvaICBMsBriefing1",
    "Mis_GDI12A_EvaICBMsBriefing2",
    "Mis_GDI12A_EvaCityUnderAttack",
    "Mis_NOD01A_SoldierMoveIt",
    "Mis_NOD01A_SoldierTiberiumIsLethal",
    "Mis_NOD01A_SoldierLaserTurrets",
    "Mis_NOD02A_SoldierSpiritHand",
    "Mis_NOD03A_SoldierStandAndIdentify",
    "Mis_NOD03A_SoldierSoundTheAlarm",
    "Mis_NOD03A_SoldierPostTK421",
    "Mis_NOD03B_SoldierInterrupted",
    "Mis_NOD12B_ChameleonSpy",
    "Mis_NOD01A_CabalHarvestTiberium",
    "Mis_NOD01A_CabalDestroyEliteGuard",
    "Mis_NOD01A_CabalBuildRefinery",
    "Mis_NOD01A_CabalEstablishingControl",
    "Mis_NOD01A_CabalControlEstablished",
    "Mis_NOD01A_CabalPowerLow",
    "Mis_NOD01A_CabalPerimeterBreached",
    "Mis_NOD01A_CabalToBuildOrTrain",
    "Mis_NOD01A_CabalTiberiumIsHazardous",
    "Mis_NOD01A_CabalToRepairStructure",
    "Mis_NOD02A_CabalCaptureTVStation",
    "Mis_NOD02A_CabalToRepairBridge",
    "Mis_NOD02A_CabalDestroyRemainderGuard",
    "Mis_NOD02A_CabalToCaptureBuilding",
    "Mis_NOD02A_CabalToDeployVehicle",
    "Mis_NOD03A_CabalMoveToOpenArea",
    "Mis_NOD03A_CabalLocatePyramid",
    "Mis_NOD03A_CabalCaptureHassan",
    "Mis_NOD03A_CabalCaptureTowers",
    "Mis_NOD03A_CabalMCVArrived",
    "Mis_NOD04B_CabalLightningRods",
    "Mis_NOD04B_CabalUseThem",
    "Mis_NOD05A_CabalGDIBaseOperational",
    "Mis_NOD05A_CabalTacitusAcquired",
    "Mis_NOD05A_CabalLifeformDetected",
    "Mis_NOD05A_CabalMutantVerminDetected",
    "Mis_NOD05A_CabalDropshipDetected",
    "Mis_NOD05A_CabalTrainDeparting",
    "Mis_NOD05A_CabalStopTrain",
    "Mis_NOD06A_CabalProtectEngineers",
    "Mis_NOD06A_CabalCongratulations",
    "Mis_NOD06A_CabalAPCToGDIBase",
    "Mis_NOD06B_CabalPlayerDetected",
    "Mis_NOD06B_CabalPreventEvacuation",
    "Mis_NOD06C_CabalTransportDetected",
    "Mis_NOD06C_CabalMutantDetected",
    "Mis_NOD07A_CabalMutantsLocated",
    "Mis_NOD07A_CabalTunnelSecured",
    "Mis_NOD07A_CabalResearchFacLocated",
    "Mis_NOD07A_CabalResearchFacDestroyed",
    "Mis_NOD07A_CabalBiotoxinsApproaching",
    "Mis_NOD07B_CabalBiotoxinsLocated",
    "Mis_NOD08A_CabalTrainArrivingOutpost",
    "Mis_NOD08A_CabalTrainArrivingMainPrison",
    "Mis_NOD08A_CabalTransportArrived",
    "Mis_NOD08A_CabalTransportLost",
    "Mis_NOD08A_CabalTransportDetected",
    "Mis_NOD09A_CabalUseCreature",
    "Mis_NOD09A_CabalTiberiumMissileReady",
    "Mis_NOD09A_CabalProtectConYard",
    "Mis_NOD09A_CabalBuildWasteFacility",
    "Mis_NOD09B_CabalConvoyTruckLost",
    "Mis_NOD09B_CabalTiberiumMissileReady",
    "Mis_NOD09B_CabalConvoyInbound",
    "Mis_NOD10A_CabalStealthIsKey",
    "Mis_NOD10A_CabalSpyLost",
    "Mis_NOD10A_CabalCommCenterInfiltrated",
    "Mis_NOD11A_CabalConvoySighted",
    "Mis_NOD11A_CabalMcNeilKilled",
    "Mis_NOD11A_CabalMcNeilCaptured",
    "Mis_NOD11A_CabalProdFacDestroyed",
    "Mis_NOD11A_CabalIfHeDetectsTrap",
    "Mis_NOD11A_CabalStopMcNeilEscape",
    "Mis_NOD11A_CabalToxinSoldiersLost",
    "Mis_NOD11A_CabalMcNeilEscaped",
    "Mis_NOD12A_CabalOrbit1Complete",
    "Mis_NOD12A_CabalOrbit2Complete",
    "Mis_NOD12A_CabalOrbit3CompleteYouFail",
    "Mis_NOD12A_CabalPerimeterDeactivated",
    "Mis_NOD12A_CabalICBMLost",
    "Mis_NOD12A_CabalICBMUnderAttack",
    "Mis_NOD12A_CabalIonCannonFiring",
    "Mis_NOD12A_CabalIonCannonIsOurs",
    "Mis_NOD12B_CabalSpyKilled",
    "Mis_NOD12B_CabalProceedToEvac",
    "Mis_GDI05B_UmagonBriefing1A",
    "Mis_GDI05B_UmagonBriefing1B",
    "Mis_GDI05B_UmagonBriefing2",
    "Mis_GDI05B_UmagonSoMuchForSublety",
    "Mis_GDI05B_UmagonTratosKilled",
    "Mis_GDI05B_UmagonGetTratosToTransport",
    "Mis_GDI05B_UmagonOurWorkIsDone",
    "Mis_GDI10A_UmagonTheySeenUs",
    "Mis_GDI09A_PilotWaitingForArrival",
    "Mis_GDI09B_MutantTrainToPowerGrid",
    "Mis_GDI09B_MutantWhatAnOpportunity",
    "Mis_GDI09C_MutantSomethingIsWrong",
    "Mis_NOD06A_MutantGDIKidnappedTratos",
    "Mis_GDI09A_MutantFemaleThanks",
    "Mis_GDI09A_MutantFemaleTheyMayBeOntoUs",
    "Mis_NOD01A_SoldierWeNeedMoreMen",
    "Mis_NOD02A_SoldierBlowTheBridge",
    "Mis_NOD03A_SoldierHassanIsEscaping",
    "Mis_NOD03A_SoldierHassanCaptured",
    "Mis_NOD04B_SoldierOldStockpile",
    "Mis_NOD07A_SoldierMutantsTurned",
    "Mis_NOD07A_SoldierTheyAreEverywhere",
    "Mis_NOD08A_SoldierSlavikToConvoyPoint",
    "Mis_NOD08A_SoldierTakePoint",
    "Mis_NOD08A_SoldierGladToSee",
    "Mis_NOD08A_SoldierOxannaIsEast",
    "Mis_NOD08A_SoldierWeakIce",
    "Mis_NOD03B_RebelCommander",
    "Mis_NOD11A_Civilian",
    "EVA_IncomingTransmission",
    "EVA_ObjectiveComplete",
    "EVA_FinalObjectiveComplete",
    "EVA_UnableToComplyMobileWarDeployed",
    "Mis_FSGDI01_EvaKodiakLocated",
    "Mis_FSGDI01_EvaTacitusAcquired",
    "Mis_FSGDI01_EvaTacitusLost",
    "Mis_FSGDI01_EvaTacitusFound",
    "Mis_FSGDI01_EvaTacitusCaptured",
    "Mis_FSGDI02_EvaFindAndEvacCivilians",
    "Mis_FSGDI02_EvaMaintainAllFactories",
    "Mis_FSGDI02_EvaEscortCivilians",
    "Mis_FSGDI03_EvaHumanLeaderNeutralized",
    "Mis_FSGDI03_EvaMutantLeaderNeutralized",
    "Mis_FSGDI03_EvaFoodCenterUnderAttack",
    "Mis_FSGDI03_EvaFoodCenterDestroyed",
    "Mis_FSGDI03_EvaWaterPurifierUnderAttack",
    "Mis_FSGDI03_EvaWaterPurifierDestroyed",
    "Mis_FSGDI03_EvaBriefing1",
    "Mis_FSGDI03_EvaBriefing2",
    "Mis_FSGDI03_EvaRiotLeaderNeutralized",
    "Mis_FSGDI03_EvaRiotLeader1Neutralized",
    "Mis_FSGDI03_EvaRiotLeader2Neutralized",
    "Mis_FSGDI03_EvaRiotLeader3Neutralized",
    "Mis_FSGDI03_EvaRiotLeader4Neutralized",
    "Mis_FSGDI03_EvaAllLeadersNeutralized",
    "Mis_FSGDI04_EvaEnemyReinforcements",
    "Mis_FSGDI04_EvaDestroyTwoBridges",
    "Mis_FSGDI04_EvaDisableCabalDefenses",
    "Mis_FSGDI04_EvaCaptureCabalCore",
    "Mis_FSGDI06_EvaOutpostLocated",
    "Mis_FSGDI06_EvaDestroyCabalForces",
    "Mis_FSGDI06_EvaEvacDrBoudreau",
    "Mis_FSGDI07_EvaGDIBaseDueEast",
    "Mis_FSGDI07_EvaProceedWithCaution",
    "Mis_FSGDI07_EvaInformVillages",
    "Mis_FSGDI07_EvaVillageWarned",
    "Mis_FSGDI07_EvaAllVillagesWarned",
    "Mis_FSGDI07_EvaCabalHasBegunOp",
    "Mis_FSGDI07_EvaWeLostContactWithBase",
    "Mis_FSGDI07_EvaGDIBaseUnderSiege",
    "Mis_FSGDI07_EvaWarnCivilianEnclaves",
    "Mis_FSGDI07_EvaWarnTwoCivOutposts",
    "Mis_FSGDI07_EvaWarnFinalVillage",
    "Mis_FSGDI07_EvaWarnTrondheim",
    "Mis_FSGDI07_EvaDestroyCabalForces",
    "Mis_FSGDI08_EvaCyborgIntroduced",
    "Mis_FSGDI08_EvaDeliverCyborgShort",
    "Mis_FSGDI08_EvaDestroyCyborgPlant",
    "Mis_FSGDI08_EvaDeliverCyborgLong",
    "Mis_FSGDI08_EvaAnotherArray",
    "Mis_FSGDI08_EvaDeliverCyborg2",
    "Mis_FSNOD07_EvaMultiMissilesDetected",
    "Mis_FSNOD08_EvaDestroyCabalHarvesters",
    "Mis_FS09_EvaControlStationCaptured",
    "Mis_FS09_EvaControlStationsBrief1",
    "Mis_FS09_EvaControlStationsBrief2",
    "Mis_FSGDI04_EvaDestroyEnemyBridges",
    "Mis_FSGDI04_EvaLocateTechnicians",
    "Mis_FSGDI04_EvaTechnicianLocation",
    "Mis_FSGDI04_EvaCABALOnline",
    "Mis_FSGDI05_CabalKillCultLeader",
    "Mis_FSGDI05_CabalGhostStalkerKilled",
    "Mis_FSGDI05_CabalJuggernautDestroyed",
    "Mis_FSGDI05_CabalFindTemple",
    "Mis_FSGDI08_CabalCyborgReplicationError",
    "Mis_FSGDI08_CabalSystemReset",
    "Mis_FSNOD02_CabalGDIPatrolNear",
    "Mis_FSNOD02_CabalTiberiumLifeform",
    "Mis_FSNOD02_CabalPlayerDetected",
    "Mis_FSNOD02_CabalRemainHidden",
    "Mis_FSNOD02_CabalDestroyCivStructures",
    "Mis_FSNOD02_CabalEliminateLife",
    "Mis_FSNOD02_CabalCaptureCivilians",
    "Mis_FSNOD02_CabalBaitLifeforms",
    "Mis_FSNOD02_CabalLeaveLifeforms",
    "Mis_FSNOD02_CabalLifeformToSettlements",
    "Mis_FSNOD02_CabalStagingPoint",
    "Mis_FSNOD02_CabalGDICannotReach",
    "Mis_FSNOD04_CabalOutpostLocated",
    "Mis_FSNOD04_CabalTacitusFound",
    "Mis_FSNOD04_CabalTacitusSafe",
    "Mis_FSNOD04_CabalExterminateMutants",
    "Mis_FSNOD04_CabalLocateOutpost",
    "Mis_FSNOD04_CabalFindTruck",
    "Mis_FSNOD04_CabalEliminateMutants",
    "Mis_FSNOD05_CabalTaunt",
    "Mis_FS_CabalYouDare",
    "Mis_FS_CabalSuperiorIntelligence",
    "Mis_FS_CabalByTheWay",
    "Mis_FS_CabalLaugh",
    "Mis_FSNOD07_CabalFirestormProtocol",
    "Mis_FS09_CabalDefenderProtocol",
    "Mis_FS09_CabalMiscalculation",
    "Mis_FSNOD_CabalYouAmuseMe",
    "Mis_FSNOD03_CabalSAMsDestroyed",
    "Mis_FSNOD03_CabalTratosEscaping",
    "Mis_FSNOD03_CabalYouFail",
    "Mis_FSNOD03_CabalArraysDestroyed",
    "Mis_FSNOD03_CabalCapturePowerPlants",
    "Mis_FSGDI01_GDIEngineer",
    "Mis_FSNOD06_NodEngineer",
    "Mis_FSGDI01_NodSomethingCrashed",
    "Mis_FSGDI01_NodFindsTacitus",
    "Mis_FSGDI01_NodFallBack",
    "Mis_FSNOD01_NodCABALCore1",
    "Mis_FSNOD01_NodSpotted",
    "Mis_FSNOD01_NodGDINearby",
    "Mis_FSNOD01_NodCABALCore2",
    "Mis_FSNOD01_NodCABALCore3",
    "Mis_FSNOD04_NodHeavyPoisoning",
    "Mis_FSNOD05_NodFindAirfield",
    "Mis_FSNOD05_NodMontaukEnroute",
    "Mis_FSNOD05_Montauk",
    "Mis_FSNOD06_NodCreateDistraction",
    "Mis_FSNOD06_NodIBetIf",
    "Mis_FSNOD06_NodHurryIntoRadar",
    "Mis_FSNOD07_NodEnemyRouted",
    "Mis_FSNOD07_NodItsATrick",
    "Mis_FSNOD07_NodRemoteSubstation",
    "Mis_FSNOD07_NodReinforcements",
    "Mis_FSNOD08_NodHelpCivilians",
    "Mis_FSNOD08_NodOnceSafe",
    "Mis_FSNOD08_NodAdditionalFunding",
    "Mis_FSNOD08_NodForgetCivilians",
    "Mis_FSNOD08_NodShouldBeEasy",
    "Mis_FSNOD08_NodGetThoseHarvesters",
    "Mis_FSNOD09_NodGDIUploading",
    "Mis_FSNOD09_NodCodeReceived",
    "Mis_FSNOD09_NodFirestormOffline",
    "Mis_FSNOD09_NodCodeFragment",
    "Mis_FSGDI05_JebWelcome",
    "Mis_FSGDI05_JebOffersBeverage",
    "Mis_FSGDI05_JebAlerted",
    "Mis_FSGDI05_ValdezNoTacitusHere",
    "Mis_FSGDI05_ValdezNothingHereEither",
    "Mis_FSGDI05_ValdezGotIt",
    "Mis_FSGDI05_ValdezNothingInHere",
    "Mis_FSGDI05_ValdezHieroglyphics",
    "Mis_FSGDI05_ValdezTempleOfTime",
    "Mis_FSGDI05_ValdezTempleOfThunder",
    "Mis_FSGDI05_ValdezTempleOfTacitus",
    "Mis_FSGDI05_ValdezCommand",
    "Mis_FSGDI05_ValdezBlueTiberium",
    "Mis_FSGDI05_ValdezBlastAPath",
    "Mis_FSGDI02_CivilianPlea1",
    "Mis_FSGDI02_CivilianPlea2",
    "Mis_FSGDI02_CivilianPlea3",
    "Mis_FSGDI02_CivilianPlea4",
    "Mis_FSGDI02_CivilianPlea5",
    "Mis_FSGDI02_CivilianPlea6",
    "Mis_FSGDI03_Civilian1",
    "Mis_FSGDI03_Civilian2",
    "Mis_FSGDI03_Civilian3",
    "Mis_FSGDI03_Priest",
    "Mis_FSGDI03_Civilian4",
    "Mis_FSGDI07_Civilian1",
    "Mis_FSGDI07_Civilian2",
    "Mis_FSGDI07_Mayor",
    "Mis_FSNOD02_Civilian1",
    "Mis_FSNOD02_Civilian2",
    "Mis_FSNOD02_Civilian3",
    "Mis_FSNOD02_Civilian4",
    "Mis_FSNOD02_Civilian5",
    "Mis_FSNOD02_Civilian6",
    "Mis_FSNOD08_CivilianPlea",
    "Mis_FSNOD08_CivilianThanks",
    "Mis_FSGDI04_Technician",
    "Mis_FSGDI03_CivilianLeader1",
    "Mis_FSGDI03_CivilianLeader2",
    "Mis_FSGDI03_CivilianLeader3",
    "Mis_FSGDI07_Civilian3",
    "Mis_FSGDI07_Civilian4",
    "Mis_FSGDI07_CivilianPlea1",
    "Mis_FSGDI07_CivilianPlea2",
    "Mis_FSGDI07_CivilianPlea3",
    "Mis_FSGDI07_CivilianPlea4",
    "Mis_FSGDI07_CivilianPlea5",
    "Mis_FSGDI07_CivilianPlea6",
    "Mis_FSGDI01_Mutant",
    "Mis_FSGDI03_MutantLeaderA1",
    "Mis_FSGDI03_MutantLeaderB1",
    "Mis_FSGDI03_MutantLeaderB2",
    "Mis_FSGDI03_MutantLeaderA2",
    "Mis_FSGDI03_MutantLeaderA3",
    "Mis_FSGDI03_MutantLeaderA4",
    "Mis_FSGDI03_MutantLeaderA5",
    "Mis_FSGDI07_Mutant",
    "Mis_FSNOD04_Mutant",
    "Mis_FSNOD03_Guard1",
    "Mis_FSNOD03_Guard2",
    "Mis_FSNOD03_Guard3",
    "Mis_FSGDI03_CommandNoCasualties",
    "Mis_FSGDI03_CommandStayVigilant",
    "Mis_FSGDI05_CommandTransportSent",
    "Mis_FSGDI05_CommandJuggernautBrief",
    "Mis_FSGDI05_CommandGetToEvac",
    "Mis_FSGDI05_CommandGetToTransport",
    "Mis_FSGDI05_CommandTakeOutLeader",
    "Mis_FSGDI06_CommandProtectDrBoudreau",
    "Mis_FSGDI07_CommanderBaseUnderAttack",
    "Mis_FSGDI07_CommanderWhatInTheWorld",
    "Mis_FSGDI07_CommanderIShouldGo",
    "Mis_FSGDI07_CommanderCABALBastard",
    "Mis_FSGDI07_CommanderCABALPrisoners",
    "Mis_FSGDI07_CommanderArmYourselves",
    "Mis_FSGDI07_CommanderBeWarned",
    "Mis_FSGDI07_CommanderAttentionMutants",
    "Mis_FSGDI07_CommanderEvacTheCity",
    "Mis_FSGDI07_CommanderCiviliansToArms",
    "Mis_FSGDI07_CommanderThisMustBeTheBase",
    "Mis_FSGDI07_CommanderWhatThe",
    "Mis_FSGDI07_CommanderWarnTrondheim",
    "Mis_FSGDI07_CommanderEvacTrondheim",
    "Mis_FSNOD08_CommandSaveCivilians1",
    "Mis_FSNOD08_CommandSaveCivilians2",
    "Mis_FSNOD08_CommandReinforcements",
    "Mis_FSGDI05_CultistAMutantAbomination",
    "Mis_FSGDI05_CultistAKillMutant",
    "Mis_FSGDI05_CultistAStopThief",
    "Mis_FSGDI05_CultistAKillHeretics",
    "Mis_FSGDI05_CultistADoNotLetEscape",
    "Mis_FSGDI05_CultistALeaderKilled",
    "Mis_FSGDI05_CultistAHereafter",
    "Mis_FSGDI06_CyborgTerminateLifeforms",
    "Mis_FSGDI06_CyborgWillOfCABAL",
    "Mis_FSGDI08_CyborgError",
    "Mis_FSGDI08_CyborgSystemFailure",
    "Mis_FSGDI08_CyborgMalfunction",
    "Mis_FSGDI08_CyborgGetIn",
    "Mis_FSGDI08_CyborgIntruderAlert",
    "Mis_FSGDI08_CyborgNotOneOfUs",
    "Mis_FSGDI08_CyborgFireInTheHole",
    "Mis_FSGDI08_CyborgProceedingToTarget",
    "Mis_FSGDI08_CyborgSequenceEngaged",
    "Mis_FSGDI06_SoldierOpenFire",
    "Mis_FSGDI03_SoldierACivsShootingAtUs",
    "Mis_FSGDI03_SoldierBSoAreShiners",
    "Mis_FSNOD07_NodCommandStationCaptured",
    "Mis_FSGDI06_GDICABALHasBetrayedUs",
    "Mis_FSNOD01_GDISoldierWhatWasThat",
    "Mis_FSNOD01_GDISoldierLetsCheck",
    "Mis_FSNOD01_GDISoldierDidYouHear",
    "Mis_FSNOD01_GDISoldierSheAintGoing",
    "Mis_FSNOD01_GDISoldierHeadBack",
    "Mis_FSNOD01_GDISoldierShootIt",
    "Mis_FSNOD02_GDISoldierGetCivsOutNow",
    "Mis_FSNOD02_GDISoldierUnderAttack",
    "Mis_FSNOD02_GDISoldierWhatTheHell",
    "Mis_FSNOD02_GDISoldierWhereAre",
    "Mis_FSGDI09_GDINodIsUploading",
    "Mis_FSGDI09_GDICodeReceived",
    "Mis_FSGDI09_GDIFirestormOffline",
    "Mis_FSGDI09_GDICodeFragmentRetreived",
    "Mis_FSGDI04_GDILaserPostsStronger",
    "EVA_DropPodsAvailable",
    "Mis_FSGDI05_CultistBWelcomeTraveler",
    "Mis_FSGDI05_JebJoinUs",
    "Mis_FSGDI05_JebJoinMe",
    "Mis_FSGDI05_CultistBExistenceIsFutile",
    "Mis_FSGDI05_CultistBComingToJoinYou",
    "Mis_FSGDI05_JebMessenger",
    "Mis_FSGDI05_JebComeToMe"
};
