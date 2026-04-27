/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Streaming audio implementation.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_streaming.h"

#include "asserthandler.h"
#include "audio_debug.h"
#include "audio_decoders.h"
#include "audio_manager.h"
#include "audio_util.h"
#include "miniaudio.h"


/**
 *  Default constructor for the audio streaming class.
 *
 *  @author: CCHyper
 */
AudioStreamingClass::AudioStreamingClass()
{
}


/**
 *  Class destructor.
 *
 *  @author: CCHyper
 */
AudioStreamingClass::~AudioStreamingClass()
{
    Close();
}


/**
 *  Opens a new audio stream with the given format parameters.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Open(const std::string& name, int sample_rate, int channels, int bits_per_sample, bool is_pcm)
{
    Close();

    LogicalName = name;
    SampleRate = sample_rate;
    Channels = channels;
    BitsPerSample = bits_per_sample;
    IsPCM = is_pcm;

    if (IsPCM) {
        return Initialize_PCM_Stream();
    } else {
        // AUD stream will be initialized on first Push_Chunk
        return true;
    }
}


/**
 *  Closes the stream, releasing the sound, decoder, and PCM buffer resources.
 *
 *  @author: CCHyper
 */
void AudioStreamingClass::Close()
{
    std::scoped_lock lock(StreamMutex);

    if (Sound) {
        ma_sound_uninit(Sound);
        delete Sound;
        Sound = nullptr;
    }

    if (DecoderInitialized) {
        ma_decoder_uninit(Decoder);
        DecoderInitialized = false;
    }

    if (PCMBuffer) {
        ma_pcm_rb_uninit(PCMBuffer);
        delete PCMBuffer;
        PCMBuffer = nullptr;
    }

    ChunkBuffer.clear();

    StreamInitialized = false;
}


/**
 *  Initializes a PCM ring buffer and attaches it as a streaming data source.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Initialize_PCM_Stream()
{
    ma_result result;

    ma_format format = Audio_GetMAFormatFromBPS(BitsPerSample);

    /**
     *  Size the ring buffer to hold several VQA audio chunks. A VQA chunk is
     *  HMIBufSize = 8192 bytes = 4096 frames for 16-bit mono, and the feeder
     *  thread refills when the available read count drops below 2048 frames —
     *  which means the buffer must hold "leftover + one full chunk" without
     *  dropping. Previously the ring buffer was sized to exactly one chunk
     *  (4096 frames), so every refill overflowed and the tail of each chunk was
     *  silently dropped. Four chunks of headroom gives the feeder room to land
     *  a full push without dropping and tolerates jitter from the audio thread
     *  and the game loop.
     */
    PCMBuffer = new ma_pcm_rb;
    result = ma_pcm_rb_init(format,
                                   Channels,
                                   4096 * 4,
                                   nullptr,
                                   nullptr,
                                   PCMBuffer);
    if (result != MA_SUCCESS) {
        delete PCMBuffer;
        PCMBuffer = nullptr;
        return false;
    }

    /**
     *  ma_pcm_rb_init leaves the ring buffer's sample rate at 0, which causes the
     *  data source to report an unknown rate. Miniaudio then treats the data as if
     *  it were already at the engine rate and does not set up a resampler — in
     *  that state ma_sound_set_pitch has nothing to act on, so attempts to
     *  compensate via pitch are silently ignored and the audio plays at engine
     *  rate (too fast). Tell the ring buffer the actual source rate up front so
     *  ma_sound_init_from_data_source builds a proper resampler.
     */
    ma_pcm_rb_set_sample_rate(PCMBuffer, (ma_uint32)SampleRate);

    /**
     *  Create a new sound object.
     */
    if (Sound != nullptr) {
        delete Sound;
        Sound = nullptr;
    }
    Sound = new ma_sound;

    // In Miniaudio, the ring buffer itself is a data source
    result = ma_sound_init_from_data_source(AudioManager.Engine,
                                        &PCMBuffer->ds,
                                        AUDIO_GROUP_STREAMING,
                                        AudioManager.SoundGroups[AUDIO_GROUP_STREAMING],
                                        Sound);
    if (result != MA_SUCCESS) {
        ma_pcm_rb_uninit(PCMBuffer);
        delete PCMBuffer;
        PCMBuffer = nullptr;
        return false;
    }

    ma_sound_set_volume(Sound, 1.0f);
    ma_sound_set_spatialization_enabled(Sound, MA_FALSE);

    StreamInitialized = true;

    return true;
}

#if 0
/**
 *  Initializes a custom AUD decoder from a memory buffer for streaming playback.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Initialize_AUD_Decoder(const void* initialData, size_t size)
{
    ma_result result;

    if (DecoderInitialized) {
        return true;
    }

    ma_sound_flags flags = ma_sound_flags(MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_STREAM);

    /**
     *  Create a new decoder object for handling any WS AUD files. This must
     *  be done within this branch to ensure we don't alloc the decoder object for
     *  non custom decoder files. ma_sound_init_from_data_source takes ownership.
     */
    Decoder = new ma_decoder;
    ASSERT(Decoder != nullptr);

    // Create a decoder config and register the custom backends.
    ma_decoder_config decoderConfig = ma_decoder_config_init_default();
    decoderConfig.pCustomBackendUserData = nullptr;
    decoderConfig.ppCustomBackendVTables = (ma_decoding_backend_vtable **)ma_custom_backend_vtable;
    decoderConfig.customBackendCount = sizeof(ma_custom_backend_vtable) / sizeof(ma_custom_backend_vtable[0]);

    //error in ma_decoder_init_from_vtable__internal?

    // Initialize the decoder using from a memory buffer with the AUD decoder backend.
    result = ma_decoder_init_memory(initialData, size, &decoderConfig, Decoder);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Load - ma_decoder_init_vfs failed (%s)!\n", ma_result_description(result));
        return false;
    }

    ASSERT_FATAL(Decoder != nullptr);

    DecoderInitialized = true; // Handy flag to ensure it was initialized correctly.

    // Attach the decoder to the sound system as a streaming data source.
    result = ma_sound_init_from_data_source(AudioManager.Engine, Decoder, flags, AudioManager.SoundGroups[AUDIO_GROUP_STREAMING], Sound);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Load - ma_sound_init_from_data_source failed (%s)!\n", ma_result_description(result));
        return false;
    }

    DecoderIsOwnedBySound = true;

    return true;
}
#endif


/**
 *  Pushes a chunk of audio data into the stream's ring buffer.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Push_Chunk(const void* data, size_t size)
{
    std::scoped_lock lock(StreamMutex);

    if (IsPCM) {

        if (!PCMBuffer) {
            return false;
        }

        size_t frames = size / (Channels * (BitsPerSample / 8));
        size_t frames_pushed = 0;
        const ma_uint8* src = (const ma_uint8*)data;

        while (frames_pushed < frames) {

            ma_uint32 frames_to_write = (ma_uint32)(frames - frames_pushed);
            void* dst;
            ma_uint32 frames_writable;

            ma_pcm_rb_acquire_write(PCMBuffer, &frames_writable, &dst);
            if (frames_writable == 0) {
                break; // buffer full
            }

            if (frames_writable > frames_to_write) frames_writable = (ma_uint32)frames_to_write;

            memcpy(dst, src + frames_pushed * Channels * (BitsPerSample/8),
                   frames_writable * Channels * (BitsPerSample/8));

            ma_pcm_rb_commit_write(PCMBuffer, frames_writable);
            frames_pushed += frames_writable;
        }

        /**
         *  Only count what actually made it into the ring buffer. If the buffer is
         *  full and we break out early, inflating FramesPushed by the full chunk
         *  size would make Get_Cursor_In_PCM_Frames over-report consumption, which
         *  skews the VQA library's audio-time estimate and can desync playback.
         */
        FramesPushed.fetch_add(frames_pushed, std::memory_order_relaxed);

    } else {

        // AUD stream: Initialize decoder on first chunk
        //if (!DecoderInitialized) {
        //    return Initialize_AUD_Decoder(data, size);
        //}

        // For full streaming AUD: a custom streaming data source would be needed.
        return true;

    }

    return true;
}


/**
 *  Starts playback of the audio stream.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Play()
{
    std::scoped_lock lock(StreamMutex);
    if (!Sound) {
        return false;
    }
    return ma_sound_start(Sound) == MA_SUCCESS;
}


/**
 *  Pauses playback of the audio stream.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Pause()
{
    std::scoped_lock lock(StreamMutex);
    if (!Sound) {
        return false;
    }
    return ma_sound_stop(Sound) == MA_SUCCESS;
}


/**
 *  Stops playback of the audio stream.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Stop()
{
    std::scoped_lock lock(StreamMutex);
    if (!Sound) {
        return false;
    }
    return ma_sound_stop(Sound) == MA_SUCCESS;
}


/**
 *  Checks if the audio stream is currently playing.
 *
 *  @author: CCHyper
 */
bool AudioStreamingClass::Is_Playing() const
{
    std::scoped_lock lock(StreamMutex);
    if (!Sound) {
        return false;
    }
    return ma_sound_is_playing(Sound);
}


/**
 *  Returns the estimated number of PCM frames that have been played.
 *
 *  @author: CCHyper
 */
uint64_t AudioStreamingClass::Get_Frames_Played() const
{
    std::scoped_lock lock(StreamMutex);

    if (IsPCM && Sound) {
        ma_uint64 cursor = 0;
        if (ma_sound_get_cursor_in_pcm_frames(Sound, &cursor) == MA_SUCCESS) {
            return cursor;
        }
    }

    const uint64_t pushed = FramesPushed.load(std::memory_order_relaxed);

    // Fallback: estimate from push-side counter minus buffered frames.
    if (IsPCM && PCMBuffer) {
        ma_uint32 frames_in_buffer = ma_pcm_rb_available_read(PCMBuffer);
        return (pushed > frames_in_buffer) ? (pushed - frames_in_buffer) : 0;
    }

    return pushed;
}


/**
 *  Returns the current playback cursor position in PCM frames.
 *
 *  @author: CCHyper
 */
uint64_t AudioStreamingClass::Get_Cursor_In_PCM_Frames() const
{
    std::scoped_lock lock(StreamMutex);

    /**
     *  For file-backed sounds, use miniaudio's cursor directly.
     */
    if (!IsPCM && Sound) {
        ma_uint64 cursor = 0;
        if (ma_sound_get_cursor_in_pcm_frames(Sound, &cursor) == MA_SUCCESS) {
            return cursor;
        }
    }

    /**
     *  For PCM ring buffer streams, miniaudio doesn't track a linear cursor.
     *  Estimate consumption as: total pushed minus what's still buffered.
     */
    if (IsPCM && PCMBuffer) {
        const uint64_t pushed = FramesPushed.load(std::memory_order_relaxed);
        ma_uint32 frames_in_buffer = ma_pcm_rb_available_read(PCMBuffer);
        return (pushed > frames_in_buffer) ? (pushed - frames_in_buffer) : 0;
    }

    return 0;
}


/**
 *  Returns the number of frames available to read from the PCM ring buffer.
 *
 *  @author: CCHyper
 */
ma_uint32 AudioStreamingClass::Get_Available_Read_Frames() const
{
    std::scoped_lock lock(StreamMutex);
    if (IsPCM && PCMBuffer) {
        return ma_pcm_rb_available_read(PCMBuffer);
    }
    return 0;
}
