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
#include "audio_manager.h"
#include "audio_voc.h"
#include "tibsun_globals.h"


DynamicVectorClass<AudioAmbientClass*> AudioAmbients;


/**
 *  Constructor; initializes the ambient sound and registers it.
 *
 *  @author: ZivDero
 */
AudioAmbientClass::AudioAmbientClass(VocType voc)
{
    ASSERT(voc >= VOC_FIRST && voc < AudioVocs.Count());

    if (voc >= VOC_FIRST && voc < AudioVocs.Count()) {
        Voc = AudioVocs[voc];
    }

    AudioAmbients.Add(this);
}

AudioAmbientClass::AudioAmbientClass(const char* name) :
    AudioAmbientClass(AudioVocClass::From_Name(name))
{
}


/**
 *  Destructor; unregisters this ambient sound from the global list.
 *
 *  @author: ZivDero
 */
AudioAmbientClass::~AudioAmbientClass()
{
    if (Handle != INVALID_AUDIO_HANDLE_ID) {
        AudioManager.Request_Stop(Handle, 0.0f);
        Handle = INVALID_AUDIO_HANDLE_ID;
    }

    AudioAmbients.Delete(this);
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

    if (Handle != INVALID_AUDIO_HANDLE_ID) {
        if (AudioManager.Query_Is_Playing(Handle)) {
            return true;
        }
        Handle = INVALID_AUDIO_HANDLE_ID;
    }

    if (Voc == nullptr) {
        return false;
    }

    LastCoord = coord;

    Handle = Voc->Internal_Play(LastCoord, 0, 1.0f, FadeInSeconds);

    if (Handle == INVALID_AUDIO_HANDLE_ID) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_VOC, "AudioAmbient::Start - Failed to start \"%s\"!\n", Voc->Name.c_str());
        return false;
    }

    return true;
}


/**
 *  Stops playback of the ambient sound with a fade-out.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Stop(float fade_out_seconds)
{
    if (Handle == INVALID_AUDIO_HANDLE_ID) {
        return false;
    }

    bool ok = AudioManager.Request_Stop(Handle, fade_out_seconds);
    Handle = INVALID_AUDIO_HANDLE_ID;
    return ok;
}


/**
 *  Updates the position of the ambient sound.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Update_Position(Coord coord)
{
    if (Handle == INVALID_AUDIO_HANDLE_ID || Voc == nullptr) {
        return false;
    }

    if (!AudioManager.Query_Is_Playing(Handle)) {
        Handle = INVALID_AUDIO_HANDLE_ID;
        return false;
    }

    LastCoord = coord;

    float vol = Voc->Get_Volume();
    float pan = 0.0f;

    if (coord != COORD_NONE) {
        Voc->Calculate_Pan_And_Volume(coord, pan, vol);
    }

    const bool volume_ok = AudioManager.Set_Volume(Handle, vol);
    const bool pan_ok = AudioManager.Set_Pan(Handle, pan);
    return volume_ok && pan_ok;
}


/**
 *  Pauses playback of the ambient sound.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Pause()
{
    return Handle != INVALID_AUDIO_HANDLE_ID && AudioManager.Request_Pause(Handle);
}


/**
 *  Resumes playback of a paused ambient sound.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Resume()
{
    return Handle != INVALID_AUDIO_HANDLE_ID && AudioManager.Request_Resume(Handle);
}


/**
 *  Checks whether the ambient sound is currently playing.
 *
 *  @author: ZivDero
 */
bool AudioAmbientClass::Is_Playing()
{
    if (Handle == INVALID_AUDIO_HANDLE_ID) {
        return false;
    }

    if (!AudioManager.Query_Is_Playing(Handle)) {
        Handle = INVALID_AUDIO_HANDLE_ID;
        return false;
    }

    return true;
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
    
        // Reduce the music group to 1/3 so it plays softly in the background.
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
    
        // Return the music group back to the original volume.
        AudioManager.Set_Group_Volume(AUDIO_GROUP_MUSIC, OldMusicVolume);

        // Destroy the ambient
        Handle.reset();
    }

    return true;
}
