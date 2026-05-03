/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Central miniaudio-backed audio manager.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_manager.h"

#include "asserthandler.h"
#include "audio_debug.h"
#include "audio_event.h"
#include "audio_instance.h"
#include "audio_io.h"
#include "audio_sample.h"
#include "audio_static_sound.h"
#include "audio_voc.h"
#include "audio_vox.h"
#include "ccfile.h"
#include "debughandler.h"
#include "miniaudio.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"

#include <algorithm>


/**
 *  Global audio manager instance.
 */
AudioManagerClass AudioManager;


namespace {

bool Is_Valid_Audio_Group(AudioGroupType group)
{
    return group >= 0 && group < AUDIO_GROUP_COUNT;
}

}


/**
 *  Background thread that processes audio requests and cleans up finished instances.
 *
 *  @author: CCHyper
 */
unsigned __stdcall AudioManagerClass::CleanupThreadFunction(void* context)
{
    AudioManagerClass* self = reinterpret_cast<AudioManagerClass*>(context);

    using clock = std::chrono::steady_clock;

    try {

        auto lastTime = clock::now();

        while (!self->ThreadExitFlag.load()) {

            auto now = clock::now();
            std::chrono::duration<float> elapsed = now - lastTime;
            float deltaTime = elapsed.count();
            lastTime = now;

            deltaTime = std::min(deltaTime, 0.25f);

            /**
             *  STEP 1: Process queued play/stop requests.
             */
            std::queue<AudioRequest> requests;
            {
                std::scoped_lock lock(self->RequestMutex);
                std::swap(requests, self->RequestQueue);
            }

            while (!requests.empty()) {
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: RequestQueue - Woke up!\n");

                AudioRequest req = std::move(requests.front());
                requests.pop();

                switch (req.Type) {
                case AudioRequestType::AUDIO_REQUEST_PLAY:
                    self->Process_Play_Request(std::move(req));
                    break;

                case AudioRequestType::AUDIO_REQUEST_STOP:
                    self->Process_Stop_Request(std::move(req));
                    break;
                }
            }

            /**
             *  STEP 2: Update active handles and prune finished instances.
             */
            std::queue<AudioRequest> promoted_requests;
            {
                std::scoped_lock lock(self->ThreadMutex);

                for (int group = 0; group < AUDIO_GROUP_COUNT; ++group) {

                    AudioGroupType groupType = static_cast<AudioGroupType>(group);
                    auto& group_vec = self->GroupedActiveInstanceMap[groupType];

                    for (auto it = group_vec.begin(); it != group_vec.end();) {

                        auto& handle = *it;

                        if (handle == nullptr) {
                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: CLEANUP - Removed a stale null pointer.\n");
                            it = group_vec.erase(it);
                            continue;
                        }

                        handle->Update(deltaTime);

                        if (handle->Is_Finished()) {
                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: CLEANUP - Removed \"%s\" as it finished\n", handle->Get_FileName().c_str());

                            // Capture identity before the handle is freed.
                            const std::string finished_file = handle->Get_FileName();
                            const AudioGroupType finished_group = handle->Get_Sample_Template().Get_Group();
                            const AudioInstanceHandle finished_id = handle->Get_ID();

                            self->Remove_Active_Handle_NoLock(handle->Get_ID()); // handles deletion and removal from both maps
                            self->Clear_Request_State(finished_id);

                            // Promote the first deferred request for this sample now that a
                            // concurrent slot has opened up.
                            if (!self->DeferredPlayQueue.empty()) {
                                std::queue<AudioRequest> still_deferred;
                                bool promoted = false;
                                while (!self->DeferredPlayQueue.empty()) {
                                    AudioRequest deferred = std::move(self->DeferredPlayQueue.front());
                                    self->DeferredPlayQueue.pop();

                                    if (self->Is_Request_Canceled(deferred.HandleID)) {
                                        self->Clear_Request_State(deferred.HandleID);
                                        continue;
                                    }

                                    if (!promoted && deferred.Filename == finished_file && deferred.Group == finished_group) {
                                        if (self->Try_Set_Request_State(deferred.HandleID, AUDIO_REQUEST_STATE_QUEUED)) {
                                            promoted_requests.push(std::move(deferred));
                                            promoted = true;
                                        }
                                    } else {
                                        still_deferred.push(std::move(deferred));
                                    }
                                }
                                self->DeferredPlayQueue = std::move(still_deferred);
                            }

                            it = group_vec.begin();
                        } else {
                            ++it;
                        }
                    }
                }

                // Prune any nullptrs from ActiveInstanceMap.
                for (auto it = self->ActiveInstanceMap.begin(); it != self->ActiveInstanceMap.end();) {
                    if (it->second == nullptr) {
                        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: CLEANUP - Removed a stale null pointer.\n");
                        it = self->ActiveInstanceMap.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            if (!promoted_requests.empty()) {
                {
                    std::scoped_lock req_lock(self->RequestMutex);
                    while (!promoted_requests.empty()) {
                        self->RequestQueue.push(std::move(promoted_requests.front()));
                        promoted_requests.pop();
                    }
                }
                self->RequestCV.notify_all();
            }

            /**
             *  STEP 3: Sleep up to 25ms or until a new request arrives.
             */
            std::unique_lock<std::mutex> wait_lock(self->RequestMutex);
            self->RequestCV.wait_for(wait_lock, std::chrono::milliseconds(25));
        }

    } catch (const std::runtime_error& e) {
        (void)e;
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: EXCEPTION! - std::runtime_error - %s\n", e.what());
    } catch (const std::logic_error& e) {
        (void)e;
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: EXCEPTION! - std::logic_error - %s\n", e.what());
    } catch (const std::exception& ex) {
        (void)ex;
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: EXCEPTION! - std::exception - %s\n", ex.what());
    } catch (...) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: EXCEPTION! - Unknown non-std exception occurred!\n");
    }

    return 0;
}


/**
 *  Generates a unique audio handle ID encoded with a magic tag, group, and counter.
 *
 *  @author: CCHyper
 */
AudioInstanceHandle AudioManagerClass::Generate_Unique_Audio_ID(AudioGroupType group)
{
    /**
     *  ID 0 is reserved as the invalid sentinel for AudioInstanceHandle, so
     *  start the counter at 1 and skip 0 on wrap-around. The group bits are
     *  preserved for human-readable debug output. Once the 24-bit counter
     *  wraps, skip any ID that is still owned by an active, queued, or
     *  deferred request.
     */
    static std::atomic<uint32_t> AudioIDCounter{1};

    constexpr uint32_t kGroupShift   = 24;       // 'Group' uses bits 24-27
    constexpr uint32_t kGroupMask    = 0xF;      // 4 bits for group (16 groups max)
    constexpr uint32_t kCounterMask  = 0xFFFFFF; // Lower 24 bits

    static std::atomic<bool> AudioIDCounterWrapped{false};

    for (uint32_t attempt = 0; attempt < kCounterMask; ++attempt) {
        const uint32_t counter_value = AudioIDCounter.fetch_add(1, std::memory_order_relaxed);
        uint32_t counter = counter_value & kCounterMask;
        bool check_collision = (counter_value & ~kCounterMask) != 0 || AudioIDCounterWrapped.load(std::memory_order_acquire);

        if (counter == 0) {
            AudioIDCounterWrapped.store(true, std::memory_order_release);
            continue;
        }

        uint32_t raw_id = ((static_cast<uint32_t>(group) & kGroupMask) << kGroupShift) | counter;
        AudioInstanceHandle id { raw_id };

        if (!check_collision) {
            return id;
        }

        std::scoped_lock thread_lock(AudioManager.ThreadMutex);
        std::scoped_lock state_lock(AudioManager.RequestStateMutex);
        if (!AudioManager.ActiveInstanceMap.contains(id) && !AudioManager.RequestStateMap.contains(id)) {
            return id;
        }
    }

    AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Generate_Unique_Audio_ID - Exhausted audio handle IDs for group %d!\n", group);

    return INVALID_AUDIO_INSTANCE_HANDLE;
}

/**
 *  Lookup a handle in ActiveInstanceMap. Caller MUST hold ThreadMutex -
 *  the returned pointer is only valid for the duration of that lock.
 */
AudioInstanceClass * AudioManagerClass::Find_Handle_By_ID_NoLock(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return nullptr;
    }

    auto it = AudioManager.ActiveInstanceMap.find(id);
    if (it != AudioManager.ActiveInstanceMap.end()) {
        return it->second.get();
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Find_Handle_By_ID: Failed to find 0x%08X.\n", id.ID);

    return nullptr;
}

/**
 *  Sets the lifecycle state for a request handle.
 *
 *  @author: ZivDero
 */
void AudioManagerClass::Set_Request_State(AudioInstanceHandle id, AudioRequestState state)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return;
    }

    std::scoped_lock lock(RequestStateMutex);
    RequestStateMap[id] = state;
}


/**
 *  Moves a request handle to a new lifecycle state if it has not been canceled.
 *
 *  @author: ZivDero
 */
bool AudioManagerClass::Try_Set_Request_State(AudioInstanceHandle id, AudioRequestState state)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return false;
    }

    std::scoped_lock lock(RequestStateMutex);
    auto it = RequestStateMap.find(id);
    if (it == RequestStateMap.end()) {
        return false;
    }

    if (it->second == AUDIO_REQUEST_STATE_CANCELING || it->second == AUDIO_REQUEST_STATE_FINISHED) {
        RequestStateMap.erase(it);
        return false;
    }

    it->second = state;
    return true;
}


/**
 *  Returns the current lifecycle state for a request handle.
 *
 *  @author: ZivDero
 */
AudioManagerClass::AudioRequestState AudioManagerClass::Get_Request_State(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return AUDIO_REQUEST_STATE_FINISHED;
    }

    std::scoped_lock lock(RequestStateMutex);
    auto it = RequestStateMap.find(id);
    return it != RequestStateMap.end() ? it->second : AUDIO_REQUEST_STATE_FINISHED;
}


/**
 *  Returns whether the request handle has been canceled before active playback.
 *
 *  @author: ZivDero
 */
bool AudioManagerClass::Is_Request_Canceled(AudioInstanceHandle id)
{
    return Get_Request_State(id) == AUDIO_REQUEST_STATE_CANCELING;
}


/**
 *  Cancels a queued or deferred request before an AudioInstanceClass is created.
 *
 *  @author: ZivDero
 */
bool AudioManagerClass::Cancel_Queued_Request(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return false;
    }

    std::scoped_lock lock(RequestStateMutex);
    auto it = RequestStateMap.find(id);
    if (it == RequestStateMap.end()) {
        return false;
    }

    if (it->second == AUDIO_REQUEST_STATE_QUEUED || it->second == AUDIO_REQUEST_STATE_DEFERRED) {
        it->second = AUDIO_REQUEST_STATE_CANCELING;
        return true;
    }

    return it->second == AUDIO_REQUEST_STATE_CANCELING;
}


/**
 *  Clears the lifecycle state for a request handle.
 *
 *  @author: ZivDero
 */
void AudioManagerClass::Clear_Request_State(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return;
    }

    std::scoped_lock lock(RequestStateMutex);
    RequestStateMap.erase(id);
}


/**
 *  Processes a queued play request on the audio thread.
 *
 *  @author: CCHyper
 */
void AudioManagerClass::Process_Play_Request(AudioRequest req)
{
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Got request to play sample \"%s\"...\n", req.Filename.c_str());

    auto finish_request = [&]() {
        Clear_Request_State(req.HandleID);
    };

    if (Is_Request_Canceled(req.HandleID)) {
        finish_request();
        return;
    }

    AudioSampleClass* sample = Find_Sample(std::string(req.Filename.c_str()), req.Group);
    if (sample == nullptr || !sample->Is_Available()) {
        finish_request();
        return;
    }

    // A per-call limit takes precedence over the sample template.
    int limit = (req.Limit > 0) ? req.Limit : sample->Get_Limit();
    if (limit <= 0) {
        finish_request();
        return;
    }

    if (limit > 0 && limit <= AUDIO_MAX_CONCURRENT_LIMIT) {
        std::scoped_lock lock(ThreadMutex);

        auto& group_vec = GroupedActiveInstanceMap[req.Group];

        // Count instances from this sample.
        int active_count = 0;
        AudioInstanceClass* lowest_priority = nullptr;

        for (auto* instance : group_vec) {
            if (!instance) {
                continue;
            }
            if (&instance->Get_Sample_Template() == sample) {
                ++active_count;

                if (!lowest_priority || instance->Get_Sample_Template().Get_Priority() < lowest_priority->Get_Sample_Template().Get_Priority()) {
                    lowest_priority = instance;
                }
            }
        }

        if (active_count >= limit) {
            if (lowest_priority && (req.Priority > lowest_priority->Get_Sample_Template().Get_Priority() || (req.Control & AUDIO_CONTROL_INTERRUPT))) {
                lowest_priority->Set_Fade(0.1f, true);
            } else {
                if (req.Control & AUDIO_CONTROL_QUEUE) {
                    if (Try_Set_Request_State(req.HandleID, AUDIO_REQUEST_STATE_DEFERRED)) {
                        DeferredPlayQueue.push(std::move(req));
                    }
                } else {
                    finish_request();
                }
                return;
            }
        }
    }

    if (Is_Request_Canceled(req.HandleID)) {
        finish_request();
        return;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Creating instance of \"%s\".\n", req.Filename.c_str());

    auto instance = std::make_unique<AudioInstanceClass>(sample, req.HandleID);
    ASSERT_FATAL(instance != nullptr, "Failed to create instance of sample \"%s\"!", sample->Get_FileName().c_str());

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: About to load sample for \"%s\".\n", req.Filename.c_str());

    if (!instance->Load()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: Failed to load sample for \"%s\"!\n", req.Filename.c_str());
        finish_request();
        return;
    }

    const bool allow_sound_looping = req.Group != AUDIO_GROUP_MUSIC;
    const bool infinite_loop = allow_sound_looping && req.Loops && req.LoopLimit <= 0;
    const int loop_limit = allow_sound_looping && req.Loops ? req.LoopLimit : 0;
    instance->Set_Looping(infinite_loop);
    instance->Set_Loop_Limit(loop_limit);

    instance->Set_Volume(req.Volume);
    if (req.FadeInSeconds > 0.0f) {
        instance->Set_Fade(req.FadeInSeconds, false);
    }
    instance->Set_Pitch(req.Pitch);
    instance->Set_Pan(req.Pan);
    instance->Set_Delay(req.DelayInSeconds);

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Sample loaded for \"%s\"!\n", req.Filename.c_str());

    if (req.StartImmediately) {
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Triggering \"%s\" to start playing!\n", req.Filename.c_str());
        instance->Play();
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Adding \"%s\" to active handle tracker.\n", req.Filename.c_str());

    {
        std::scoped_lock lock(ThreadMutex);

        {
            std::scoped_lock state_lock(RequestStateMutex);
            auto state_it = RequestStateMap.find(req.HandleID);
            if (state_it == RequestStateMap.end() || state_it->second == AUDIO_REQUEST_STATE_CANCELING) {
                RequestStateMap.erase(req.HandleID);
                return;
            }
            state_it->second = AUDIO_REQUEST_STATE_ACTIVE;
        }

        Add_Active_Handle_NoLock(std::move(instance));
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Added \"%s\" to active tracker!\n", req.Filename.c_str());
}


/**
 *  Processes a queued stop request on the audio thread.
 *
 *  @author: CCHyper
 */
void AudioManagerClass::Process_Stop_Request(AudioRequest req)
{
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Got request to stop \"%s\"...\n", req.Filename.c_str());

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass* instance = Find_Handle_By_ID_NoLock(req.HandleID);
    if (!instance) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THREAD, "AudioThread: Stop request - handle not found for ID 0x%08X!\n", req.HandleID.ID);
        return;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: It was \"%s\"!\n", instance->Get_FileName().c_str());

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Stopping \"%s\" (fade seconds %f)...\n", instance->Get_FileName().c_str(), req.FadeOutSeconds);
    if (req.FadeOutSeconds > 0.0f) {
        instance->Set_Fade(req.FadeOutSeconds, true);
    } else {
        instance->End();
    }
}


/**
 *  Default constructor.
 *
 *  @author: CCHyper
 */
AudioManagerClass::AudioManagerClass()
{
}


/**
 *  Destructor; shuts down the audio engine.
 *
 *  @author: CCHyper
 */
AudioManagerClass::~AudioManagerClass()
{
    End();
}


/**
 *  Initializes the audio engine, device, sound groups, and background thread.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Init(HWND hWnd)
{
    ma_result result;

    if (IsInitialized) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr: System already initialized!\n");
        return true;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Init...\n");

    /**
     *  Assign our custom vfs using the engine io.
     */
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pResourceManagerVFS = &ma_custom_vfs_callbacks;
    engineConfig.noAutoStart = MA_TRUE;

    // Configure the engine output format.
    engineConfig.sampleRate = 48000;
    engineConfig.channels = 2;
    engineConfig.monoExpansionMode = ma_mono_expansion_mode_stereo_only;

    Engine = new ma_engine;

    result = ma_engine_init(&engineConfig, Engine);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to initialize engine (%s)!\n", ma_result_description(result));
        delete Engine;
        Engine = nullptr;
        return false;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Engine initialized.\n");

    /**
     *  Initialize and start all sound groups.
     */
    for (int group = 0; group < AUDIO_GROUP_COUNT; ++group) {

        std::unique_ptr<ma_sound_group> sound_group = std::make_unique<ma_sound_group>();

        result = ma_sound_group_init(Engine, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, sound_group.get());
        if (result != MA_SUCCESS) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to initialize sound group %d (%s)!\n", group, ma_result_description(result));
            End();
            return false;
        }

        result = ma_sound_group_start(sound_group.get());
        if (result != MA_SUCCESS) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to start sound group %d (%s)!\n", group, ma_result_description(result));
            ma_sound_group_uninit(sound_group.get());
            End();
            return false;
        }

        SoundGroups[group] = sound_group.release();
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Sound group %d initialized.\n", group);
    }

    /**
     *  Set the initial master volume to maximum.
     */
    if (!Set_Master_Volume(AUDIO_VOLUME_MAX)) {
        End();
        return false;
    }

    /**
     *  Start the background cleanup and request processing thread.
     */
    ThreadExitFlag = false;
    try {
        CleanupThread = std::thread(&AudioManagerClass::CleanupThreadFunction, this);
    } catch (const std::exception& ex) {
        (void)ex;
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to start cleanup thread (%s)!\n", ex.what());
        End();
        return false;
    }

    /**
     *  Start the miniaudio engine for audio playback.
     */
    if (!Start_Engine(true)) {
        End();
        return false;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Init done!\n");

    IsInitialized = true;

    return true;
}


/**
 *  Shuts down the audio engine, releases all resources, and joins the background thread.
 *
 *  @author: CCHyper
 */
void AudioManagerClass::End()
{
    const bool has_runtime_state = IsInitialized || Engine != nullptr || CleanupThread.joinable();
    if (!has_runtime_state) {
        return;
    }

    /*
     *  Stop the worker before tearing down miniaudio state so it cannot race
     *  with handle destruction or touch the engine after it is gone.
     */
    IsInitialized = false;
    AudioVocClass::Wait_For_Scan();
    AudioVoxClass::Wait_For_Scan();
    ThreadExitFlag = true;
    RequestCV.notify_all();
    if (CleanupThread.joinable()) {
        CleanupThread.join();
    }

    if (Engine != nullptr) {
        Stop_Engine();
    }

    /**
     *  The CleanupThread.join() above is the synchronization point for both
     *  RequestQueue and DeferredPlayQueue - once the worker is gone, no other
     *  thread touches them, so the clears below need no mutex coverage of their
     *  own (RequestMutex is taken purely for the in-flight invariant).
     */
    {
        std::scoped_lock lock(RequestMutex);
        RequestQueue = {};
        DeferredPlayQueue = {};
    }

    {
        std::scoped_lock lock(RequestStateMutex);
        RequestStateMap.clear();
    }

    /**
     *  Clear all active audio handles while the engine and groups still exist.
     */
    Clear_All_Active_Handles();

    {
        std::scoped_lock lock(SubmissionMutex);
        SamplesMap.clear();
    }

    /**
     *  Uninitialize the sound groups.
     */
    for (auto& sound_group : SoundGroups) {
        if (sound_group != nullptr) {
            ma_sound_group_uninit(sound_group);
            delete sound_group;
            sound_group = nullptr;
        }
    }

    /**
     *  Now we can uninitialize the engine.
     */
    if (Engine != nullptr) {
        ma_engine_uninit(Engine);
        delete Engine;
        Engine = nullptr;
    }

    ThreadExitFlag = false;
}


/**
 *  Returns whether the audio manager has been successfully initialized.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Is_Available() const
{
    return IsInitialized;
}


/**
 *  Starts the miniaudio engine for audio playback.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Start_Engine(bool forced)
{
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Start_Engine().\n");

    if (Engine == nullptr || (!forced && !IsInitialized)) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr::Start_Engine - Engine is not initialized.\n");
        return false;
    }

    ma_result result;

    result = ma_engine_start(Engine);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to start engine (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Stops the miniaudio engine.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Stop_Engine()
{
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Stop_Engine().\n");

    if (Engine == nullptr) {
        return true;
    }

    ma_result result;

    result = ma_engine_stop(Engine);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to stop engine (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Handle application focus loss.
 * 
 *  @author: CCHyper
 */
void AudioManagerClass::Focus_Loss()
{
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Focus_Loss().\n");

    if (Engine == nullptr) {
        return;
    }

    ma_result result;

    FocusRestoreVolume.store(ma_engine_get_volume(Engine), std::memory_order_relaxed);

    result = ma_engine_set_volume(Engine, 0.0f);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Focus_Loss - ma_engine_set_volume failed (%s)!\n", ma_result_description(result));
    }
}


/**
 *  Handle application focus restore.
 * 
 *  @author: CCHyper
 */
void AudioManagerClass::Focus_Restore()
{
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Focus_Restore().\n");

    if (Engine == nullptr) {
        return;
    }

    ma_result result;

    result = ma_engine_set_volume(Engine, FocusRestoreVolume.load(std::memory_order_relaxed));
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Focus_Restore - ma_engine_set_volume failed (%s)!\n", ma_result_description(result));
    }
}


/**
 *  Per-frame maintenance tick for the audio manager.
 *
 *  @author: ZivDero
 */
void AudioManagerClass::Sound_Callback()
{
    AudioEventSystem::AI();
    Tracked_Static_Sounds_AI();
}


/**
 *  Submits a play request to the audio thread and waits for the instance to be created.
 *
 *  @author: CCHyper
 */
AudioInstanceHandle AudioManagerClass::Request_Play(const std::string& filename, AudioGroupType group, float volume, float pitch, float pan, AudioPriorityType priority, int limit, float fade_in_seconds, float delay_in_seconds, bool start, bool looping, int loop_limit, AudioControlType control)
{
    /**
     *  If the sample preloader (ScanAsync) hasn't finished with this file yet,
     *  drop the play and warn - we never want to stall the game thread waiting
     *  on disk I/O. Missing an early one-shot is preferable to a visible hitch.
     */
    if (!AudioManager.Query_Sample_Ready(filename, group)) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr::Request_Play - Sample \"%s\" not ready, dropping request.\n", filename.c_str());
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    AudioInstanceHandle id = Generate_Unique_Audio_ID(group);
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Request_Play - Unable to allocate handle for request \"%s\".\n", filename.c_str());
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Request_Play - Generated id '0x%08X' for request \"%s\".\n", id.ID, filename.c_str());

    // Insert into the state map before queuing to eliminate the race where the
    // worker processes the request before the handle becomes queryable.
    Set_Request_State(id, AUDIO_REQUEST_STATE_QUEUED);

    {
        std::scoped_lock lock(RequestMutex);

        RequestQueue.emplace(
            id,
            filename,
            group,
            volume,
            pitch,
            pan,
            priority,
            limit,
            fade_in_seconds,
            delay_in_seconds,
            start,
            looping,
            loop_limit,
            control
        );
    }

    RequestCV.notify_all();

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Request_Play - Request to play \"%s\" submitted.\n", filename.c_str());

    return id;
}


/**
 *  Submits a stop request for the given audio handle to the audio thread.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Request_Stop(AudioInstanceHandle id, float fade_out)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Request_Stop - Invalid handle!\n");
        return false;
    }

    if (Cancel_Queued_Request(id)) {
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Request_Stop - Canceled queued/deferred handle 0x%08X.\n", id.ID);
        RequestCV.notify_all();
        return true;
    }

    {
        std::scoped_lock lock(RequestMutex);
        RequestQueue.emplace(id, fade_out);
    }

    RequestCV.notify_all();

#ifndef NDEBUG
    /**
     *  Debug-only lookup of the filename for logging. Must use ThreadMutex
     *  (which protects ActiveInstanceMap), not RequestMutex.
     */
    {
        std::scoped_lock lock(ThreadMutex);
        if (AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id)) {
            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Request_Stop - Request to stop \"%s\" submitted.\n", handle->Get_FileName().c_str());
        } else {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr::Request_Stop - Handle not found for ID 0x%08X\n", id.ID);
        }
    }
#endif

    return true;
}


/**
 *  Pauses playback of the audio instance with the given handle.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Request_Pause(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Request_Pause - Invalid handle!\n");
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return handle->Pause();
}


/**
 *  Resumes playback of a paused audio instance.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Request_Resume(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Request_Resume - Invalid handle!\n");
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return handle->Resume();
}


/**
 *  Queries whether the audio instance with the given handle is currently playing.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Query_Is_Playing(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        //AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Query_Is_Playing - Invalid handle!\n");
        return false;
    }

    AudioRequestState request_state = Get_Request_State(id);
    if (request_state == AUDIO_REQUEST_STATE_QUEUED || request_state == AUDIO_REQUEST_STATE_DEFERRED) {
        return true;
    }
    if (request_state == AUDIO_REQUEST_STATE_CANCELING) {
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return handle->Is_Playing();
}


/**
 *  Queries whether the audio instance with the given handle is currently active (i.e. exists and is not done).
 *
 *  @author: CCHyper, ZivDero
 */
bool AudioManagerClass::Query_Is_Active(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return false;
    }

    AudioRequestState request_state = Get_Request_State(id);
    if (request_state == AUDIO_REQUEST_STATE_QUEUED || request_state == AUDIO_REQUEST_STATE_DEFERRED) {
        return true;
    }
    if (request_state == AUDIO_REQUEST_STATE_CANCELING) {
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass* handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return !handle->Is_Finished();
}


/**
 *  Queries whether the audio instance with the given handle is currently paused.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Query_Is_Paused(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Query_Is_Paused - Invalid handle!\n");
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return handle->Is_Paused();
}


/**
 *  Checks whether a sample has been submitted and is available for playback.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Query_Sample_Ready(std::string name, AudioGroupType group)
{
    std::scoped_lock lock(SubmissionMutex);

    AudioSampleKey key { name, group };

    auto it = SamplesMap.find(key);

    return it != SamplesMap.end() && it->second != nullptr && it->second->Is_Available();
}


/**
 *  Sets the volume of an active audio instance by handle.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Set_Volume(AudioInstanceHandle id, float volume)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Set_Volume - Invalid handle!\n");
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return handle->Set_Volume(volume);
}


/**
 *  Sets the stereo pan of an active audio instance by handle.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Set_Pan(AudioInstanceHandle id, float pan)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Set_Pan - Invalid handle!\n");
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return handle->Set_Pan(pan);
}


/**
 *  Sets the pitch of an active audio instance by handle.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Set_Pitch(AudioInstanceHandle id, float pitch)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Set_Pitch - Invalid handle!\n");
        return false;
    }

    std::scoped_lock lock(ThreadMutex);

    AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
    if (handle == nullptr) {
        return false;
    }

    return handle->Set_Pitch(pitch);
}


/**
 *  Registers a new audio sample template for later playback.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Submit_Sample(
    const std::string& filename,
    AudioFileType filetype,
    AudioGroupType group,
    AudioPriorityType priority,
    AudioControlType control,
    AudioSoundType type,
    unsigned int limit)
{
    if (SubmissionsLocked) {
        return false;
    }

    AudioSampleKey key { filename, group };

    {
        std::scoped_lock lock(SubmissionMutex);

        if (SamplesMap.contains(key)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr::Submit_Sample - Sample with the filename \"%s\" already exists!\n", filename.c_str());
            return false;
        }

        std::unique_ptr<AudioSampleClass> sample = std::make_unique<AudioSampleClass>();
        sample->FileName = filename;
        sample->FileType = filetype;
        sample->Group = group;
        sample->Priority = priority;
        sample->Control = control;
        sample->Type = type;
        sample->ConcurrentLimit = limit;

        SamplesMap.emplace(std::move(key), std::move(sample));
    }

    return true;
}


/**
 *  Clears all preloaded audio samples from the sample map.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Clear_Samples(AudioGroupType group)
{
    std::scoped_lock lock(SubmissionMutex);

    bool any_cleared = false;

    for (auto it = SamplesMap.begin(); it != SamplesMap.end();) {
        const auto& sample = it->second;

        if (sample != nullptr) {
            if (group == AUDIO_GROUP_NONE || sample->Get_Group() == group) {
                it = SamplesMap.erase(it);
                any_cleared = true;
                continue;
            }
        }

        ++it;
    }

    return any_cleared;
}


/**
 *  Checks whether a sample with the given filename has already been submitted.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Has_Been_Submitted(const std::string& filename, AudioGroupType group)
{
    std::scoped_lock lock(SubmissionMutex);

    AudioSampleKey key { filename, group };

    if (group == AUDIO_GROUP_NONE) {
        for (const auto& pair : SamplesMap) {
            if (pair.first.Filename == filename) {
                return true;
            }
        }
        return false;
    } else {
        return SamplesMap.contains(key);
    }
}


/**
 *  Returns whether the given audio handle ID refers to an active instance.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Is_Handle_Valid(AudioInstanceHandle id)
{
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Is_Handle_Valid - Invalid handle!\n");
        return false;
    }

    std::scoped_lock lock(ThreadMutex);
    return Find_Handle_By_ID_NoLock(id) != nullptr;
}


/**
 *  Sets the master volume on the audio engine.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Set_Master_Volume(float volume) const
{
    ma_result result;

    result = ma_engine_set_volume(Engine, std::clamp(volume, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX));
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to set engine volume (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Sets the volume for a specific sound group.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Set_Group_Volume(AudioGroupType group, float volume)
{
    if (!Is_Valid_Audio_Group(group) || SoundGroups[group] == nullptr) {
        return false;
    }

    ma_sound_group_set_volume(SoundGroups[group], std::clamp(volume, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX));

    return true;
}


/**
 *  Returns the current volume for a specific sound group.
 *
 *  @author: CCHyper
 */
float AudioManagerClass::Get_Group_Volume(AudioGroupType group)
{
    if (!Is_Valid_Audio_Group(group) || SoundGroups[group] == nullptr) {
        return 0.0f;
    }

    float volume = ma_sound_group_get_volume(SoundGroups[group]);

    return volume;
}


/**
 *  Returns whether a sound group is currently playing.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Is_Group_Playing(AudioGroupType group) const
{
    if (!Is_Valid_Audio_Group(group) || SoundGroups[group] == nullptr) {
        return false;
    }

    ma_bool32 result;

    result = ma_sound_group_is_playing(SoundGroups[group]);

    return result;
}


/**
 *  Starts playback of a sound group.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Start_Group(AudioGroupType group) const
{
    if (!Is_Valid_Audio_Group(group) || SoundGroups[group] == nullptr) {
        return false;
    }

    ma_result result;

    result = ma_sound_group_start(SoundGroups[group]);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Start_Group - ma_sound_group_start failed (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Stops playback of a sound group.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Stop_Group(AudioGroupType group) const
{
    if (!Is_Valid_Audio_Group(group) || SoundGroups[group] == nullptr) {
        return false;
    }

    ma_result result;

    result = ma_sound_group_stop(SoundGroups[group]);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Stop_Group - ma_sound_group_stop failed (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Stops a sound group with a fade-out duration (currently stops immediately).
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Stop_And_Fade_Out_Group(AudioGroupType group, float duration)
{
    if (!Is_Valid_Audio_Group(group) || SoundGroups[group] == nullptr) {
        return false;
    }

    if (duration <= 0.0f) {
        Stop_Group(group);
        return true;
    }

    std::scoped_lock lock(ThreadMutex);

    auto& groupVec = GroupedActiveInstanceMap[group];
    bool faded_any = false;
    for (auto* handle : groupVec) {
        if (handle != nullptr && handle->Is_Playing()) {
            handle->Set_Fade(duration, true);
            faded_any = true;
        }
    }

    if (!faded_any) {
        Stop_Group(group);
    }

    return true;
}


/**
 *  Adds an audio instance to the active handle trackers (thread-safe).
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Add_Active_Handle(std::unique_ptr<AudioInstanceClass> audio_handle)
{
    std::scoped_lock lock(ThreadMutex);
    return Add_Active_Handle_NoLock(std::move(audio_handle));
}


/**
 *  Removes an audio instance from the active handle trackers (thread-safe).
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Remove_Active_Handle(AudioInstanceHandle audio_id)
{
    std::scoped_lock lock(ThreadMutex);
    return Remove_Active_Handle_NoLock(audio_id);
}


/**
 *  Adds an audio instance to both the active instance map and grouped map (no lock).
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Add_Active_Handle_NoLock(std::unique_ptr<AudioInstanceClass> audio_handle)
{
    if (!audio_handle) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Add_Active_Handle - Invalid handle!\n");
        return false;
    }

    AudioInstanceHandle id = audio_handle->Get_ID();
    AudioGroupType group = audio_handle->Get_Sample_Template().Get_Group();

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Add_Active_Handle: About to add 0x%08X to trackers...\n", id.ID);

    AudioInstanceClass * raw_ptr = audio_handle.get();

    ActiveInstanceMap[id] = std::move(audio_handle);
    GroupedActiveInstanceMap[group].push_back(raw_ptr);

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Add_Active_Handle: 0x%08X added to trackers\n", id.ID);

    return true;
}


/**
 *  Removes an audio instance from both the active instance map and grouped map (no lock).
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Remove_Active_Handle_NoLock(AudioInstanceHandle audio_id)
{
    auto it = ActiveInstanceMap.find(audio_id);
    if (it == ActiveInstanceMap.end()) {
        return false;
    }

    AudioInstanceClass * audio_handle = it->second.get();

    if (audio_handle != nullptr) {
        AudioGroupType group = audio_handle->Get_Sample_Template().Get_Group();

        auto& groupVec = GroupedActiveInstanceMap[group];
        auto groupIt = std::find(groupVec.begin(), groupVec.end(), audio_handle);

        if (groupIt != groupVec.end()) {
            groupVec.erase(groupIt);
        }
    }

    ActiveInstanceMap.erase(it);

    return true;
}


/**
 *  Clears all active audio instances from both tracking maps.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Clear_All_Active_Handles()
{
    {
        std::scoped_lock lock(ThreadMutex);

        ActiveInstanceMap.clear();
        GroupedActiveInstanceMap.clear();
    }

    return true;
}


AudioSampleClass * AudioManagerClass::Find_Sample(const std::string & filename, AudioGroupType group)
{
    std::scoped_lock lock(SubmissionMutex);

    AudioSampleKey key{ filename, group };

    auto it = SamplesMap.find(key);
    if (it != SamplesMap.end()) {
        return it->second.get();
    }

    return nullptr;
}


/**
 *  Returns whether the given audio file type is supported by the current build configuration.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Is_FileType_Supported(AudioFileType type) const
{
    switch (type) {
#ifndef MA_NO_VORBIS
        case AUDIO_TYPE_OGG: return true;
#endif
#ifndef MA_NO_FLAC
        case AUDIO_TYPE_FLAC: return true;
#endif
#ifndef MA_NO_MP3
        case AUDIO_TYPE_MP3: return true;
#endif
#ifndef MA_NO_WAV
        case AUDIO_TYPE_WAV: return true;
#endif
#ifndef MA_NO_AUD
        case AUDIO_TYPE_AUD: return true;
#endif
        default: break;
    };

    return false;
}


/**
 *  Builds a full filename by appending the appropriate extension for the given audio type.
 *
 *  @author: CCHyper
 */
std::string AudioManagerClass::Build_Filename_From_Type(AudioFileType type, std::string name)
{
    switch (type) {
#ifndef MA_NO_VORBIS
        case AUDIO_TYPE_OGG: return name + ".OGG";
#endif
#ifndef MA_NO_FLAC
        case AUDIO_TYPE_FLAC: return name + ".FLAC";
#endif
#ifndef MA_NO_MP3
        case AUDIO_TYPE_MP3: return name + ".MP3";
#endif
#ifndef MA_NO_WAV
        case AUDIO_TYPE_WAV: return name + ".WAV";
#endif
#ifndef MA_NO_AUD
        case AUDIO_TYPE_AUD: return name + ".AUD";
#endif
        default: break;
    };

    return {};
}


/**
 *  Converts an AudioPriorityType enum value to a 0-255 integer priority.
 *
 *  @author: CCHyper
 */
int AudioManagerClass::AudioPriority_To_Priority(AudioPriorityType priority)
{
    switch (priority) {
        case AUDIO_PRIORITY_LOWEST: return 0;
        case AUDIO_PRIORITY_LOW: return 64;
        case AUDIO_PRIORITY_NORMAL: return 128;
        case AUDIO_PRIORITY_HIGH: return 192;
        case AUDIO_PRIORITY_CRITICAL: return 255;
        default: break;
    }

    return 128;
}


/**
 *  Utility function for converting from game priority to the new priority type.
 *
 *  @author: CCHyper
 */
AudioPriorityType AudioManagerClass::Priority_To_AudioPriority(int priority)
{
    priority = std::clamp(priority, 0, 255);

    if (priority <= 51) {
        return AUDIO_PRIORITY_LOWEST;
    }
    if (priority <= 102) {
        return AUDIO_PRIORITY_LOW;
    }
    if (priority <= 153) {
        return AUDIO_PRIORITY_NORMAL;
    }
    if (priority <= 204) {
        return AUDIO_PRIORITY_HIGH;
    }

    return AUDIO_PRIORITY_CRITICAL;
}


/**
 *  Utility functions for converting integer audio volume (original DSAudio values) to and from float (miniaudio).
 *
 *  @author: CCHyper
 */
unsigned int AudioManagerClass::fVolume_To_iVolume(float vol)
{
    vol = std::clamp(vol, 0.0f, 1.0f);
    return (vol * 255);
}

float AudioManagerClass::iVolume_To_fVolume(unsigned int vol)
{
    return float(vol) / 255.0f;
}


/**
 *  Check to see if the audio file exists in known formats. If found, store
 *  its type and filename so it can be passed into the audio engine at play.
 * 
 *  Priority: FLAC -> WAV -> OGG -> MP3 -> AUD
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Get_File_Info(const std::string& name, AudioFileType& filetype, std::string& filename, bool ignore_error)
{
#ifndef MA_NO_FLAC
    if (Is_File_Available(AUDIO_TYPE_FLAC, name)) {
        filetype = AUDIO_TYPE_FLAC;
        filename = Build_Filename_From_Type(AUDIO_TYPE_FLAC, name);
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Found \"%s\".\n", filename.c_str());
        return true;
    }
#endif

#ifndef MA_NO_WAV
    if (Is_File_Available(AUDIO_TYPE_WAV, name)) {
        filetype = AUDIO_TYPE_WAV;
        filename = Build_Filename_From_Type(AUDIO_TYPE_WAV, name);
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Found \"%s\".\n", filename.c_str());
        return true;
    }
#endif

#ifndef MA_NO_VORBIS
    if (Is_File_Available(AUDIO_TYPE_OGG, name)) {
        filetype = AUDIO_TYPE_OGG;
        filename = Build_Filename_From_Type(AUDIO_TYPE_OGG, name);
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Found \"%s\".\n", filename.c_str());
        return true;
    }
#endif

#ifndef MA_NO_MP3
    if (Is_File_Available(AUDIO_TYPE_MP3, name)) {
        filetype = AUDIO_TYPE_MP3;
        filename = Build_Filename_From_Type(AUDIO_TYPE_MP3, name);
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Found \"%s\".\n", filename.c_str());
        return true;
    }
#endif

#ifndef MA_NO_AUD
    if (Is_File_Available(AUDIO_TYPE_AUD, name)) {
        filetype = AUDIO_TYPE_AUD;
        filename = Build_Filename_From_Type(AUDIO_TYPE_AUD, name);
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Found \"%s\".\n", filename.c_str());
        return true;
    }
#endif

    if (ignore_error) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr: Unable to find \"%s\" in a supported format!\n", name.c_str());
    }

    return false;
}

/**
 *  Specific filetype check.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Is_File_Available(AudioFileType type, std::string name) const
{
    switch (type) {
#ifndef MA_NO_VORBIS
        case AUDIO_TYPE_OGG:
            name += ".OGG";
            break;
#endif
#ifndef MA_NO_FLAC
        case AUDIO_TYPE_FLAC:
            name += ".FLAC";
            break;
#endif
#ifndef MA_NO_MP3
        case AUDIO_TYPE_MP3:
            name += ".MP3";
            break;
#endif
#ifndef MA_NO_WAV
        case AUDIO_TYPE_WAV:
            name += ".WAV";
            break;
#endif
#ifndef MA_NO_AUD
        case AUDIO_TYPE_AUD:
            name += ".AUD";
            break;
#endif
        default:
            return false;
    };

    /**
     *  Search for the file in the mix files.
     */
    if (CCFileClass(name.c_str()).Is_Available()) {
        return true;
    }

    return false;
}

/**
 *  Checks whether an audio file exists in any supported format.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Is_File_Available(std::string name) const
{
#ifndef MA_NO_FLAC
    if (Is_File_Available(AUDIO_TYPE_FLAC, name)) {
        return true;
    }
#endif

#ifndef MA_NO_WAV
    if (Is_File_Available(AUDIO_TYPE_WAV, name)) {
        return true;
    }
#endif

#ifndef MA_NO_VORBIS
    if (Is_File_Available(AUDIO_TYPE_OGG, name)) {
        return true;
    }
#endif

#ifndef MA_NO_MP3
    if (Is_File_Available(AUDIO_TYPE_MP3, name)) {
        return true;
    }
#endif

#ifndef MA_NO_AUD
    if (Is_File_Available(AUDIO_TYPE_AUD, name)) {
        return true;
    }
#endif

    return false;
}
