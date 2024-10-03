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

#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "warheadtype.h"


class Verses
{
public:
    Verses() = delete;

    static HRESULT Save(IStream* pStm);
    static HRESULT Load(IStream* pStm);

    static void Resize();

    static void Set_Modifier(ArmorType armor, WarheadType warhead, double value);
    static double Get_Modifier(ArmorType armor, WarheadType warhead);

    static void Set_Modifier(ArmorType armor, WarheadTypeClass* warhead, double value) { Set_Modifier(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static double Get_Modifier(ArmorType armor, WarheadTypeClass* warhead) { return Get_Modifier(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

    static void Set_ForceFire(ArmorType armor, WarheadType warhead, bool value);
    static bool Get_ForceFire(ArmorType armor, WarheadType warhead);

    static void Set_ForceFire(ArmorType armor, WarheadTypeClass* warhead, bool value) { Set_ForceFire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static bool Get_ForceFire(ArmorType armor, WarheadTypeClass* warhead) { return Get_ForceFire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

    static void Set_PassiveAcquire(ArmorType armor, WarheadType warhead, bool value);
    static bool Get_PassiveAcquire(ArmorType armor, WarheadType warhead);

    static void Set_PassiveAcquire(ArmorType armor, WarheadTypeClass* warhead, bool value) { Set_PassiveAcquire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static bool Get_PassiveAcquire(ArmorType armor, WarheadTypeClass* warhead) { return Get_PassiveAcquire(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

    static void Set_Retaliate(ArmorType armor, WarheadType warhead, bool value);
    static bool Get_Retaliate(ArmorType armor, WarheadType warhead);

    static void Set_Retaliate(ArmorType armor, WarheadTypeClass* warhead, bool value) { Set_Retaliate(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead)), value); }
    static bool Get_Retaliate(ArmorType armor, WarheadTypeClass* warhead) { return Get_Retaliate(armor, static_cast<WarheadType>(WarheadTypes.ID(warhead))); }

private:
    template <typename T>
    static HRESULT Save_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector, const char* heap_name);

    template <typename T>
    static HRESULT Load_2D_Vector(IStream* pStm, std::vector<std::vector<T>>& vector, const char* heap_name);

private:
    /**
     *  The warhead damage is reduced depending on the the type of armor the
     *  defender has. This table is what gives weapons their "character".
     */
    static std::vector<std::vector<double>> Modifier;

    /**
     *  The warhead may be forbidden from targeting the defender depending the
     *  type of armor it has.
     */
    static std::vector<std::vector<unsigned char>> ForceFire;
    static std::vector<std::vector<unsigned char>> PassiveAcquire;
    static std::vector<std::vector<unsigned char>> Retaliate;
};
