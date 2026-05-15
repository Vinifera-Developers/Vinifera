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

#include <objidl.h>
#include <vector>


namespace {

    struct TrackedStaticSound {
        AudioEventHandle Handle;
        Coord Position;
        VocType Voc;
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

    AudioEventHandle handle = AudioEventSystem::Start(*AudioVocs[voc], coord, -1, 1.0f, 0.0f);
    if (!handle.Is_Valid()) {
        return false;
    }

    TrackedSounds.emplace_back(handle, coord, voc);
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


/**
 *  Writes the currently-playing tracked static sounds to the save stream so
 *  they can be replayed on load. Handles are intentionally not persisted -
 *  miniaudio handle IDs are process-local.
 *
 *  @author: ZivDero
 */
void Save_Tracked_Static_Sounds(IStream* stm)
{
    uint32_t count = static_cast<uint32_t>(TrackedSounds.size());
    stm->Write(&count, sizeof(count), nullptr);

    for (auto& entry : TrackedSounds) {
        stm->Write(&entry.Voc, sizeof(entry.Voc), nullptr);
        stm->Write(&entry.Position, sizeof(entry.Position), nullptr);
    }
}


/**
 *  Rebuilds the tracked static sounds list from the save stream by re-issuing
 *  each saved sound. Entries whose VocType is out of range in the current
 *  SOUND.INI are silently dropped by Play_Tracked_Static_Sound.
 *
 *  @author: ZivDero
 */
void Load_Tracked_Static_Sounds(IStream* stm)
{
    Clear_Tracked_Static_Sounds();

    uint32_t count = 0;
    stm->Read(&count, sizeof(count), nullptr);

    for (uint32_t i = 0; i < count; i++) {
        VocType voc;
        Coord pos;
        stm->Read(&voc, sizeof(voc), nullptr);
        stm->Read(&pos, sizeof(pos), nullptr);
        Play_Tracked_Static_Sound(voc, pos);
    }
}
