/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  VQA movie audio handler using miniaudio streaming.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_ahandle.h"

#include "ahandle.h"
#include "asserthandler.h"
#include "audio_manager.h"
#include "audio_streaming.h"
#include "debughandler.h"
#include "gametime.h"

#include <mutex>


// Callbacks
long (__cdecl *MoveHMIAudioBlock_Callback)(VQAHandle *) = nullptr;
long (__cdecl *VQASync_Callback)(VQAHandle *, void *) = nullptr;


static std::mutex AudioHandleMutex;


static bool IsHandleOpen;
//static float Volume;

static unsigned short SampleRate = 22050;
static unsigned char Channels = 1;
static unsigned char BitsPerSample = 8;

static unsigned long LastBytesPlayed = 0;
static unsigned long LastUpdateTime = 0;
static int PauseAdjust = 0;
static unsigned long PlaybackTime60Hz = 0;
static unsigned long Flags = 0;

static void * CallbackBuffers[2] = { nullptr, nullptr };
static int CurrentBufferIndex = 0;

static void * CallbackBufferPtr = nullptr;

// Our new streaming instance
static AudioStreamingClass * StreamInstance = nullptr;

static std::atomic<bool> AudioHandleThreadExit = {false};
static std::thread AudioHandleThread;

/**
 *  Audio handler thread for VQA playback. Runs at 60Hz to match the original
 *  game's timer callback rate. Only feeds new audio blocks when the ring buffer
 *  is running low, mimicking the original DirectSound callback behavior.
 *
 *  @author: CCHyper
 */
unsigned __stdcall Audio_Handler_Thread(void * context)
{
    auto next_tick = std::chrono::steady_clock::now();

    VQAHandle *vqa = (VQAHandle *)context;
    VQAHandleP *vqap = (VQAHandleP *)context;

    while (!AudioHandleThreadExit.load(std::memory_order_relaxed)) {

        if (!(Flags & AHANDLEF_IS_PAUSED)) {

            /**
             *  Only feed new audio data when the ring buffer is running low.
             *  This mirrors the original DirectSound callback which only wrote
             *  more data when the play cursor had advanced past the last chunk.
             */
            bool needs_data = true;
            if (StreamInstance) {
                ma_uint32 available = StreamInstance->Get_Available_Read_Frames();
                needs_data = (available < 2048);
            }

            if (needs_data) {
                if (MoveHMIAudioBlock_Callback) {
                    MoveHMIAudioBlock_Callback((VQAHandle*)vqa);
                }

                if (VQASync_Callback && CallbackBufferPtr) {
                    VQASync_Callback((VQAHandle*)vqa, (char*)CallbackBufferPtr);
                }
            }
        }

        next_tick += std::chrono::milliseconds(1000 / 60); // 60 Hz
        std::this_thread::sleep_until(next_tick);
    }

    return 0;
}


/**
 *  Timer callback for VQA audio playback that tracks time in 60Hz ticks.
 *
 *  @author: CCHyper
 */
unsigned long __cdecl AudioHandleClass::Timer_Callback_Audio_Handler(VQAHandle *vqa)
{
    VQAHandleP* vqap = (VQAHandleP*)vqa;
    VQAConfig* config = &vqap->Config;

    if (Flags & AHANDLEF_IS_PAUSED) {
        return PlaybackTime60Hz;
    }

    unsigned long current_time = Simple_Timer_Callback_Audio_Handler(nullptr) - PauseAdjust;
    unsigned long bytes_played = Get_Total_Bytes_Played(vqa, config);

    if (bytes_played > 0 && bytes_played <= LastBytesPlayed) {
        if (current_time > LastUpdateTime) {
            bytes_played = (current_time - LastUpdateTime);
            LastUpdateTime = current_time;
            PlaybackTime60Hz += bytes_played;
        }
    } else {
        LastBytesPlayed = bytes_played;
        LastUpdateTime = current_time;
        PlaybackTime60Hz = (60 * (bytes_played / (Channels * (BitsPerSample / 8))) / SampleRate);

        if (PlaybackTime60Hz >= config->LatencyAdjustment) {
            PlaybackTime60Hz -= config->LatencyAdjustment;
        } else {
            PlaybackTime60Hz = 0;
        }
    }
    
//    DEBUG_INFO("AudioHandle: Timer_Callback_Audio_Handler -> returning %d\n", PlaybackTime60Hz);

    return PlaybackTime60Hz;
}


/**
 *  Compute bytes played using miniaudio's sound cursor.
 *  This queries the actual read position of the data source, which is
 *  the closest equivalent to DirectSound's GetCurrentPosition DMA cursor.
 */
unsigned long AudioHandleClass::Get_Total_Bytes_Played(VQAHandle *vqa, VQAConfig *config)
{
    if (!StreamInstance) {
        return 0;
    }

    std::scoped_lock lock(AudioHandleMutex);

    uint64_t frames_played = StreamInstance->Get_Cursor_In_PCM_Frames();
    uint32_t frame_size = Channels * (BitsPerSample / 8);
    uint32_t total_bytes_played = static_cast<unsigned long>(frames_played * frame_size);

    return total_bytes_played;
}


/**
 *  Main streaming dispatch function for VQA audio operations.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Stream_Audio_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes)
{
    VQAHandleP *vqap = (VQAHandleP *)vqa;
    VQAConfig *config = &vqap->Config;
    long error = VQAERR_NONE;

    switch(action) {
        case 1: // Callback
            config->TimerCallback = Simple_Timer_Callback_Audio_Handler;
            config->RefreshRate = 60;
            error = VQAERR_NONE;
            break;
        case 2: // Open
            error = Open_Audio_Handler(vqap, (AhandleInitParams *)buffer, nbytes);
            break;
        case 3: // Close
            error = Close_Audio_Handler(vqap);
            break;
        case 4: // Start
            error = Start_Audio_Handler(vqap);
            break;
        case 5: // Load
            error = Load_Audio_Handler(vqap, buffer, nbytes);
            break;
        case 6: // Pause
            error = Pause_Audio_Handler(vqap);
            break;
        case 7: // Stop
            error = Stop_Audio_Handler(vqap);
            break;
        case 8: // Play
            error = Play_Audio_Handler(vqap);
            break;
        default:
            break;
    }

    return(error);
}


/**
 *  Opens the audio handler for VQA playback and initializes the streaming instance.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Open_Audio_Handler(VQAHandleP *vqap, AhandleInitParams *params, long b)
{
    DEBUG_INFO("AudioHandle: Opening VQ audio handler\n");

    if (!AudioManager.Is_Available()) {
        return VQAERR_AUDIO;
    }

    VQAConfig * config = &vqap->Config;

    IsHandleOpen = true;
    // Volume = config->Volume;

    std::scoped_lock lock(AudioHandleMutex);

    SampleRate = (config->AudioRate != -1) ? config->AudioRate
                 : (config->FrameRate != vqap->FrameRate)
                   ? params->SampleRate * (unsigned)config->FrameRate / vqap->FrameRate
                   : params->SampleRate;

    Channels = params->Channels;
    BitsPerSample = params->BitsPerSample;
    Flags = params->Flags;

    MoveHMIAudioBlock_Callback = params->Callback1; // 006A98D0, Move HMI Audio Block -> feeds next chunk to audio
    VQASync_Callback = params->Callback2; // 006A9A20, Sync callback -> informs VQA player that block is processed

    /**
     *  Reset timing state for the new VQA playback session.
     */
    LastBytesPlayed = 0;
    LastUpdateTime = 0;
    PauseAdjust = 0;
    PlaybackTime60Hz = 0;
    CallbackBufferPtr = nullptr;
    CurrentBufferIndex = 0;

    DEBUG_INFO("AudioHandle: Open_Audio_Handler -> %d %d %d\n", SampleRate, Channels, BitsPerSample);

    if (StreamInstance) {
        delete StreamInstance;
        StreamInstance = nullptr;
    }

    StreamInstance = new AudioStreamingClass();
    if (!StreamInstance->Open(std::string("VQA_AUDIO_STREAM"), SampleRate, Channels, BitsPerSample, true)) {
        DEBUG_ERROR("AudioHandle: Failed to open VQ audio handler!\n");
        return VQAERR_AUDIO;
    }

    AudioHandleThreadExit.store(false, std::memory_order_relaxed);
    AudioHandleThread = std::thread(&Audio_Handler_Thread, (VQAHandle *)vqap);

    DEBUG_INFO("AudioHandle: VQ audio handler opened\n");

    return VQAERR_NONE;
}


/**
 *  Closes the audio handler and releases all VQA audio resources.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Close_Audio_Handler(VQAHandleP *vqap)
{
    DEBUG_INFO("AudioHandle: Closing VQ audio handler\n");

    if (!IsHandleOpen) {
        return VQAERR_NONE;
    }

    /**
     *  Signal the thread to exit first, then join it BEFORE locking the mutex.
     *  The thread's MoveHMIAudioBlock_Callback path calls Load_Audio_Handler
     *  which locks AudioHandleMutex, so we must not hold it while joining.
     */
    AudioHandleThreadExit.store(true, std::memory_order_relaxed);
    if (AudioHandleThread.joinable()) {
        AudioHandleThread.join();
    }

    /**
     *  Now safe to lock and clean up - the thread is no longer running.
     */
    std::scoped_lock lock(AudioHandleMutex);

    if (StreamInstance) {
        StreamInstance->Stop();
        delete StreamInstance;
        StreamInstance = nullptr;
    }

    IsHandleOpen = false;

    LastBytesPlayed = 0;
    LastUpdateTime = 0;
    PauseAdjust = 0;
    PlaybackTime60Hz = 0;
    CallbackBufferPtr = nullptr;
    CurrentBufferIndex = 0;

    return VQAERR_NONE;
}


/**
 *  Starts VQA audio playback by preloading initial chunks and beginning playback.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Start_Audio_Handler(VQAHandleP *vqap)
{
    if (!IsHandleOpen || StreamInstance == nullptr) {
        return VQAERR_AUDIO;
    }

    /**
     *  If we are paused, just resume playback (matches original behavior).
     */
    if (Flags & AHANDLEF_IS_PAUSED) {
        DEBUG_INFO("AudioHandle: Start_Audio_Handler -> Resuming from paused state.\n");
        return Play_Audio_Handler(vqap);
    }

    /**
     *  Pre-load two audio blocks before starting playback, matching
     *  the original which called _AHandleCallbackFunc1 twice.
     */
    if (MoveHMIAudioBlock_Callback) {
        MoveHMIAudioBlock_Callback((VQAHandle *)vqap);
        MoveHMIAudioBlock_Callback((VQAHandle *)vqap);
    }

    DEBUG_INFO("AudioHandle: Starting VQ audio handler\n");
    return Play_Audio_Handler(vqap);
}


/**
 *  Loads an audio chunk into the streaming buffer for VQA playback.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Load_Audio_Handler(VQAHandleP *vqap, void *buffer, long nbytes)
{
    if (!StreamInstance || !buffer || nbytes <= 0)
        return VQAERR_AUDIO;

    std::scoped_lock lock(AudioHandleMutex);

    CallbackBuffers[CurrentBufferIndex] = buffer;
    CallbackBufferPtr = CallbackBuffers[CurrentBufferIndex];

    // Immediately push chunk into stream
    StreamInstance->Push_Chunk(buffer, nbytes);

    // Rotate index for next load
    CurrentBufferIndex = (CurrentBufferIndex + 1) % 2;

//    DEBUG_INFO("AudioHandle: Rotated buffer -> idx = %d ptr = %p\n", CurrentBufferIndex, CallbackBufferPtr);
//    DEBUG_INFO("AudioHandle: Pushed bytes %d to stream handler\n", nbytes);

    return VQAERR_NONE;
}


/**
 *  Pauses VQA audio playback.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Pause_Audio_Handler(VQAHandleP *vqap)
{
    std::scoped_lock lock(AudioHandleMutex);
    if (StreamInstance) {
        StreamInstance->Pause();
    }
    Flags |= AHANDLEF_IS_PAUSED;
    DEBUG_INFO("AudioHandle: Pausing VQ audio handler\n");
    return VQAERR_NONE;
}


/**
 *  Resumes or starts VQA audio playback.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Play_Audio_Handler(VQAHandleP *vqap)
{
    std::scoped_lock lock(AudioHandleMutex);
    if (!StreamInstance) {
        return VQAERR_AUDIO;
    }
    Flags &= ~AHANDLEF_IS_PAUSED;
    DEBUG_INFO("AudioHandle: Playing VQ audio handler\n");
    return StreamInstance->Play() ? VQAERR_NONE : VQAERR_AUDIO;
}


/**
 *  Stops VQA audio playback.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Stop_Audio_Handler(VQAHandleP *vqap)
{
    std::scoped_lock lock(AudioHandleMutex);
    DEBUG_INFO("AudioHandle: Stopping VQ audio handler\n");
    if (StreamInstance) {
        StreamInstance->Stop();
    }
    return VQAERR_NONE;
}


/**
 *  Simple timer callback that returns the current game time in 60Hz ticks.
 *
 *  @author: CCHyper
 */
unsigned long __cdecl AudioHandleClass::Simple_Timer_Callback_Audio_Handler(VQAHandle *vqa)
{
    return Get_Game_Time_60Hz();
}


/**
 *  Legacy lock function for compatibility with original VQA audio interface.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Lock_Audio_Handler(void)
{
    DEBUG_INFO("AudioHandle: Locking handler\n");
    return(1);
}


/**
 *  Legacy unlock function for compatibility with original VQA audio interface.
 *
 *  @author: CCHyper
 */
long __cdecl AudioHandleClass::Unlock_Audio_Handler(void)
{
    DEBUG_INFO("AudioHandle: Unlocking handler\n");
    return(1);
}
