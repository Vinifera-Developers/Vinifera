#include "verses.h"

#include "armortype.h"
#include "asserthandler.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"

std::vector<std::vector<double>> Verses::Modifier;
std::vector<std::vector<unsigned char>> Verses::ForceFire;
std::vector<std::vector<unsigned char>> Verses::PassiveAcquire;
std::vector<std::vector<unsigned char>> Verses::Retaliate;

HRESULT Verses::Save(IStream* pStm)
{
    HRESULT hr = Save_2D_Vector(pStm, Modifier);
    if (FAILED(hr))
        return hr;

    hr = Save_2D_Vector(pStm, ForceFire);
    if (FAILED(hr))
        return hr;

    hr = Save_2D_Vector(pStm, PassiveAcquire);
    if (FAILED(hr))
        return hr;

    hr = Save_2D_Vector(pStm, Retaliate);
    return hr;
}


HRESULT Verses::Load(IStream* pStm)
{
    HRESULT hr = Load_2D_Vector(pStm, Modifier);
    if (FAILED(hr))
        return hr;

    hr = Load_2D_Vector(pStm, ForceFire);
    if (FAILED(hr))
        return hr;

    hr = Load_2D_Vector(pStm, PassiveAcquire);
    if (FAILED(hr))
        return hr;

    hr = Load_2D_Vector(pStm, Retaliate);
    return hr;
}


void Verses::Resize()
{
    const int old_armor_count = Modifier.size();

    // Add new arrays for new armors
    Modifier.resize(ArmorTypes.Count());
    ForceFire.resize(ArmorTypes.Count());
    PassiveAcquire.resize(ArmorTypes.Count());
    Retaliate.resize(ArmorTypes.Count());

    // Resize the old arrays for new warheads and set defaults
    for (int i = 0; i < old_armor_count; i++)
    {
        Modifier[i].resize(WarheadTypes.Count(), DBL_MAX);
        ForceFire[i].resize(WarheadTypes.Count(), UCHAR_MAX);
        PassiveAcquire[i].resize(WarheadTypes.Count(), UCHAR_MAX);
        Retaliate[i].resize(WarheadTypes.Count(), UCHAR_MAX);
    }

    // Create new arrays for new armors
    for (int i = old_armor_count; i < ArmorTypes.Count(); i++)
    {
        Modifier[i] = std::vector<double>(WarheadTypes.Count(), DBL_MAX);
        ForceFire[i] = std::vector<unsigned char>(WarheadTypes.Count(), UCHAR_MAX);
        PassiveAcquire[i] = std::vector<unsigned char>(WarheadTypes.Count(), UCHAR_MAX);
        Retaliate[i] = std::vector<unsigned char>(WarheadTypes.Count(), UCHAR_MAX);
    }
}


void Verses::Set_Modifier(ArmorType armor, WarheadType warhead, double value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    Modifier[armor][warhead] = value;
}


double Verses::Get_Modifier(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (Modifier[armor][warhead] == DBL_MAX)
        return ArmorTypes[armor]->Modifier;

    return Modifier[armor][warhead];
}


void Verses::Set_ForceFire(ArmorType armor, WarheadType warhead, bool value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    ForceFire[armor][warhead] = static_cast<unsigned char>(value);
}


bool Verses::Get_ForceFire(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (ForceFire[armor][warhead] == UCHAR_MAX)
        return ArmorTypes[armor]->ForceFire;

    return static_cast<bool>(ForceFire[armor][warhead]);
}


void Verses::Set_PassiveAcquire(ArmorType armor, WarheadType warhead, bool value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    PassiveAcquire[armor][warhead] = static_cast<unsigned char>(value);
}


bool Verses::Get_PassiveAcquire(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (PassiveAcquire[armor][warhead] == UCHAR_MAX)
        return ArmorTypes[armor]->PassiveAcquire;

    return static_cast<bool>(PassiveAcquire[armor][warhead]);
}


void Verses::Set_Retaliate(ArmorType armor, WarheadType warhead, bool value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    Retaliate[armor][warhead] = static_cast<unsigned char>(value);
}


bool Verses::Get_Retaliate(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (Retaliate[armor][warhead] == UCHAR_MAX)
        return ArmorTypes[armor]->Retaliate;

    return static_cast<bool>(Retaliate[armor][warhead]);
}

template <typename T>
HRESULT Verses::Save_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector)
{
    int count = vector.size();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr))
        return hr;

    for (int i = 0; i < count; i++)
    {
        static char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "vector[%d]", i);

        HRESULT hr = Save_Primitive_Vector(pStm, vector[i], buffer);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}

template <typename T>
HRESULT Verses::Load_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector)
{
    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr))
        return hr;

    vector = std::vector<std::vector<T>>(count);

    for (int i = 0; i < count; i++)
    {
        static char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "vector[%d]", i);

        HRESULT hr = Load_Primitive_Vector(pStm, vector[i], buffer);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}
