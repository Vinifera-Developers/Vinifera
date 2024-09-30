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

#define MODIFIER_DEFAULT DBL_MAX
#define FORCEFIRE_DEFAULT UCHAR_MAX
#define PASSIVEACQUIRE_DEFAULT UCHAR_MAX
#define RETALIATE_DEFAULT UCHAR_MAX


std::vector<std::vector<double>> Verses::Modifier;
std::vector<std::vector<unsigned char>> Verses::ForceFire;
std::vector<std::vector<unsigned char>> Verses::PassiveAcquire;
std::vector<std::vector<unsigned char>> Verses::Retaliate;

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
        Modifier[i].resize(WarheadTypes.Count(), MODIFIER_DEFAULT);
        ForceFire[i].resize(WarheadTypes.Count(), FORCEFIRE_DEFAULT);
        PassiveAcquire[i].resize(WarheadTypes.Count(), PASSIVEACQUIRE_DEFAULT);
        Retaliate[i].resize(WarheadTypes.Count(), RETALIATE_DEFAULT);
    }

    // Create new arrays for new armors
    for (int i = old_armor_count; i < ArmorTypes.Count(); i++)
    {
        Modifier[i] = std::vector<double>(WarheadTypes.Count(), MODIFIER_DEFAULT);
        ForceFire[i] = std::vector<unsigned char>(WarheadTypes.Count(), FORCEFIRE_DEFAULT);
        PassiveAcquire[i] = std::vector<unsigned char>(WarheadTypes.Count(), PASSIVEACQUIRE_DEFAULT);
        Retaliate[i] = std::vector<unsigned char>(WarheadTypes.Count(), RETALIATE_DEFAULT);
    }
}


/**
 *  Sets the Verses modifier for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
void Verses::Set_Modifier(ArmorType armor, WarheadType warhead, double value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    Modifier[armor][warhead] = value;
}


/**
 *  Gets the Verses modifier for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
double Verses::Get_Modifier(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (Modifier[armor][warhead] == MODIFIER_DEFAULT)
        return ArmorTypes[armor]->Modifier;

    return Modifier[armor][warhead];
}


/**
 *  Sets the Verses force-fire flag for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
void Verses::Set_ForceFire(ArmorType armor, WarheadType warhead, bool value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    ForceFire[armor][warhead] = static_cast<unsigned char>(value);
}


/**
 *  Gets the Verses force-fire flag for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
bool Verses::Get_ForceFire(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (ForceFire[armor][warhead] == FORCEFIRE_DEFAULT)
        return ArmorTypes[armor]->ForceFire;

    return static_cast<bool>(ForceFire[armor][warhead]);
}


/**
 *  Sets the Verses passive acquire flag for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
void Verses::Set_PassiveAcquire(ArmorType armor, WarheadType warhead, bool value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    PassiveAcquire[armor][warhead] = static_cast<unsigned char>(value);
}


/**
 *  Gets the Verses passive acquire flag for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
bool Verses::Get_PassiveAcquire(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (PassiveAcquire[armor][warhead] == PASSIVEACQUIRE_DEFAULT)
        return ArmorTypes[armor]->PassiveAcquire;

    return static_cast<bool>(PassiveAcquire[armor][warhead]);
}


/**
 *  Sets the Verses retaliate flag for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
void Verses::Set_Retaliate(ArmorType armor, WarheadType warhead, bool value)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    Retaliate[armor][warhead] = static_cast<unsigned char>(value);
}


/**
 *  Gets the Verses retaliate flag for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
bool Verses::Get_Retaliate(ArmorType armor, WarheadType warhead)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    if (Retaliate[armor][warhead] == RETALIATE_DEFAULT)
        return ArmorTypes[armor]->Retaliate;

    return static_cast<bool>(Retaliate[armor][warhead]);
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
        static char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "vector[%d]", i);

        HRESULT hr = Save_Primitive_Vector(pStm, vector[i], buffer);
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
        static char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "vector[%d]", i);

        HRESULT hr = Load_Primitive_Vector(pStm, vector[i], buffer);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}
