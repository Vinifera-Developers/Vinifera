/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Streaming audio class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_defines.h"
#include "miniaudio.h"

#include <atomic>
#include <mutex>


/**
 *  Streaming audio playback class, used for VQA movie audio.
 */
class AudioStreamingClass
{
    friend class AudioManagerClass;

public:
    AudioStreamingClass();
    virtual ~AudioStreamingClass();

    bool Open(const std::string& name, int sampleRate, int channels, int bitsPerSample, bool isPCM = true);
    void Close();

    bool Push_Chunk(const void* data, size_t size); // Raw PCM or AUD depending on isPCM

    bool Play();
    bool Pause();
    bool Stop();

    bool Is_Playing() const;
    uint64_t Get_Frames_Played() const;
    uint64_t Get_Cursor_In_PCM_Frames() const;
    ma_uint32 Get_Available_Read_Frames() const;

private:
    bool Initialize_PCM_Stream();

private:
    /**
     *  Pointer to the Miniaudio engine sound object
     */
    ma_sound* Sound = nullptr;

    /**
     *  Serializes Open/Close/Push_Chunk/Play/Pause/Stop on this stream.
     */
    mutable std::mutex StreamMutex;

    bool IsPCM = false;
    ma_pcm_rb* PCMBuffer = nullptr;

    std::string LogicalName;

    int SampleRate = 22050;
    int Channels = 1;
    int BitsPerSample = 16;

    /**
     *  Total frames pushed into the ring buffer. Atomic because the getters
     *  read it lock-free from the main thread while Push_Chunk increments it
     *  on the VQA feeder thread.
     */
    std::atomic<uint64_t> FramesPushed {0};

public:
    AudioStreamingClass(const AudioStreamingClass&) = delete;
    AudioStreamingClass& operator=(const AudioStreamingClass&) = delete;
};
