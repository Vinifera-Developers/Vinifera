// SPDX-License-Identifier: GPL-3.0-or-later
#include "always.h"
#include "visceroid_ownership.h"
#include "abstract.h"
#include "bullet.h"
#include "hooker.h"
#include "house.h"
#include "particle.h"
#include "particlesys.h"
#include "syringe.h"
#include "techno.h"
#include "unit.h"
#include "vinifera_globals.h"

namespace {
// Each continuation follows whole, relative-free native instructions. The
// wrappers keep every original return path inside an RAII attribution scope.
__declspec(naked) void Fire_Original()
{
    __asm { push ebp }
    __asm { mov ebp, esp }
    __asm { and esp, -8 }
    __asm { push 0x00630356 }
    __asm { ret }
}
__declspec(naked) void Bullet_AI_Original()
{
    __asm { push ebp }
    __asm { mov ebp, esp }
    __asm { and esp, -8 }
    __asm { push 0x004446F6 }
    __asm { ret }
}
__declspec(naked) void Bullet_Explosion_Original()
{
    __asm { sub esp, 0x24 }
    __asm { push ebx }
    __asm { push ebp }
    __asm { push 0x004463D5 }
    __asm { ret }
}
__declspec(naked) void Bullet_Create_Original()
{
    __asm { push ecx }
    __asm { push esi }
    __asm { lea eax, [esp + 4] }
    __asm { push 0x00447226 }
    __asm { ret }
}
__declspec(naked) void Particle_AI_Original()
{
    __asm { push esi }
    __asm { mov esi, ecx }
    __asm { mov eax, [esi + 0x4c] }
    __asm { push 0x005A4536 }
    __asm { ret }
}
__declspec(naked) void System_AI_Original()
{
    __asm { push esi }
    __asm { mov esi, ecx }
    __asm { mov eax, [esi + 0x4c] }
    __asm { push 0x005A7426 }
    __asm { ret }
}
__declspec(naked) void Particle_Ctor_Original()
{
    __asm { sub esp, 0x18 }
    __asm { push ebx }
    __asm { push ebp }
    __asm { push 0x005A2C95 }
    __asm { ret }
}
__declspec(naked) void System_Ctor_Original()
{
    __asm { sub esp, 0x18 }
    __asm { push ebx }
    __asm { push ebp }
    __asm { push 0x005A5305 }
    __asm { ret }
}

HouseClass* Techno_House(AbstractClass* object)
{
    if (!object) return nullptr;
    switch (object->Fetch_RTTI()) {
    case RTTI_UNIT: case RTTI_INFANTRY: case RTTI_BUILDING: case RTTI_AIRCRAFT:
        return static_cast<TechnoClass*>(object)->House;
    default: return nullptr;
    }
}

const BulletClass* __fastcall Fire(TechnoClass* object, void*, AbstractClass* target, WeaponSlotType slot)
{
    VisceroidOwnership::Scope context(object->House);
    using Function = const BulletClass* (__thiscall*)(TechnoClass*, AbstractClass*, WeaponSlotType);
    return reinterpret_cast<Function>(&Fire_Original)(object, target, slot);
}
void __fastcall Bullet_AI(BulletClass* object, void*)
{
    VisceroidOwnership::Scope context(VisceroidOwnership::Origin(object));
    using Function = void (__thiscall*)(BulletClass*);
    reinterpret_cast<Function>(&Bullet_AI_Original)(object);
}
void __fastcall Bullet_Explosion(BulletClass* object, void*, bool forced)
{
    VisceroidOwnership::Scope context(VisceroidOwnership::Origin(object));
    using Function = void (__thiscall*)(BulletClass*, bool);
    reinterpret_cast<Function>(&Bullet_Explosion_Original)(object, forced);
}
BulletClass* __fastcall Bullet_Create(BulletTypeClass* type, AbstractClass* target, TechnoClass* source, int damage, WarheadTypeClass* warhead, int speed, int range, bool bright)
{
    HouseClass* house = VisceroidOwnership::Has_Context() ? VisceroidOwnership::Current() : Techno_House(source);
    VisceroidOwnership::Scope context(house);
    using Function = BulletClass* (__fastcall*)(BulletTypeClass*, AbstractClass*, TechnoClass*, int, WarheadTypeClass*, int, int, bool);
    auto bullet = reinterpret_cast<Function>(&Bullet_Create_Original)(type, target, source, damage, warhead, speed, range, bright);
    VisceroidOwnership::Set(bullet, house);
    return bullet;
}
void __fastcall Particle_AI(ParticleClass* object, void*)
{
    VisceroidOwnership::Scope context(VisceroidOwnership::Origin(object));
    using Function = void (__thiscall*)(ParticleClass*);
    reinterpret_cast<Function>(&Particle_AI_Original)(object);
}
void __fastcall System_AI(ParticleSystemClass* object, void*)
{
    VisceroidOwnership::Scope context(VisceroidOwnership::Origin(object));
    using Function = void (__thiscall*)(ParticleSystemClass*);
    reinterpret_cast<Function>(&System_AI_Original)(object);
}
ParticleClass* __fastcall Particle_Ctor(ParticleClass* object, void*, ParticleTypeClass* type, Coord& coord, Coord& target, ParticleSystemClass* system)
{
    HouseClass* house = VisceroidOwnership::Has_Context() ? VisceroidOwnership::Current() : VisceroidOwnership::Origin(system);
    VisceroidOwnership::Scope context(house);
    using Function = ParticleClass* (__thiscall*)(ParticleClass*, ParticleTypeClass*, Coord&, Coord&, ParticleSystemClass*);
    auto result = reinterpret_cast<Function>(&Particle_Ctor_Original)(object, type, coord, target, system);
    VisceroidOwnership::Set(result, house);
    return result;
}
ParticleSystemClass* __fastcall System_Ctor(ParticleSystemClass* object, void*, const ParticleSystemTypeClass* type, const Coord& coord, AbstractClass* target, AbstractClass* source, const Coord& destination)
{
    HouseClass* house = VisceroidOwnership::Has_Context() ? VisceroidOwnership::Current() : Techno_House(source);
    VisceroidOwnership::Scope context(house);
    using Function = ParticleSystemClass* (__thiscall*)(ParticleSystemClass*, const ParticleSystemTypeClass*, const Coord&, AbstractClass*, AbstractClass*, const Coord&);
    auto result = reinterpret_cast<Function>(&System_Ctor_Original)(object, type, coord, target, source, destination);
    VisceroidOwnership::Set(result, house);
    return result;
}
}

// Base destruction covers all captured effect classes, including heap reuse.
DEFINE_HOOK(0x00405B90, _AbstractClass_Destructor_Mutation_Origin, 6)
{
    VisceroidOwnership::Forget(reinterpret_cast<AbstractClass*>(R->ECX()));
    return 0;
}

// Native child constructors omit the parent system; copy from the particle,
// including when MasterParticle contains clouds from several different houses.
DEFINE_HOOK(0x005A5D7A, _ParticleSystem_Gas_Child_Mutation_Origin, 6)
{
    auto child = reinterpret_cast<ParticleClass*>(R->EAX());
    VisceroidOwnership::Set(child, VisceroidOwnership::Origin(reinterpret_cast<ParticleClass*>(R->ESI())));
    R->EBX(child);
    return child ? 0x005A5D80 : 0x005A5DF4;
}
DEFINE_HOOK(0x005A64B5, _ParticleSystem_Smoke_First_Child_Mutation_Origin, 6)
{
    VisceroidOwnership::Set(reinterpret_cast<ParticleClass*>(R->EAX()), VisceroidOwnership::Origin(reinterpret_cast<ParticleClass*>(R->ESI())));
    return 0;
}
DEFINE_HOOK(0x005A655D, _ParticleSystem_Smoke_Second_Child_Mutation_Origin, 6)
{
    auto child = reinterpret_cast<ParticleClass*>(R->EAX());
    VisceroidOwnership::Set(child, VisceroidOwnership::Origin(reinterpret_cast<ParticleClass*>(R->ESI())));
    R->EBP(child);
    return child ? 0x005A6563 : 0x005A65DB;
}
DEFINE_HOOK(0x005A736A, _ParticleSystem_Web_Child_Mutation_Origin, 6)
{
    auto child = reinterpret_cast<ParticleClass*>(R->EAX());
    VisceroidOwnership::Set(child, VisceroidOwnership::Origin(reinterpret_cast<ParticleClass*>(R->ESI())));
    R->EBX(child);
    return child ? 0x005A7370 : 0x005A73E4;
}

DEFINE_HOOK(0x005B58C0, _Compute_Game_CRC_Mutation_Origin, 6)
{
    R->ECX(VisceroidOwnership::Network_CRC(R->ECX()));
    return 0;
}

// Reject cross-house partners in discovery, movement admission and commit.
DEFINE_HOOK(0x0064EE65, _Visceroid_Discover_Same_House, 6)
{
    GET(UnitClass*, unit, ESI);
    GET(UnitClass*, partner, EDI);
    if (unit->House != partner->House) return 0x0064EE75;
    // Do not commandeer a human player's attack/guard/move order.
    if (partner->House && partner->House->IsHuman &&
        (partner->TarCom || partner->NavCom ||
         (partner->CurrentMission != MISSION_NONE && partner->CurrentMission != MISSION_GUARD))) return 0x0064EE75;
    return 0;
}
DEFINE_HOOK(0x006557B9, _Visceroid_Enter_Same_House, 6)
{
    GET(UnitClass*, unit, EBX);
    GET(UnitClass*, occupier, ESI);
    // AL is the small-visceroid flag tested by the displaced JNE.
    return R->AL() && unit->House == occupier->House ? 0x006554A5 : 0x006557BF;
}
DEFINE_HOOK(0x0065124B, _Visceroid_Merge_Same_House, 6)
{
    GET(UnitClass*, unit, EBP);
    GET(UnitClass*, partner, ESI);
    return unit->House == partner->House ? 0 : 0x0065130C;
}
DEFINE_HOOK(0x0064EDC8, _Visceroid_Preserve_Human_Orders, 6)
{
    GET(UnitClass*, unit, ESI);
    if (unit->House && unit->House->IsHuman &&
        (unit->TarCom || unit->NavCom ||
         (unit->CurrentMission != MISSION_NONE && unit->CurrentMission != MISSION_GUARD))) return 0x0064EF01;
    return 0;
}
DEFINE_HOOK(0x0064EE7B, _Visceroid_Human_Idle, 7)
{
    GET(UnitClass*, unit, ESI);
    // Preserve same-house idle discovery above, but suppress wildlife roaming
    // and healing orders for every human house (including remote players).
    return unit->House && unit->House->IsHuman ? 0x0064EF01 : 0;
}

namespace VisceroidOwnership {
void Hooks()
{
    Patch_Jump(0x00630350, &Fire);
    Patch_Jump(0x004446F0, &Bullet_AI);
    Patch_Jump(0x004463D0, &Bullet_Explosion);
    Patch_Jump(0x00447220, &Bullet_Create);
    Patch_Jump(0x005A4530, &Particle_AI);
    Patch_Jump(0x005A7420, &System_AI);
    Patch_Jump(0x005A2C90, &Particle_Ctor);
    Patch_Jump(0x005A5300, &System_Ctor);
}
}
