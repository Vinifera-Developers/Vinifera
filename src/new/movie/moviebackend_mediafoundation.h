/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Declarations for the Media Foundation movie decoder backend.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "movieplayback_backend.h"


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
        bool IsFinished() const override;
        int GetVideoWidth() const override;
        int GetVideoHeight() const override;
        const char *GetName() const override;

    private:
        class Impl;
        Impl *Implementation;
};
