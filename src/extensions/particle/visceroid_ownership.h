// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <objidl.h>

class AbstractClass;
class HouseClass;

namespace VisceroidOwnership {

enum class Owner : int { Neutral, Invoker, Victim };

// A house identity, rather than a firing unit, survives death and capture.
void Set(AbstractClass* effect, HouseClass* house);
HouseClass* Origin(const AbstractClass* effect);
HouseClass* Current();
bool Has_Context();

class Scope {
public:
    explicit Scope(HouseClass* house);
    ~Scope();
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
private:
    HouseClass* Previous;
    bool PreviousActive;
};

void Capture(AbstractClass* effect);
void Forget(AbstractClass* effect);
void Clear();
bool Save(IStream* stream);
bool Load(IStream* stream);
unsigned Network_CRC(unsigned seed);
void Hooks();

}
