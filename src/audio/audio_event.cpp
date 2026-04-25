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

#include "audio_manager.h"
#include "audio_voc.h"
#include "tibsun_inline.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>


namespace {

AudioEventHandle Generate_Event_Handle()
{
    /**
     *  ID 0 is reserved as the invalid sentinel for AudioEventHandle, so
     *  start the counter at 1 and skip 0 on wrap-around. The 0xE0000000 tag
     *  bits are kept purely for human-readable debug output (so an event
     *  handle is visually distinguishable from a manager handle).
     */
    static std::atomic<uint32_t> EventIDCounter{1};
    uint32_t counter = EventIDCounter.fetch_add(1, std::memory_order_relaxed) & 0x0FFFFFFF;
    if (counter == 0) {
        counter = EventIDCounter.fetch_add(1, std::memory_order_relaxed) & 0x0FFFFFFF;
    }
    return AudioEventHandle{0xE0000000u | counter};
}

std::mutex EventMutex;
std::vector<std::unique_ptr<AudioEventClass>> ActiveEvents;

} // namespace


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
        Attack.push_back(filenames.front());
        filenames.erase(filenames.begin());
    }

    if ((voc.Control & AUDIO_CONTROL_DECAY) != 0 && filenames.size() > 1) {
        Decay.push_back(filenames.back());
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
    if (BodyFilenames.empty() && Attack.empty() && !Decay.empty()) {
        BodyFilenames.swap(Decay);
        Mode = BodyMode::Single;
    }

    Build_Body_Order_For_Cycle();
}


bool AudioEventClass::Matches(AudioEventHandle handle) const
{
    return PublicHandle == handle;
}


/**
 *  Rebuilds BodyOrder for the upcoming body cycle. This is the heart of
 *  the LOOP+RANDOM fix — Random mode re-rolls every cycle, so each loop
 *  iteration gets a fresh random sample instead of repeating one pick.
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
        BodyOrder.reserve(pool_size);
        for (size_t i = 0; i < pool_size; ++i) {
            BodyOrder.push_back(i);
        }
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
 *  Returns true if this event qualifies for miniaudio-native looping —
 *  a single sample looping in place with no per-iteration variation,
 *  delays, or staged samples. This avoids the audible gap that the
 *  per-sample retrigger model introduces.
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

    if (!Attack.empty() || !Decay.empty()) {
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


void AudioEventClass::Start_Next()
{
    if (Voc == nullptr) {
        Finished = true;
        return;
    }

    std::string filename;
    bool launch_native_loop = false;

    if (!Attack.empty()) {
        filename = Attack.front();
        Attack.erase(Attack.begin());

    } else if (StopRequested) {
        if (!Decay.empty() && !DecayStarted) {
            filename = Decay.front();
            Decay.erase(Decay.begin());
            DecayStarted = true;
        } else {
            Finished = true;
            return;
        }

    } else if (!BodyOrder.empty()) {

        if (BodyIndex >= BodyOrder.size()) {
            ++CompletedBodyCycles;
            Build_Body_Order_For_Cycle();
            if (BodyOrder.empty()) {
                Finished = true;
                return;
            }
        }

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

    } else if (!Decay.empty() && !DecayStarted) {
        filename = Decay.front();
        Decay.erase(Decay.begin());
        DecayStarted = true;

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
     *  Stop was requested and the underlying sound stopped). The event has
     *  no further samples to play unless a Decay tail is queued.
     */
    if (NativeLoopActive) {
        CurrentHandle = INVALID_AUDIO_INSTANCE_HANDLE;
        SeenCurrentPlaying = false;
        NativeLoopActive = false;

        if (StopRequested && !Decay.empty() && !DecayStarted) {
            NextStartTime = now;
            return;
        }

        Finished = true;
        return;
    }

    CurrentHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    SeenCurrentPlaying = false;

    /**
     *  When BodyOrder is exhausted we've completed a full body cycle.
     *  Decide whether to start another or wind down. Skip if a Stop is
     *  already pending — we must not rebuild a fresh cycle when winding
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

    if (StopRequested && Decay.empty() && Attack.empty() && BodyOrder.empty()) {
        Finished = true;
        return;
    }

    const float post_delay = ((Voc->Control & AUDIO_CONTROL_PREDELAY) == 0) ? Voc->Random_Delay_Seconds() : 0.0f;
    NextStartTime = now + std::chrono::milliseconds(static_cast<int>(post_delay * 1000.0f));
}


bool AudioEventClass::Stop(float fade_out_seconds)
{
    StopRequested = true;

    if (CurrentHandle != INVALID_AUDIO_INSTANCE_HANDLE) {
        return AudioManager.Request_Stop(CurrentHandle, fade_out_seconds);
    }

    if (!Decay.empty()) {
        NextStartTime = EventClock::now();
    } else {
        Finished = true;
    }

    return true;
}


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


void AudioEventClass::Clear()
{
    if (CurrentHandle != INVALID_AUDIO_INSTANCE_HANDLE) {
        AudioManager.Request_Stop(CurrentHandle, 0.0f);
        CurrentHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    }

    NativeLoopActive = false;
    Finished = true;
}


AudioEventHandle AudioEventSystem::Start(AudioVocClass const& voc, Coord const& coord, int variation, float volume, float fade_in_seconds)
{
    const AudioEventHandle public_handle = Generate_Event_Handle();
    auto event = std::make_unique<AudioEventClass>(public_handle, voc, coord, variation, volume, fade_in_seconds);

    if (event == nullptr || event->Is_Finished()) {
        return INVALID_AUDIO_EVENT_HANDLE;
    }

    {
        std::scoped_lock lock(EventMutex);
        ActiveEvents.push_back(std::move(event));
    }

    AI();

    std::scoped_lock lock(EventMutex);
    const auto found = std::find_if(ActiveEvents.begin(), ActiveEvents.end(), [public_handle](const std::unique_ptr<AudioEventClass>& event_ptr) {
        return event_ptr != nullptr && event_ptr->Get_Public_Handle() == public_handle;
    });

    return found != ActiveEvents.end() ? public_handle : INVALID_AUDIO_EVENT_HANDLE;
}


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
