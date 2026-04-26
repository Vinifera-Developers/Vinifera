/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Audio sample instance implementation.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_instance.h"

#include "asserthandler.h"
#include "audio_debug.h"
#include "audio_decoders.h"
#include "audio_io.h"
#include "audio_manager.h"
#include "audio_util.h"
#include "ccfile.h"
#include "miniaudio.h"

#include <chrono>


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
AudioInstanceClass::AudioInstanceClass(AudioSampleClass * tmpl, AudioInstanceHandle id) :
    Template(tmpl),
    HandleID(id)
{
}


/**
 *  Class destructor.
 *
 *  @author: CCHyper
 */
AudioInstanceClass::~AudioInstanceClass()
{
    Free();
}


/**
 *  Loads the sound from the sample template, initializing the decoder and sound object.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Load(/*std::string filename, ma_sound_group *group*/)
{
    ma_result result;

    /**
     *  Sound is already loaded, return true.
     */
    if (IsLoaded) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_INSTANCE, "AudioInstance::Load - Sound is already loaded, why are you calling Load()?!\n");
        return true;
    }

    if (!Template->Is_Available()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Load - Unable to find \"%s\"!\n", Template->Get_FileName().c_str());
        return false;
    }

    /**
     *  Create a new sound object.
     */
    Sound = new ma_sound;
    ASSERT(Sound != nullptr);

    Decoder = nullptr;

    // TODO: Short one-shot SFX should use MA_SOUND_FLAG_DECODE to decode into memory
    //       instead of streaming, which adds unnecessary I/O overhead for tiny samples.
    ma_sound_flags flags = ma_sound_flags(MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_STREAM);

    // Check if the file is a Westwood AUD (IMA-ADPCM) file.
    if (Audio_IsAUDFile(Template->Get_FileName())) {

        /**
         *  Create a new decoder object for handling any WS AUD files. This must
         *  be done within this branch to ensure we don't alloc the decoder object for
         *  non custom decoder files.
         */
        Decoder = new ma_decoder;
        ASSERT(Decoder != nullptr);

        // Create a decoder config and register the custom backends.
        ma_decoder_config decoderConfig = ma_decoder_config_init_default();
        decoderConfig.pCustomBackendUserData = nullptr;
        decoderConfig.ppCustomBackendVTables = (ma_decoding_backend_vtable **)ma_custom_backend_vtable;
        decoderConfig.customBackendCount = sizeof(ma_custom_backend_vtable) / sizeof(ma_custom_backend_vtable[0]);

        //error in ma_decoder_init_from_vtable__internal?

        // Initialize the decoder using our custom VFS.
        result = ma_decoder_init_vfs(&ma_custom_vfs_callbacks, Template->Get_FileName().c_str(), &decoderConfig, Decoder);
        if (result != MA_SUCCESS) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Load - ma_decoder_init_vfs failed (%s)!\n", ma_result_description(result));
            Free();
            return false;
        }

        ASSERT_FATAL(Decoder != nullptr);

        DecoderInitialized = true; // Handy flag to ensure it was initialized correctly.

        // Attach the decoder to the sound system as a streaming data source.
        result = ma_sound_init_from_data_source(AudioManager.Engine, Decoder, flags, AudioManager.SoundGroups[Template->Get_Group()], Sound);
        if (result != MA_SUCCESS) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Load - ma_sound_init_from_data_source failed (%s)!\n", ma_result_description(result));
            Free();
            return false;
        }

        DecoderIsOwnedBySound = false;

    } else {

        // Not an AUD file � fall back to standard decoder set (MP3, WAV, OGG, etc.).
        result = ma_sound_init_from_file(AudioManager.Engine, Template->Get_FileName().c_str(), flags, AudioManager.SoundGroups[Template->Get_Group()], nullptr, Sound);
        if (result != MA_SUCCESS) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Load - ma_sound_init_from_file failed (%s)!\n", ma_result_description(result));
            Free();
            return false;
        }

    }

    ma_sound_set_spatialization_enabled(Sound, MA_FALSE);

    IsLoaded = true;

    return true;
}


/**
 *  Uninitializes and releases the sound and decoder resources.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Free()
{
    bool freed = false;

    if (Sound) {
        ma_sound_uninit(Sound);
        delete Sound;
        Sound = nullptr;
        freed = true;
    }

    /*
     *  Custom decoders passed to ma_sound_init_from_data_source() remain
     *  caller-owned, so we must tear them down after the sound is detached.
     */
    if (Decoder) {
        if (DecoderInitialized) {
            ma_decoder_uninit(Decoder);
        }
        delete Decoder;
        Decoder = nullptr;
        DecoderInitialized = false;
        DecoderIsOwnedBySound = false;
        freed = true;
    }

    IsLoaded = false;

    return freed;
}


/**
 *  Checks whether this instance has a valid template and an active data source.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Is_Valid() const
{
    if (Template == nullptr) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Is_Valid - Sample template is null!\n");
        return false;
    }

    if (!Sound) {
        //AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Is_Valid - Sound is null!\n");  // Not really an error when making query.
        return false;
    }

    return Sound->pDataSource;
}

/**
 *  Starts playback of this audio instance.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Play()
{
    // Sanity check: Decoder should not be alive unless Sound was successfully created from it.
    ASSERT(!(Sound == nullptr && Decoder != nullptr));

    ASSERT(Sound != nullptr);

    ma_result result = ma_sound_start(Sound);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Play - ma_sound_start failed (%s)!\n", ma_result_description(result));
        return false;
    }

    CurrentState = AudioHandleState::AUDIO_STATE_PLAYING;

    return true;
}


/**
 *  Stops playback and resets the sound position to the beginning.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::End()
{
    ma_result result;

    if (!Sound) {
        //AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::End - Sound is null!\n"); // Not really an error when stopping.
        return false;
    }

    result = ma_sound_stop(Sound);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::End - ma_sound_stop failed (%s)!\n", ma_result_description(result));
        return false;
    }

    result = ma_sound_seek_to_pcm_frame(Sound, 0);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::End - ma_sound_seek_to_pcm_frame failed (%s)!\n", ma_result_description(result));
    }

    CurrentState = AudioHandleState::AUDIO_STATE_FINISHED;

    return true;
}


/**
 *  Pauses a currently playing sound.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Pause()
{
    ma_result result;

    ASSERT(Sound != nullptr);

    if (CurrentState != AudioHandleState::AUDIO_STATE_PLAYING) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Pause - Attmpted to pause a sound that is not playing!\n");
        return false;
    }

    result = ma_sound_stop(Sound);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Pause - ma_sound_stop failed (%s)!\n", ma_result_description(result));
        return false;
    }

    CurrentState = AudioHandleState::AUDIO_STATE_PAUSED;
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Pause - State changed: PLAYING -> PAUSED\n");

    return true;
}


/**
 *  Resumes playback of a paused sound.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Resume()
{
    ASSERT(Sound != nullptr);

    if (CurrentState != AudioHandleState::AUDIO_STATE_PAUSED) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Resume - Attmpted to resume a sound that is not paused!\n");
        return false;
    }

    // As long as we didn't call ma_sound_uninit on this instance, it will resume where we last paused it.
    ma_result result = ma_sound_start(Sound);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Resume - ma_sound_start failed (%s)!\n", ma_result_description(result));
        return false;
    }

    CurrentState = AudioHandleState::AUDIO_STATE_PLAYING;
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Resume - State changed: PAUSED -> PLAYING\n");

    return true;
}


/**
 *  Seeks the sound back to the beginning without stopping playback.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Restart()
{
    ASSERT(Sound != nullptr);

    ma_result result = ma_sound_seek_to_pcm_frame(Sound, 0);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Restart - ma_sound_seek_to_pcm_frame failed (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Returns whether this sound is currently playing.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Is_Playing() const
{
    ASSERT(Sound != nullptr);

    ma_bool32 result = ma_sound_is_playing(Sound);

    return (CurrentState == AudioHandleState::AUDIO_STATE_PLAYING && result == MA_TRUE);
}


/**
 *  Returns whether this sound is currently fading out.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Is_Fading_Out() const
{
    ma_bool32 result = ma_sound_is_playing(Sound);
    if (result == MA_FALSE) {
        return false;
    }

    return CurrentState == AudioHandleState::AUDIO_STATE_FADING_OUT;
}


/**
 *  Returns whether this sound is currently fading in.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Is_Fading_In() const
{
    ma_bool32 result = ma_sound_is_playing(Sound);
    if (result == MA_FALSE) {
        return false;
    }

    return CurrentState == AudioHandleState::AUDIO_STATE_FADING_IN;
}


/**
 *  Returns whether this sound has finished playback and reached the end.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Is_Finished() const
{
    return CurrentState == AudioHandleState::AUDIO_STATE_FINISHED;
}


#if 0
// A more passive version of checking if it has ended.
bool AudioInstanceClass::Has_Ended() const
{
    ASSERT(!(Sound == nullptr && Decoder != nullptr));
    return !Is_Playing() && Is_Finished();
}
#endif


/**
 *  Initiates a volume fade over the specified duration.
 *
 *  NOTE: If 'out' is false, fade in assumed!
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Set_Fade(float seconds, bool out, bool end_after_out)
{
    ASSERT(Sound != nullptr);

    FadeStartVolume = Get_Volume();
    FadeTime = seconds;
    FadeDuration = seconds;

    if (out) {
        CurrentState = AudioHandleState::AUDIO_STATE_FADING_OUT;
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Set_Fade - State changed: FADING_OUT\n");
    } else {
        CurrentState = AudioHandleState::AUDIO_STATE_FADING_IN;
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Set_Fade - State changed: FADING_IN\n");
    }

    //ma_sound_set_fade_in_milliseconds(Sound, Get_Volume(), 0.0f, 1000 * seconds);

    return true;
}


/**
 *  Returns whether this sound is set to loop.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Is_Looping() const
{
    ASSERT(Sound != nullptr);

    ma_bool32 result = ma_sound_is_looping(Sound);

    return IsLooping || result == MA_TRUE;
}


/**
 *  Returns the current volume of this sound.
 *
 *  @author: CCHyper
 */
float AudioInstanceClass::Get_Volume() const
{
    ma_float result;

    ASSERT(Sound != nullptr);

    result = ma_sound_get_volume(Sound);

    return result;
}


/**
 *  Returns the current pitch of this sound.
 *
 *  @author: CCHyper
 */
float AudioInstanceClass::Get_Pitch() const
{
    ma_float result;

    ASSERT(Sound != nullptr);

    result = ma_sound_get_pitch(Sound);

    return result;
}


/**
 *  Returns the current stereo pan of this sound.
 *
 *  @author: CCHyper
 */
float AudioInstanceClass::Get_Pan() const
{
    ma_float result;

    ASSERT(Sound != nullptr);

    result = ma_sound_get_pan(Sound);

    return result;
}


/**
 *  Returns the current playback cursor position in seconds.
 *
 *  @author: CCHyper
 */
float AudioInstanceClass::Get_Time() const
{
    ma_float time = 0.0f;

    ASSERT(Sound != nullptr);

    ma_sound_get_cursor_in_seconds(Sound, &time);

    return time;
}


/**
 *  Returns the total length of this sound in seconds.
 *
 *  #NOTE: ma_sound_get_length_in_seconds will return zero for Vorbis sounds!
 *
 *  @author: CCHyper
 */
float AudioInstanceClass::Get_Length() const
{
    ma_float time = 0.0f;

    ASSERT(Sound != nullptr);

    ma_sound_get_length_in_seconds(Sound, &time);

    return time;
}


/**
 *  Returns the sample rate of the underlying data source.
 *
 *  @author: CCHyper
 */
unsigned AudioInstanceClass::Get_Sample_Rate() const
{
    ASSERT(Sound != nullptr);

    ma_uint32 sampleRate;

    ma_result result = ma_data_source_get_data_format(Sound->pDataSource, nullptr, nullptr, &sampleRate, nullptr, 0);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Get_Sample_Rate - ma_data_source_get_data_format failed (%s)!\n", ma_result_description(result));
        return 0;
    }

    return sampleRate;
}


/**
 *  Returns the channel count of the underlying data source.
 *
 *  @author: CCHyper
 */
unsigned AudioInstanceClass::Get_Channels() const
{
    ASSERT(Sound != nullptr);

    ma_uint32 channels;

    ma_result result = ma_data_source_get_data_format(Sound->pDataSource, nullptr, &channels, nullptr, nullptr, 0);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Get_Channels - ma_data_source_get_data_format failed (%s)!\n", ma_result_description(result));
        return 0;
    }

    return channels;
}


/**
 *  Sets whether this sound should loop.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Set_Looping(bool loop)
{
    ASSERT(Sound != nullptr);

    IsLooping = loop;
    ma_sound_set_looping(Sound, loop);

    return true;
}


/**
 *  Sets the total number of plays for a loop-enabled request.
 *
 *  @author: ZivDero
 */
bool AudioInstanceClass::Set_Loop_Limit(int total_plays)
{
    RemainingLoopRepeats = std::max(total_plays - 1, 0);
    return true;
}


/**
 *  Sets the volume of this sound, clamped to [0.0, 2.0].
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Set_Volume(float volume)
{
    ASSERT(Sound != nullptr);

    // Clamp volume
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 2.0f) volume = 2.0f;

    ma_sound_set_volume(Sound, volume);

    return true;
}


/**
 *  Sets the pitch of this sound, clamped to [0.1, 2.0].
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Set_Pitch(float pitch)
{
    ASSERT(Sound != nullptr);

    // Clamp pitch
    if (pitch < 0.1f) pitch = 0.1f;
    if (pitch > 2.0f) pitch = 2.0f;

    ma_sound_set_pitch(Sound, pitch);

    return true;
}


/**
 *  Sets the stereo pan of this sound, clamped to [-1.0, 1.0].
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Set_Pan(float pan)
{
    ASSERT(Sound != nullptr);

    // Clamp pan
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;

    ma_sound_set_pan(Sound, pan);

    return true;
}


/**
 *  Seeks the playback position to the specified time in seconds.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Set_Time(float time_in_seconds)
{
    ma_result result;
    ma_uint64 lengthInPCMFrames;
    ma_uint32 sampleRate;

    ASSERT(Sound != nullptr);

    if (Sound->pDataSource == nullptr) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Set_Time - pDataSource is null!\n");
        return false;
    }

    float timePCM = 0;

    result = ma_data_source_get_data_format(Sound->pDataSource, nullptr, nullptr, &sampleRate, nullptr, 0);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Set_Time - ma_data_source_get_data_format failed (%s)!\n", ma_result_description(result));
        return false;
    }

    lengthInPCMFrames = time_in_seconds * sampleRate;

    result = ma_sound_seek_to_pcm_frame(Sound, lengthInPCMFrames);
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Set_Time - ma_sound_seek_to_pcm_frame failed (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Schedules the sound to start playing after the specified delay in seconds.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Set_Delay(float delay_in_seconds)
{
    if (delay_in_seconds <= 0.0f) {
        return true;
    }

    ma_uint64 startTime = static_cast<ma_uint64>(delay_in_seconds * 1000.0f) + ma_engine_get_time_in_milliseconds(AudioManager.Engine);
    ma_sound_set_start_time_in_milliseconds(Sound, startTime);

    DelayedStartTimeMs = startTime;
    IsDelaySet = true;

    return true;
}


/**
 *  Seeks to the specified position in seconds by converting to PCM frames.
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Seek(float time)
{
    ma_result result = ma_sound_seek_to_pcm_frame(Sound, time * ma_engine_get_sample_rate(ma_sound_get_engine(Sound)));
    if (result != MA_SUCCESS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Seek - ma_sound_seek_to_pcm_frame failed (%s)!\n", ma_result_description(result));
        return false;
    }

    return true;
}


/**
 *  Main per tick update function!
 *
 *  @author: CCHyper
 */
bool AudioInstanceClass::Update(float deltaTime)
{
    ASSERT_FATAL(Sound != nullptr, "Sound object should not be NULL here!");

    switch (CurrentState)
    {

        case AudioHandleState::AUDIO_STATE_PLAYING:
        {
            /**
             *  While a start delay is pending, miniaudio reports the sound as
             *  not-yet-playing even though playback has been successfully requested.
             *  Guard against prematurely transitioning to FINISHED by checking
             *  whether the scheduled start time has actually elapsed.
             */
            if (IsDelaySet) {
                if (ma_engine_get_time_in_milliseconds(AudioManager.Engine) < DelayedStartTimeMs) {
                    break; // Still inside the delay window; keep waiting.
                }
                IsDelaySet = false; // Delay has elapsed; resume normal state tracking.
            }

            if (!Is_Playing()) {
                if (RemainingLoopRepeats > 0 && ma_sound_at_end(Sound) == MA_TRUE) {
                    if (!Restart() || !Play()) {
                        CurrentState = AudioHandleState::AUDIO_STATE_FINISHED;
                        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_INSTANCE, "AudioInstance::Update - Failed to restart finite loop for \"%s\"\n", Get_FileName().c_str());
                    } else {
                        --RemainingLoopRepeats;
                        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Update - Replaying \"%s\", repeats left: %d\n", Get_FileName().c_str(), RemainingLoopRepeats);
                    }
                } else {
                    CurrentState = AudioHandleState::AUDIO_STATE_FINISHED;
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Update - State changed: PLAYING > FINISHED\n");
                }
            }
            break;
        }

        case AudioHandleState::AUDIO_STATE_FADING_IN:
        {
            FadeTime -= deltaTime;

            if (FadeDuration > 0.0f) {
                float volume = std::clamp(1.0f - (FadeTime / FadeDuration), 0.0f, 1.0f);
                Set_Volume(volume * FadeStartVolume);
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Update - FadeIn: t=%.2f, vol=%.2f\n", FadeTime, volume);
            }

            if (FadeTime <= 0.0f || !Is_Fading_In()) {
                Set_Volume(FadeStartVolume);
                CurrentState = AudioHandleState::AUDIO_STATE_PLAYING;
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Update - State changed: FADINGIN > PLAYING\n");
            }

            break;
        }

        case AudioHandleState::AUDIO_STATE_FADING_OUT:
        {
            FadeTime -= deltaTime;

            if (FadeDuration > 0.0f) {
                float volume = std::clamp(FadeTime / FadeDuration, 0.0f, 1.0f);
                Set_Volume(volume * FadeStartVolume);
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Update - FadeOut: t=%.2f, vol=%.2f\n", FadeTime, volume);
            }

            if (FadeTime <= 0.0f || !Is_Fading_Out()) {
                CurrentState = AudioHandleState::AUDIO_STATE_FINISHED;
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_INSTANCE, "AudioInstance::Update - State changed: FADINGOUT > FINISHED\n");
                return false; // Signal to manager that we can delete this handle
            }

            break;
        }

        case AudioHandleState::AUDIO_STATE_PAUSED:
        {
            // Do nothing while paused
            break;
        }

        case AudioHandleState::AUDIO_STATE_FINISHED:
        {
            // Done, ready for cleaning up!
            return false;
        }

    };

    return (CurrentState != AudioHandleState::AUDIO_STATE_FINISHED); // Keep alive!
}
