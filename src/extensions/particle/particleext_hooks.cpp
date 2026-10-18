/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended ParticleClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "aircraft.h"
#include "hooker.h"
#include "house.h"
#include "housetype.h"
#include "particlesys.h"
#include "rules.h"
#include "scenario.h"
#include "syringe.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "unit.h"
#include "techno.h"
#include "particle.h"
#include "particletypeext.h"
#include "extension.h"
#include "visceroid_ownership.h"
#include <unordered_map>


namespace {
struct MutationSnapshot {
    bool Eligible;
    HouseClass* Owner;
};
// A native Take_Damage call has six stack arguments (24 bytes). Its post-call
// stack pointer distinguishes nested damage calls without retaining the victim.
std::unordered_map<unsigned, MutationSnapshot> MutationSnapshots;
}

DEFINE_HOOK(0x005A3896, _ParticleClass_Gas_Capture_Victim_Owner, 6)
{
    GET(ObjectClass*, victim, ESI);
    GET(ParticleClass*, particle, EBP);
    const bool techno = victim->Is_Techno();
    const bool eligible = victim->RTTI == RTTI_INFANTRY || (techno && victim->TClass->IsCrew);
    HouseClass* owner = nullptr;
    switch (Extension::Fetch(particle->Class)->VisceroidOwner) {
    case VisceroidOwnership::Owner::Invoker:
        owner = VisceroidOwnership::Origin(particle);
        break;
    case VisceroidOwnership::Owner::Victim:
        if (techno) owner = static_cast<TechnoClass*>(victim)->House;
        break;
    default:
        break;
    }
    MutationSnapshots[R->ESP() + 24] = {eligible, owner};
    return 0; // Replay the original virtual Take_Damage call.
}

/**
 *  Fixes a bug where gas clouds are able to turn everything into visceroids, including
 *  non-crewed vehicles and even terrain objects.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x005A389C, _ParticleClass_Smoke_And_WeakGas_Behaviour_AI_Tiberium_Death_Patch, 0)
{
    GET(ResultType, result, EAX);
    GET_STACK(ObjectClass*, nextobject, 0x40);

    enum {
        ContinueVisceroidPlacement = 0x005A38FC,
        SkipToNextObjectOnCell = 0x005A3965
    };

    R->ESI(nextobject);

    const auto pending = MutationSnapshots.find(R->ESP());
    MutationSnapshot mutation = {false, nullptr};
    if (pending != MutationSnapshots.end()) {
        mutation = pending->second;
        MutationSnapshots.erase(pending);
    }

    if (result != RESULT_DESTROYED) {
        // Object was not destroyed, do not create visceroid.
        return SkipToNextObjectOnCell;
    }

    if (!Scen->IsTiberiumDeathToVisceroid) {
        // Visceroids spawning from Tiberium death is disabled, do not create visceroid.
        return SkipToNextObjectOnCell;
    }

    if (!mutation.Eligible) return SkipToNextObjectOnCell;

    HouseClass* owner = mutation.Owner;
    if (!owner) owner = House_From_HousesType(HouseTypeClass::From_Name("Neutral"));
    UnitClass* visceroid = new UnitClass(Rule->SmallVisceroid, owner);
    if (visceroid == nullptr) {
        // No visceroid was created.
        return SkipToNextObjectOnCell;
    }

    R->EDI(visceroid);
    return ContinueVisceroidPlacement;
}


/**
 *  Main function for patching the hooks.
 */
void ParticleClassExtension_Hooks()
{
    VisceroidOwnership::Hooks();
}
