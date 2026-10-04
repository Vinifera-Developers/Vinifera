// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "status_timing.h"
#include "vector.h"

class CCINIClass;
class TechnoClass;
class TechnoClassExtension;
class TechnoTypeClassExtension;
class WarheadTypeClass;
class ParticleClass;
struct IStream;
class CRCEngine;

namespace StatusEffects {
enum class Response : int { Inherit, Damage, Heal, Ignore };
struct Definition {
    char Name[128];
    int EligibleMask;
    int Damage;
    int Count;
    int Interval;
    int FirstDelay;
    Response TiberiumHeal;
    bool ApplyStatuses;
    WarheadTypeClass* Warhead;
    bool operator==(const Definition&) const = default;
};
struct Binding {
    int Effect;
    Response TiberiumHeal;
    bool Persist;
    bool ReplaceLegacy;
    bool operator==(const Binding&) const = default;
};
struct TargetRule {
    int Effect;
    int Eligible; // -1 inherits the definition's category mask.
    bool Immune;
    int DamagePercent;
    Response HealthResponse;
    bool operator==(const TargetRule&) const = default;
};
struct Instance {
    int Effect;
    Response TiberiumHeal;
    bool Persist;
    bool AffectsAllies;
    Timing Clock;
    int ExposureFrame;
    int SourceHouse;
    TechnoClass* Invoker;
    bool operator==(const Instance&) const = default;
};
constexpr Binding EmptyBinding() { return {-1, Response::Inherit, true, false}; }

void Read_Definitions(CCINIClass& ini);
void Read_Binding(CCINIClass& ini, const char* section, Binding& binding, bool environment);
void Read_Target(CCINIClass& ini, const char* section, DynamicVectorClass<TargetRule>& rules);
void Queue_Application(TechnoClass* target, const Binding& binding, TechnoClass* invoker,
                       bool affects_allies = true, bool exposure = false);
void Weapon_Impact(TechnoClass* target, const WarheadTypeClass* warhead, TechnoClass* invoker);
void Complete_Frame();
void Detach(TechnoClass* object);
void Transfer(TechnoClass* from, TechnoClass* to);
bool Replaces_Tiberium_Health(TechnoClass* target);
bool Replaces_Gas_Health(ParticleClass* particle);
void CRC(const Instance& state, CRCEngine& crc);
unsigned long Network_CRC(unsigned long native_crc);
HRESULT Load_Instances(IStream* stream, DynamicVectorClass<Instance>& states);
HRESULT Save_Instances(IStream* stream, const DynamicVectorClass<Instance>& states);
HRESULT Load_Definitions(IStream* stream, DynamicVectorClass<Definition>& definitions);
HRESULT Save_Definitions(IStream* stream, const DynamicVectorClass<Definition>& definitions);
HRESULT Load_Targets(IStream* stream, DynamicVectorClass<TargetRule>& rules);
HRESULT Save_Targets(IStream* stream, const DynamicVectorClass<TargetRule>& rules);
}
