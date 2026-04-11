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
#include "sdlsurface.h"
#include "iomap.h"
#include "movie.h"
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

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <memory>
#include <mfapi.h>
#include <string>
#include <vector>


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
    struct ModernIngameDummyVQA
    {
        std::uint8_t Padding[0x544] = {};
        bool IsPaused = false;
        bool field_545 = false;
    };


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
        int area_width = SDLWindowWidth;
        int area_height = SDLWindowHeight;

        if (SDLWindow) {
            SDL_GetWindowSize(SDLWindow, &area_width, &area_height);
        }

        Rect destination(0, 0, frame.Width, frame.Height);

        if (allow_stretch && Options.StretchMovies) {
            Scale_Video_Rect(destination, area_width, area_height, true);
        } else {
            Scale_Video_Rect(destination, static_cast<int>(640.0f * (1.0 / SDL_XScale())), static_cast<int>(400.0f * (1.0 / SDL_YScale())), true);

            if (destination.Width > frame.Width || destination.Height > frame.Height) {
                destination.Width = frame.Width;
                destination.Height = frame.Height;
            }

            destination.X = (area_width - destination.Width) / 2;
            destination.Y = (area_height - destination.Height) / 2;
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


    struct IngameMoviePlaybackState
    {
        ~IngameMoviePlaybackState()
        {
            SDL_DestroySurface(CachedNV12Surface);
            SDL_DestroySurface(CachedRGB565Surface);
        }

        VQHandle *Handle = nullptr;
        MoviePlatformRuntime Runtime;
        std::unique_ptr<IMovieDecoderBackend> Backend;
        std::unique_ptr<ModernIngameDummyVQA> DummyVQA;
        Rect DestinationRect = Rect(0, 0, 0, 0);
        bool ClockStarted = false;
        std::int64_t BaseTimestampMs = 0;
        Uint64 BaseTicksMs = 0;
        bool Paused = false;
        Uint64 PauseStartedMs = 0;
        Uint64 PausedMs = 0;
        bool EndOfStream = false;
        bool HasPendingVideoFrame = false;
        MovieVideoFrame PendingVideoFrame;
        MovieDecodeOutput DecoderOutput;
        SDL_Surface *CachedNV12Surface = nullptr;
        SDL_Surface *CachedRGB565Surface = nullptr;
        int CachedFrameWidth = 0;
        int CachedFrameHeight = 0;
    };


    std::vector<std::unique_ptr<IngameMoviePlaybackState>> IngameMoviePlaybackStates;
    auto const Movie_Queue_Ingame_Function = reinterpret_cast<void (__fastcall *)(VQHandle *)>(0x00564630);


    static_assert(offsetof(VQHandle, field_45) == 0x45, "Unexpected VQHandle::field_45 offset.");
    static_assert(offsetof(ModernIngameDummyVQA, IsPaused) == 0x544, "Unexpected VQAClass::IsPaused offset.");


    IngameMoviePlaybackState *Find_Ingame_Movie_State(VQHandle *handle)
    {
        for (const auto &state : IngameMoviePlaybackStates) {
            if (state && state->Handle == handle) {
                return state.get();
            }
        }

        return nullptr;
    }


    void Remove_Ingame_Movie_State(VQHandle *handle)
    {
        IngameMoviePlaybackStates.erase(
            std::remove_if(
                IngameMoviePlaybackStates.begin(),
                IngameMoviePlaybackStates.end(),
                [handle](const std::unique_ptr<IngameMoviePlaybackState> &state) {
                    return !state || state->Handle == handle;
                }),
            IngameMoviePlaybackStates.end());
    }


    bool Present_Ingame_Frame(const MovieVideoFrame &frame, SDLSurface &surface, const Rect &destination_rect, IngameMoviePlaybackState &state)
    {
        if (frame.Format != MOVIE_VIDEO_NV12 || frame.Width <= 0 || frame.Height <= 0 || !destination_rect.Is_Valid()) {
            return false;
        }

        SDL_Surface *dst_sdl = surface.Get_SDL_Surface();
        if (!dst_sdl) {
            return false;
        }

        // Reallocate cached surfaces only when frame dimensions change.
        if (state.CachedFrameWidth != frame.Width || state.CachedFrameHeight != frame.Height) {
            SDL_DestroySurface(state.CachedNV12Surface);
            SDL_DestroySurface(state.CachedRGB565Surface);

            state.CachedNV12Surface = SDL_CreateSurface(frame.Width, frame.Height, SDL_PIXELFORMAT_NV12);
            state.CachedRGB565Surface = SDL_CreateSurface(frame.Width, frame.Height, SDL_PIXELFORMAT_RGB565);
            state.CachedFrameWidth = frame.Width;
            state.CachedFrameHeight = frame.Height;

            if (!state.CachedNV12Surface || !state.CachedRGB565Surface) {
                DEBUG_ERROR("Present_Ingame_Frame: Failed to allocate cached surfaces: %s\n", SDL_GetError());
                SDL_DestroySurface(state.CachedNV12Surface);
                SDL_DestroySurface(state.CachedRGB565Surface);
                state.CachedNV12Surface = nullptr;
                state.CachedRGB565Surface = nullptr;
                state.CachedFrameWidth = 0;
                state.CachedFrameHeight = 0;
                return false;
            }
        }

        // Copy Y and UV planes into the pre-allocated NV12 surface.
        SDL_Surface *nv12 = state.CachedNV12Surface;
        const int uv_rows = (frame.Height + 1) / 2;
        auto *buf = static_cast<std::uint8_t *>(nv12->pixels);

        for (int row = 0; row < frame.Height; ++row) {
            std::memcpy(buf + row * nv12->pitch,
                        frame.Pixels.data() + row * frame.Pitch,
                        frame.Width);
        }

        std::uint8_t *uv = buf + static_cast<std::size_t>(nv12->pitch) * frame.Height;
        for (int row = 0; row < uv_rows; ++row) {
            std::memcpy(uv + row * nv12->pitch,
                        frame.SecondaryPixels.data() + row * frame.SecondaryPitch,
                        frame.Width);
        }

        // Convert NV12 → RGB565 directly into the pre-allocated surface (no allocation).
        if (!SDL_ConvertPixels(
                frame.Width, frame.Height,
                SDL_PIXELFORMAT_NV12, nv12->pixels, nv12->pitch,
                SDL_PIXELFORMAT_RGB565, state.CachedRGB565Surface->pixels, state.CachedRGB565Surface->pitch)) {
            DEBUG_ERROR("Present_Ingame_Frame: SDL_ConvertPixels failed: %s\n", SDL_GetError());
            return false;
        }

        SDL_SetSurfaceBlendMode(state.CachedRGB565Surface, SDL_BLENDMODE_NONE);
        SDL_Rect dst_rect = { destination_rect.X, destination_rect.Y,
                              destination_rect.Width, destination_rect.Height };
        const bool ok = SDL_BlitSurfaceScaled(state.CachedRGB565Surface, nullptr, dst_sdl, &dst_rect, SDL_SCALEMODE_NEAREST);

        if (!ok) {
            DEBUG_ERROR("Present_Ingame_Frame: SDL_BlitSurfaceScaled failed: %s\n", SDL_GetError());
        }

        return ok;
    }


    bool Is_Video_Frame_Due(const IngameMoviePlaybackState &state, const MovieVideoFrame &frame)
    {
        if (!state.ClockStarted) {
            return true;
        }

        const Uint64 target_ms = static_cast<Uint64>(std::max<std::int64_t>(0, frame.TimestampMs - state.BaseTimestampMs));
        const Uint64 elapsed_ms = SDL_GetTicks() - state.BaseTicksMs - state.PausedMs;
        return elapsed_ms >= target_ms;
    }


    bool Can_Finish_Ingame_Movie() 
    {
        return SDL_Movie_Get_Queued_Audio_Size() <= 0;
    }


    bool Advance_Ingame_Movie(IngameMoviePlaybackState &state, bool &done)
    {
        done = false;

        if (!state.Backend || !state.Handle || !state.Handle->DrawSurface) {
            done = true;
            return false;
        }

        auto &sdl_surface = static_cast<SDLSurface &>(*state.Handle->DrawSurface);

        if (state.HasPendingVideoFrame) {
            if (!Is_Video_Frame_Due(state, state.PendingVideoFrame)) {
                return false;
            }

            if (!Present_Ingame_Frame(state.PendingVideoFrame, sdl_surface, state.DestinationRect, state)) {
                done = true;
                return false;
            }

            state.HasPendingVideoFrame = false;

            if (state.EndOfStream && Can_Finish_Ingame_Movie()) {
                done = true;
            }

            return true;
        }

        static constexpr int MaxPumpsPerTick = 8;
        int pump_count = 0;
        bool frame_presented = false;

        while (!state.EndOfStream && pump_count < MaxPumpsPerTick) {
            ++pump_count;

            if (!state.Backend->Pump(state.DecoderOutput)) {
                done = true;
                return frame_presented;
            }

            if (state.DecoderOutput.EndOfStream) {
                state.EndOfStream = true;
                break;
            }

            if (state.DecoderOutput.HasAudioChunk) {
                if (!state.ClockStarted) {
                    state.ClockStarted = true;
                    state.BaseTimestampMs = state.DecoderOutput.AudioChunk.TimestampMs;
                    state.BaseTicksMs = SDL_GetTicks();
                }

                if (!SDL_Movie_Queue_Audio(
                        state.DecoderOutput.AudioChunk.Samples.data(),
                        static_cast<int>(state.DecoderOutput.AudioChunk.Samples.size()),
                        state.DecoderOutput.AudioChunk.SampleRate,
                        state.DecoderOutput.AudioChunk.Channels,
                        state.DecoderOutput.AudioChunk.Format)) {
                    done = true;
                    return frame_presented;
                }
            }

            if (state.DecoderOutput.HasVideoFrame) {
                if (!state.ClockStarted) {
                    state.ClockStarted = true;
                    state.BaseTimestampMs = state.DecoderOutput.VideoFrame.TimestampMs;
                    state.BaseTicksMs = SDL_GetTicks();
                }

                std::swap(state.PendingVideoFrame, state.DecoderOutput.VideoFrame);
                state.HasPendingVideoFrame = true;

                if (!Is_Video_Frame_Due(state, state.PendingVideoFrame)) {
                    return frame_presented;
                }

                if (!Present_Ingame_Frame(state.PendingVideoFrame, sdl_surface, state.DestinationRect, state)) {
                    done = true;
                    return false;
                }

                state.HasPendingVideoFrame = false;
                frame_presented = true;
                continue;
            }

            if (!state.DecoderOutput.HasAudioChunk) {
                break;
            }
        }

        if (state.EndOfStream && Can_Finish_Ingame_Movie()) {
            done = true;
        }

        return frame_presented;
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


bool MoviePlayback_Play_Ingame(const char *basename)
{
    const std::string filename = Resolve_Movie_Filename(basename);
    if (filename.empty() || Session.Type != GAME_NORMAL || !SidebarSurface) {
        return false;
    }

    auto state = std::make_unique<IngameMoviePlaybackState>();
    if (!state->Runtime.Initialize()) {
        return false;
    }

    state->Backend = Open_Movie_Backend(filename.c_str());
    if (!state->Backend) {
        DEBUG_WARNING("Failed to open ingame movie \"%s\" with any modern movie backend.\n", filename.c_str());
        return false;
    }

    const int width = state->Backend->GetVideoWidth();
    const int height = state->Backend->GetVideoHeight();
    if (width <= 0 || height <= 0) {
        DEBUG_WARNING("Modern ingame movie \"%s\" reported invalid dimensions.\n", filename.c_str());
        return false;
    }

    state->DummyVQA = std::make_unique<ModernIngameDummyVQA>();
    state->Handle = new (std::nothrow) VQHandle();
    if (!state->Handle || !state->DummyVQA) {
        delete state->Handle;
        return false;
    }

    state->Handle->VQA = reinterpret_cast<VQAClass *>(state->DummyVQA.get());
    state->Handle->field_4 = -1;
    state->Handle->DrawSurface = SidebarSurface;
    state->Handle->field_C = 0;
    state->Handle->field_10 = 0;
    state->Handle->field_14 = 0;
    state->Handle->field_18 = 1;
    state->Handle->field_1C = 0;
    state->Handle->field_20 = 0;
    // Set InitialRect to the radar movie display size (140x110).
    // Movie_Queue_Ingame will overwrite X/Y with RadX+RadOffX, RadY+RadOffY
    // (SidebarSurface-local coordinates) while keeping Width/Height.
    state->Handle->InitialRect = Rect(0, 0, 140, 110);
    state->Handle->StretchRect = state->Handle->InitialRect;
    state->Handle->field_44 = false;
    state->Handle->field_45 = true;

    Movie_Queue_Ingame_Function(state->Handle);
    // After Movie_Queue_Ingame, InitialRect = {RadX+RadOffX, RadY+RadOffY, RadWidth, RadHeight}
    state->DestinationRect = state->Handle->InitialRect;

    DEBUG_INFO("Play_Ingame_Movie \"%s\" with %s.\n", filename.c_str(), state->Backend->GetName());
    IngameMoviePlaybackStates.push_back(std::move(state));
    return true;
}


MoviePlaybackIngameAdvanceResult MoviePlayback_Advance_Ingame(VQHandle *handle, bool &done)
{
    IngameMoviePlaybackState *state = Find_Ingame_Movie_State(handle);
    if (!state) {
        return MOVIEPLAYBACK_INGAME_NOT_HANDLED;
    }

    if (CurrentVQ != nullptr) {
        done = false;
        return MOVIEPLAYBACK_INGAME_NO_FRAME;
    }

    CurrentVQ = handle;
    const bool advanced = Advance_Ingame_Movie(*state, done);
    CurrentVQ = nullptr;

    return advanced ? MOVIEPLAYBACK_INGAME_FRAME_ADVANCED : MOVIEPLAYBACK_INGAME_NO_FRAME;
}


bool MoviePlayback_Destroy_Ingame(VQHandle *handle)
{
    IngameMoviePlaybackState *state = Find_Ingame_Movie_State(handle);
    if (!state) {
        return false;
    }

    if (state->Backend) {
        state->Backend->Stop();
    }

    SDL_Movie_Shutdown();

    if (CurrentVQ == handle) {
        CurrentVQ = nullptr;
    }

    if (handle) {
        handle->VQA = nullptr;
        handle->field_45 = false;
    }

    Remove_Ingame_Movie_State(handle);
    return true;
}


bool MoviePlayback_Pause_Ingame(VQHandle *handle)
{
    IngameMoviePlaybackState *state = Find_Ingame_Movie_State(handle);
    if (!state) {
        return false;
    }

    if (!state->Paused) {
        state->Paused = true;
        state->PauseStartedMs = SDL_GetTicks();

        if (state->Backend) {
            state->Backend->Pause();
        }

        SDL_Movie_Pause_Audio();

        if (state->DummyVQA) {
            state->DummyVQA->IsPaused = true;
        }
    }

    return true;
}


bool MoviePlayback_Resume_Ingame(VQHandle *handle)
{
    IngameMoviePlaybackState *state = Find_Ingame_Movie_State(handle);
    if (!state) {
        return false;
    }

    if (state->Paused) {
        state->Paused = false;

        if (state->PauseStartedMs != 0) {
            state->PausedMs += SDL_GetTicks() - state->PauseStartedMs;
            state->PauseStartedMs = 0;
        }

        if (state->Backend) {
            state->Backend->Resume();
        }

        SDL_Movie_Resume_Audio();

        if (state->DummyVQA) {
            state->DummyVQA->IsPaused = false;
        }
    }

    return true;
}
