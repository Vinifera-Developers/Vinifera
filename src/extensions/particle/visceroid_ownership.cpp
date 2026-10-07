// SPDX-License-Identifier: GPL-3.0-or-later
#include "always.h"
#include "visceroid_ownership.h"

#include "abstract.h"
#include "wwcrc.h"
#include "particletypeext.h"
#include "extension.h"
#include "house.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace VisceroidOwnership {
namespace {
struct Record { AbstractClass* Effect; int House; };
std::unordered_map<const AbstractClass*, int> Origins;
std::vector<Record> Loading;
HouseClass* Context = nullptr;
bool ContextActive = false;

HouseClass* House(int id)
{
    for (int i = 0; i < Houses.Count(); ++i)
        if (Houses[i] && static_cast<int>(Houses[i]->HeapID) == id) return Houses[i];
    return nullptr;
}

void Resolve_Loaded()
{
    // Swizzle requests point into Loading. Keep its allocation unchanged until
    // Vinifera has completed native and extension pointer remapping.
    if (Vinifera_PerformingLoad || Loading.empty()) return;
    for (const auto& record : Loading)
        if (record.Effect && House(record.House)) Origins[record.Effect] = record.House;
    Loading.clear();
}

std::vector<Record> Records()
{
    Resolve_Loaded();
    std::vector<Record> records;
    for (const auto& record : Origins)
        records.push_back({const_cast<AbstractClass*>(record.first), record.second});
    std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
        if (a.Effect->Fetch_RTTI() != b.Effect->Fetch_RTTI())
            return a.Effect->Fetch_RTTI() < b.Effect->Fetch_RTTI();
        return a.Effect->Fetch_ID() < b.Effect->Fetch_ID();
    });
    return records;
}
}

void Set(AbstractClass* effect, HouseClass* house)
{
    if (!effect || Vinifera_PerformingLoad) return;
    Resolve_Loaded();
    if (house) Origins[effect] = static_cast<int>(house->HeapID);
    else Origins.erase(effect);
}
HouseClass* Origin(const AbstractClass* effect)
{
    Resolve_Loaded();
    const auto record = Origins.find(effect);
    return record == Origins.end() ? nullptr : House(record->second);
}
HouseClass* Current() { return Context; }
bool Has_Context() { return ContextActive; }
Scope::Scope(HouseClass* house) : Previous(Context), PreviousActive(ContextActive)
{
    Context = house;
    ContextActive = true;
}
Scope::~Scope() { Context = Previous; ContextActive = PreviousActive; }
void Capture(AbstractClass* effect) { Set(effect, Context); }
void Forget(AbstractClass* effect)
{
    if (Vinifera_PerformingLoad) return;
    // Resolve queued swizzle records before erasing: otherwise a destruction
    // immediately after load could leave a queued record that revives later.
    Resolve_Loaded();
    Origins.erase(effect);
}
void Clear() { Origins.clear(); Loading.clear(); Context = nullptr; ContextActive = false; }

bool Save(IStream* stream)
{
    const auto records = Records();
    const int count = static_cast<int>(records.size());
    ULONG written = 0;
    if (FAILED(stream->Write(&count, sizeof(count), &written)) || written != sizeof(count)) return false;
    for (const auto& record : records) {
        written = 0;
        if (FAILED(stream->Write(&record, sizeof(record), &written)) || written != sizeof(record)) return false;
    }
    return true;
}
bool Load(IStream* stream)
{
    Origins.clear(); Loading.clear();
    int count = 0;
    ULONG read = 0;
    if (FAILED(stream->Read(&count, sizeof(count), &read)) || read != sizeof(count) || count < 0 || count > 1000000) return false;
    Loading.resize(count);
    for (auto& record : Loading) {
        read = 0;
        if (FAILED(stream->Read(&record, sizeof(record), &read)) || read != sizeof(record) || record.House < 0) return false;
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(record.Effect, "Mutation.OriginEffect");
    }
    return true;
}
unsigned Network_CRC(unsigned seed)
{
    const auto records = Records();
    bool configured = false;
    for (int i = 0; i < ParticleTypes.Count(); ++i)
        if (Extension::Fetch(ParticleTypes[i])->VisceroidOwner != Owner::Neutral) configured = true;
    if (records.empty() && !configured) return seed;
    CRCEngine crc;
    crc(seed);
    for (int i = 0; i < ParticleTypes.Count(); ++i) {
        crc(ParticleTypes[i]->Fetch_Heap_ID());
        crc(static_cast<int>(Extension::Fetch(ParticleTypes[i])->VisceroidOwner));
    }
    crc(static_cast<int>(records.size()));
    for (const auto& record : records) {
        crc(static_cast<int>(record.Effect->Fetch_RTTI()));
        crc(record.Effect->Fetch_ID());
        crc(record.House);
    }
    return crc.CRC_Value();
}
}
