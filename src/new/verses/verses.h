/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VERSES.H
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
#pragma once 

#include <vector>

#include "armortype.h"
#include "asserthandler.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "warheadtype.h"


class Verses
{
private:
    /**
     *  Holds a value for a specific armor-warhead pair, and whether it was customized.
     *  If not set, should instruct to fall back to the default/inherited value.
     *  Kinda like an optional, but std::optional was introduced in C++17.
     */
    template <typename T>
    struct VersesData
    {
        T Value;
        bool IsSet;
    };

public:
    Verses() = delete;

    static HRESULT Save(IStream* pStm);
    static HRESULT Load(IStream* pStm);

    static void Resize();
    static void Clear();

    static void Set_Modifier(ArmorType armor, WarheadType warhead, double value) { Set_Value(armor, warhead, value, Modifier); }
    static double Get_Modifier(ArmorType armor, WarheadType warhead) { return Get_Value(armor, warhead, Modifier, &ArmorTypeClass::Modifier); }

    static void Set_Modifier(ArmorType armor, WarheadTypeClass* warhead, double value) { Set_Modifier(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static double Get_Modifier(ArmorType armor, WarheadTypeClass* warhead) { return Get_Modifier(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

    static void Set_ForceFire(ArmorType armor, WarheadType warhead, bool value) { Set_Value(armor, warhead, value, ForceFire); }
    static bool Get_ForceFire(ArmorType armor, WarheadType warhead) { return Get_Value(armor, warhead, ForceFire, &ArmorTypeClass::ForceFire); }

    static void Set_ForceFire(ArmorType armor, WarheadTypeClass* warhead, bool value) { Set_ForceFire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static bool Get_ForceFire(ArmorType armor, WarheadTypeClass* warhead) { return Get_ForceFire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

    static void Set_PassiveAcquire(ArmorType armor, WarheadType warhead, bool value) { Set_Value(armor, warhead, value, PassiveAcquire); }
    static bool Get_PassiveAcquire(ArmorType armor, WarheadType warhead) { return Get_Value(armor, warhead, PassiveAcquire, &ArmorTypeClass::PassiveAcquire); }

    static void Set_PassiveAcquire(ArmorType armor, WarheadTypeClass* warhead, bool value) { Set_PassiveAcquire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static bool Get_PassiveAcquire(ArmorType armor, WarheadTypeClass* warhead) { return Get_PassiveAcquire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

    static void Set_Retaliate(ArmorType armor, WarheadType warhead, bool value) { Set_Value(armor, warhead, value, Retaliate); }
    static bool Get_Retaliate(ArmorType armor, WarheadType warhead) { return Get_Value(armor, warhead, Retaliate, &ArmorTypeClass::Retaliate); }

    static void Set_Retaliate(ArmorType armor, WarheadTypeClass* warhead, bool value) { Set_Retaliate(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static bool Get_Retaliate(ArmorType armor, WarheadTypeClass* warhead) { return Get_Retaliate(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

private:
    template <typename T>
    static void Set_Value(ArmorType armor, WarheadType warhead, T value, std::vector<std::vector<VersesData<T>>>& vector);

    template <typename T>
    static T Get_Value(ArmorType armor, WarheadType warhead, std::vector<std::vector<VersesData<T>>>& vector, T ArmorTypeClass::* specific);

    template <typename T>
    static HRESULT Save_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector, const char* heap_name);

    template <typename T>
    static HRESULT Load_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector, const char* heap_name);

private:
    /**
     *  The warhead damage is reduced depending on the the type of armor the
     *  defender has. This table is what gives weapons their "character".
     */
    static std::vector<std::vector<VersesData<double>>> Modifier;

    /**
     *  The warhead may be forbidden from targeting the defender depending the
     *  type of armor it has.
     */
    static std::vector<std::vector<VersesData<bool>>> ForceFire;
    static std::vector<std::vector<VersesData<bool>>> PassiveAcquire;
    static std::vector<std::vector<VersesData<bool>>> Retaliate;
};


/**
 *  Sets the Verses modifier for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
template <typename T>
void Verses::Set_Value(ArmorType armor, WarheadType warhead, T value, std::vector<std::vector<VersesData<T>>>& vector)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    vector[armor][warhead].Value = value;
    vector[armor][warhead].IsSet = true;
}


/**
 *  Gets the Verses modifier for an armor and warhead combination.
 *
 *  @author: ZivDero
 */
template <typename T>
T Verses::Get_Value(ArmorType armor, WarheadType warhead, std::vector<std::vector<VersesData<T>>>& vector, T ArmorTypeClass::* specific)
{
    ASSERT(armor >= ARMOR_FIRST && armor < ArmorTypes.Count());
    ASSERT(warhead >= WARHEAD_FIRST && warhead < WarheadTypes.Count());

    /**
     *  If this armor-warhead combo has a custom value set, use that.
     */
    if (vector[armor][warhead].IsSet)
        return vector[armor][warhead].Value;

    /**
     *  Check if the armor has a base armor. If it does, fall back to that.
     */
    const auto armortype = ArmorTypes[armor];
    if (armortype->BaseArmor != ARMOR_NULL && armortype->BaseArmor != armor)
        return Get_Value(armortype->BaseArmor, warhead, vector, specific);

    /**
     *  Return the default for this armor.
     */
    return ArmorTypes[armor]->*specific;
}
