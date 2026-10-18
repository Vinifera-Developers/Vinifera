// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace StatusEffects {
struct Timing {
    int Remaining;
    int Delay;
    int LastFrame;
    bool operator==(const Timing&) const = default;
};
inline void Refresh(Timing& state, int count)
{
    if (state.Remaining < count) state.Remaining = count;
}
// Called once at simulation-frame completion, including paused targets.
inline bool Advance(Timing& state, int frame, int interval, bool paused)
{
    if (state.Remaining <= 0) return false;
    if (frame > state.LastFrame && !paused && state.Delay > 0) --state.Delay;
    state.LastFrame = frame;
    if (paused || state.Delay > 0) return false;
    --state.Remaining;
    state.Delay = interval;
    return true;
}
}
