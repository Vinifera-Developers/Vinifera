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
#include "audio_streaming.h"
#include "ccfile.h"
#include "debughandler.h"
#include "iomap.h"
#include "movie.h"
#include "moviebackend_mediafoundation.h"
#include "movieskip.h"
#include "playmovie.h"
#include "sdl_functions.h"
#include "sdl_movie.h"
#include "sdlsurface.h"
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
#include <deque>
#include <memory>
#include <string>
#include <vector>


/**
 *  Strips any file extension and uppercases the name for consistent
 *  lookups regardless of how the caller spelled the basename.
 */
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


/**
 *  Fake VQAClass placed at VQHandle::VQA for modern in-game movie handles.
 *  Only the IsPaused field is read by the game, so the padding is sized
 *  to match VQAClass::IsPaused at offset 0x544.
 */
struct ModernIngameDummyVQA
{
    std::uint8_t Padding[0x544] = {};
    bool IsPaused = false;
    bool field_545 = false;
};


/**
 *  Presentation clock that tracks wall-clock playback time relative to
 *  the first decoded timestamp, with accumulated pause duration subtracted.
 */
struct MoviePlaybackClock
{
    bool Started = false;
    bool Paused = false;
    std::int64_t BaseTimestampMs = 0;
    Uint64 BaseTicksMs = 0;
    Uint64 PauseStartedMs = 0;
    Uint64 PausedMs = 0;

    void Start_If_Needed(std::int64_t timestamp_ms)
    {
        if (!Started) {
            Started = true;
            BaseTimestampMs = timestamp_ms;
            BaseTicksMs = SDL_GetTicks();
        }
    }

    Uint64 Elapsed_Ms() const
    {
        return SDL_GetTicks() - BaseTicksMs - PausedMs;
    }

    void On_Pause()
    {
        if (!Paused) {
            Paused = true;
            PauseStartedMs = SDL_GetTicks();
        }
    }

    void On_Resume()
    {
        if (Paused) {
            Paused = false;
            if (PauseStartedMs != 0) {
                PausedMs += SDL_GetTicks() - PauseStartedMs;
                PauseStartedMs = 0;
            }
        }
    }

    bool Is_Frame_Due(std::int64_t timestamp_ms) const
    {
        if (!Started) {
            return true;
        }

        const Uint64 target_ms = static_cast<Uint64>(std::max<std::int64_t>(0, timestamp_ms - BaseTimestampMs));
        return Elapsed_Ms() >= target_ms;
    }
};


/**
 *  Maps a decoded sample format to its bits-per-sample width, for the
 *  miniaudio streaming format mapping in AudioStreamingClass::Open.
 */
static int Movie_Sample_Bits_Per_Sample(MovieSampleFormat format)
{
    switch (format) {
        case MOVIE_SAMPLE_U8:  return 8;
        case MOVIE_SAMPLE_S16: return 16;
        case MOVIE_SAMPLE_F32: return 32;
        default:               return 0;
    }
}


/**
 *  Drives a single decode session: owns the backend, presentation clock
 *  and latest decode output. Propagates pause/resume to all subsystems
 *  and handles the ESC breakout from Update_Playback_State.
 */
struct MoviePlayer
{
    std::unique_ptr<IMovieDecoderBackend> Backend;
    std::unique_ptr<AudioStreamingClass> AudioStream;
    MoviePlaybackClock Clock;
    MovieDecodeOutput Output;
    bool EndOfStream = false;
    bool AudioOpenAttempted = false;

    bool Open(const char *filename)
    {
        Backend = Create_MediaFoundationMovieBackend();
        if (Backend && Backend->Open(filename)) {
            return true;
        }

        Backend.reset();
        return false;
    }

    ma_uint32 Queued_Audio_Frames() const
    {
        return AudioStream ? AudioStream->Get_Available_Read_Frames() : 0;
    }

    /**
     *  Lazily opens the miniaudio streaming sink on the first audio chunk
     *  using the chunk's negotiated format. The streaming class routes to
     *  AUDIO_GROUP_STREAMING, so the sound-volume option applies automatically.
     */
    bool Ensure_Audio_Stream(const MovieAudioChunk &chunk)
    {
        if (AudioStream || AudioOpenAttempted) {
            return AudioStream != nullptr;
        }

        AudioOpenAttempted = true;

        const int bps = Movie_Sample_Bits_Per_Sample(chunk.Format);
        if (bps <= 0 || chunk.SampleRate <= 0 || chunk.Channels <= 0) {
            return false;
        }

        auto stream = std::make_unique<AudioStreamingClass>();
        if (!stream->Open("MOVIE_AUDIO_STREAM", chunk.SampleRate, chunk.Channels, bps, true)) {
            DEBUG_ERROR("Failed to open movie audio stream.\n");
            return false;
        }

        stream->Play();
        AudioStream = std::move(stream);
        return true;
    }

    bool Pump_Once()
    {
        Output.Reset();

        if (!Backend || EndOfStream) {
            EndOfStream = true;
            return true;
        }

        if (!Backend->Pump(Output)) {
            return false;
        }

        if (Output.EndOfStream) {
            EndOfStream = true;
            return true;
        }

        if (Output.HasAudioChunk) {
            Clock.Start_If_Needed(Output.AudioChunk.TimestampMs);

            if (Ensure_Audio_Stream(Output.AudioChunk)) {
                AudioStream->Push_Chunk(Output.AudioChunk.Samples.data(), Output.AudioChunk.Samples.size());
            }
        }

        if (Output.HasVideoFrame) {
            Clock.Start_If_Needed(Output.VideoFrame.TimestampMs);
        }

        return true;
    }

    void Pause()
    {
        Clock.On_Pause();
        if (Backend) {
            Backend->Pause();
        }
        if (AudioStream) {
            AudioStream->Pause();
        }
    }

    void Resume()
    {
        Clock.On_Resume();
        if (Backend) {
            Backend->Resume();
        }
        if (AudioStream) {
            AudioStream->Play();
        }
    }

    void Stop()
    {
        if (Backend) {
            Backend->Stop();
        }
        AudioStream.reset();
    }

    bool Is_Finished() const
    {
        return EndOfStream || !Backend || Backend->Is_Finished();
    }

    bool Update_Playback_State();
    bool Wait_Until_Timestamp(std::int64_t timestamp_ms);
    bool Drain_Audio();
};


/**
 *  Searches for a playable movie file by trying each supported container
 *  extension in turn against the game's file system.
 */
static std::string Resolve_Movie_Filename(const char *basename)
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


/**
 *  Pumps the Windows message loop, synchronises pause state with window
 *  focus and checks for an ESC keypress. Returns false if playback
 *  should stop.
 */
bool MoviePlayer::Update_Playback_State()
{
    if (!VQA_Movie_Message_Loop()) {
        Stop();
        return false;
    }

    bool resume = !Session.Singleplayer_Game() || GameInFocus;

    if (!resume && !Clock.Paused) {
        Pause();
    } else if (resume && Clock.Paused) {
        Resume();
    }

    MoviePlayback_Update_Networking();

    if (Session.Singleplayer_Game() || MovieSkip::Is_Local_Skip_Allowed()) {
        if (Keyboard->Check() && Keyboard->Get() == (KN_RLSE_BIT | KN_ESC)) {
            DEBUG_INFO("{}: Breakout.\n", Backend->Get_Name());
            Stop();
            UpdateWindow(MainWindow);
            return false;
        }
    } else {
        MovieSkip::Update_Input();

        if (MovieSkip::Should_Skip()) {
            DEBUG_INFO("{}: Multiplayer movie skip vote is unanimous.\n", Backend->Get_Name());
            Stop();
            UpdateWindow(MainWindow);
            return false;
        }
    }

    return true;
}


/**
 *  Spins in short sleeps until the presentation clock reaches timestamp_ms.
 *  Returns false if Update_Playback_State signals a stop.
 */
bool MoviePlayer::Wait_Until_Timestamp(std::int64_t timestamp_ms)
{
    if (!Clock.Started) {
        Clock.Start_If_Needed(timestamp_ms);
        return true;
    }

    const Uint64 target_ms = static_cast<Uint64>(std::max<std::int64_t>(0, timestamp_ms - Clock.BaseTimestampMs));

    while (true) {
        if (!Update_Playback_State()) {
            return false;
        }

        if (Clock.Paused) {
            SDL_Delay(33);
            continue;
        }

        if (Clock.Elapsed_Ms() >= target_ms) {
            return true;
        }

        const Uint64 remaining = target_ms - Clock.Elapsed_Ms();
        SDL_Delay(static_cast<Uint32>(std::min<Uint64>(remaining, 2)));
    }
}


/**
 *  Computes the destination rect for a video frame: stretched to fill the
 *  window when allowed, otherwise letterboxed inside 640x400 and centred.
 */
static Rect Build_Destination_Rect(const MovieVideoFrame &frame, bool allow_stretch)
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


static void Clear_Movie_Screen()
{
    HiddenSurface->Clear();
    Update_Visible_Surface();
    SDL_Update_Screen(HiddenSurface);
    InvalidateRect(MainWindow, nullptr, FALSE);
}


/**
 *  Waits for the miniaudio ring buffer to fully drain, keeping playback
 *  state updated. Ensures audio finishes before the screen is cleared.
 *  Bounded by a 2s deadline so we cannot hang if the audio engine stops
 *  consuming for any reason.
 */
bool MoviePlayer::Drain_Audio()
{
    if (!AudioStream) {
        return true;
    }

    const Uint64 deadline_ms = SDL_GetTicks() + 2000;

    while (AudioStream->Get_Available_Read_Frames() > 0) {
        if (SDL_GetTicks() > deadline_ms) {
            DEBUG_WARNING("Drain_Audio: timeout waiting for audio buffer to drain.\n");
            break;
        }

        if (!Update_Playback_State()) {
            return false;
        }

        if (Clock.Paused) {
            SDL_Delay(33);
            continue;
        }

        SDL_Delay(10);
    }

    return true;
}


/**
 *  Active playback context for a single in-game (radar area) movie.
 *  Decodes via MoviePlayer and blits each frame into DrawSurface at
 *  DestinationRect, converting NV12 to RGB565 via two cached surfaces.
 */
struct IngameMoviePlayback
{
    ~IngameMoviePlayback()
    {
        SDL_DestroySurface(CachedNV12Surface);
        SDL_DestroySurface(CachedRGB565Surface);
    }

    VQHandle *Handle = nullptr;
    MoviePlayer Player;
    std::unique_ptr<ModernIngameDummyVQA> DummyVQA;
    Rect DestinationRect = Rect(0, 0, 0, 0);
    std::deque<MovieVideoFrame> PendingVideoFrames;
    SDL_Surface *CachedNV12Surface = nullptr;
    SDL_Surface *CachedRGB565Surface = nullptr;
    int CachedFrameWidth = 0;
    int CachedFrameHeight = 0;

    bool Present_Frame(const MovieVideoFrame &frame);
    bool Advance(bool &done);
};


static std::vector<std::unique_ptr<IngameMoviePlayback>> IngameMoviePlaybacks;


static_assert(offsetof(VQHandle, field_45) == 0x45, "Unexpected VQHandle::field_45 offset.");
static_assert(offsetof(ModernIngameDummyVQA, IsPaused) == 0x544, "Unexpected VQAClass::IsPaused offset.");


static IngameMoviePlayback *Find_Ingame_Movie(VQHandle *handle)
{
    for (const auto &state : IngameMoviePlaybacks) {
        if (state && state->Handle == handle) {
            return state.get();
        }
    }

    return nullptr;
}


static void Remove_Ingame_Movie(VQHandle *handle)
{
    IngameMoviePlaybacks.erase(
        std::remove_if(
            IngameMoviePlaybacks.begin(),
            IngameMoviePlaybacks.end(),
            [handle](const std::unique_ptr<IngameMoviePlayback> &state) {
                return !state || state->Handle == handle;
            }),
        IngameMoviePlaybacks.end());
}


/**
 *  Converts a decoded NV12 frame to RGB565 and blits it scaled into
 *  the sidebar surface at DestinationRect.
 */
bool IngameMoviePlayback::Present_Frame(const MovieVideoFrame &frame)
{
    if (frame.Format != MOVIE_VIDEO_NV12 || frame.Width <= 0 || frame.Height <= 0 || !DestinationRect.Is_Valid()) {
        return false;
    }

    auto &surface = static_cast<SDLSurface &>(*Handle->DrawSurface);
    SDL_Surface *dst_sdl = surface.Get_SDL_Surface();
    if (!dst_sdl) {
        return false;
    }

    /**
     *  Reallocate cached surfaces only when frame dimensions change.
     */
    if (CachedFrameWidth != frame.Width || CachedFrameHeight != frame.Height) {
        SDL_DestroySurface(CachedNV12Surface);
        SDL_DestroySurface(CachedRGB565Surface);

        CachedNV12Surface = SDL_CreateSurface(frame.Width, frame.Height, SDL_PIXELFORMAT_NV12);
        CachedRGB565Surface = SDL_CreateSurface(frame.Width, frame.Height, SDL_PIXELFORMAT_RGB565);
        CachedFrameWidth = frame.Width;
        CachedFrameHeight = frame.Height;

        if (!CachedNV12Surface || !CachedRGB565Surface) {
            DEBUG_ERROR("Present_Ingame_Frame: Failed to allocate cached surfaces: {}\n", SDL_GetError());
            SDL_DestroySurface(CachedNV12Surface);
            SDL_DestroySurface(CachedRGB565Surface);
            CachedNV12Surface = nullptr;
            CachedRGB565Surface = nullptr;
            CachedFrameWidth = 0;
            CachedFrameHeight = 0;
            return false;
        }
    }

    /**
     *  Copy Y and UV planes into the pre-allocated NV12 surface.
     */
    SDL_Surface *nv12 = CachedNV12Surface;
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

    /**
     *  Convert NV12 to RGB565 directly into the pre-allocated surface (no allocation).
     */
    if (!SDL_ConvertPixels(
            frame.Width, frame.Height,
            SDL_PIXELFORMAT_NV12, nv12->pixels, nv12->pitch,
            SDL_PIXELFORMAT_RGB565, CachedRGB565Surface->pixels, CachedRGB565Surface->pitch)) {
        DEBUG_ERROR("Present_Ingame_Frame: SDL_ConvertPixels failed: {}\n", SDL_GetError());
        return false;
    }

    SDL_SetSurfaceBlendMode(CachedRGB565Surface, SDL_BLENDMODE_NONE);
    SDL_Rect dst_rect = { DestinationRect.X, DestinationRect.Y,
                          DestinationRect.Width, DestinationRect.Height };
    const bool ok = SDL_BlitSurfaceScaled(CachedRGB565Surface, nullptr, dst_sdl, &dst_rect, SDL_SCALEMODE_NEAREST);

    if (!ok) {
        DEBUG_ERROR("Present_Ingame_Frame: SDL_BlitSurfaceScaled failed: {}\n", SDL_GetError());
    }

    return ok;
}


/**
 *  Pumps the decoder and presents every video frame that is now due. Keeps
 *  pumping past not-yet-due video frames (queuing them) so audio chunks
 *  interleaved later in the source reader still reach the streaming sink
 *  this tick — preventing audio underruns when Advance is called at a low
 *  rate (e.g. at slow game speeds). Sets done when the stream ends and all
 *  queued audio has drained. Returns true if any frame was presented.
 */
bool IngameMoviePlayback::Advance(bool &done)
{
    done = false;

    if (!Player.Backend || !Handle || !Handle->DrawSurface) {
        done = true;
        return false;
    }

    bool frame_presented = false;

    /**
     *  Drain any previously-queued frames that are now due. Multiple frames
     *  may be presented in a single call when catching up from a stall.
     */
    while (!PendingVideoFrames.empty() && Player.Clock.Is_Frame_Due(PendingVideoFrames.front().TimestampMs)) {
        if (!Present_Frame(PendingVideoFrames.front())) {
            done = true;
            return frame_presented;
        }
        PendingVideoFrames.pop_front();
        frame_presented = true;
    }

    static constexpr int MaxPumpsPerTick = 8;
    static constexpr std::size_t MaxPendingVideoFrames = 3;
    int pump_count = 0;

    while (!Player.EndOfStream && pump_count < MaxPumpsPerTick) {
        /**
         *  If we are already comfortably ahead on video, stop pumping for
         *  this tick — the audio ring buffer will be well-fed and we avoid
         *  unbounded queue growth if Advance is called rarely.
         */
        if (PendingVideoFrames.size() >= MaxPendingVideoFrames) {
            break;
        }

        ++pump_count;

        if (!Player.Pump_Once()) {
            done = true;
            return frame_presented;
        }

        if (Player.EndOfStream) {
            break;
        }

        if (Player.Output.HasVideoFrame) {
            if (Player.Clock.Is_Frame_Due(Player.Output.VideoFrame.TimestampMs)) {
                if (!Present_Frame(Player.Output.VideoFrame)) {
                    done = true;
                    return frame_presented;
                }
                frame_presented = true;
            } else {
                PendingVideoFrames.emplace_back();
                std::swap(PendingVideoFrames.back(), Player.Output.VideoFrame);
            }
            continue;
        }

        if (!Player.Output.HasAudioChunk) {
            break;
        }
    }

    if (Player.EndOfStream && PendingVideoFrames.empty() && Player.Queued_Audio_Frames() == 0) {
        done = true;
    }

    return frame_presented;
}


bool MoviePlayback_Is_Available(const char *basename)
{
    return !Resolve_Movie_Filename(basename).empty();
}


/**
 *  Plays a fullscreen modern movie by basename, blocking until it finishes
 *  or ESC is pressed. Optionally queues a theme song alongside the video.
 */
bool MoviePlayback_Play(const char *basename, ThemeType theme, bool clear_before, bool stretch_allowed, bool clear_after)
{
    const std::string filename = Resolve_Movie_Filename(basename);
    if (filename.empty()) {
        DEBUG_WARNING("Failed to resolve modern movie filename for \"{}\".\n", basename);
        return false;
    }

    MoviePlayer player;
    if (!player.Open(filename.c_str())) {
        DEBUG_WARNING("Failed to open \"{}\" with any modern movie backend.\n", filename);
        return false;
    }

    DEBUG_INFO("Play_Movie \"{}\" with {}.\n", filename, player.Backend->Get_Name());

    Keyboard->Clear();
    MouseCursor->Hide_Mouse();
    Vinifera_ModernMoviePlaying = true;

    if (theme != THEME_NONE) {
        Theme.Queue_Song(theme);
    }

    if (clear_before) {
        Clear_Movie_Screen();
    }

    bool aborted = false;
    bool success = true;

    while (!player.Is_Finished()) {
        if (!player.Update_Playback_State()) {
            aborted = true;
            break;
        }

        if (player.Clock.Paused) {
            SDL_Delay(33);
            continue;
        }

        if (!player.Pump_Once()) {
            success = false;
            break;
        }

        if (player.EndOfStream) {
            break;
        }

        if (player.Output.HasVideoFrame) {
            if (!player.Wait_Until_Timestamp(player.Output.VideoFrame.TimestampMs)) {
                aborted = true;
                break;
            }

            const Rect destination = Build_Destination_Rect(player.Output.VideoFrame, stretch_allowed);
            if (!SDL_Movie_Present_Frame(player.Output.VideoFrame, destination)) {
                success = false;
                break;
            }
        }

        if (player.Output.HasAudioChunk) {
            /**
             *  Throttle the decode loop when ~0.5 sec of audio is already queued,
             *  but cap the threshold at 3/4 of the ring buffer so it is actually
             *  reachable at high sample rates - otherwise Push_Chunk silently
             *  drops the tail of every chunk once the buffer fills.
             */
            const ma_uint32 half_second = player.Output.AudioChunk.SampleRate > 0
                ? static_cast<ma_uint32>(player.Output.AudioChunk.SampleRate / 2)
                : 0;
            const ma_uint32 capacity_cap = AudioStreamingClass::PCM_RING_BUFFER_FRAMES * 3 / 4;
            const ma_uint32 max_frames = std::min(half_second, capacity_cap);

            while (max_frames > 0 && player.Queued_Audio_Frames() > max_frames) {
                if (!player.Update_Playback_State()) {
                    aborted = true;
                    break;
                }

                if (player.Clock.Paused) {
                    SDL_Delay(33);
                } else {
                    SDL_Delay(5);
                }
            }

            if (aborted) {
                break;
            }
        }

        if (!player.Output.HasAudioChunk && !player.Output.HasVideoFrame) {
            SDL_Delay(1);
        }
    }

    if (success && !aborted) {
        player.Drain_Audio();
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


/**
 *  Begins in-game movie playback in the radar area. Creates a VQHandle
 *  with a DummyVQA and registers it via Movie_Queue_Ingame so the
 *  existing radar bookkeeping still works with the new backend.
 */
bool MoviePlayback_Play_Ingame(const char *basename)
{
    const std::string filename = Resolve_Movie_Filename(basename);
    if (filename.empty() || Session.Type != GAME_NORMAL || !SidebarSurface) {
        return false;
    }

    auto state = std::make_unique<IngameMoviePlayback>();

    if (!state->Player.Open(filename.c_str())) {
        DEBUG_WARNING("Failed to open ingame movie \"{}\" with any modern movie backend.\n", filename);
        return false;
    }

    const int width = state->Player.Backend->Get_Video_Width();
    const int height = state->Player.Backend->Get_Video_Height();
    if (width <= 0 || height <= 0) {
        DEBUG_WARNING("Modern ingame movie \"{}\" reported invalid dimensions.\n", filename);
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
    /**
     *  Set InitialRect to the radar movie display size (140x110).
     *  Movie_Queue_Ingame will overwrite X/Y with RadX+RadOffX, RadY+RadOffY
     *  (SidebarSurface-local coordinates) while keeping Width/Height.
     */
    state->Handle->InitialRect = Rect(0, 0, 140, 110);
    state->Handle->StretchRect = state->Handle->InitialRect;
    state->Handle->field_44 = false;
    state->Handle->field_45 = true;

    Movie_Queue_Ingame(state->Handle);

    /**
     *  After Movie_Queue_Ingame, InitialRect = {RadX+RadOffX, RadY+RadOffY, RadWidth, RadHeight}.
     */
    state->DestinationRect = state->Handle->InitialRect;

    DEBUG_INFO("MoviePlayback_Play_Ingame \"{}\" with {}.\n", filename, state->Player.Backend->Get_Name());
    IngameMoviePlaybacks.push_back(std::move(state));
    return true;
}


/**
 *  Called by the radar draw code each frame. Pumps and presents the next
 *  frame for handle, setting done when playback is complete. Returns
 *  MOVIEPLAYBACK_INGAME_NOT_HANDLED if the handle is not managed here.
 */
MoviePlaybackIngameAdvanceResult MoviePlayback_Advance_Ingame(VQHandle *handle, bool &done)
{
    IngameMoviePlayback *state = Find_Ingame_Movie(handle);
    if (!state) {
        return MOVIEPLAYBACK_INGAME_NOT_HANDLED;
    }

    if (CurrentVQ != nullptr) {
        done = false;
        return MOVIEPLAYBACK_INGAME_NO_FRAME;
    }

    CurrentVQ = handle;
    const bool advanced = state->Advance(done);
    CurrentVQ = nullptr;

    return advanced ? MOVIEPLAYBACK_INGAME_FRAME_ADVANCED : MOVIEPLAYBACK_INGAME_NO_FRAME;
}


/**
 *  Stops playback, releases the audio stream via MoviePlayer::Stop, and
 *  removes the state associated with handle. The fullscreen SDL movie
 *  texture is owned by MoviePlayback_Play and not touched here.
 */
bool MoviePlayback_Destroy_Ingame(VQHandle *handle)
{
    IngameMoviePlayback *state = Find_Ingame_Movie(handle);
    if (!state) {
        return false;
    }

    state->Player.Stop();

    if (CurrentVQ == handle) {
        CurrentVQ = nullptr;
    }

    if (handle) {
        handle->VQA = nullptr;
        handle->field_45 = false;
    }

    Remove_Ingame_Movie(handle);
    return true;
}


bool MoviePlayback_Pause_Ingame(VQHandle *handle)
{
    IngameMoviePlayback *state = Find_Ingame_Movie(handle);
    if (!state) {
        return false;
    }

    if (!state->Player.Clock.Paused) {
        state->Player.Pause();

        if (state->DummyVQA) {
            state->DummyVQA->IsPaused = true;
        }
    }

    return true;
}


bool MoviePlayback_Resume_Ingame(VQHandle *handle)
{
    IngameMoviePlayback *state = Find_Ingame_Movie(handle);
    if (!state) {
        return false;
    }

    if (state->Player.Clock.Paused) {
        state->Player.Resume();

        if (state->DummyVQA) {
            state->DummyVQA->IsPaused = false;
        }
    }

    return true;
}


/**
 *  Periodically sends a message to other players in multiplayer during movie playback
 *  so they and the CnCNet tunnel server don't forget about us.
 */
void MoviePlayback_Update_Networking()
{
    if (!Session.Singleplayer_Game() && !MovieSkip::Is_Local_Skip_Allowed()) {

        // Static initializer is run when this block is first executed
        static DWORD LastNetworkRefreshTime = timeGetTime() + 1000;
        static DWORD LastNetworkServiceTime = timeGetTime() + 50;
        const DWORD now = timeGetTime();

        // timeGetTime wraps around every 49.7 days of runtime,
        // using subtraction prevents the wrap-around from causing issues
        if (now - LastNetworkRefreshTime >= 1000) {
            Session.Loading_Callback(100);
            LastNetworkRefreshTime = now;
        }

        /**
         *  Service incoming global packets more frequently than the keepalive
         *  refresh so a unanimous skip responds promptly rather than waiting
         *  for up to a full second.
         */
        if (now - LastNetworkServiceTime >= 50) {
            Call_Back();
            LastNetworkServiceTime = now;
        }
    }
}
