/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ARMORTYPE.H
 *
 *  @authors       CCHyper
 *
 *  @brief         x
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

// [ArmorTypes]
// NewArm


class ArmorTypeClass
{
    public:
        ArmorTypeClass(const char *name);
        virtual ~ArmorTypeClass();

        bool Read_INI(CCINIClass &ini);

        bool Is_Allowed_Zero_Damage() const { return IsZeroDamageAllowed; }
        double Get_Value() const { return Value; }

        static bool One_Time();

        static const char *Get_Modifier_Default_String();

        static ArmorType From_Name(const char *name);
        static const char *Name_From(ArmorType type);

        static const ArmorTypeClass *Find_Or_Make(const char *name);

    private:
        /**
         *  The name of this armor type, used for identification purposes.
         */
        Wstring Name;

    public:
        /**
         *  
         */
        double Value; // TODO rename?

        // ; NOTE: (Double Note, a float math bug caused 2 % to be used when 1 % broke down. Fixing the bug is why I made 2 %)
        //     ; A 0 % means no force fire, no retaliate, no passive acquire
        //     ; A 1 % means no retaliate, no passive acquire
        //     ; A 2 % means no passive acquire

        /**
         *  Can the order the unit to force fire on the target?
         */
        bool IsForceFire;

        /**
         *  Can the unit retaliate against this target?
         */
        bool IsRetaliate;

        /**
         *  Can the unit passively acquire this target?
         */
        bool IsPassiveAcquire;

        /**
         *  Is this armor allowed to have zero damage dealt upon it?
         */
        bool IsZeroDamageAllowed;
};
