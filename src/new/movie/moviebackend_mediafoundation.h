/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Media Foundation movie decoder backend.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "movieplayback_backend.h"

#include <memory>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

class CCFileClass;


class MediaFoundationMovieBackend final : public IMovieDecoderBackend
{
    public:
        MediaFoundationMovieBackend();
        ~MediaFoundationMovieBackend() override;

        bool Open(const char *filename) override;
        bool Pump(MovieDecodeOutput &output) override;
        void Pause() override;
        void Resume() override;
        void Stop() override;
        bool Is_Finished() const override;
        int Get_Video_Width() const override;
        int Get_Video_Height() const override;
        const char *Get_Name() const override;

    private:
        void Reset();
        bool Find_Stream_Index(const GUID &major_type, DWORD &stream_index) const;
        bool Configure_Video_Stream();
        bool Configure_Audio_Stream();
        bool Decode_Video_Sample(IMFSample *sample, LONGLONG timestamp, MovieDecodeOutput &output);
        bool Decode_Audio_Sample(IMFSample *sample, LONGLONG timestamp, MovieDecodeOutput &output);
        bool All_Streams_Finished() const;

    private:
        bool ComInitialized = false;
        bool MediaFoundationStarted = false;
        std::unique_ptr<CCFileClass> File;
        Microsoft::WRL::ComPtr<IMFByteStream> ByteStream;
        Microsoft::WRL::ComPtr<IMFSourceReader> Reader;
        bool Paused = false;
        bool Finished = false;
        bool HasVideoStream = false;
        bool HasAudioStream = false;
        bool VideoStreamFinished = false;
        bool AudioStreamFinished = false;
        DWORD VideoStreamIndex = DWORD(-1);
        DWORD AudioStreamIndex = DWORD(-1);
        UINT32 VideoWidth = 0;
        UINT32 VideoHeight = 0;
        LONG VideoStride = 0;
        UINT32 AudioRate = 0;
        UINT32 AudioChannels = 0;
        UINT32 AudioBits = 0;
        MovieSampleFormat AudioFormat = MOVIE_SAMPLE_INVALID;
};


std::unique_ptr<IMovieDecoderBackend> Create_MediaFoundationMovieBackend();
