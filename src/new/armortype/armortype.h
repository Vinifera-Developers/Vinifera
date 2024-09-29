/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ARMORTYPE.H
 *
 *  @authors       CCHyper, ZivDero
 *
 *  @brief         New ArmorType class.
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

#include "always.h"
#include "tibsun_defines.h"
#include "wstring.h"

class CCINIClass;


class ArmorTypeClass
{
public:
        ArmorTypeClass(const char *name);
        virtual ~ArmorTypeClass();

        bool Read_INI(CCINIClass& ini);

        static bool One_Time();
        static void Clear_All();

        static const char *Get_Modifier_Default_String();
        static const char *Get_Boolean_Default_String();

        static ArmorType From_Name(const char *name);
        static const char *Name_From(ArmorType type);

        static const ArmorTypeClass *Find_Or_Make(const char *name);

private:
        /**
         *  The name of this armor type, used for identification purposes.
         */
        char Name[256];

public:
        /**
         *  The warhead damage is reduced depending on the the type of armor the
         *  defender has. This is the default value for this armor.
         */
        double Modifier;

        /**
         *  The warhead may be forbidden from targeting the defender depending the
         *  type of armor it has. This is the default value for this armor.
         */
        bool ForceFire;
        bool PassiveAcquire;
        bool Retaliate;
};
