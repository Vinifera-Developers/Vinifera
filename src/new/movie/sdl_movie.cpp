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

#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "debughandler.h"
#include "options.h"
#include "optionsext.h"
#include "sdl_functions.h"
#include "vinifera_globals.h"
#include "vinifera_imgui.h"


static SDL_Texture *MovieTexture = nullptr;
static int MovieTextureWidth = 0;
static int MovieTextureHeight = 0;
static MovieVideoPixelFormat MovieTextureFormat = MOVIE_VIDEO_INVALID;
static Rect MovieDestinationRect(0, 0, 0, 0);
static bool MovieHasFrame = false;


static SDL_PixelFormat Movie_To_SDL_Texture_Format(MovieVideoPixelFormat format)
{
    switch (format) {
        case MOVIE_VIDEO_NV12:
            return SDL_PIXELFORMAT_NV12;

        default:
            return SDL_PIXELFORMAT_UNKNOWN;
    }
}


/**
 *  Creates or reuses the streaming SDL texture for video output.
 *  Recreates whenever the frame dimensions or pixel format change.
 */
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
        DEBUG_ERROR("Failed to create SDL movie texture! SDL Error: {}\n", SDL_GetError());
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


/**
 *  Uploads the NV12 frame to the movie texture, renders it at
 *  destination_rect and presents the renderer.
 */
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
        DEBUG_ERROR("Failed to update SDL movie texture! SDL Error: {}\n", SDL_GetError());
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
        DEBUG_ERROR("Failed to render SDL movie texture! SDL Error: {}\n", SDL_GetError());
        return false;
    }

    MovieDestinationRect = destination_rect;
    MovieHasFrame = true;
    ViniferaImGui::Render_Movie_Overlay();
    SDL_RenderPresent(SDLWindowRenderer);
    return true;
}


/**
 *  Re-renders the last presented frame without decoding a new one.
 *  Used to restore the movie image after a window repaint event.
 */
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
            DEBUG_ERROR("Failed to repaint SDL movie texture! SDL Error: {}\n", SDL_GetError());
            return false;
        }
    }

    ViniferaImGui::Render_Movie_Overlay();
    SDL_RenderPresent(SDLWindowRenderer);
    return true;
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
}
