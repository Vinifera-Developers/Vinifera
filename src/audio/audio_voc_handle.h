/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Long-lived owned handle to a voc sound played via AudioEventSystem.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_defines.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"

class AudioVocClass;


/**
 *  Long-lived RAII handle around a single AudioEventSystem event for a voc.
 *  Use this when a caller owns the lifetime of a sound (e.g. a looping
 *  ambient tied to a game object) - fire-and-forget VOC plays should go
 *  through AudioVocClass::Play() directly.
 */
class AudioVocHandle
{
public:
    AudioVocHandle(VocType voc);
    AudioVocHandle(const char* name);
    ~AudioVocHandle();

    bool Start(Coord const& coord = COORD_NONE, float fade_in_seconds = 1.0f);
    bool Stop(float fade_out_seconds = 1.0f);
    bool Is_Playing();

    bool Update_Position(Coord coord);

private:
    /**
     *  Bound voc definition pointer.
     */
    AudioVocClass* Voc = nullptr;

    /**
     *  Active event handle for this voc, or invalid when not playing.
     *  Operations route through AudioEventSystem so the event can swap
     *  underlying samples (decay tail, looped retrigger) without us caring.
     */
    AudioEventHandle Handle{};
};

namespace IonAmbient
{
VocType Voc_Type();
bool Is_Available();
bool Is_Playing();
bool Start();
bool Stop();

/**
 *  Music volume filter for the ion storm duck. Whenever the music group volume
 *  is set from the options (AudioThemeClass::Set_Volume), it must pass through
 *  here so that a slider change during a storm keeps the duck applied and the
 *  post-storm restore picks up the new setting.
 */
float Filter_Music_Volume(float volume);
}
