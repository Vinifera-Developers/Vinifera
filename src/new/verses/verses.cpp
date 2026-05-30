/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  New Verses handler.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "verses.h"

#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"


std::vector<std::vector<Verses::VersesData<double>>> Verses::Modifier;
std::vector<std::vector<Verses::VersesData<bool>>> Verses::ForceFire;
std::vector<std::vector<Verses::VersesData<bool>>> Verses::PassiveAcquire;
std::vector<std::vector<Verses::VersesData<bool>>> Verses::Retaliate;

/**
 *  Saves all the Verses arrays to the stream.
 *
 *  @author: ZivDero
 */
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


/**
 *  Loads all the Verses arrays from the stream.
 *
 *  @author: ZivDero
 */
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


/**
 *  Resizes the Verses arrays to match the armor and warhead counts.
 *
 *  @author: ZivDero
 */
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
        Modifier[i].resize(Warheads.Count());
        ForceFire[i].resize(Warheads.Count());
        PassiveAcquire[i].resize(Warheads.Count());
        Retaliate[i].resize(Warheads.Count());
    }

    // Create new arrays for new armors
    for (int i = old_armor_count; i < ArmorTypes.Count(); i++)
    {
        Modifier[i] = std::vector<VersesData<double>>(Warheads.Count());
        ForceFire[i] = std::vector<VersesData<bool>>(Warheads.Count());
        PassiveAcquire[i] = std::vector<VersesData<bool>>(Warheads.Count());
        Retaliate[i] = std::vector<VersesData<bool>>(Warheads.Count());
    }
}


/**
 *  Clears all Verses arrays.
 *
 *  @author: ZivDero
 */
void Verses::Clear()
{
    Modifier.clear();
    ForceFire.clear();
    PassiveAcquire.clear();
    Retaliate.clear();
}


/**
 *  Saves a 2D std::vector to the stream
 *
 *  @author: ZivDero
 */
template <typename T>
HRESULT Verses::Save_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector)
{
    int count = vector.size();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr))
        return hr;

    for (int i = 0; i < count; i++)
    {
        hr = Save_Primitive_Vector(pStm, vector[i]);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}


/**
 *  Loads a 2D std::vector from the stream
 *
 *  @author: ZivDero
 */
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
        hr = Load_Primitive_Vector(pStm, vector[i]);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}
