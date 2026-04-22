/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Live playback instance of a voc sound owned by a long-lived caller.
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
 *  Manages playback of an ambient sound with fade-in/out support.
 */
class AudioAmbientClass
{
public:
    AudioAmbientClass(VocType voc);
    AudioAmbientClass(const char* name);
    ~AudioAmbientClass();

    bool Start(Coord const& coord = COORD_NONE);
    bool Stop(float fade_out_seconds = 1.0f);

    bool Update_Position(Coord coord);

    bool Pause();
    bool Resume();

    bool Is_Playing();

    void Set_Fade_In(float seconds) { FadeInSeconds = seconds; }

private:
    /**
     *  Bound voc definition pointer.
     */
    AudioVocClass* Voc = nullptr;

    /**
     *  Active audio handle, or INVALID_AUDIO_HANDLE_ID when not playing.
     */
    AudioHandleID Handle = INVALID_AUDIO_HANDLE_ID;

    /**
     *  Last coordinate passed to Start()/Update_Position(). COORD_NONE for
     *  non-positional playback.
     */
    Coord LastCoord = COORD_NONE;

    /**
     *  Fade-in applied on Start().
     */
    float FadeInSeconds = 1.0f;
};

namespace IonAmbient
{
VocType Voc_Type();
bool Is_Available();
bool Is_Playing();
bool Start();
bool Stop();
}
