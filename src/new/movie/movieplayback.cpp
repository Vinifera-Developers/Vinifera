/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Backend-agnostic movie playback service.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "movieplayback.h"

#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "ccfile.h"
#include "debughandler.h"
#include "dsurface.h"
#include "iomap.h"
#include "moviebackend_mediafoundation.h"
#include "playmovie.h"
#include "sdl_functions.h"
#include "sdl_movie.h"
#include "session.h"
#include "theme.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "wwkeyboard.h"
#include "wwmouse.h"

#include <array>
#include <cctype>
#include <memory>
#include <mfapi.h>
#include <string>


std::string Normalize_Movie_Basename(const char *basename)
{
    if (!basename) {
        return {};
    }

    std::string normalized(basename);
    std::size_t extension = normalized.find_last_of('.');
    if (extension != std::string::npos) {
        normalized.erase(extension);
    }

    for (char &character : normalized) {
        character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }

    return normalized;
}


namespace
{
    struct MoviePlatformRuntime
    {
        ~MoviePlatformRuntime()
        {
            if (MediaFoundationStarted) {
                MFShutdown();
            }

            if (ComInitialized) {
                CoUninitialize();
            }
        }

        bool Initialize()
        {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr) || hr == S_FALSE) {
                ComInitialized = true;
            } else if (hr != RPC_E_CHANGED_MODE) {
                DEBUG_ERROR("Movie playback failed to initialize COM! Error code: 0x%08x.\n", hr);
                return false;
            }

            hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
            if (FAILED(hr)) {
                DEBUG_ERROR("Movie playback failed to initialize Media Foundation! Error code: 0x%08x.\n", hr);
                return false;
            }

            MediaFoundationStarted = true;
            return true;
        }

        bool ComInitialized = false;
        bool MediaFoundationStarted = false;
    };


    std::string Resolve_Movie_Filename(const char *basename)
    {
        static constexpr std::array<const char *, 4> Extensions = { ".MP4", ".WMV", ".MPG", ".AVI" };

        const std::string normalized = Normalize_Movie_Basename(basename);
        if (normalized.empty()) {
            return {};
        }

        for (const char *extension : Extensions) {
            std::string filename = normalized + extension;

            CCFileClass file(filename.c_str());
            if (file.Is_Available()) {
                return filename;
            }
        }

        return {};
    }


    bool Update_Playback_State(IMovieDecoderBackend &backend, bool &paused, Uint64 &pause_started_ms, Uint64 &paused_ms)
    {
        if (!VQA_Movie_Message_Loop()) {
            backend.Stop();
            return false;
        }

        if (!GameInFocus && !paused) {
            backend.Pause();
            paused = true;
            pause_started_ms = SDL_GetTicks();
        } else if (GameInFocus && paused) {
            backend.Resume();
            paused = false;
            paused_ms += SDL_GetTicks() - pause_started_ms;
            pause_started_ms = 0;
        }

        if (Keyboard->Check()) {
            if (Keyboard->Get() == (KN_RLSE_BIT | KN_ESC)) {
                DEBUG_INFO("%s: Breakout.\n", backend.GetName());
                backend.Stop();
                UpdateWindow(MainWindow);
                return false;
            }
        }

        return true;
    }


    bool Wait_Until_Timestamp(IMovieDecoderBackend &backend, std::int64_t timestamp_ms, bool &clock_started, std::int64_t &base_timestamp_ms, Uint64 &base_ticks_ms, bool &paused, Uint64 &pause_started_ms, Uint64 &paused_ms)
    {
        if (!clock_started) {
            clock_started = true;
            base_timestamp_ms = timestamp_ms;
            base_ticks_ms = SDL_GetTicks();
            return true;
        }

        const Uint64 target_ms = static_cast<Uint64>(std::max<std::int64_t>(0, timestamp_ms - base_timestamp_ms));

        while (true) {
            if (!Update_Playback_State(backend, paused, pause_started_ms, paused_ms)) {
                return false;
            }

            if (paused) {
                SDL_Delay(33);
                continue;
            }

            const Uint64 elapsed = SDL_GetTicks() - base_ticks_ms - paused_ms;
            if (elapsed >= target_ms) {
                return true;
            }

            const Uint64 remaining = target_ms - elapsed;
            SDL_Delay(static_cast<Uint32>(std::min<Uint64>(remaining, 2)));
        }
    }


    Rect Build_Destination_Rect(const MovieVideoFrame &frame, bool allow_stretch)
    {
        int area_width;
        int area_height;

        if (allow_stretch && Options.StretchMovies) {
            area_width = SDLWindowWidth;
            area_height = SDLWindowHeight;
        } else {
            area_width = 640;
            area_height = 400;
        }

        if (SDLWindow) {
            SDL_GetWindowSize(SDLWindow, &area_width, &area_height);
        }

        Rect destination(0, 0, frame.Width, frame.Height);

        if (allow_stretch && Options.StretchMovies) {
            Scale_Video_Rect(destination, area_width, area_height, true);
        } else {
            Scale_Video_Rect(destination, area_width, area_height, true);
            destination.X = std::max((area_width - frame.Width) / 2, 0);
            destination.Y = std::max((area_height - frame.Height) / 2, 0);
            destination.Width = frame.Width;
            destination.Height = frame.Height;
        }

        return destination;
    }


    void Clear_Movie_Screen()
    {
        HiddenSurface->Clear();
        Update_Visible_Surface();
        SDL_Update_Screen(HiddenSurface);
        InvalidateRect(MainWindow, nullptr, FALSE);
    }


    int Audio_Bytes_Per_Second(const MovieAudioChunk &chunk)
    {
        int bytes_per_sample = 0;

        switch (chunk.Format) {
            case MOVIE_SAMPLE_U8:
                bytes_per_sample = 1;
                break;

            case MOVIE_SAMPLE_S16:
                bytes_per_sample = 2;
                break;

            case MOVIE_SAMPLE_F32:
                bytes_per_sample = 4;
                break;

            default:
                break;
        }

        return chunk.SampleRate * chunk.Channels * bytes_per_sample;
    }


    bool Drain_Movie_Audio(IMovieDecoderBackend &backend, bool &paused, Uint64 &pause_started_ms, Uint64 &paused_ms)
    {
        if (!SDL_Movie_Has_Audio()) {
            return true;
        }

        SDL_Movie_Flush_Audio();

        while (SDL_Movie_Get_Queued_Audio_Size() > 0) {
            if (!Update_Playback_State(backend, paused, pause_started_ms, paused_ms)) {
                return false;
            }

            if (paused) {
                SDL_Delay(33);
                continue;
            }

            SDL_Delay(10);
        }

        return true;
    }


    std::unique_ptr<IMovieDecoderBackend> Open_Movie_Backend(const char *filename)
    {
        std::unique_ptr<IMovieDecoderBackend> backend = Create_MediaFoundationMovieBackend();
        if (backend && backend->Open(filename)) {
            return backend;
        }

        return nullptr;
    }
}


bool MoviePlayback_Is_Available(const char *basename)
{
    return !Resolve_Movie_Filename(basename).empty();
}


bool MoviePlayback_Play(const char *basename, ThemeType theme, bool clear_before, bool stretch_allowed, bool clear_after)
{
    const std::string filename = Resolve_Movie_Filename(basename);
    if (filename.empty()) {
        return false;
    }

    MoviePlatformRuntime runtime;
    if (!runtime.Initialize()) {
        return false;
    }

    std::unique_ptr<IMovieDecoderBackend> backend = Open_Movie_Backend(filename.c_str());
    if (!backend) {
        DEBUG_WARNING("Failed to open \"%s\" with any modern movie backend.\n", filename.c_str());
        return false;
    }

    DEBUG_INFO("Play_Movie \"%s\" with %s.\n", filename.c_str(), backend->GetName());

    Keyboard->Clear();
    MouseCursor->Hide_Mouse();
    Vinifera_ModernMoviePlaying = true;

    if (theme != THEME_NONE) {
        Theme.Queue_Song(theme);
    }

    if (clear_before) {
        Clear_Movie_Screen();
    }

    bool paused = false;
    bool clock_started = false;
    bool aborted = false;
    std::int64_t base_timestamp_ms = 0;
    Uint64 base_ticks_ms = 0;
    Uint64 pause_started_ms = 0;
    Uint64 paused_ms = 0;
    bool success = true;

    while (!backend->IsFinished()) {
        if (!Update_Playback_State(*backend, paused, pause_started_ms, paused_ms)) {
            aborted = true;
            break;
        }

        if (paused) {
            SDL_Delay(33);
            continue;
        }

        MovieDecodeOutput output;
        if (!backend->Pump(output)) {
            success = false;
            break;
        }

        if (output.EndOfStream) {
            break;
        }

        if (output.HasAudioChunk) {
            if (!clock_started) {
                clock_started = true;
                base_timestamp_ms = output.AudioChunk.TimestampMs;
                base_ticks_ms = SDL_GetTicks();
            }

            if (!SDL_Movie_Queue_Audio(
                    output.AudioChunk.Samples.data(),
                    static_cast<int>(output.AudioChunk.Samples.size()),
                    output.AudioChunk.SampleRate,
                    output.AudioChunk.Channels,
                    output.AudioChunk.Format)) {
                success = false;
                break;
            }
        }

        if (output.HasVideoFrame) {
            if (!Wait_Until_Timestamp(*backend, output.VideoFrame.TimestampMs, clock_started, base_timestamp_ms, base_ticks_ms, paused, pause_started_ms, paused_ms)) {
                aborted = true;
                break;
            }

            const Rect destination = Build_Destination_Rect(output.VideoFrame, stretch_allowed);
            if (!SDL_Movie_Present_Frame(output.VideoFrame, destination)) {
                success = false;
                break;
            }

        }

        if (output.HasAudioChunk) {
            const int bytes_per_second = Audio_Bytes_Per_Second(output.AudioChunk);
            const int max_buffer = bytes_per_second > 0 ? bytes_per_second / 2 : 0;

            while (max_buffer > 0 && SDL_Movie_Get_Queued_Audio_Size() > max_buffer) {
                if (!Update_Playback_State(*backend, paused, pause_started_ms, paused_ms)) {
                    aborted = true;
                    break;
                }

                if (paused) {
                    SDL_Delay(33);
                } else {
                    SDL_Delay(5);
                }
            }

            if (aborted) {
                break;
            }
        }

        if (!output.HasAudioChunk && !output.HasVideoFrame) {
            SDL_Delay(1);
        }
    }

    if (success && !aborted) {
        Drain_Movie_Audio(*backend, paused, pause_started_ms, paused_ms);
    }

    if (clear_after) {
        Clear_Movie_Screen();
    }

    SDL_Movie_Shutdown();
    Vinifera_ModernMoviePlaying = false;

    MouseCursor->Show_Mouse();
    Keyboard->Clear();
    Map.Flag_To_Redraw(2);

    return success;
}
