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
#include "vinifera_thread.h"
#include "vinifera_util.h"
#include "vox.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <deque>
#include <optional>
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
std::deque<VoxQueueEntry> CriticalQueue;
std::deque<VoxQueueEntry> InterruptQueue;
std::optional<VoxQueueEntry> StandardQueue;
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
 *  Returns true if the voice is already pending in any queue or slot.
 */
bool Is_Queued(VoxType voice)
{
    auto match = [&](const VoxQueueEntry& x) { return x.voice == voice; };
    return std::any_of(NormalQueue.begin(), NormalQueue.end(), match)
        || std::any_of(CriticalQueue.begin(), CriticalQueue.end(), match)
        || std::any_of(InterruptQueue.begin(), InterruptQueue.end(), match)
        || (StandardQueue.has_value() && StandardQueue->voice == voice);
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


const char* Priority_Name(VoxPriorityType priority)
{
    switch (priority) {
    case VOX_PRIORITY_LOW:       return "LOW";
    case VOX_PRIORITY_NORMAL:    return "NORMAL";
    case VOX_PRIORITY_IMPORTANT: return "IMPORTANT";
    case VOX_PRIORITY_CRITICAL:  return "CRITICAL";
    }
    return "NORMAL";
}

/**
 *  Defaults for vanilla speeches.
 */
constexpr std::array<std::string_view, 16> DialogType_QUEUE = {
    "EVA_ChemicalMissileReady",
    "EVA_ClusterMissileReady",
    "EVA_IonCannonReady",
    "EVA_EMPulseCannonReady",
    "EVA_FirestormDefenseReady",
    "EVA_FirestormDefenseOffline",
    "EVA_PlayerResigned",
    "EVA_PlayerDefeated",
    "EVA_ReinforcementsHaveArrived",
    "EVA_NewTerrainDiscovered",
    "EVA_AllianceFormed",
    "EVA_AllianceBroken",
    "EVA_OurAllyIsUnderAttack",
    "EVA_NewConstructionOptions",
    "EVA_LowPower",
    "EVA_BuildingCaptured",
};

constexpr std::array<std::string_view, 4> DialogPriority_CRITICAL = {
    "EVA_MissileLaunchDetected",
    "EVA_IonStormApproaching",
    "EVA_MeteorStormApproaching",
    "EVA_EstablishBattlefieldControl",
};

constexpr std::array<std::string_view, 8> DialogPriority_IMPORTANT = {
    "EVA_PlayerResigned",
    "EVA_PlayerDefeated",
    "EVA_ReinforcementsHaveArrived",
    "EVA_NewTerrainDiscovered",
    "EVA_AllianceFormed",
    "EVA_AllianceBroken",
    "EVA_LowPower",
    "EVA_UnitLost",
};

constexpr std::array <std::string_view, 24> DialogPriority_LOW = {
    "EVA_OurAllyIsUnderAttack",
    "EVA_BridgeRepaired",
    "EVA_UnableToComply",
    "EVA_ConstructionComplete",
    "EVA_NewConstructionOptions",
    "EVA_Canceled",
    "EVA_Building",
    "EVA_PrimaryBuildingSet",
    "EVA_OnHold",
    "EVA_Repairing",
    "EVA_StructureSold",
    "EVA_BaseDefensesOffline",
    "EVA_BuildingOffline",
    "EVA_BuildingOnline",
    "EVA_UnitReady",
    "EVA_CannotDeployHere",
    "EVA_SelectTarget",
    "EVA_Training",
    "EVA_UnitRepaired",
    "EVA_UnitSold",
    "EVA_UnitFirepowerUpgraded",
    "EVA_UnitArmorUpgraded",
    "EVA_UnitSpeedUpgraded",
};


SubtitleCategoryType Vanilla_Category_For_Name(const char* name)
{
    if (name != nullptr && std::strncmp(name, "EVA_", 4) == 0) {
        // EVA_Tutorial# are designated as scenario despite starting with EVA_
        if (std::strncmp(name, "EVA_Tutorial", 12) != 0) {
            return SUBTITLE_CATEGORY_SYSTEM;
        }
    }
    return SUBTITLE_CATEGORY_SCENARIO;
}


std::optional<VoxControlType> Vanilla_Control_For_Name(std::string_view name)
{
    if (std::ranges::find(DialogType_QUEUE.begin(), DialogType_QUEUE.end(), name) != DialogType_QUEUE.end()) {
        return VOX_CONTROL_QUEUE;
    }

    return std::nullopt;
}


std::optional<VoxPriorityType> Vanilla_Priority_For_Name(const char* name)
{
    if (std::ranges::find(DialogPriority_CRITICAL.begin(), DialogPriority_CRITICAL.end(), name) != DialogPriority_CRITICAL.end()) {
        return VOX_PRIORITY_CRITICAL;
    }
    if (std::ranges::find(DialogPriority_IMPORTANT.begin(), DialogPriority_IMPORTANT.end(), name) != DialogPriority_IMPORTANT.end()) {
        return VOX_PRIORITY_IMPORTANT;
    }
    if (std::ranges::find(DialogPriority_LOW.begin(), DialogPriority_LOW.end(), name) != DialogPriority_LOW.end()) {
        return VOX_PRIORITY_LOW;
    }

    return std::nullopt;
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
static std::mutex VoxScanThreadMutex;
static std::thread VoxScanThread;


/**
 *  Joins the owned asynchronous EVA/voice scan thread, if one is active.
 *
 *  @author: ZivDero
 */
static void Join_Vox_Scan_Thread()
{
    std::scoped_lock lock(VoxScanThreadMutex);
    if (VoxScanThread.joinable()) {
        VoxScanThread.join();
    }
}


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
 *  Applies the built-in vanilla defaults for this speech entry.
 *
 *  @author: ZivDero
 */
void AudioVoxClass::Set_Vanilla_Defaults()
{
    const char *name = Name.c_str();

    SubtitleCategory = Vanilla_Category_For_Name(name);

    if (auto control = Vanilla_Control_For_Name(name)) {
        Control = *control;
    }

    if (auto priority = Vanilla_Priority_For_Name(name)) {
        Priority = *priority;
    }
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
        if (stricmp(catbuf, "Scenario") == 0) {
            SubtitleCategory = SUBTITLE_CATEGORY_SCENARIO;
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
        voxptr->Set_Vanilla_Defaults();
    }
}


/**
 *  Processes the speech INI database, loading defaults and creating/updating vox entries.
 *
 *  @author: CCHyper
 */
bool AudioVoxClass::Process(CCINIClass const& ini)
{
    Join_Vox_Scan_Thread();

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

        voxptr->Available = false;
        voxptr->FileType = AUDIO_TYPE_NONE;
        voxptr->FileName.clear();

        std::string preferred_name = voxptr->Get_Sound_Name().empty() ? voxptr->Name : voxptr->Get_Sound_Name();
        bool available = AudioManager.Get_File_Info(preferred_name, voxptr->FileType, voxptr->FileName, true);

        auto try_fallback = [&](const std::string& sound) {
            if (available || sound.empty() || sound == preferred_name) {
                return;
            }
            available = AudioManager.Get_File_Info(sound, voxptr->FileType, voxptr->FileName, true);
        };

        try_fallback(voxptr->Name);
        try_fallback(voxptr->Sound);
        for (const std::string& side_sound : voxptr->SideSounds) {
            try_fallback(side_sound);
        }

        if (!available) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Scan - File \"%s\" was not found in any supported formats!\n", preferred_name.c_str());
            continue;
        }

        voxptr->Available = available;
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

        auto submit_sound = [&](const std::string& sound) {
            if (sound.empty()) {
                return;
            }

            std::string filename;
            AudioFileType filetype = AUDIO_TYPE_NONE;
            if (!AudioManager.Get_File_Info(sound, filetype, filename, true)) {
                return;
            }

            if (AudioManager.Has_Been_Submitted(filename, AUDIO_GROUP_SPEECH)) {
                AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOX, "Vox::Preload - File \"%s\" has already been submitted to the audio manager!\n", filename.c_str());
                return;
            }

            /**
             *  Submit this speech sample to the audio manager. Speech is always voice-typed
             *  and the queue/interrupt logic is owned by AudioVoxClass, not the audio engine,
             *  so we hardcode AUDIO_SOUND_VOICE / AUDIO_CONTROL_NORMAL here.
             */
            bool submitted = AudioManager.Submit_Sample(
                filename,
                filetype,
                AUDIO_GROUP_SPEECH,
                To_Audio_Priority(voxptr->Get_Priority()),
                AUDIO_CONTROL_NORMAL,
                AUDIO_SOUND_VOICE,
                1);

            if (submitted) {
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_VOX, "Vox::Preload - Submitted \"%s\" to audio manager.\n", filename.c_str());
            } else {
                AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::Preload - Failed to submit \"%s\" to audio manager!\n", filename.c_str());
            }
        };

        submit_sound(voxptr->Name);
        submit_sound(voxptr->Sound);
        for (const std::string& side_sound : voxptr->SideSounds) {
            submit_sound(side_sound);
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
    std::scoped_lock lock(VoxScanThreadMutex);
    if (VoxScanThread.joinable()) {
        VoxScanThread.join();
    }
    IsVoxScanComplete.store(false);

    VoxScanThread = std::thread([] {
        Vinifera_Run_Thread([] {
            Scan();
            IsVoxScanComplete.store(true);
        });
    });
}


/**
 *  Waits for any asynchronous EVA/voice scan to finish.
 *
 *  @author: ZivDero
 */
void AudioVoxClass::Wait_For_Scan()
{
    Join_Vox_Scan_Thread();
}


/**
 *  Destroys all registered vox instances and frees associated memory.
 *
 *  @author: CCHyper
 */
void AudioVoxClass::Clear()
{
    Join_Vox_Scan_Thread();
    Stop_Speaking();

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
        if (entry.priority == VOX_PRIORITY_CRITICAL) {
            Insert_Sorted(CriticalQueue, entry);
            break;
        }

        if (!InterruptQueue.empty() || !CriticalQueue.empty()) {
            return;
        }

        if (StandardQueue.has_value() && StandardQueue->priority >= entry.priority) {
            return;
        }

        StandardQueue = entry;
        break;

    case VOX_CONTROL_INTERRUPT:
        if (AudioManager.Query_Is_Active(SpeechHandle)) {
            AudioManager.Request_Stop(SpeechHandle, 0.25f);
        }
        ActiveVoice = VOX_NONE;
        NormalQueue.clear();
        CriticalQueue.clear();
        InterruptQueue.clear();
        StandardQueue.reset();
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

    if (CriticalQueue.empty() && InterruptQueue.empty() && !StandardQueue.has_value() && NormalQueue.empty()) {
        return;
    }

    /**
     *  Pop the next entry: critical entries and queued interrupts drain before
     *  the single standard slot, which drains before the normal queue.
     */
    VoxQueueEntry next = { VOX_NONE, VOX_PRIORITY_LOW };
    if (!CriticalQueue.empty()) {
        next = CriticalQueue.front();
        CriticalQueue.pop_front();
    } else if (!InterruptQueue.empty()) {
        next = InterruptQueue.front();
        InterruptQueue.pop_front();
    } else if (StandardQueue.has_value()) {
        next = *StandardQueue;
        StandardQueue.reset();
    } else {
        next = NormalQueue.front();
        NormalQueue.pop_front();
    }

    AudioVoxClass *voxptr = Voxs[next.voice];
    if (!voxptr) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::AI - voxptr is null!\n");
        return;
    }

    std::string name = voxptr->Name;
    std::string filename;

    AudioInstanceHandle handle = INVALID_AUDIO_INSTANCE_HANDLE;

    {
        std::scoped_lock lock(VoxScanMutex);

        std::string sound_name = voxptr->Get_Sound_Name().empty() ? voxptr->Name : voxptr->Get_Sound_Name();
        AudioFileType filetype = AUDIO_TYPE_NONE;
        if (!AudioManager.Get_File_Info(sound_name, filetype, filename, true)) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOX, "Vox::AI - Failed to resolve \"%s\"!\n", sound_name.c_str());
            return;
        }

        voxptr->FileType = filetype;
        voxptr->FileName = filename;

        /**
         *  Speech file was found, now play it.
         */
        float vol = std::clamp(voxptr->Get_Volume(), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
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
    CriticalQueue.clear();
    InterruptQueue.clear();
    StandardQueue.reset();

    if (AudioManager.Is_Available() && SpeechHandle.Is_Valid()) {
        AudioManager.Request_Stop(SpeechHandle, 0.5f);
    }

    SpeechHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    ActiveVoice = VOX_NONE;
    SpeakTimer = 0;

    if (TacticalMapExtension) {
        TacticalMapExtension->Clear_Subtitle();
    }
}


/**
 *  Checks to see if the EVA voice is still playing.
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

    return !NormalQueue.empty() || !CriticalQueue.empty() || !InterruptQueue.empty() || StandardQueue.has_value() ||
        (ActiveVoice != VOX_NONE && AudioManager.Query_Is_Active(SpeechHandle));
}


/**
 *  Sets the global speech volume to that specified.
 * 
 *  @author: CCHyper
 */
void AudioVoxClass::Set_Speech_Volume(int vol)
{
    float volf = std::clamp(AudioManagerClass::iVolume_To_fVolume(vol), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
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
    static char const * const DIALOGLIST = "DialogList";

    /**
     *  Clear out all existing base data from the ini file.
     */
    ini.Clear();

    /**
     *  Write out each speech entry to the DialogList section.
     */
    for (VoxType vox = VOX_FIRST; vox < std::size(EvaNames); ++vox) {
        const char * vox_name = EvaNames[vox];
        const char * sound_name = Speech[vox] != nullptr ? Speech[vox] : "";

        /**
         *  Format the entry index as the INI key.
         */
        char entrybuff[8];
        std::snprintf(entrybuff, sizeof(entrybuff), "%d", vox);

        ini.Put_String(DIALOGLIST, entrybuff, vox_name);

        /**
         *  Now write the keys for its section.
         */
        if (sound_name[0] != '\0' && std::strcmp(vox_name, sound_name) != 0) {
            ini.Put_String(vox_name, "Sound", sound_name);
        }
        if (Vanilla_Category_For_Name(vox_name) == SUBTITLE_CATEGORY_SCENARIO) {
            ini.Put_String(vox_name, "Category", "Scenario");
        }
        if (auto control = Vanilla_Control_For_Name(vox_name); control && *control == VOX_CONTROL_QUEUE) {
            ini.Put_String(vox_name, "Control", "QUEUE");
        }
        if (auto priority = Vanilla_Priority_For_Name(vox_name)) {
            ini.Put_String(vox_name, "Priority", Priority_Name(*priority));
        }
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
 *  Looks up a VoxType by any configured sound filename slot.
 *
 *  @author: ZivDero
 */
VoxType AudioVoxClass::From_Sound_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (name == nullptr || !strcasecmp(name, "<none>") || !strcasecmp(name, "none")) {
        return VOX_NONE;
    }

    for (VoxType index = VOX_FIRST; index < Voxs.Count(); ++index) {
        AudioVoxClass *voxptr = Voxs[index];
        if (voxptr == nullptr) {
            continue;
        }

        if (!voxptr->Sound.empty() && strcasecmp(voxptr->Sound.c_str(), name) == 0) {
            return index;
        }

        for (const std::string& side_sound : voxptr->SideSounds) {
            if (!side_sound.empty() && strcasecmp(side_sound.c_str(), name) == 0) {
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
    if (Scen != nullptr && Scen->SpeechSide >= SIDE_FIRST && static_cast<size_t>(Scen->SpeechSide) < SideSounds.size()) {
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
    "EVA_PlayerResigned",
    "EVA_PlayerDefeated",
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
