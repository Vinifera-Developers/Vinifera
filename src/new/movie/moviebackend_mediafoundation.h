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
        const char *GetName() const override;

    private:
        class Impl;
        Impl *Implementation;
};
