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


enum MovieSampleFormat
{
    MOVIE_SAMPLE_INVALID = -1,
    MOVIE_SAMPLE_U8,
    MOVIE_SAMPLE_S16,
    MOVIE_SAMPLE_F32,
};


enum MovieVideoPixelFormat
{
    MOVIE_VIDEO_INVALID = -1,
    MOVIE_VIDEO_NV12,
};


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


struct MovieAudioChunk
{
    std::int64_t TimestampMs = 0;
    int SampleRate = 0;
    int Channels = 0;
    MovieSampleFormat Format = MOVIE_SAMPLE_INVALID;
    std::vector<std::uint8_t> Samples;
};


struct MovieDecodeOutput
{
    bool HasVideoFrame = false;
    bool HasAudioChunk = false;
    bool EndOfStream = false;
    MovieVideoFrame VideoFrame;
    MovieAudioChunk AudioChunk;

    void Reset()
    {
        HasVideoFrame = false;
        HasAudioChunk = false;
        EndOfStream = false;
        VideoFrame.TimestampMs = 0;
        VideoFrame.Width = 0;
        VideoFrame.Height = 0;
        VideoFrame.Format = MOVIE_VIDEO_INVALID;
        VideoFrame.Pitch = 0;
        VideoFrame.SecondaryPitch = 0;
        VideoFrame.Pixels.clear();
        VideoFrame.SecondaryPixels.clear();
        AudioChunk.TimestampMs = 0;
        AudioChunk.SampleRate = 0;
        AudioChunk.Channels = 0;
        AudioChunk.Format = MOVIE_SAMPLE_INVALID;
        AudioChunk.Samples.clear();
    }
};


class IMovieDecoderBackend
{
    public:
        virtual ~IMovieDecoderBackend() = default;

        virtual bool Open(const char *filename) = 0;
        virtual bool Pump(MovieDecodeOutput &output) = 0;
        virtual void Pause() = 0;
        virtual void Resume() = 0;
        virtual void Stop() = 0;
        virtual bool IsFinished() const = 0;
        virtual const char *GetName() const = 0;
};


std::unique_ptr<IMovieDecoderBackend> Create_MediaFoundationMovieBackend();
