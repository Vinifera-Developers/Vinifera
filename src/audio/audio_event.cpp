/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Runtime audio event sequencing for VOC sound definitions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_event.h"

#include "audio_debug.h"
#include "audio_manager.h"
#include "audio_voc.h"
#include "tibsun_inline.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <numeric>


namespace {

std::mutex EventMutex;
std::vector<std::unique_ptr<AudioEventClass>> ActiveEvents;

/**
 *  Allocates a unique handle for a new audio event. The upper-nibble tag
 *  0xE0000000 makes event handles visually distinct from audio manager
 *  instance handles in debug output. ID 0 is the invalid sentinel and is
 *  skipped on counter wrap-around. Once the 28-bit counter wraps, skip any
 *  handle still owned by an active event.
 *
 *  @author: ZivDero
 */
AudioEventHandle Generate_Event_Handle()
{
    /**
     *  ID 0 is reserved as the invalid sentinel for AudioEventHandle, so
     *  start the counter at 1 and skip 0 on wrap-around. The 0xE0000000 tag
     *  bits are kept purely for human-readable debug output (so an event
     *  handle is visually distinguishable from a manager handle).
     */
    static std::atomic<uint32_t> EventIDCounter{1};

    constexpr uint32_t kEventTag = 0xE0000000u;
    constexpr uint32_t kCounterMask = 0x0FFFFFFFu;

    static std::atomic<bool> EventIDCounterWrapped{false};

    for (uint32_t attempt = 0; attempt < kCounterMask; ++attempt) {
        const uint32_t counter_value = EventIDCounter.fetch_add(1, std::memory_order_relaxed);
        uint32_t counter = counter_value & kCounterMask;
        bool check_collision = (counter_value & ~kCounterMask) != 0 || EventIDCounterWrapped.load(std::memory_order_acquire);

        if (counter == 0) {
            EventIDCounterWrapped.store(true, std::memory_order_release);
            continue;
        }

        AudioEventHandle handle { kEventTag | counter };

        if (!check_collision) {
            return handle;
        }

        std::scoped_lock lock(EventMutex);
        const bool in_use = std::any_of(ActiveEvents.begin(), ActiveEvents.end(), [handle](const std::unique_ptr<AudioEventClass>& event) {
            return event != nullptr && event->Get_Public_Handle() == handle;
        });

        if (!in_use) {
            return handle;
        }
    }

    AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "AudioEventSystem::Generate_Event_Handle - Exhausted audio event handle IDs!\n");

    return INVALID_AUDIO_EVENT_HANDLE;
}

} // namespace


/**
 *  Initializes the event state from the given voc definition. Resolves
 *  filenames, applies the PREDELAY offset, splits attack and decay tails
 *  from the body pool, and determines which body mode drives sample
 *  selection. Sets Finished immediately if no filenames resolve.
 *
 *  @author: ZivDero
 */
AudioEventClass::AudioEventClass(AudioEventHandle public_handle, AudioVocClass const& voc, Coord const& coord, int variation, float volume, float fade_in_seconds) :
    PublicHandle(public_handle),
    Voc(&voc),
    Position(coord),
    Volume(volume),
    FadeInSeconds(fade_in_seconds),
    NextStartTime(EventClock::now())
{
    std::vector<std::string> filenames = voc.Build_Filename_Pool(variation);

    if (filenames.empty()) {
        Finished = true;
        return;
    }

    if ((voc.Control & AUDIO_CONTROL_PREDELAY) != 0) {
        const float initial_delay = voc.Random_Delay_Seconds();
        NextStartTime += std::chrono::milliseconds(static_cast<int>(initial_delay * 1000.0f));
    }

    /**
     *  Pop the first sample as Attack and the last as Decay if requested.
     *  These are consumed once and never repeat. We only do so if there is
     *  more than one sample, so the body always has at least one entry.
     */
    if ((voc.Control & AUDIO_CONTROL_ATTACK) != 0 && filenames.size() > 1) {
        Attack = std::move(filenames.front());
        filenames.erase(filenames.begin());
    }

    if ((voc.Control & AUDIO_CONTROL_DECAY) != 0 && filenames.size() > 1) {
        Decay = std::move(filenames.back());
        filenames.pop_back();
    }

    /**
     *  A forced variation collapses the pool to a single entry; the body
     *  then runs as Single mode regardless of what Control says about
     *  selection (RANDOM/SEQUENTIAL would have nothing to choose from).
     */
    BodyFilenames = std::move(filenames);

    if (BodyFilenames.size() <= 1 || variation >= 0) {
        Mode = BodyMode::Single;
    } else if ((voc.Control & AUDIO_CONTROL_ALL) != 0) {
        Mode = BodyMode::AllSequence;
    } else if ((voc.Control & AUDIO_CONTROL_RANDOM) != 0) {
        Mode = BodyMode::Random;
    } else if ((voc.Control & AUDIO_CONTROL_SEQUENTIAL) != 0) {
        Mode = BodyMode::Sequential;
    } else {
        Mode = BodyMode::Single;
    }

    /**
     *  If only Decay was specified (no body samples remained after the split),
     *  treat the decay sample as the single body sample so something plays.
     */
    if (BodyFilenames.empty() && !Attack.has_value() && Decay.has_value()) {
        BodyFilenames.push_back(std::move(*Decay));
        Decay.reset();
        Mode = BodyMode::Single;
    }

    Build_Body_Order_For_Cycle();
}


/**
 *  Returns true if this event owns the given public handle.
 *
 *  @author: ZivDero
 */
bool AudioEventClass::Matches(AudioEventHandle handle) const
{
    return PublicHandle == handle;
}


/**
 *  Rebuilds BodyOrder for the upcoming body cycle. Random mode re-rolls
 *  on every cycle so each loop iteration selects a different sample from
 *  the pool. Sequential mode advances the voc's persistent index.
 *
 *  @author: ZivDero
 */
void AudioEventClass::Build_Body_Order_For_Cycle()
{
    BodyOrder.clear();
    BodyIndex = 0;

    if (BodyFilenames.empty()) {
        return;
    }

    const size_t pool_size = BodyFilenames.size();

    switch (Mode) {
    case BodyMode::AllSequence:
        BodyOrder.resize(pool_size);
        std::iota(BodyOrder.begin(), BodyOrder.end(), size_t{0});
        break;

    case BodyMode::Random:
        BodyOrder.push_back(static_cast<size_t>(Sim_Random_Pick(0, static_cast<int>(pool_size) - 1)));
        break;

    case BodyMode::Sequential: {
        const size_t idx = Voc != nullptr ? Voc->Advance_Sequential_Index() : 0;
        BodyOrder.push_back(idx % pool_size);
        break;
    }

    case BodyMode::Single:
    default:
        BodyOrder.push_back(0);
        break;
    }
}


/**
 *  Returns true if this event qualifies for miniaudio-native looping -
 *  a single sample looping with no per-iteration variation, delays, or
 *  staged samples. Native looping eliminates the inter-iteration gap
 *  that the retrigger model produces.
 *
 *  @author: ZivDero
 */
bool AudioEventClass::Should_Use_Native_Loop() const
{
    if (Voc == nullptr) {
        return false;
    }

    if ((Voc->Control & AUDIO_CONTROL_LOOP) == 0) {
        return false;
    }

    if (Mode != BodyMode::Single) {
        return false;
    }

    if (Attack.has_value() || Decay.has_value()) {
        return false;
    }

    if (BodyOrder.size() != 1 || BodyFilenames.size() != 1) {
        return false;
    }

    /**
     *  Inter-sample delay only takes effect between iterations. Native
     *  looping has no inter-iteration hook, so any non-zero delay (or
     *  PREDELAY-with-non-zero range) forces the retrigger model.
     */
    if (Voc->Delay.X > 0 || Voc->Delay.Y > 0) {
        return false;
    }

    /**
     *  VShift/FShift get re-rolled per Start_File call. Forcing native
     *  loop would freeze the first roll for every iteration; fall back
     *  to retrigger so the variation actually varies.
     */
    if (Voc->VolumeShift.X != AUDIO_VSHIFT_MIN || Voc->VolumeShift.Y != AUDIO_VSHIFT_MIN) {
        return false;
    }
    if (Voc->FrequencyShift.X != AUDIO_FSHIFT_MIN || Voc->FrequencyShift.Y != AUDIO_FSHIFT_MIN) {
        return false;
    }

    return true;
}


/**
 *  Selects and starts the next sample in sequence. Consumes attack
 *  entries first, then advances through the current body cycle, then
 *  plays the decay tail. Sets Finished when nothing remains to play.
 *
 *  @author: ZivDero
 */
void AudioEventClass::Start_Next()
{
    if (Voc == nullptr) {
        Finished = true;
        return;
    }

    std::string filename;
    bool launch_native_loop = false;

    if (Attack.has_value()) {
        filename = std::move(*Attack);
        Attack.reset();

    } else if (StopRequested) {
        if (Decay.has_value()) {
            filename = std::move(*Decay);
            Decay.reset();
        } else {
            Finished = true;
            return;
        }

    } else if (!BodyOrder.empty()) {

        filename = BodyFilenames[BodyOrder[BodyIndex]];
        ++BodyIndex;

        /**
         *  When the cycle just started (single-sample modes pick BodyOrder[0]
         *  as the very first body play), and the configuration is eligible,
         *  hand off to miniaudio-native looping. The cycle counter advances
         *  to cover the entire native loop in one go.
         */
        if (BodyIndex == 1 && BodyOrder.size() == 1 && CompletedBodyCycles == 0 && Should_Use_Native_Loop()) {
            launch_native_loop = true;
        }

    } else if (Decay.has_value()) {
        filename = std::move(*Decay);
        Decay.reset();

    } else {
        Finished = true;
        return;
    }

    if (launch_native_loop) {
        const int loop_limit = std::max(0, Voc->LoopLimit);
        CurrentHandle = Voc->Start_File(filename, Position, Volume, FadeInSeconds, true, loop_limit);
        NativeLoopActive = (CurrentHandle != INVALID_AUDIO_INSTANCE_HANDLE);
    } else {
        CurrentHandle = Voc->Start_File(filename, Position, Volume, FadeInSeconds, false, 0);
    }

    SeenCurrentPlaying = CurrentHandle != INVALID_AUDIO_INSTANCE_HANDLE;

    if (CurrentHandle == INVALID_AUDIO_INSTANCE_HANDLE) {
        Finished = true;
    }
}


/**
 *  Per-frame update. Drives the sample sequence, inter-sample delays,
 *  loop cycle counting, and stop/decay transitions. Called under
 *  EventMutex by AudioEventSystem::AI().
 *
 *  @author: ZivDero
 */
void AudioEventClass::AI()
{
    if (Finished || Voc == nullptr) {
        return;
    }

    const auto now = EventClock::now();

    if (CurrentHandle == INVALID_AUDIO_INSTANCE_HANDLE) {
        if (now >= NextStartTime) {
            Start_Next();
        }
        return;
    }

    if (AudioManager.Query_Is_Playing(CurrentHandle)) {
        SeenCurrentPlaying = true;
        return;
    }

    if (!SeenCurrentPlaying) {
        return;
    }

    /**
     *  The native-loop sample has finished (miniaudio applied LoopLimit, or
     *  Stop was requested and the underlying sound stopped). Native looping
     *  requires no attack, decay, or variation, so nothing more can play.
     */
    if (NativeLoopActive) {
        CurrentHandle = INVALID_AUDIO_INSTANCE_HANDLE;
        SeenCurrentPlaying = false;
        NativeLoopActive = false;
        Finished = true;
        return;
    }

    CurrentHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    SeenCurrentPlaying = false;

    /**
     *  When BodyOrder is exhausted we've completed a full body cycle.
     *  Decide whether to start another or wind down. Skip if a Stop is
     *  already pending - we must not rebuild a fresh cycle when winding
     *  down (e.g. after a Decay sample finishes at the end of a cycle).
     */
    if (BodyIndex >= BodyOrder.size() && !StopRequested) {

        ++CompletedBodyCycles;

        const bool looping = (Voc->Control & AUDIO_CONTROL_LOOP) != 0;

        if (!looping) {
            StopRequested = true;
        } else if (Voc->LoopLimit > 0 && CompletedBodyCycles >= Voc->LoopLimit) {
            StopRequested = true;
        } else {
            Build_Body_Order_For_Cycle();
            if (BodyOrder.empty()) {
                StopRequested = true;
            }
        }
    }

    if (StopRequested && !Decay.has_value() && !Attack.has_value() && BodyOrder.empty()) {
        Finished = true;
        return;
    }

    const float post_delay = ((Voc->Control & AUDIO_CONTROL_PREDELAY) == 0) ? Voc->Random_Delay_Seconds() : 0.0f;
    NextStartTime = now + std::chrono::milliseconds(static_cast<int>(post_delay * 1000.0f));
}


/**
 *  Requests a graceful stop with the given fade-out duration. Fades out
 *  any currently-playing sample. If a decay tail is queued it will play
 *  next; otherwise the event is marked finished immediately.
 *
 *  @author: ZivDero
 */
bool AudioEventClass::Stop(float fade_out_seconds)
{
    StopRequested = true;

    if (CurrentHandle != INVALID_AUDIO_INSTANCE_HANDLE) {
        return AudioManager.Request_Stop(CurrentHandle, fade_out_seconds);
    }

    Attack.reset();

    if (Decay.has_value()) {
        NextStartTime = EventClock::now();
    } else {
        Finished = true;
    }

    return true;
}


/**
 *  Updates the world position and pushes new pan and volume values to the
 *  currently-playing sample. Caches the coord regardless so the next
 *  sample in the sequence picks it up even during inter-sample silence gaps.
 *
 *  @author: ZivDero
 */
bool AudioEventClass::Update_Position(Coord coord)
{
    Position = coord;

    if (CurrentHandle == INVALID_AUDIO_INSTANCE_HANDLE || Voc == nullptr) {
        return true;
    }

    float vol = Voc->Get_Volume();
    float pan = 0.0f;

    if (coord != COORD_NONE) {
        Voc->Calculate_Pan_And_Volume(coord, pan, vol);
    }

    const bool volume_ok = AudioManager.Set_Volume(CurrentHandle, vol);
    const bool pan_ok = AudioManager.Set_Pan(CurrentHandle, pan);
    return volume_ok && pan_ok;
}


/**
 *  Immediately stops any live sample and marks the event finished,
 *  with no fade-out and no decay tail.
 *
 *  @author: ZivDero
 */
void AudioEventClass::Clear()
{
    if (CurrentHandle != INVALID_AUDIO_INSTANCE_HANDLE) {
        AudioManager.Request_Stop(CurrentHandle, 0.0f);
        CurrentHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    }

    NativeLoopActive = false;
    Finished = true;
}


/**
 *  Creates a new event for the given voc, registers it in the active list,
 *  runs its first AI tick, and returns the public handle. Returns
 *  INVALID_AUDIO_EVENT_HANDLE if the event finished immediately (e.g. no
 *  files resolved, or the audio manager rejected the sample).
 *
 *  @author: ZivDero
 */
AudioEventHandle AudioEventSystem::Start(AudioVocClass const& voc, Coord const& coord, int variation, float volume, float fade_in_seconds)
{
    const AudioEventHandle public_handle = Generate_Event_Handle();
    if (public_handle == INVALID_AUDIO_EVENT_HANDLE) {
        return INVALID_AUDIO_EVENT_HANDLE;
    }

    auto event = std::make_unique<AudioEventClass>(public_handle, voc, coord, variation, volume, fade_in_seconds);

    if (event->Is_Finished()) {
        return INVALID_AUDIO_EVENT_HANDLE;
    }

    /**
     *  Hold EventMutex over the push and the first AI tick. Lock order is
     *  EventMutex -> AudioManager::ThreadMutex (the AI() callbacks reach into
     *  AudioManager); the manager never calls back into the event system, so
     *  there is no inversion.
     */
    std::scoped_lock lock(EventMutex);
    AudioEventClass * raw = event.get();
    ActiveEvents.push_back(std::move(event));
    raw->AI();

    return raw->Is_Finished() ? INVALID_AUDIO_EVENT_HANDLE : public_handle;
}


/**
 *  Ticks all active events and removes any that have finished.
 *
 *  @author: ZivDero
 */
void AudioEventSystem::AI()
{
    std::scoped_lock lock(EventMutex);

    for (auto& event : ActiveEvents) {
        if (event != nullptr) {
            event->AI();
        }
    }

    ActiveEvents.erase(
        std::remove_if(ActiveEvents.begin(), ActiveEvents.end(), [](const std::unique_ptr<AudioEventClass>& event) {
            return event == nullptr || event->Is_Finished();
        }),
        ActiveEvents.end());
}


/**
 *  Requests a stop on the event matching the given handle, fading out
 *  over the specified duration.
 *
 *  @author: ZivDero
 */
bool AudioEventSystem::Stop(AudioEventHandle handle, float fade_out_seconds)
{
    if (!handle.Is_Valid()) {
        return false;
    }

    std::scoped_lock lock(EventMutex);

    for (auto& event : ActiveEvents) {
        if (event != nullptr && event->Matches(handle)) {
            return event->Stop(fade_out_seconds);
        }
    }

    return false;
}


/**
 *  Returns true if the event matching the given handle is still active.
 *
 *  @author: ZivDero
 */
bool AudioEventSystem::Is_Playing(AudioEventHandle handle)
{
    if (!handle.Is_Valid()) {
        return false;
    }

    std::scoped_lock lock(EventMutex);

    for (const auto& event : ActiveEvents) {
        if (event != nullptr && event->Matches(handle)) {
            return event->Is_Playing();
        }
    }

    return false;
}


/**
 *  Pushes a new world position to the event matching the given handle.
 *
 *  @author: ZivDero
 */
bool AudioEventSystem::Update_Position(AudioEventHandle handle, Coord coord)
{
    if (!handle.Is_Valid()) {
        return false;
    }

    std::scoped_lock lock(EventMutex);

    for (auto& event : ActiveEvents) {
        if (event != nullptr && event->Matches(handle)) {
            return event->Update_Position(coord);
        }
    }

    return false;
}


/**
 *  Immediately stops and removes all active events, with no fade-out.
 *
 *  @author: ZivDero
 */
void AudioEventSystem::Clear()
{
    std::scoped_lock lock(EventMutex);

    for (auto& event : ActiveEvents) {
        if (event != nullptr) {
            event->Clear();
        }
    }

    ActiveEvents.clear();
}
