/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Central miniaudio-backed audio manager.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_defines.h"
#include "vector.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>


/**
 *  Forward declarations.
 */
struct ma_engine;
struct ma_device;
struct ma_sound;
typedef ma_sound ma_sound_group;
class AudioSampleClass;


/**
 *  Central audio manager that handles playback, sample management, and group control via miniaudio.
 */
class AudioManagerClass
{
    friend class AudioSampleClass;
    friend class AudioInstanceClass;
    friend class AudioStreamingClass;

public:
    AudioManagerClass();
    ~AudioManagerClass();

    /**
     *  Audio engine IO.
     */
    bool Init(HWND hWnd);
    void End();

    bool Is_Available() const;

    bool Start_Engine(bool forced = false);
    bool Stop_Engine();

    void Focus_Loss();
    void Focus_Restore();

    void Sound_Callback();

    /**
     *  Sound playback control.
     */
    AudioInstanceHandle Request_Play(const std::string& filename, AudioGroupType group, float volume = 1.0f, float pitch = 1.0f, float pan = 0.0f, AudioPriorityType priority = AUDIO_PRIORITY_NORMAL, int limit = -1, float fade_in_seconds = 0.0f, float delay_in_seconds = 0.0f, bool start = true, bool looping = false, int loop_limit = 0, AudioControlType control = AUDIO_CONTROL_NORMAL);
    bool Request_Stop(AudioInstanceHandle id, float fade_out = 0.0f);

    bool Request_Pause(AudioInstanceHandle id);
    bool Request_Resume(AudioInstanceHandle id);

    /**
     *  Query functions.
     */
    bool Query_Is_Playing(AudioInstanceHandle id);
    bool Query_Is_Active(AudioInstanceHandle id);
    bool Query_Is_Paused(AudioInstanceHandle id);

    bool Query_Sample_Ready(std::string name, AudioGroupType group);

    /**
     *  Set properties functions.
     */
    bool Set_Volume(AudioInstanceHandle id, float volume);
    bool Set_Pan(AudioInstanceHandle id, float pan);
    bool Set_Pitch(AudioInstanceHandle id, float pitch);

    /**
     *  Submission functions.
     */
    bool Submit_Sample(const std::string& filename, AudioFileType filetype, AudioGroupType group, AudioPriorityType priority, AudioControlType control, AudioSoundType type, unsigned int limit);
    bool Clear_Samples(AudioGroupType group = AUDIO_GROUP_NONE);
    bool Has_Been_Submitted(const std::string& filename, AudioGroupType group = AUDIO_GROUP_NONE);

    void Lock_Submissions() { SubmissionsLocked = true; }

    /**
     *  Master volume control.
     */
    bool Set_Master_Volume(float volume) const;

    /**
     *  Audio group control.
     */
    float Get_Group_Volume(AudioGroupType group);
    bool Set_Group_Volume(AudioGroupType group, float volume);
    bool Is_Group_Playing(AudioGroupType group) const;
    bool Start_Group(AudioGroupType group) const;
    bool Stop_Group(AudioGroupType group) const;
    bool Stop_And_Fade_Out_Group(AudioGroupType group, float duration);

    /**
     *  Supported formats query functions.
     */
    bool Is_File_Available(AudioFileType type, std::string name) const;
    bool Is_File_Available(std::string name) const;
    bool Is_FileType_Supported(AudioFileType type) const;
    std::string Build_Filename_From_Type(AudioFileType type, std::string name);

    bool Get_File_Info(const std::string& name, AudioFileType& filetype, std::string& filename, bool ignore_error = false);

    /**
     *  Utility functions.
     */
    static int AudioPriority_To_Priority(AudioPriorityType priority);
    static AudioPriorityType Priority_To_AudioPriority(int priority);
    static unsigned int fVolume_To_iVolume(float vol);
    static float iVolume_To_fVolume(unsigned int vol);

    bool Is_Handle_Valid(AudioInstanceHandle id);

#ifndef NDEBUG
    bool Create_Debug_Window();
    bool Close_Debug_Window();
    void Debug_Window_Message_Handler();
    void Debug_Window_Loop();
#endif

private:
    /**
     *  Active handle management
     */
    bool Add_Active_Handle(std::unique_ptr<AudioInstanceClass> handle);
    bool Add_Active_Handle_NoLock(std::unique_ptr<AudioInstanceClass> handle);
    bool Remove_Active_Handle(AudioInstanceHandle id);
    bool Remove_Active_Handle_NoLock(AudioInstanceHandle id);
    bool Clear_All_Active_Handles();

    AudioSampleClass* Find_Sample(const std::string& filename, AudioGroupType group);

private:
    static unsigned __stdcall CleanupThreadFunction(void* context);

    /**
     *  Utility functions to handle unique handle id's
     */
    static AudioInstanceHandle Generate_Unique_Audio_ID(AudioGroupType group);

    /**
     *  Lookup a handle in ActiveInstanceMap. Caller MUST hold ThreadMutex -
     *  the returned pointer is only valid for the duration of that lock.
     */
    static AudioInstanceClass* Find_Handle_By_ID_NoLock(AudioInstanceHandle id);

private:
    /**
     *  The miniaudio engine and device instances.
     */
    ma_engine* Engine = nullptr;

    /**
     *  Sound groups for per-group volume and playback control.
     */
    ma_sound_group* SoundGroups[AUDIO_GROUP_COUNT] = {};

    /**
     *  Has the audio engine been successfully initialized?
     */
    bool IsInitialized = false;

    /**
     *  When the window loses focus, the current engine volume is stored here
     *  so it can be restored when focus is restored. Atomic because Focus_Loss
     *  and Focus_Restore can be called from the WndProc thread.
     */
    std::atomic<float> FocusRestoreVolume {AUDIO_VOLUME_MAX};

    /**
     *  Tracks all the currently playing sounds.
     */
    std::unordered_map<AudioInstanceHandle, std::unique_ptr<AudioInstanceClass>> ActiveInstanceMap;

    /**
     *  Tracks all the currently playing sounds in their respective groups types. We can use raw
     *  pointers here and ActiveInstanceMap has ownership of the audio instances being added.
     */
    std::unordered_map<AudioGroupType, std::vector<AudioInstanceClass*>> GroupedActiveInstanceMap;

    /**
     *  Stores all available samples submitted to the manager.
     */
    std::unordered_map<AudioSampleKey, std::unique_ptr<AudioSampleClass>> SamplesMap;

    /**
     *  Lock down the manager to accepting any further submissions.
     */
    bool SubmissionsLocked = false;

private:
    /**
     *  Types of audio requests that can be queued for processing.
     */
    typedef enum AudioRequestType {
        AUDIO_REQUEST_PLAY,
        AUDIO_REQUEST_STOP
    } AudioRequestType;

    /**
     *  Lightweight lifecycle for requests that have a public handle but may
     *  not have an AudioInstanceClass yet. This doubles as the cancellation
     *  token for queued/deferred playback.
     */
    typedef enum AudioRequestState {
        AUDIO_REQUEST_STATE_QUEUED,
        AUDIO_REQUEST_STATE_DEFERRED,
        AUDIO_REQUEST_STATE_ACTIVE,
        AUDIO_REQUEST_STATE_CANCELING,
        AUDIO_REQUEST_STATE_FINISHED
    } AudioRequestState;

    /**
     *  Queued audio playback or stop request with all associated parameters.
     */
    typedef struct AudioRequest {

        AudioRequestType Type = AudioRequestType::AUDIO_REQUEST_PLAY;
        AudioInstanceHandle HandleID = INVALID_AUDIO_INSTANCE_HANDLE;
        std::string Filename;
        AudioGroupType Group = AUDIO_GROUP_NONE;
        float Volume = 1.0f;
        float Pitch = 1.0f;
        float Pan = 0.0f;
        AudioPriorityType Priority = AUDIO_PRIORITY_NORMAL;
        int Limit = -1;
        float FadeInSeconds = 0.0f;
        float DelayInSeconds = 0.0f;
        bool StartImmediately = true;
        bool Loops = false;
        int LoopLimit = 0;
        AudioControlType Control = AUDIO_CONTROL_NORMAL;
        float FadeOutSeconds = 0.0f;

        AudioRequest() = default;

        AudioRequest(AudioInstanceHandle id, std::string filename, AudioGroupType group, float volume, float pitch, float pan, AudioPriorityType priority, int limit, float fadeIn, float delay, bool start, bool looping, int loop_limit, AudioControlType control) :
            Type(AudioRequestType::AUDIO_REQUEST_PLAY),
            HandleID(id),
            Filename(std::move(filename)),
            Group(group),
            Volume(volume),
            Pitch(pitch),
            Pan(pan),
            Priority(priority),
            Limit(limit),
            FadeInSeconds(fadeIn),
            DelayInSeconds(delay),
            StartImmediately(start),
            Loops(looping),
            LoopLimit(loop_limit),
            Control(control)
        {
        }

        AudioRequest(AudioInstanceHandle id, float fade_out) : Type(AudioRequestType::AUDIO_REQUEST_STOP), HandleID(id), FadeOutSeconds(fade_out) {}

    } AudioRequest;

    void Set_Request_State(AudioInstanceHandle id, AudioRequestState state);
    bool Try_Set_Request_State(AudioInstanceHandle id, AudioRequestState state);
    AudioRequestState Get_Request_State(AudioInstanceHandle id);
    bool Is_Request_Canceled(AudioInstanceHandle id);
    bool Cancel_Queued_Request(AudioInstanceHandle id);
    void Clear_Request_State(AudioInstanceHandle id);
    void Process_Play_Request(AudioRequest req);
    void Process_Stop_Request(AudioRequest req);

private:
    /**
     *  Background thread and synchronization primitives for cleanup and request processing.
     */
    std::thread CleanupThread;
    std::atomic<bool> ThreadExitFlag {false};

    /**
     *  Protects ActiveInstanceMap, GroupedActiveInstanceMap, AND every operation
     *  on the AudioInstanceClass objects they own. AudioInstanceClass methods are
     *  not internally synchronized - callers must hold this mutex end-to-end when
     *  invoking them. Lock order: EventMutex (audio_event.cpp) -> RequestMutex
     *  for queue insertion, and EventMutex -> ThreadMutex for instance access.
     *  Never hold RequestMutex while waiting on ThreadMutex.
     *  RequestStateMutex may be taken while holding ThreadMutex; do not take
     *  ThreadMutex while holding RequestStateMutex.
     */
    std::mutex ThreadMutex;

    std::queue<AudioRequest> RequestQueue;
    std::mutex RequestMutex;
    std::condition_variable_any RequestCV;

    /**
     *  Request states for handles that may be queued, deferred, active, or
     *  canceling. This is the authoritative cancellation token for handles
     *  before an AudioInstanceClass exists.
     */
    std::unordered_map<AudioInstanceHandle, AudioRequestState> RequestStateMap;
    std::mutex RequestStateMutex;

    /**
     *  Requests with AUDIO_CONTROL_QUEUE that were deferred due to concurrent limit.
     *  Worker-thread-only - no mutex required.
     */
    std::queue<AudioRequest> DeferredPlayQueue;

    std::mutex SubmissionMutex;
};


/**
 *  Global audio manager instance.
 */
extern AudioManagerClass AudioManager;
