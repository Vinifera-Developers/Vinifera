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

#include "audio_ambient.h"

#include <vector>


namespace {

    struct TrackedStaticSound {
        AudioAmbientClass* Ambient;
        Coord Position;
    };

    std::vector<TrackedStaticSound> TrackedSounds;
}


bool Play_Tracked_Static_Sound(VocType voc, Coord const& coord)
{
    AudioAmbientClass* ambient = new AudioAmbientClass(voc);
    if (!ambient->Start(coord)) {
        delete ambient;
        return false;
    }

    TrackedSounds.emplace_back(ambient, coord);
    return true;
}


int Stop_Tracked_Static_Sounds_At(Coord const& coord, float fade_out_seconds)
{
    int stopped = 0;
    auto it = TrackedSounds.begin();
    while (it != TrackedSounds.end()) {
        if (it->Position == coord) {
            it->Ambient->Stop(fade_out_seconds);
            delete it->Ambient;
            it = TrackedSounds.erase(it);
            ++stopped;
        } else {
            ++it;
        }
    }
    return stopped;
}


void Tracked_Static_Sounds_AI()
{
    auto it = TrackedSounds.begin();
    while (it != TrackedSounds.end()) {
        if (!it->Ambient->Is_Playing()) {
            delete it->Ambient;
            it = TrackedSounds.erase(it);
        } else {
            it->Ambient->Update_Position(it->Position);
            ++it;
        }
    }
}


void Clear_Tracked_Static_Sounds()
{
    for (auto& entry : TrackedSounds) {
        entry.Ambient->Stop(0.0f);
        delete entry.Ambient;
    }
    TrackedSounds.clear();
}
