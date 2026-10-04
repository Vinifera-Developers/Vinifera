// SPDX-License-Identifier: GPL-3.0-or-later
#include "always.h"
#include "status_effects.h"
#include "aircraftext.h"
#include "buildingext.h"
#include "cell.h"
#include "ccini.h"
#include "extension.h"
#include "extension_globals.h"
#include "fatal.h"
#include "house.h"
#include "infantryext.h"
#include "mouse.h"
#include "particle.h"
#include "particletype.h"
#include "particletypeext.h"
#include "rulesext.h"
#include "technoext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tiberiumext.h"
#include "tibsun_globals.h"
#include "unitext.h"
#include "vinifera_saveload.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "wwcrc.h"
#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_set>

namespace StatusEffects {
namespace {
struct Request {
    TechnoClass* Target;
    Binding Policy;
    TechnoClass* Invoker;
    int House;
    bool Allies;
    bool Exposure;
    int Frame;
    int TargetRTTI;
    int TargetID;
};
std::vector<Request> Requests;
std::unordered_set<TechnoClass*> FrameObjects;
bool TickOrigin = false;
bool AllowTickApplications = false;

[[noreturn]] void Invalid(const char* section, const char* tag)
{
    Fatal("Invalid status-effect configuration: [%s] %s", section, tag);
    std::abort();
}
bool Text(CCINIClass& ini, const char* section, const char* tag, char (&value)[128])
{
    const int size = ini.Get_String(section, tag, "", value, sizeof(value));
    if (size >= 127) Invalid(section, tag);
    return size > 0;
}
int Integer(CCINIClass& ini, const char* section, const char* tag, int fallback, bool percent = false)
{
    char value[128];
    if (!Text(ini, section, tag, value)) return fallback;
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(value, &end, 10);
    if (percent && *end == '%') ++end;
    while (*end == ' ' || *end == '\t') ++end;
    if (end == value || *end || errno || parsed > INT_MAX || parsed < INT_MIN) Invalid(section, tag);
    return static_cast<int>(parsed);
}
bool Boolean(CCINIClass& ini, const char* section, const char* tag, bool fallback)
{
    char value[128];
    if (!Text(ini, section, tag, value)) return fallback;
    if (!_stricmp(value, "yes") || !_stricmp(value, "true") || !strcmp(value, "1")) return true;
    if (!_stricmp(value, "no") || !_stricmp(value, "false") || !strcmp(value, "0")) return false;
    Invalid(section, tag);
}
Response ReadResponse(CCINIClass& ini, const char* section, const char* tag, Response fallback)
{
    char value[128];
    if (!Text(ini, section, tag, value)) return fallback;
    if (!_stricmp(value, "Inherit")) return Response::Inherit;
    if (!_stricmp(value, "Damage")) return Response::Damage;
    if (!_stricmp(value, "Heal")) return Response::Heal;
    if (!_stricmp(value, "Ignore")) return Response::Ignore;
    Invalid(section, tag);
}
int Find(const char* name)
{
    const auto& definitions = RuleExtension->StatusDefinitions;
    for (int i = 0; i < definitions.Count(); ++i)
        if (!_stricmp(name, definitions[i].Name)) return i;
    return -1;
}
int Mask(RTTIType type)
{
    switch (type) {
        case RTTI_INFANTRY: return 1;
        case RTTI_UNIT: return 2;
        case RTTI_AIRCRAFT: return 4;
        case RTTI_BUILDING: return 8;
        default: return 0;
    }
}
TargetRule TargetSettings(TechnoClass* object, int effect)
{
    const auto& rules = Extension::Fetch(object->TClass)->StatusRules;
    for (int i = 0; i < rules.Count(); ++i) if (rules[i].Effect == effect) return rules[i];
    return {effect, -1, false, 100, Response::Inherit};
}
bool Eligible(TechnoClass* object, int effect)
{
    if (!object || !object->IsActive || object->Strength <= 0 ||
        effect < 0 || effect >= RuleExtension->StatusDefinitions.Count()) return false;
    const auto settings = TargetSettings(object, effect);
    const bool category = (RuleExtension->StatusDefinitions[effect].EligibleMask & Mask(object->Fetch_RTTI())) != 0;
    return !settings.Immune && (settings.Eligible < 0 ? category : settings.Eligible != 0);
}
HouseClass* House(int id)
{
    return id >= 0 && id < Houses.Count() ? Houses[id] : nullptr;
}
bool Allied(int source_house, TechnoClass* target)
{
    auto house = House(source_house);
    return house && target->House && house->Is_Ally(target->House);
}
std::vector<TechnoClass*> Objects()
{
    std::vector<TechnoClass*> objects;
    auto append = [&](const auto& heap) {
        for (int i = 0; i < heap.Count(); ++i) objects.push_back(heap[i]->This());
    };
    append(InfantryExtensions); append(UnitExtensions); append(AircraftExtensions); append(BuildingExtensions);
    std::sort(objects.begin(), objects.end(), [](TechnoClass* a, TechnoClass* b) {
        return a->Fetch_RTTI() != b->Fetch_RTTI() ? a->Fetch_RTTI() < b->Fetch_RTTI() : a->Fetch_Heap_ID() < b->Fetch_Heap_ID();
    });
    return objects;
}
// A damage call can delete either the target or another object. Never dereference
// the old extension after it returns without checking the live extension heaps.
bool Live(TechnoClass* object)
{
    return FrameObjects.contains(object);
}
Binding Terrain(TechnoClass* object)
{
    if (object->IsInLimbo || !object->IsDown || object->In_Air() || object->Fetch_RTTI() == RTTI_BUILDING) return EmptyBinding();
    const auto coord = object->Center_Coord();
    auto type = Map[coord].Tiberium_Type_Here();
    if (type < 0 || type >= Tiberiums.Count()) return EmptyBinding();
    return Extension::Fetch(Tiberiums[type])->StatusBinding;
}
void Accept(const Request& request)
{
    auto target = request.Target;
    if (!Eligible(target, request.Policy.Effect) || (!request.Allies && Allied(request.House, target))) return;
    const auto& definition = RuleExtension->StatusDefinitions[request.Policy.Effect];
    const auto response = request.Policy.TiberiumHeal == Response::Inherit ? definition.TiberiumHeal : request.Policy.TiberiumHeal;
    auto& states = Extension::Fetch(target)->StatusInstances;
    for (int i = 0; i < states.Count(); ++i) {
        auto& state = states[i];
        if (state.Effect == request.Policy.Effect && state.TiberiumHeal == response && state.Persist == request.Policy.Persist) {
            Refresh(state.Clock, definition.Count);
            if (request.Exposure) state.ExposureFrame = Frame;
            return;
        }
    }
    Instance state{};
    state.Effect = request.Policy.Effect;
    state.TiberiumHeal = response;
    state.Persist = request.Policy.Persist;
    state.AffectsAllies = request.Allies;
    state.Clock = {definition.Count, definition.FirstDelay, Frame};
    state.ExposureFrame = request.Exposure ? Frame : -1;
    state.SourceHouse = request.House;
    state.Invoker = request.Invoker;
    states.Add(state);
}
HRESULT WriteExact(IStream* stream, const void* data, ULONG size)
{
    ULONG written = 0;
    const auto hr = stream->Write(data, size, &written);
    return FAILED(hr) ? hr : written == size ? S_OK : STG_E_WRITEFAULT;
}
HRESULT ReadExact(IStream* stream, void* data, ULONG size)
{
    ULONG read = 0;
    const auto hr = stream->Read(data, size, &read);
    return FAILED(hr) ? hr : read == size ? S_OK : STG_E_READFAULT;
}
template<class T> HRESULT WriteRecords(IStream* stream, const DynamicVectorClass<T>& records)
{
    int count = records.Count();
    HRESULT hr = WriteExact(stream, &count, sizeof(count));
    if (FAILED(hr)) return hr;
    for (int i = 0; i < count; ++i) {
        hr = WriteExact(stream, &records[i], sizeof(T));
        if (FAILED(hr)) return hr;
    }
    return S_OK;
}
template<class T> HRESULT ReadRecords(IStream* stream, DynamicVectorClass<T>& records)
{
    int count = 0;
    HRESULT hr = ReadExact(stream, &count, sizeof(count));
    if (FAILED(hr)) return hr;
    if (count < 0 || count > 65536) return E_FAIL;
    new (&records) DynamicVectorClass<T>();
    if (!records.Resize(count)) return E_OUTOFMEMORY;
    for (int i = 0; i < count; ++i) {
        T item{};
        hr = ReadExact(stream, &item, sizeof(T));
        if (FAILED(hr)) return hr;
        if (!records.Add(item)) return E_OUTOFMEMORY;
    }
    return S_OK;
}
}

void Read_Definitions(CCINIClass& ini)
{
    auto& definitions = RuleExtension->StatusDefinitions;
    char name[128];
    const int count = ini.Entry_Count("StatusEffectTypes");
    for (int i = 0; i < count; ++i) {
        if (!Text(ini, "StatusEffectTypes", ini.Get_Entry("StatusEffectTypes", i), name)) Invalid("StatusEffectTypes", "empty entry");
        if (Find(name) < 0) {
            Definition definition{};
            strcpy_s(definition.Name, name);
            definition.EligibleMask = 1;
            definition.Damage = 1; definition.Count = 1; definition.Interval = 15; definition.FirstDelay = -1;
            definition.TiberiumHeal = Response::Damage;
            definitions.Add(definition);
        }
    }
    for (int i = 0; i < definitions.Count(); ++i) {
        auto& definition = definitions[i];
        const char* section = definition.Name;
        char value[128];
        if (Text(ini, section, "EligibleTypes", value)) {
            definition.EligibleMask = 0;
            char* context = nullptr;
            for (char* token = strtok_s(value, ", \t", &context); token; token = strtok_s(nullptr, ", \t", &context)) {
                if (!_stricmp(token, "Infantry")) definition.EligibleMask |= 1;
                else if (!_stricmp(token, "Vehicle") || !_stricmp(token, "Unit")) definition.EligibleMask |= 2;
                else if (!_stricmp(token, "Aircraft")) definition.EligibleMask |= 4;
                else if (!_stricmp(token, "Building")) definition.EligibleMask |= 8;
                else Invalid(section, "EligibleTypes");
            }
        }
        definition.Damage = Integer(ini, section, "Damage.PerTick", definition.Damage);
        definition.Count = Integer(ini, section, "Tick.Count", definition.Count);
        definition.Interval = Integer(ini, section, "Tick.Interval", definition.Interval);
        if (Text(ini, section, "Tick.FirstDelay", value) && Integer(ini, section, "Tick.FirstDelay", 0) < 0)
            Invalid(section, "Tick.FirstDelay");
        definition.FirstDelay = Integer(ini, section, "Tick.FirstDelay", definition.FirstDelay);
        if (definition.FirstDelay < 0) definition.FirstDelay = definition.Interval;
        definition.TiberiumHeal = ReadResponse(ini, section, "TiberiumHeal.Response", definition.TiberiumHeal);
        definition.ApplyStatuses = Boolean(ini, section, "Tick.ApplyStatuses", definition.ApplyStatuses);
        if (Text(ini, section, "Damage.Warhead", value)) {
            definition.Warhead = nullptr;
            for (int w = 0; w < Warheads.Count(); ++w) if (!_stricmp(Warheads[w]->Name(), value)) definition.Warhead = Warheads[w];
            if (!definition.Warhead) Invalid(section, "Damage.Warhead");
        }
        if (!definition.Warhead) Invalid(section, "Damage.Warhead is required");
        if (definition.Damage <= 0 || definition.Count <= 0 || definition.Interval <= 0 || definition.FirstDelay < 0 ||
            static_cast<long long>(definition.FirstDelay) + static_cast<long long>(definition.Count - 1) * definition.Interval > INT_MAX)
            Invalid(section, "damage/count/interval/delay");
        if (Text(ini, section, "Reapply", value) && _stricmp(value, "Refresh")) Invalid(section, "Reapply (v1 requires Refresh)");
        if (Integer(ini, section, "StackLimit", 1) != 1) Invalid(section, "StackLimit (v1 requires 1)");
        if (Text(ini, section, "Duration", value) || Text(ini, section, "Tick.Duration", value)) Invalid(section, "duration is unsupported; use Tick.Count");
    }
}
void Read_Binding(CCINIClass& ini, const char* section, Binding& binding, bool environment)
{
    char value[128];
    if (Text(ini, section, "Status.Apply", value)) {
        if (!_stricmp(value, "none")) binding = EmptyBinding();
        binding.Effect = !_stricmp(value, "none") ? -1 : Find(value);
        if (binding.Effect < 0 && _stricmp(value, "none")) Invalid(section, "Status.Apply");
        if (environment && binding.Effect >= 0) binding.ReplaceLegacy = true;
    }
    binding.ReplaceLegacy = Boolean(ini, section, "Status.ReplaceLegacyHealth", binding.ReplaceLegacy);
    if (binding.Effect < 0) {
        if (binding.ReplaceLegacy) Invalid(section, "Status.ReplaceLegacyHealth requires Status.Apply");
        return;
    }
    if (environment && !binding.ReplaceLegacy) Invalid(section, "environmental status requires legacy health replacement");
    const std::string name = RuleExtension->StatusDefinitions[binding.Effect].Name;
    binding.TiberiumHeal = ReadResponse(ini, section, (name + ".TiberiumHeal.Response").c_str(), binding.TiberiumHeal);
    binding.Persist = Boolean(ini, section, (name + ".PersistAfterExit").c_str(), binding.Persist);
    if (!environment && !binding.Persist) Invalid(section, "weapon status requires finite persistence");
}
void Read_Target(CCINIClass& ini, const char* section, DynamicVectorClass<TargetRule>& rules)
{
    for (int effect = 0; effect < RuleExtension->StatusDefinitions.Count(); ++effect) {
        TargetRule rule{effect, -1, false, 100, Response::Inherit};
        int index = -1;
        for (int i = 0; i < rules.Count(); ++i) if (rules[i].Effect == effect) { rule = rules[i]; index = i; }
        const std::string prefix = RuleExtension->StatusDefinitions[effect].Name;
        char value[128];
        if (Text(ini, section, (prefix + ".Eligible").c_str(), value)) rule.Eligible = Boolean(ini, section, (prefix + ".Eligible").c_str(), false);
        rule.Immune = Boolean(ini, section, (prefix + ".Immune").c_str(), rule.Immune);
        rule.DamagePercent = Integer(ini, section, (prefix + ".DamageMultiplier").c_str(), rule.DamagePercent, true);
        if (rule.DamagePercent < 0 || rule.DamagePercent > 10000) Invalid(section, "DamageMultiplier (0..10000%)");
        rule.HealthResponse = ReadResponse(ini, section, (prefix + ".Response").c_str(), rule.HealthResponse);
        if (index < 0) rules.Add(rule); else rules[index] = rule;
    }
}
void Queue_Application(TechnoClass* target, const Binding& binding, TechnoClass* invoker, bool allies, bool exposure)
{
    if (!target || binding.Effect < 0 || target->Strength <= 0 || !target->IsActive ||
        (TickOrigin && !AllowTickApplications)) return;
    const int frame = Frame + (TickOrigin ? 1 : 0);
    const int house = invoker && invoker->House ? static_cast<int>(invoker->House->HeapID) : -1;
    // Multiple cloud particles/exposure checks in a frame cannot grow the queue.
    for (const auto& request : Requests)
        if (request.Target == target && request.Frame == frame && request.Policy.Effect == binding.Effect &&
            request.Policy.TiberiumHeal == binding.TiberiumHeal && request.Policy.Persist == binding.Persist &&
            request.Exposure == exposure && request.Invoker == invoker) return;
    Requests.push_back({target, binding, invoker, house, allies, exposure, frame, target->Fetch_RTTI(), target->Fetch_Heap_ID()});
}
void Weapon_Impact(TechnoClass* target, const WarheadTypeClass* warhead, TechnoClass* invoker)
{
    if (!warhead) return;
    const auto extension = Extension::Fetch(warhead);
    Queue_Application(target, extension->StatusBinding, invoker, extension->IsAffectsAllies);
}
bool Replaces_Tiberium_Health(TechnoClass* target)
{
    const auto binding = Terrain(target);
    return binding.Effect >= 0 && binding.ReplaceLegacy;
}
bool Replaces_Gas_Health(ParticleClass* particle)
{
    const auto binding = Extension::Fetch(particle->Class)->StatusBinding;
    return binding.Effect >= 0 && binding.ReplaceLegacy;
}
void Complete_Frame()
{
    if (!RuleExtension || RuleExtension->StatusDefinitions.Count() == 0) return;
    auto objects = Objects();
    FrameObjects.clear();
    FrameObjects.insert(objects.begin(), objects.end());
    for (auto object : objects) {
        if (!object->IsActive || object->IsInLimbo || object->Strength <= 0) continue;
        Queue_Application(object, Terrain(object), nullptr, true, true);
    }
    for (int i = 0; i < Particles.Count(); ++i) {
        auto particle = Particles[i];
        if (!particle->IsActive || particle->IsInLimbo) continue;
        const auto binding = Extension::Fetch(particle->Class)->StatusBinding;
        if (binding.Effect < 0) continue;
        const auto coord = particle->Center_Coord();
        auto occupant = Map[coord].Cell_Occupier();
        while (occupant) {
            if (occupant->Is_Techno() && occupant->IsDown && !occupant->IsInLimbo && !occupant->In_Air())
                Queue_Application(static_cast<TechnoClass*>(occupant), binding, nullptr, true, true);
            occupant = occupant->Next;
        }
    }
    std::stable_sort(Requests.begin(), Requests.end(), [](const Request& a, const Request& b) {
        if (a.Frame != b.Frame) return a.Frame < b.Frame;
        if (a.TargetRTTI != b.TargetRTTI) return a.TargetRTTI < b.TargetRTTI;
        if (a.TargetID != b.TargetID) return a.TargetID < b.TargetID;
        return a.Policy.Effect < b.Policy.Effect;
    });
    auto ready = std::find_if(Requests.begin(), Requests.end(), [](const Request& request) { return request.Frame > Frame; });
    for (auto request = Requests.begin(); request != ready; ++request) if (request->Target) Accept(*request);
    Requests.erase(Requests.begin(), ready);
    for (auto object : objects) {
        if (!Live(object)) continue;
        auto extension = Extension::Fetch(object);
        for (int i = 0; i < extension->StatusInstances.Count();) {
            auto& state = extension->StatusInstances[i];
            if (!Eligible(object, state.Effect) || (!state.Persist && state.ExposureFrame != Frame && !object->IsInLimbo)) {
                extension->StatusInstances.Delete(i); continue;
            }
            const auto definition = RuleExtension->StatusDefinitions[state.Effect];
            const bool paused = object->IsInLimbo || !object->IsDown;
            if (!Advance(state.Clock, Frame, definition.Interval, paused)) { ++i; continue; }
            const auto settings = TargetSettings(object, state.Effect);
            Response response = settings.HealthResponse;
            if (response == Response::Inherit) response = object->TClass->IsTiberiumHeal ? state.TiberiumHeal : Response::Damage;
            int damage = response == Response::Heal ? -definition.Damage :
                static_cast<int>(std::min<long long>(INT_MAX, static_cast<long long>(definition.Damage) * settings.DamagePercent / 100));
            if (response == Response::Ignore || (!state.AffectsAllies && Allied(state.SourceHouse, object))) damage = 0;
            TechnoClass* invoker = state.Invoker;
            if (invoker && (!invoker->House || static_cast<int>(invoker->House->HeapID) != state.SourceHouse)) invoker = nullptr;
            if (damage) {
                TickOrigin = true; AllowTickApplications = definition.ApplyStatuses;
                object->Take_Damage(damage, 0, definition.Warhead, invoker, false);
                TickOrigin = false; AllowTickApplications = false;
                if (!Live(object)) break;
                extension = Extension::Fetch(object);
            }
            if (object->Strength <= 0) { extension->StatusInstances.Clear(); break; }
            if (extension->StatusInstances[i].Clock.Remaining <= 0) extension->StatusInstances.Delete(i);
            else ++i;
        }
    }
}
void Detach(TechnoClass* object)
{
    FrameObjects.erase(object);
    for (auto& request : Requests) {
        if (request.Target == object) request.Target = nullptr;
        if (request.Invoker == object) request.Invoker = nullptr;
    }
}
void Transfer(TechnoClass* from, TechnoClass* to)
{
    auto& source = Extension::Fetch(from)->StatusInstances;
    auto& destination = Extension::Fetch(to)->StatusInstances;
    for (int i = 0; i < source.Count(); ++i) if (Eligible(to, source[i].Effect)) destination.Add(source[i]);
    source.Clear();
    for (auto& request : Requests) if (request.Target == from) {
        request.Target = to; request.TargetRTTI = to->Fetch_RTTI(); request.TargetID = to->Fetch_Heap_ID();
    }
}
void CRC(const Instance& state, CRCEngine& crc)
{
    crc(state.Effect); crc(static_cast<int>(state.TiberiumHeal)); crc(state.Persist); crc(state.AffectsAllies);
    crc(state.Clock.Remaining); crc(state.Clock.Delay); crc(state.Clock.LastFrame); crc(state.ExposureFrame); crc(state.SourceHouse);
    crc(state.Invoker ? static_cast<int>(state.Invoker->Fetch_RTTI()) : -1);
    crc(state.Invoker ? state.Invoker->Fetch_Heap_ID() : -1);
}
unsigned long Network_CRC(unsigned long native_crc)
{
    if (!RuleExtension || !RuleExtension->StatusDefinitions.Count()) return native_crc;
    CRCEngine crc;
    const auto& definitions = RuleExtension->StatusDefinitions;
    crc(definitions.Count());
    for (int i = 0; i < definitions.Count(); ++i) {
        const auto& d = definitions[i];
        crc(d.Name); crc(d.EligibleMask); crc(d.Damage); crc(d.Count);
        crc(d.Interval); crc(d.FirstDelay); crc(static_cast<int>(d.TiberiumHeal));
        crc(d.ApplyStatuses); crc(d.Warhead ? d.Warhead->Fetch_Heap_ID() : -1);
    }
    auto binding = [&](const Binding& b) {
        crc(b.Effect); crc(static_cast<int>(b.TiberiumHeal)); crc(b.Persist); crc(b.ReplaceLegacy);
    };
    for (int i = 0; i < Warheads.Count(); ++i) binding(Extension::Fetch(Warheads[i])->StatusBinding);
    for (int i = 0; i < Tiberiums.Count(); ++i) binding(Extension::Fetch(Tiberiums[i])->StatusBinding);
    for (int i = 0; i < ParticleTypes.Count(); ++i) binding(Extension::Fetch(ParticleTypes[i])->StatusBinding);
    for (auto object : Objects()) {
        const auto& states = Extension::Fetch(object)->StatusInstances;
        crc(static_cast<int>(object->Fetch_RTTI())); crc(object->Fetch_Heap_ID()); crc(states.Count());
        for (int i = 0; i < states.Count(); ++i) CRC(states[i], crc);
        const auto& targets = Extension::Fetch(object->TClass)->StatusRules;
        for (int i = 0; i < targets.Count(); ++i) {
            const auto& t = targets[i];
            crc(t.Effect); crc(t.Eligible); crc(t.Immune); crc(t.DamagePercent); crc(static_cast<int>(t.HealthResponse));
        }
    }
    crc(static_cast<int>(Requests.size()));
    for (const auto& r : Requests) {
        crc(r.Target ? static_cast<int>(r.Target->Fetch_RTTI()) : -1);
        crc(r.Target ? r.Target->Fetch_Heap_ID() : -1); binding(r.Policy);
        crc(r.Invoker ? static_cast<int>(r.Invoker->Fetch_RTTI()) : -1);
        crc(r.Invoker ? r.Invoker->Fetch_Heap_ID() : -1);
        crc(r.House); crc(r.Allies); crc(r.Exposure); crc(r.Frame);
    }
    return ((native_crc << 1) | (native_crc >> 31)) + static_cast<unsigned long>(crc());
}
HRESULT Save_Instances(IStream* stream, const DynamicVectorClass<Instance>& states) { return WriteRecords(stream, states); }
HRESULT Load_Instances(IStream* stream, DynamicVectorClass<Instance>& states)
{
    const auto hr = ReadRecords(stream, states);
    if (FAILED(hr)) return hr;
    for (int i = 0; i < states.Count(); ++i) {
        auto& state = states[i];
        if (state.Effect < 0 || state.Clock.Remaining <= 0 || state.Clock.Delay < 0 ||
            state.TiberiumHeal < Response::Inherit || state.TiberiumHeal > Response::Ignore || state.SourceHouse < -1) return E_FAIL;
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(state.Invoker, "Status.Invoker");
    }
    return S_OK;
}
HRESULT Save_Definitions(IStream* stream, const DynamicVectorClass<Definition>& definitions)
{
    auto hr = WriteRecords(stream, definitions);
    if (FAILED(hr)) return hr;
    const int count = static_cast<int>(Requests.size());
    hr = WriteExact(stream, &count, sizeof(count));
    if (FAILED(hr)) return hr;
    for (const auto& request : Requests) {
        hr = WriteExact(stream, &request, sizeof(request));
        if (FAILED(hr)) return hr;
    }
    return S_OK;
}
HRESULT Load_Definitions(IStream* stream, DynamicVectorClass<Definition>& definitions)
{
    const auto hr = ReadRecords(stream, definitions);
    if (FAILED(hr)) return hr;
    for (int i = 0; i < definitions.Count(); ++i) {
        const auto& d = definitions[i];
        if (!memchr(d.Name, 0, sizeof(d.Name)) || d.Damage <= 0 || d.Count <= 0 || d.Interval <= 0 || d.FirstDelay < 0 ||
            d.TiberiumHeal < Response::Inherit || d.TiberiumHeal > Response::Ignore) return E_FAIL;
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(definitions[i].Warhead, "Status.Warhead");
    }
    Requests.clear();
    int count = 0;
    auto pending_hr = ReadExact(stream, &count, sizeof(count));
    if (FAILED(pending_hr)) return pending_hr;
    if (count < 0 || count > 65536) return E_FAIL;
    Requests.resize(count);
    for (auto& r : Requests) {
        pending_hr = ReadExact(stream, &r, sizeof(r));
        if (FAILED(pending_hr)) return pending_hr;
        if (r.Policy.Effect < 0 || r.Policy.Effect >= definitions.Count()) return E_FAIL;
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(r.Target, "Status.PendingTarget");
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(r.Invoker, "Status.PendingInvoker");
    }
    return S_OK;
}
HRESULT Save_Targets(IStream* stream, const DynamicVectorClass<TargetRule>& rules) { return WriteRecords(stream, rules); }
HRESULT Load_Targets(IStream* stream, DynamicVectorClass<TargetRule>& rules) { return ReadRecords(stream, rules); }
}
