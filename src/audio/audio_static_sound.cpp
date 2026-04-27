/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Tracked positional sound list for PLAY_SOUND_AT / STOP_SOUNDS_AT
 *  triggers.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_static_sound.h"

#include "audio_event.h"
#include "audio_voc.h"

#include <vector>


namespace {

    struct TrackedStaticSound {
        AudioEventHandle Handle;
        Coord Position;
    };

    std::vector<TrackedStaticSound> TrackedSounds;
}


/**
 *  Plays a static positional sound and registers it for later control.
 *
 *  @author: ZivDero
 */
bool Play_Tracked_Static_Sound(VocType voc, Coord const& coord)
{
    if (voc < VOC_FIRST || voc >= AudioVocs.Count()) {
        return false;
    }

    AudioVocClass* voc_def = AudioVocs[voc];
    if (voc_def == nullptr) {
        return false;
    }

    AudioEventHandle handle = AudioEventSystem::Start(*voc_def, coord, -1, 1.0f, 0.0f);
    if (!handle.Is_Valid()) {
        return false;
    }

    TrackedSounds.emplace_back(handle, coord);
    return true;
}


/**
 *  Stops all static sounds playing at a specific coordinate.
 *
 *  @author: ZivDero
 */
int Stop_Tracked_Static_Sounds_At(Coord const& coord, float fade_out_seconds)
{
    int stopped = 0;
    auto it = TrackedSounds.begin();
    while (it != TrackedSounds.end()) {
        if (it->Position == coord) {
            AudioEventSystem::Stop(it->Handle, fade_out_seconds);
            it = TrackedSounds.erase(it);
            ++stopped;
        } else {
            ++it;
        }
    }
    return stopped;
}


/**
 *  Updates all tracked static sounds and removes finished ones.
 *
 *  @author: ZivDero
 */
void Tracked_Static_Sounds_AI()
{
    auto it = TrackedSounds.begin();
    while (it != TrackedSounds.end()) {
        if (!AudioEventSystem::Is_Playing(it->Handle)) {
            it = TrackedSounds.erase(it);
        } else {
            AudioEventSystem::Update_Position(it->Handle, it->Position);
            ++it;
        }
    }
}


/**
 *  Stops and clears all tracked static sounds.
 *
 *  @author: ZivDero
 */
void Clear_Tracked_Static_Sounds()
{
    for (auto& entry : TrackedSounds) {
        AudioEventSystem::Stop(entry.Handle, 0.0f);
    }
    TrackedSounds.clear();
}
