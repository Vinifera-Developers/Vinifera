/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Runtime audio event sequencing for VOC sound definitions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_defines.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"

#include <chrono>
#include <string>
#include <vector>


class AudioVocClass;


/**
 *  Runtime state machine for one active VOC event.
 */
class AudioEventClass
{
public:
    /**
     *  Selection strategy for the event's body cycle. Derived from the voc's
     *  Control flags at construction; determines how BodyOrder is rebuilt at
     *  the start of every body cycle.
     */
    enum class BodyMode
    {
        Single,       // Always play the same one filename.
        AllSequence,  // Play every filename in order, every cycle (Control & ALL).
        Random,       // Pick one random filename; re-roll per cycle (Control & RANDOM).
        Sequential,   // Pick the next filename via the voc's persistent index (Control & SEQUENTIAL).
    };

    AudioEventClass(AudioEventHandle public_handle, AudioVocClass const& voc, Coord const& coord, int variation, float volume, float fade_in_seconds);

    AudioEventHandle Get_Public_Handle() const { return PublicHandle; }
    AudioInstanceHandle Get_Current_Handle() const { return CurrentHandle; }

    bool Matches(AudioEventHandle handle) const;
    bool Is_Finished() const { return Finished; }
    bool Is_Playing() const { return !Finished; }

    void AI();
    bool Stop(float fade_out_seconds);
    bool Update_Position(Coord coord);
    void Clear();

private:
    void Start_Next();
    void Build_Body_Order_For_Cycle();
    bool Should_Use_Native_Loop() const;

private:
    using EventClock = std::chrono::steady_clock;

    AudioEventHandle PublicHandle{};
    AudioInstanceHandle CurrentHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    AudioVocClass const* Voc = nullptr;
    Coord Position = COORD_NONE;
    float Volume = 1.0f;
    float FadeInSeconds = 0.0f;
    int CompletedBodyCycles = 0;
    bool SeenCurrentPlaying = false;
    bool StopRequested = false;
    bool DecayStarted = false;
    bool Finished = false;

    /**
     *  When true, the underlying ma_sound is looping natively in miniaudio.
     *  AI() must not retrigger; Body cycles are tracked by miniaudio, not us.
     */
    bool NativeLoopActive = false;

    EventClock::time_point NextStartTime = EventClock::now();

    /**
     *  Attack/Decay are consumed (popped) as their samples play. BodyFilenames
     *  is the stable pool for the body cycle; BodyOrder is the index sequence
     *  for the current cycle and is rebuilt by Build_Body_Order_For_Cycle().
     */
    std::vector<std::string> Attack;
    std::vector<std::string> Decay;
    std::vector<std::string> BodyFilenames;
    std::vector<size_t> BodyOrder;
    size_t BodyIndex = 0;
    BodyMode Mode = BodyMode::Single;
};


namespace AudioEventSystem
{
AudioEventHandle Start(AudioVocClass const& voc, Coord const& coord, int variation, float volume, float fade_in_seconds);
void AI();
bool Stop(AudioEventHandle handle, float fade_out_seconds = 1.0f);
bool Is_Playing(AudioEventHandle handle);
bool Update_Position(AudioEventHandle handle, Coord coord);
void Clear();
}
