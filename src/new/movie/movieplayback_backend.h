/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Shared movie playback backend interfaces and data types.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "always.h"

#include <cstdint>
#include <memory>
#include <vector>


/**
 *  PCM sample formats supported by the movie audio pipeline.
 */
enum MovieSampleFormat
{
    MOVIE_SAMPLE_INVALID = -1,
    MOVIE_SAMPLE_U8,
    MOVIE_SAMPLE_S16,
    MOVIE_SAMPLE_F32,
};


/**
 *  Pixel formats supported by the movie video pipeline.
 */
enum MovieVideoPixelFormat
{
    MOVIE_VIDEO_INVALID = -1,
    MOVIE_VIDEO_NV12,
};


/**
 *  A single decoded video frame with timestamp and plane data. NV12
 *  frames carry luma (Y) in Pixels and interleaved chroma (UV) in
 *  SecondaryPixels.
 */
struct MovieVideoFrame
{
    std::int64_t TimestampMs = 0;
    int Width = 0;
    int Height = 0;
    MovieVideoPixelFormat Format = MOVIE_VIDEO_INVALID;
    int Pitch = 0;
    int SecondaryPitch = 0;
    std::vector<std::uint8_t> Pixels;
    std::vector<std::uint8_t> SecondaryPixels;
};


/**
 *  A decoded audio chunk with timestamp and raw PCM sample data.
 */
struct MovieAudioChunk
{
    std::int64_t TimestampMs = 0;
    int SampleRate = 0;
    int Channels = 0;
    MovieSampleFormat Format = MOVIE_SAMPLE_INVALID;
    std::vector<std::uint8_t> Samples;
};


/**
 *  Output produced by a single IMovieDecoderBackend::Pump call.
 *  At most one video frame and one audio chunk are returned per call.
 */
struct MovieDecodeOutput
{
    bool HasVideoFrame = false;
    bool HasAudioChunk = false;
    bool EndOfStream = false;
    MovieVideoFrame VideoFrame;
    MovieAudioChunk AudioChunk;

    void Reset()
    {
        *this = MovieDecodeOutput{};
    }
};


/**
 *  Abstract interface for movie decoder backends. An implementation opens
 *  a file and decodes it into video frames and audio chunks one pump
 *  at a time.
 */
class IMovieDecoderBackend
{
public:
    virtual ~IMovieDecoderBackend() = default;

    virtual bool Open(const char* filename) = 0;
    virtual bool Pump(MovieDecodeOutput& output) = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Stop() = 0;
    virtual bool Is_Finished() const = 0;
    virtual int Get_Video_Width() const = 0;
    virtual int Get_Video_Height() const = 0;
    virtual const char* Get_Name() const = 0;
};
