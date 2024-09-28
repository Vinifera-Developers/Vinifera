/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ARMORTYPE.CPP
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
#include "armortype.h"
#include "ccini.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "asserthandler.h"


/**
 *  Basic constructor for armor objects.
 * 
 *  @author: CCHyper
 */
ArmorTypeClass::ArmorTypeClass(const char *name) :
    Name(nullptr),
    Modifier(1.0),
    ForceFire(true),
    PassiveAcquire(true),
    Retaliate(true)
{
    ASSERT_FATAL_PRINT(name != nullptr, "Invalid name for ArmorType!");

    /**
     *  Force armor types to be lower case to retain original armor name formatting.
     */
    Name = name;
    Name.To_Lower();

    ArmorTypes.Add(this);
}


/**
 *  Class destructor.
 *
 *  @author: CCHyper
 */
ArmorTypeClass::~ArmorTypeClass()
{
    ArmorTypes.Delete(this);
}


/**
 *  Retrieves the ArmorType for given name.
 *
 *  @author: CCHyper
 */
ArmorType ArmorTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    /**
     *  Force armor types to be lower case to retain original armor name formatting.
     */
    Wstring _name = name;
    _name.To_Lower();

    if (_name.Is_Not_Empty()) {
        for (ArmorType armor = ARMOR_FIRST; armor < ArmorTypes.Count(); armor++) {
            if (ArmorTypes[armor]->Name == _name) {
                return armor;
            }
        }
    }

    return ARMOR_NONE;
}


/**
 *  Returns name for given armor type.
 *
 *  @author: CCHyper
 */
const char *ArmorTypeClass::Name_From(ArmorType type)
{
    ASSERT(type >= ARMOR_FIRST && type < ArmorTypes.Count());

    return ArmorTypes[type]->Name.Peek_Buffer();
}


/**
 *  Find or create a armor of the type specified.
 *
 *  @author: CCHyper
 */
const ArmorTypeClass *ArmorTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    /**
     *  Force armor types to be lower case to retain original armor name formatting.
     */
    Wstring _name = name;
    _name.To_Lower();

    for (ArmorType armor = ARMOR_FIRST; armor < ArmorTypes.Count(); ++armor) {
        if (ArmorTypes[armor]->Name == _name) {
            return ArmorTypes[armor];
        }
    }

    ArmorTypeClass *ptr = new ArmorTypeClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}


/**
 *  Performs one time initialization of the armor type class.
 *
 *  @warning: Do not change this function, otherwise it will break support
 *            with the original game!
 *
 *  @author: CCHyper
 */
bool ArmorTypeClass::One_Time()
{
    ArmorTypeClass *armor = nullptr;

    /**
     *  Create the default armor types.
     */

    armor = new ArmorTypeClass(ArmorName[ARMOR_NONE]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_WOOD]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_ALUMINUM]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_STEEL]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_CONCRETE]);
    ASSERT(armor != nullptr);

    return true;
}


/**
 *  Build the default Verses value string representing all the available ArmorTypes.
 *
 *  @author: CCHyper
 */
const char *ArmorTypeClass::Get_Modifier_Default_String()
{
    static char _buffer[1024];

    std::memset(_buffer, 0, sizeof(_buffer));

    for (ArmorType index = ARMOR_FIRST; index < ArmorTypes.Count(); index++) {
        std::strcat(_buffer, "100%%");
        if (index < ArmorTypes.Count() - 1) {
            std::strcat(_buffer, ",");
        }
    }

    return _buffer;
}


/**
 *  Build the default boolean flag value string representing all the available ArmorTypes.
 *
 *  @author: ZivDero
 */
const char* ArmorTypeClass::Get_Boolean_Default_String()
{
    static char _buffer[1024];

    std::memset(_buffer, 0, sizeof(_buffer));

    for (ArmorType index = ARMOR_FIRST; index < ArmorTypes.Count(); index++) {
        std::strcat(_buffer, "yes");
        if (index < ArmorTypes.Count() - 1) {
            std::strcat(_buffer, ",");
        }
    }

    return _buffer;
}
