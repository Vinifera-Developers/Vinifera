/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VERSES.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         New Verses handler.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "verses.h"

#include "armortype.h"
#include "asserthandler.h"
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
    HRESULT hr = Save_2D_Vector(pStm, Modifier, "Verses::Modifier");
    if (FAILED(hr))
        return hr;

    hr = Save_2D_Vector(pStm, ForceFire, "Verses::ForceFire");
    if (FAILED(hr))
        return hr;

    hr = Save_2D_Vector(pStm, PassiveAcquire, "Verses::PassiveAcquire");
    if (FAILED(hr))
        return hr;

    hr = Save_2D_Vector(pStm, Retaliate, "Verses::Retaliate");
    return hr;
}


/**
 *  Loads all the Verses arrays from the stream.
 *
 *  @author: ZivDero
 */
HRESULT Verses::Load(IStream* pStm)
{
    HRESULT hr = Load_2D_Vector(pStm, Modifier, "Verses::Modifier");
    if (FAILED(hr))
        return hr;

    hr = Load_2D_Vector(pStm, ForceFire, "Verses::ForceFire");
    if (FAILED(hr))
        return hr;

    hr = Load_2D_Vector(pStm, PassiveAcquire, "Verses::PassiveAcquire");
    if (FAILED(hr))
        return hr;

    hr = Load_2D_Vector(pStm, Retaliate, "Verses::Retaliate");
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
        Modifier[i].resize(WarheadTypes.Count());
        ForceFire[i].resize(WarheadTypes.Count());
        PassiveAcquire[i].resize(WarheadTypes.Count());
        Retaliate[i].resize(WarheadTypes.Count());
    }

    // Create new arrays for new armors
    for (int i = old_armor_count; i < ArmorTypes.Count(); i++)
    {
        Modifier[i] = std::vector<VersesData<double>>(WarheadTypes.Count());
        ForceFire[i] = std::vector<VersesData<bool>>(WarheadTypes.Count());
        PassiveAcquire[i] = std::vector<VersesData<bool>>(WarheadTypes.Count());
        Retaliate[i] = std::vector<VersesData<bool>>(WarheadTypes.Count());
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
HRESULT Verses::Save_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector, const char* heap_name)
{
    int count = vector.size();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr))
        return hr;

    for (int i = 0; i < count; i++)
    {
        static char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%s[%d]", heap_name, i);

        hr = Save_Primitive_Vector(pStm, vector[i], buffer);
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
HRESULT Verses::Load_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector, const char* heap_name)
{
    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr))
        return hr;

    vector = std::vector<std::vector<T>>(count);

    for (int i = 0; i < count; i++)
    {
        static char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%s[%d]", heap_name, i);

        hr = Load_Primitive_Vector(pStm, vector[i], buffer);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}
