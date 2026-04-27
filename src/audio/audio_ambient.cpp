/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Live playback instance of a voc sound owned by a long-lived caller.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_ambient.h"

#include "asserthandler.h"
#include "audio_debug.h"
#include "audio_event.h"
#include "audio_manager.h"
#include "audio_voc.h"
#include "tibsun_globals.h"


/**
 *  Constructor; initializes the ambient sound from a VocType.
 *
 *  @author: ZivDero
 */
AudioAmbientClass::AudioAmbientClass(VocType voc)
{
    ASSERT(voc >= VOC_FIRST && voc < AudioVocs.Count());

    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        Voc = AudioVocs[voc];
    }
}

AudioAmbientClass::AudioAmbientClass(const char* name) :
    AudioAmbientClass(AudioVocClass::From_Name(name))
{
}


/**
 *  Destructor; stops playback (immediate, no fade) and unregisters this
 *  ambient from the global list.
 *
 *  @author: ZivDero
 */
AudioAmbientClass::~AudioAmbientClass()
{
    if (Handle.Is_Valid()) {
        AudioEventSystem::Stop(Handle, 0.0f);
        Handle = INVALID_AUDIO_EVENT_HANDLE;
    }
}


/**
 *  Starts playback of the ambient sound with fade-in.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Start(Coord const& coord)
{
    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return false;
    }

    if (Handle.Is_Valid()) {
        if (AudioEventSystem::Is_Playing(Handle)) {
            return true;
        }
        Handle = INVALID_AUDIO_EVENT_HANDLE;
    }

    if (Voc == nullptr) {
        return false;
    }

    Handle = Voc->Internal_Play(coord, -1, 1.0f, FadeInSeconds);

    if (!Handle.Is_Valid()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "AudioAmbient::Start - Failed to start \"%s\"!\n", Voc->Name.c_str());
        return false;
    }

    return true;
}


/**
 *  Stops playback of the ambient sound with a fade-out (event system honors
 *  Decay control flag and fade duration).
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Stop(float fade_out_seconds)
{
    if (!Handle.Is_Valid()) {
        return false;
    }

    const bool ok = AudioEventSystem::Stop(Handle, fade_out_seconds);
    Handle = INVALID_AUDIO_EVENT_HANDLE;
    return ok;
}


/**
 *  Updates the position of the ambient sound.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Update_Position(Coord coord)
{
    if (!Handle.Is_Valid() || Voc == nullptr) {
        return false;
    }

    if (AudioEventSystem::Update_Position(Handle, coord)) {
        return true;
    }

    /**
     *  The event system reports the event no longer exists (finished or
     *  was cleared). Drop our stale handle so subsequent calls don't
     *  keep probing it.
     */
    Handle = INVALID_AUDIO_EVENT_HANDLE;
    return false;
}


/**
 *  Checks whether the ambient sound is currently playing.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Is_Playing()
{
    if (!Handle.Is_Valid()) {
        return false;
    }

    if (AudioEventSystem::Is_Playing(Handle)) {
        return true;
    }

    /**
     *  Event finished — clear our cached handle so we don't keep referring
     *  to a dead event.
     */
    Handle = INVALID_AUDIO_EVENT_HANDLE;
    return false;
}

namespace IonAmbient
{
std::unique_ptr<AudioAmbientClass> Handle = nullptr;
float OldMusicVolume = 1.0f;
} // namespace IonAmbient


VocType IonAmbient::Voc_Type()
{
    return AudioVocClass::From_Name("IONSTORM");
}


bool IonAmbient::Is_Available()
{
    VocType ion = Voc_Type();
    return ion >= VOC_FIRST && ion < AudioVocs.Count() && AudioVocs[ion] != nullptr && AudioVocs[ion]->Is_Available();
}


bool IonAmbient::Is_Playing()
{
    return Handle != nullptr && Handle->Is_Playing();
}


bool IonAmbient::Start()
{
    if (!Is_Available()) return false;

    if (Handle == nullptr) {
        Handle = std::make_unique<AudioAmbientClass>(Voc_Type());
    }

    if (!Handle->Is_Playing()) {
        Handle->Start();

        // Reduce music to 1/3 volume so the ion storm ambient is clearly audible.
        OldMusicVolume = AudioManager.Get_Group_Volume(AUDIO_GROUP_MUSIC);
        AudioManager.Set_Group_Volume(AUDIO_GROUP_MUSIC, OldMusicVolume * 0.33f);
    }

    return true;
}


bool IonAmbient::Stop()
{
    if (!Is_Available()) return false;

    if (Handle == nullptr) return true;

    if (Handle->Is_Playing()) {
        Handle->Stop();
        AudioManager.Set_Group_Volume(AUDIO_GROUP_MUSIC, OldMusicVolume);
        Handle.reset();
    }

    return true;
}
