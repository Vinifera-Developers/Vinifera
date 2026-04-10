/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SDL helpers for movie playback.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sdl_movie.h"

#include "SDL3/SDL_audio.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "debughandler.h"
#include "optionsext.h"
#include "vinifera_globals.h"


static SDL_Texture *MovieTexture = nullptr;
static int MovieTextureWidth = 0;
static int MovieTextureHeight = 0;
static MovieVideoPixelFormat MovieTextureFormat = MOVIE_VIDEO_INVALID;
static Rect MovieDestinationRect(0, 0, 0, 0);
static bool MovieHasFrame = false;
static SDL_AudioStream *MovieAudioStream = nullptr;
static int MovieAudioRate = 0;
static int MovieAudioChannels = 0;
static MovieSampleFormat MovieAudioFormat = MOVIE_SAMPLE_INVALID;


static SDL_AudioFormat Movie_To_SDL_Audio_Format(MovieSampleFormat format)
{
    switch (format) {
        case MOVIE_SAMPLE_U8:
            return SDL_AUDIO_U8;

        case MOVIE_SAMPLE_S16:
            return SDL_AUDIO_S16LE;

        case MOVIE_SAMPLE_F32:
            return SDL_AUDIO_F32LE;

        default:
            return SDL_AUDIO_UNKNOWN;
    }
}


static SDL_PixelFormat Movie_To_SDL_Texture_Format(MovieVideoPixelFormat format)
{
    switch (format) {
        case MOVIE_VIDEO_NV12:
            return SDL_PIXELFORMAT_NV12;

        default:
            return SDL_PIXELFORMAT_UNKNOWN;
    }
}


static bool Ensure_Movie_Texture(int width, int height, MovieVideoPixelFormat format)
{
    if (!SDLWindowRenderer) {
        DEBUG_ERROR("SDL movie renderer unavailable.\n");
        return false;
    }

    if (MovieTexture && MovieTextureWidth == width && MovieTextureHeight == height && MovieTextureFormat == format) {
        return true;
    }

    const SDL_PixelFormat texture_format = Movie_To_SDL_Texture_Format(format);
    if (texture_format == SDL_PIXELFORMAT_UNKNOWN) {
        DEBUG_ERROR("Unsupported SDL movie texture format.\n");
        return false;
    }

    SDL_DestroyTexture(MovieTexture);
    MovieTexture = SDL_CreateTexture(SDLWindowRenderer, texture_format, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!MovieTexture) {
        DEBUG_ERROR("Failed to create SDL movie texture! SDL Error: %s\n", SDL_GetError());
        MovieTextureWidth = 0;
        MovieTextureHeight = 0;
        MovieTextureFormat = MOVIE_VIDEO_INVALID;
        return false;
    }

    MovieTextureWidth = width;
    MovieTextureHeight = height;
    MovieTextureFormat = format;

    if (OptionsExtension->ScaleMode != SDL_SCALEMODE_INVALID) {
        SDL_SetTextureScaleMode(MovieTexture, OptionsExtension->ScaleMode);
    }

    return true;
}


static bool Ensure_Movie_Audio_Stream(int sample_rate, int channels, MovieSampleFormat format)
{
    if (MovieAudioStream
     && MovieAudioRate == sample_rate
     && MovieAudioChannels == channels
     && MovieAudioFormat == format) {
        return true;
    }

    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) && !SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        DEBUG_ERROR("Failed to initialize SDL audio for movies! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_DestroyAudioStream(MovieAudioStream);
    MovieAudioStream = nullptr;

    SDL_AudioSpec spec = {};
    spec.freq = sample_rate;
    spec.channels = channels;
    spec.format = Movie_To_SDL_Audio_Format(format);

    if (spec.format == SDL_AUDIO_UNKNOWN) {
        DEBUG_ERROR("Unsupported SDL movie audio format.\n");
        return false;
    }

    MovieAudioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!MovieAudioStream) {
        DEBUG_ERROR("Failed to create SDL movie audio stream! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(MovieAudioStream)) {
        DEBUG_ERROR("Failed to resume SDL movie audio stream! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyAudioStream(MovieAudioStream);
        MovieAudioStream = nullptr;
        return false;
    }

    MovieAudioRate = sample_rate;
    MovieAudioChannels = channels;
    MovieAudioFormat = format;

    return true;
}


bool SDL_Movie_Present_Frame(const MovieVideoFrame &frame, const Rect &destination_rect)
{
    if (!Ensure_Movie_Texture(frame.Width, frame.Height, frame.Format)) {
        return false;
    }

    const bool updated = SDL_UpdateNVTexture(
        MovieTexture,
        nullptr,
        frame.Pixels.data(),
        frame.Pitch,
        frame.SecondaryPixels.data(),
        frame.SecondaryPitch);

    if (!updated) {
        DEBUG_ERROR("Failed to update SDL movie texture! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(SDLWindowRenderer, 0, 0, 0, 255);
    SDL_RenderClear(SDLWindowRenderer);

    SDL_FRect dst_rect = {
        static_cast<float>(destination_rect.X),
        static_cast<float>(destination_rect.Y),
        static_cast<float>(destination_rect.Width),
        static_cast<float>(destination_rect.Height)
    };

    if (!SDL_RenderTexture(SDLWindowRenderer, MovieTexture, nullptr, &dst_rect)) {
        DEBUG_ERROR("Failed to render SDL movie texture! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    MovieDestinationRect = destination_rect;
    MovieHasFrame = true;
    SDL_RenderPresent(SDLWindowRenderer);
    return true;
}


bool SDL_Movie_Repaint()
{
    if (!SDLWindowRenderer) {
        return false;
    }

    SDL_SetRenderDrawColor(SDLWindowRenderer, 0, 0, 0, 255);
    SDL_RenderClear(SDLWindowRenderer);

    if (MovieTexture && MovieHasFrame && MovieDestinationRect.Is_Valid()) {
        SDL_FRect dst_rect = {
            static_cast<float>(MovieDestinationRect.X),
            static_cast<float>(MovieDestinationRect.Y),
            static_cast<float>(MovieDestinationRect.Width),
            static_cast<float>(MovieDestinationRect.Height)
        };

        if (!SDL_RenderTexture(SDLWindowRenderer, MovieTexture, nullptr, &dst_rect)) {
            DEBUG_ERROR("Failed to repaint SDL movie texture! SDL Error: %s\n", SDL_GetError());
            return false;
        }
    }

    SDL_RenderPresent(SDLWindowRenderer);
    return true;
}


bool SDL_Movie_Queue_Audio(const void *data, int data_length, int sample_rate, int channels, MovieSampleFormat format)
{
    if (!data || data_length <= 0) {
        return true;
    }

    if (!Ensure_Movie_Audio_Stream(sample_rate, channels, format)) {
        return false;
    }

    if (!SDL_PutAudioStreamData(MovieAudioStream, data, data_length)) {
        DEBUG_ERROR("Failed to queue SDL movie audio! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}


void SDL_Movie_Flush_Audio()
{
    if (MovieAudioStream) {
        SDL_FlushAudioStream(MovieAudioStream);
    }
}


int SDL_Movie_Get_Queued_Audio_Size()
{
    if (!MovieAudioStream) {
        return 0;
    }

    return SDL_GetAudioStreamQueued(MovieAudioStream);
}


bool SDL_Movie_Has_Audio()
{
    return MovieAudioStream != nullptr;
}


void SDL_Movie_Shutdown()
{
    SDL_DestroyTexture(MovieTexture);
    MovieTexture = nullptr;
    MovieTextureWidth = 0;
    MovieTextureHeight = 0;
    MovieTextureFormat = MOVIE_VIDEO_INVALID;
    MovieDestinationRect = Rect(0, 0, 0, 0);
    MovieHasFrame = false;

    SDL_DestroyAudioStream(MovieAudioStream);
    MovieAudioStream = nullptr;
    MovieAudioRate = 0;
    MovieAudioChannels = 0;
    MovieAudioFormat = MOVIE_SAMPLE_INVALID;
}
