// SPDX-License-Identifier: GPL-3.0-or-later
#include "always.h"
#include "status_effects.h"
#include "syringe.h"
#include "techno.h"
#include "technotype.h"
#include "particle.h"

// The native checksum's last store follows its RNG contribution. ECX is the
// completed checksum; replay the six-byte mov [809894h],ecx after mixing status
// state. This covers both network protocol branches without drawing RNG.
DEFINE_HOOK(0x005B58C0, _Compute_Game_CRC_Status_State, 6)
{
    R->ECX(StatusEffects::Network_CRC(R->ECX()));
    return 0;
}

// Verified TS 2.03 native instruction span (reaudit for each executable):
// mov eax,[esp+14h]; mov dword ptr [esp+10h],6D1398h. Returning zero
// executes those displaced instructions before normal LogicClass::AI cleanup.
DEFINE_HOOK(0x005071E4, _LogicClass_AI_Status_Completion, 12)
{
    StatusEffects::Complete_Frame();
    return 0;
}

// ESI is FootClass*, EAX its TechnoTypeClass*. Preserve the original CL load.
DEFINE_HOOK(0x004A58B9, _FootClass_AI_Status_Tiberium_Healing, 6)
{
    GET(TechnoClass*, object, ESI);
    GET(TechnoTypeClass*, type, EAX);
    if (StatusEffects::Replaces_Tiberium_Health(object)) return 0x004A598F;
    R->ECX((R->ECX() & 0xFFFFFF00u) | static_cast<unsigned>(type->IsTiberiumHeal));
    return 0x004A58BF;
}

// EBP is ParticleClass*. Converted clouds retain movement and aging but skip
// RemainingDC/legacy health handling. Exposure is sampled at frame completion.
DEFINE_HOOK(0x005A3779, _ParticleClass_AI_Status_Gas_Health, 10)
{
    GET(ParticleClass*, particle, EBP);
    if (StatusEffects::Replaces_Gas_Health(particle)) return 0x005A396D;
    return 0;
}
