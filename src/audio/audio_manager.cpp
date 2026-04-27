/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Installable MiniAudio audio driver.
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


/**
 *  Background thread that processes audio requests and cleans up finished instances.
 *
 *  @author: CCHyper
 */
unsigned __stdcall AudioManagerClass::CleanupThreadFunction(void * context)
{
    AudioManagerClass * _this = reinterpret_cast<AudioManagerClass *>(context);

    using clock = std::chrono::steady_clock;

    try {

        auto lastTime = clock::now();

        while (!_this->ThreadExitFlag.load()) {

            auto now = clock::now();
            std::chrono::duration<float> elapsed = now - lastTime;
            float deltaTime = elapsed.count();  // in seconds
            lastTime = now;

            // Clamp to 250ms max step
            deltaTime = std::min(deltaTime, 0.25f);

            /**
             *  STEP 1: Process requests first!
             */
            { // local scope start
            
                // Ensure thread-safe access to audio handles.
                std::scoped_lock lock(_this->RequestMutex);

                while (!_this->RequestQueue.empty()) {

                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: RequestQueue - Woke up!\n");

                    AudioRequest req = std::move(_this->RequestQueue.front());
                    _this->RequestQueue.pop();

                    switch (req.Type) {
                        case AudioRequestType::AUDIO_REQUEST_PLAY:
                        {
                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Got request to play sample \"%s\"...\n", req.Filename.c_str());

                            // Remove this ID from the pending set on any exit that does not
                            // result in a live instance or a deferred (queued) request.
                            auto drop_pending = [&]() {
                                std::scoped_lock plock(_this->PendingMutex);
                                _this->PendingHandleIDs.erase(req.HandleID);
                            };

                            AudioSampleClass * sample = _this->Find_Sample(std::string(req.Filename.c_str()), req.Group);
                            if (sample == nullptr || !sample->Is_Available()) {
                                // TODO, debug print
                                drop_pending();
                                break;
                            }

                            // Check the concurrent limit of this sample before playing.
                            // A per-call limit from the request takes precedence over the sample template.
                            int limit = (req.Limit > 0) ? req.Limit : sample->Get_Limit();
                            if (limit <= 0) {
                                drop_pending();
                                break; // Just skip odd cases
                            }

                            if (limit > 0 && limit <= AUDIO_MAX_CONCURRENT_LIMIT) {
                                auto& groupVec = _this->GroupedActiveInstanceMap[req.Group];

                                // Count instances from this sample
                                int activeCount = 0;
                                AudioInstanceClass * lowestPriority = nullptr;

                                for (auto* inst : groupVec) {
                                    if (!inst) {
                                        continue;
                                    }
                                    if (&inst->Get_Sample_Template() == sample) {
                                        ++activeCount;

                                        if (!lowestPriority || inst->Get_Sample_Template().Get_Priority() < lowestPriority->Get_Sample_Template().Get_Priority()) {
                                            lowestPriority = inst;
                                        }
                                    }
                                }

                                if (activeCount >= limit) {
                                    const bool can_interrupt = (req.Control & (AUDIO_CONTROL_INTERRUPT | AUDIO_CONTROL_QUEUED_INTERRUPT)) != 0;
                                    if (lowestPriority && (req.Priority > lowestPriority->Get_Sample_Template().Get_Priority() || can_interrupt)) {
                                        // Replace lower priority sound
                                        lowestPriority->Set_Fade(0.1f, true, true);
                                        //lowestPriority->Stop(0.1f); // Fade out a little bit.
                                    } else {
                                        if (req.Control & AUDIO_CONTROL_QUEUE) {
                                            // ID stays in PendingHandleIDs so Query_Is_Playing
                                            // returns true while the request waits for a slot.
                                            _this->DeferredPlayQueue.push(req);
                                        } else {
                                            drop_pending();
                                        }
                                        break;
                                    }
                                }
                            }
                        
                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Creating instance of \"%s\".\n", req.Filename.c_str());

                            // Create new instance from sample
                            auto instance = std::make_unique<AudioInstanceClass>(sample, req.HandleID);
                            ASSERT_FATAL(instance != nullptr, "Failed to create instance of sample \"%s\"!", sample->Get_FileName().c_str());

                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: About to load sample for \"%s\".\n", req.Filename.c_str());

                            if (!instance->Load()) {
                                AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: Failed to load sample for \"%s\"!\n", req.Filename.c_str());
                                drop_pending();
                                break;
                            }

                            // ThemeClass owns music repetition, so do not apply sound-level looping
                            // to AUDIO_GROUP_MUSIC. Non-music requests may use infinite or finite loops.
                            const bool allow_sound_looping = req.Group != AUDIO_GROUP_MUSIC;
                            const bool infinite_loop = allow_sound_looping && req.Loops && req.LoopLimit <= 0;
                            const int loop_limit = allow_sound_looping && req.Loops ? req.LoopLimit : 0;
                            instance->Set_Looping(infinite_loop);
                            instance->Set_Loop_Limit(loop_limit);

                            instance->Set_Fade(req.FadeInSeconds, false);
                            instance->Set_Volume(req.Volume);
                            instance->Set_Pitch(req.Pitch);
                            instance->Set_Pan(req.Pan);
                            instance->Set_Delay(req.DelayInSeconds);

                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Sample loaded for \"%s\"!\n", req.Filename.c_str());

                            if (req.StartImmediately) {
                                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Triggering \"%s\" to start playing!\n", req.Filename.c_str());
                                instance->Play();
                            }

                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Adding \"%s\" to active handle tracker.\n", req.Filename.c_str());

                            _this->Add_Active_Handle_NoLock(std::move(instance));
                            drop_pending();

                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Added \"%s\" to active tracker!\n", req.Filename.c_str());

                            break;
                        }

                        case AudioRequestType::AUDIO_REQUEST_STOP:
                        {
                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Got request to stop \"%s\"...\n", req.Filename.c_str());

                            AudioInstanceClass * instance = Find_Handle_By_ID_NoLock(req.HandleID);
                            if (!instance) {
                                AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THREAD, "AudioThread: Stop request - handle not found for ID 0x%08X!\n", req.HandleID.ID);
                                break;
                            }

                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: It was \"%s\"!\n", instance->Get_FileName().c_str());

                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: Stopping \"%s\" (with fade seconds %f)...\n", req.Filename.c_str(), req.FadeOutSeconds);
                            if (req.FadeOutSeconds > 0.0f) {
                                instance->Set_Fade(req.FadeOutSeconds, true, true);
                            } else {
                                instance->End();
                            }

                            break;
                        }
                    };
                }

            } // local scope end

            /**
             *  STEP 3: Update active handles
             */
            { // local scope start

                // Ensure thread-safe access to the audio handles map.
                std::scoped_lock lock(_this->ThreadMutex);

                // 3.1 - Iterate over each audio group.
                for (int group = 0; group < AUDIO_GROUP_COUNT; ++group) {

                    AudioGroupType groupType = static_cast<AudioGroupType>(group);
                    auto& groupVec = _this->GroupedActiveInstanceMap[groupType];

                    // Use iterator to safely erase while iterating
                    for (auto it = groupVec.begin(); it != groupVec.end();) {

                        auto& handle = *it;
                     
                        // Clean up null entries just in case.
                        if (handle == nullptr) {
                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: CLEANUP - Removed a stay NULL ptr...\n");
                            it = groupVec.erase(it);
                            continue;
                        }

                        // Update the handle with the time delta.
                        handle->Update(deltaTime);

                        // If the handle has finished playback (e.g. stopped, completed fade out, etc.)
                        if (handle->Is_Finished()) {
                            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: CLEANUP - Removed \"%s\" as it finished\n", handle->Get_FileName().c_str());

                            // Capture identity before the handle is freed.
                            const std::string finishedFile = handle->Get_FileName();
                            const AudioGroupType finishedGroup = handle->Get_Sample_Template().Get_Group();

                            _this->Remove_Active_Handle_NoLock(handle->Get_ID()); // handles deletion and removal from both maps

                            // Promote the first deferred request for this sample now that a
                            // concurrent slot has opened up.
                            if (!_this->DeferredPlayQueue.empty()) {
                                std::queue<AudioRequest> stillDeferred;
                                bool promoted = false;
                                while (!_this->DeferredPlayQueue.empty()) {
                                    AudioRequest deferred = std::move(_this->DeferredPlayQueue.front());
                                    _this->DeferredPlayQueue.pop();
                                    if (!promoted && deferred.Filename == finishedFile && deferred.Group == finishedGroup) {
                                        std::scoped_lock reqLock(_this->RequestMutex);
                                        _this->RequestQueue.push(std::move(deferred));
                                        promoted = true;
                                    } else {
                                        stillDeferred.push(std::move(deferred));
                                    }
                                }
                                _this->DeferredPlayQueue = std::move(stillDeferred);
                            }

                            //it = groupVec.erase(it); // erase from vector     // Not required, Remove_Active_Handle now removes it for us.
                            it = groupVec.begin(); // Start from beginning again
                        } else {
                            ++it;
                        }
                    }

                } // local scope end

                // 3.2 - Sanity cleanup � remove any nullptrs from ActiveInstanceMap
                for (auto it = _this->ActiveInstanceMap.begin(); it != _this->ActiveInstanceMap.end(); ) {
                    if (it->second == nullptr) {
                        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THREAD, "AudioThread: CLEANUP - Removed a stay NULL ptr...\n");
                        it = _this->ActiveInstanceMap.erase(it);
                    } else {
                        ++it;
                    }
                }

            }

            /**
             *  STEP 4: Wait up to 25ms or until a new request
             * 
             *  Sleep to avoid CPU spinning. 25ms (~40Hz update rate) is enough for audio responsiveness.
             */
            std::unique_lock<std::mutex> wait_lock(_this->RequestMutex);
            _this->RequestCV.wait_for(wait_lock, std::chrono::milliseconds(25));
        }

    } catch (const std::runtime_error& e) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: EXCEPTION! - std::runtime_error - %s\n", e.what());
    } catch (const std::logic_error& e) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THREAD, "AudioThread: EXCEPTION! - std::logic_error - %s\n", e.what());
    } catch (const std::exception & ex) {
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
     *  preserved for human-readable debug output.
     */
    static std::atomic<uint32_t> AudioIDCounter{1};

    constexpr uint32_t kGroupShift   = 24;       // 'Group' uses bits 24-27
    constexpr uint32_t kGroupMask    = 0xF;      // 4 bits for group (16 groups max)
    constexpr uint32_t kCounterMask  = 0xFFFFFF; // Lower 24 bits

    uint32_t counter = AudioIDCounter.fetch_add(1, std::memory_order_relaxed) & kCounterMask;
    if (counter == 0) {
        counter = AudioIDCounter.fetch_add(1, std::memory_order_relaxed) & kCounterMask;
    }

    uint32_t id = ((static_cast<uint32_t>(group) & kGroupMask) << kGroupShift) | counter;

    return AudioInstanceHandle{id};
}

// Causes a deadlock as its called within other locking functions, NoLock instead.
AudioInstanceClass * AudioManagerClass::Find_Handle_By_ID(AudioInstanceHandle id)
{
    std::scoped_lock lock(AudioManager.ThreadMutex);
    return Find_Handle_By_ID_NoLock(id);
}

AudioInstanceClass * AudioManagerClass::Find_Handle_By_ID_NoLock(AudioInstanceHandle id)
{
    // Just to be safe...
    //if (!AudioManager.Is_Available()) {
    //    return nullptr;
    //}

    // Reject invalid ID immediately
    if (id == INVALID_AUDIO_INSTANCE_HANDLE) {
        return nullptr;
    }

    for (const auto& pair : AudioManager.ActiveInstanceMap) {
        //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Find_Handle_By_ID: At this time, the map contains: 0x%08X %s...\n", pair.first, pair.second->Get_FileName().c_str());
    }
    //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Find_Handle_By_ID: Looking for: 0x%08X...\n", id);

    auto it = AudioManager.ActiveInstanceMap.find(id);
    if (it != AudioManager.ActiveInstanceMap.end()) {
        //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Find_Handle_By_ID: Found 0x%08X - \"%s\".\n", id, it->second.get()Get_FileName().c_str());
        return it->second.get();  // Return raw pointer to object, not ownership
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Find_Handle_By_ID: Failed to find 0x%08X.\n", id.ID);

    return nullptr; // Not found!
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

#ifndef NDEBUG
    /**
     *  Create the audio debug window if debug mode is enabled.
     */
    if (Vinifera_AudioDebug) {
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: About to create debug window.\n");
        if (!Create_Debug_Window()) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to create debug window!\n");
            return false;
        }
    }
#endif

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
    ASSERT(Engine != nullptr);

    result = ma_engine_init(&engineConfig, Engine);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to initialize engine (%s)!\n", ma_result_description(result));
        return false;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Engine initialized.\n");

    /**
     *  Initialize and start all sound groups.
     */
    for (int group = 0; group < AUDIO_GROUP_COUNT; ++group) {

        SoundGroups[group] = new ma_sound_group;
        ASSERT(SoundGroups[group] != nullptr);

        result = ma_sound_group_init(Engine, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, SoundGroups[group]);
        if (result != MA_SUCCESS) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to initialize sound group %d (%s)!\n", group, ma_result_description(result));
            return false;
        }

        result = ma_sound_group_start(SoundGroups[group]);
        if (result != MA_SUCCESS) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr: Failed to start sound group %d (%s)!\n", group, ma_result_description(result));
            return false;
        }

        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr: Sound group %d initialized.\n", group);
    }

    /**
     *  Set the initial master volume to maximum.
     */
    Set_Master_Volume(AUDIO_VOLUME_MAX);

    /**
     *  Start the background cleanup and request processing thread.
     */
    ThreadExitFlag = false;
    CleanupThread = std::thread(&AudioManagerClass::CleanupThreadFunction, this);

    /**
     *  Start the miniaudio engine for audio playback.
     */
    Start_Engine(true);

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
    ThreadExitFlag = true;
    RequestCV.notify_all();
    ThreadWakeSignal.notify_all();
    if (CleanupThread.joinable()) {
        CleanupThread.join();
    }

    if (Engine != nullptr) {
        Stop_Engine();
    }

    {
        std::scoped_lock lock(RequestMutex);
        RequestQueue = {};
    }

    {
        std::scoped_lock lock(PendingMutex);
        PendingHandleIDs.clear();
    }

    DeferredPlayQueue = {};

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

    ma_result result;

    FocusRestoreVolume = ma_engine_get_volume(Engine);

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

    ma_result result;

    result = ma_engine_set_volume(Engine, FocusRestoreVolume);
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
     *  drop the play and warn — we never want to stall the game thread waiting
     *  on disk I/O. Missing an early one-shot is preferable to a visible hitch.
     */
    if (!AudioManager.Query_Sample_Ready(filename, group)) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr::Request_Play - Sample \"%s\" not ready, dropping request.\n", filename.c_str());
        return INVALID_AUDIO_INSTANCE_HANDLE;
    }

    AudioInstanceHandle id = Generate_Unique_Audio_ID(group);
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Request_Play - Generated id '0x%08X' for request \"%s\".\n", id.ID, filename.c_str());

    // Mark as pending before pushing to the queue so Query_Is_Playing returns true
    // immediately. Insert first to eliminate the race where the worker processes
    // the request before we can insert the ID.
    {
        std::scoped_lock plock(PendingMutex);
        PendingHandleIDs.insert(id);
    }

    // Enqueue request
    {
        std::scoped_lock lock(RequestMutex);

        RequestQueue.emplace( // Uses the "Play" constructor
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

    // Notify the thread we did something!
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

    {
        std::scoped_lock lock(RequestMutex);

        RequestQueue.emplace(id, fade_out); // Uses the "Stop" constructor

        AudioInstanceClass * handle = Find_Handle_By_ID_NoLock(id);
        if (handle) {
            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_MANAGER, "AudioMgr::Request_Stop - Request to stop \"%s\" submitted.\n", handle->Get_FileName().c_str());
        } else {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr::Request_Stop - Handle not found for ID 0x%08X\n", id.ID);
        }
    }

    // Notify the thread we did something!
    RequestCV.notify_all();

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

    AudioInstanceClass * handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        ASSERT_FATAL(handle != nullptr);
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

    AudioInstanceClass * handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        ASSERT_FATAL(handle != nullptr);
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

    // A request that is submitted but not yet processed by the worker is still
    // "playing" from the caller's perspective — report it as active.
    {
        std::scoped_lock plock(PendingMutex);
        if (PendingHandleIDs.contains(id)) {
            return true;
        }
    }

    AudioInstanceClass * handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        //ASSERT_FATAL(handle != nullptr);
        if (handle == nullptr) { // Sometimes the returned handle is just null when a track ends and the thread cleans it before this query is performed.
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Query_Is_Playing - Find_Handle_By_ID returned null (ID: 0x%08X)!\n", id.ID);
            return false;
        }
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
        // AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Query_Is_Playing - Invalid handle!\n");
        return false;
    }

    // A request that is submitted but not yet processed by the worker is still
    // "playing" from the caller's perspective — report it as active.
    {
        std::scoped_lock plock(PendingMutex);
        if (PendingHandleIDs.contains(id)) {
            return true;
        }
    }

    AudioInstanceClass* handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        // ASSERT_FATAL(handle != nullptr);
        if (handle == nullptr) { // Sometimes the returned handle is just null when a track ends and the thread cleans it before this query is performed.
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Query_Is_Playing - Find_Handle_By_ID returned null (ID: 0x%08X)!\n", id.ID);
            return false;
        }
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

    AudioInstanceClass * handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        ASSERT_FATAL(handle != nullptr);
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

    std::string str(name.c_str());
    AudioSampleKey key { str, group };

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
    
    AudioInstanceClass * handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        if (handle == nullptr) {
            return false;
        }
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

    AudioInstanceClass * handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        if (handle == nullptr) {
            return false;
        }
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
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_MANAGER, "AudioMgr::Set_Pan - Invalid handle!\n");
        return false;
    }

    AudioInstanceClass * handle = nullptr;
    {
        std::scoped_lock lock(ThreadMutex); // Lock against Cleanup thread

        handle = Find_Handle_By_ID_NoLock(id);
        if (handle == nullptr) {
            return false;
        }
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

    std::string str(filename.c_str());
    AudioSampleKey key { str, group };

    {
        std::scoped_lock lock(SubmissionMutex);

        if (SamplesMap.contains(key)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_MANAGER, "AudioMgr::Submit_Sample - Sample with the filename \"%s\" already exists!\n", filename.c_str());
            return false; // Sample with this filename/group already exists
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
            // Clear matching group or all if AUDIO_GROUP_NONE
            if (group == AUDIO_GROUP_NONE || sample->Get_Group() == group) {
                it = SamplesMap.erase(it); // Unique_ptr will auto-delete
                any_cleared = true;
                continue;
            }
        }

        ++it; // Advance normally if not erased
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

    std::string str(filename.c_str());
    AudioSampleKey key { str, group };

    if (group == AUDIO_GROUP_NONE) {
        for (const auto& pair : SamplesMap) {
            if (pair.first.Filename == str) {
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

    AudioInstanceClass * handle = Find_Handle_By_ID(id);
#ifndef DEBUG
    ASSERT_FATAL(handle != nullptr);
#endif
    return handle != nullptr;
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
    ma_sound_group_set_volume(SoundGroups[group], volume);

    return true;
}


/**
 *  Returns the current volume for a specific sound group.
 *
 *  @author: CCHyper
 */
float AudioManagerClass::Get_Group_Volume(AudioGroupType group)
{
    if (group < 0 || group >= AUDIO_GROUP_COUNT) return 0.0f;

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
    if (duration <= 0.0f) {
        Stop_Group(group);
        return true;
    }

    std::scoped_lock lock(ThreadMutex);

    auto& groupVec = GroupedActiveInstanceMap[group];
    bool faded_any = false;
    for (auto* handle : groupVec) {
        if (handle != nullptr && handle->Is_Playing()) {
            handle->Set_Fade(duration, true, true);
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

    ActiveInstanceMap[id] = std::move(audio_handle); // store ownership
    GroupedActiveInstanceMap[group].push_back(raw_ptr); // just track, no ownership

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

    // Get a raw pointer for comparison (but don't delete it manually!)
    AudioInstanceClass * audio_handle = it->second.get();

    // Also remove from GroupedActiveInstanceMap
    if (audio_handle != nullptr) {
        AudioGroupType group = audio_handle->Get_Sample_Template().Get_Group();

        auto& groupVec = GroupedActiveInstanceMap[group];
        auto groupIt = std::find(groupVec.begin(), groupVec.end(), audio_handle);

        if (groupIt != groupVec.end()) {
            groupVec.erase(groupIt);  // The instance is deleted here automatically
        }
    }

    // Automatically deletes the instance when erased
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
        std::scoped_lock lock(ThreadMutex); // Protect both maps

        // Smart pointers handle deletion automatically.
        ActiveInstanceMap.clear();
        GroupedActiveInstanceMap.clear();
    }

    return true;
}


AudioSampleClass * AudioManagerClass::Find_Sample(const std::string & filename, AudioGroupType group)
{
    {
        std::scoped_lock lock(SubmissionMutex);

        std::string str(filename.c_str());
        AudioSampleKey key{ str, group };

        auto it = SamplesMap.find(key);
        if (it != SamplesMap.end()) {
            return it->second.get(); // safe: Return the raw pointer, caller doesn't take ownership.
        }

        return nullptr;
    }
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

    return nullptr;
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
 *  Utility functions for converting the integer audio volume (orignal DSAudio values) to and from float (Miniaudio).
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
        default: break;
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
