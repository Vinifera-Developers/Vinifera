/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CCINIEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended CCINIClass.
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
#include "houseext_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "ccini.h"
#include "housetype.h"
#include "weapontype.h"
#include "animtype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class CCINIClassFake final : public CCINIClass
{
    public:
        TypeList<AnimTypeClass *> Get_AnimType_List(const char *section, const char *entry, const TypeList<AnimTypeClass *> defvalue);

        long _Get_Owners(const char *section, const char *entry, const long defvalue);
        bool _Put_Owners(const char *section, const char *entry, long value);
};


/**
 *  Fetch the owners (list of house bits).
 */
long CCINIClassFake::_Get_Owners(const char *section, const char *entry, const long defvalue)
{
    char buffer[128];

    long ownable = defvalue;

    if (CCINIClass::Get_String(section, entry, "", buffer, sizeof(buffer)) > 0) {
        ownable = 0;
        char *name = std::strtok(buffer, ",");
        while (name) {
            ownable |= Owner_From_Name(name);
            name = std::strtok(nullptr, ",");
        }
    }

    return ownable;
}


/**
 *  Store the house bitfield to the INI database.
 */
bool CCINIClassFake::_Put_Owners(const char *section, const char *entry, long value)
{
    char buffer[128];

    buffer[0] = '\0';

    if (!value || HouseTypes.Count() <= 0) {
        return true;
    }

    for (HousesType house = HOUSE_FIRST; house < HouseTypes.Count(); ++house) {
        HouseTypeClass *htptr = HouseTypes[house];
        if ((value & (1 << htptr->House)) != 0) {
            if (buffer[0] != '\0') {
                std::strcat(buffer, ",");
            }
            std::strcat(buffer, HouseTypeClass::As_Reference(house).Name());
        }
    }

    if (buffer[0] != '\0') {
        return CCINIClass::Put_String(section, entry, buffer);
    }

    return true;
}


/**
 *  Fetch a list of AnimTypes.
 * 
 *  @author: CCHyper
 */
TypeList<AnimTypeClass *> CCINIClassFake::Get_AnimType_List(const char *section, const char *entry, const TypeList<AnimTypeClass *> defvalue)
{
    /**
     *  #issue-391
     * 
     *  Increases the buffer size from 128 to 2048.
     * 
     *  @author: CCHyper
     */
    //char buffer[128];
    char buffer[2048];

    if (CCINIClass::Get_String(section, entry, "", buffer, sizeof(buffer)) > 0) {

        //DEV_DEBUG_INFO("Get_AnimType_List(\"%s\",\"%s\") - \"%s\"\n", section, entry, buffer);

        TypeList<AnimTypeClass *> list;

        char *name = std::strtok(buffer, ",");
        while (name) {
            AnimTypeClass *animtype = const_cast<AnimTypeClass *>(AnimTypeClass::Find_Or_Make(name));
            if (animtype) {
                list.Add(animtype);
            }
            name = std::strtok(nullptr, ",");
        }

        return list;
    }

    return defvalue;
}


/**
 *  #issue-391
 *
 *  This is actually a patch in WeaponTypeClass:Read_INI, but because
 *  Get_AnimType_List is inlined there, its best to have it with all
 *  the other CCINIClass hooks.
 * 
 *  @author: CCHyper
 */
static void WeaponTypeClass_Read_INI_Get_AnimType_List_Encapsultator(WeaponTypeClass *this_ptr, CCINIClassFake &ini, const char *ini_name)
{
    this_ptr->Anim = ini.Get_AnimType_List(ini_name, "Anim", this_ptr->Anim);
}

DECLARE_PATCH(_WeaponTypeClass_Read_INI_Get_AnimType_List_Patch)
{
    GET_REGISTER_STATIC(WeaponTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(CCINIClassFake *, ini, ebx);
    GET_REGISTER_STATIC(const char *, ini_name, edi);

    /**
     *  Load the AnimType list.
     * 
     *  We need to use an encapsulation function as we are replacing an inlined
     *  function and the return value from Get_AnimType_List is an TypeList
     *  instance, so it will trash the stack.
     */
    WeaponTypeClass_Read_INI_Get_AnimType_List_Encapsultator(this_ptr, *ini, ini_name);

    /**
     *  Clear ECX and restore some registers to be safe.
     */
    _asm { xor ecx, ecx }
    _asm { mov edi, ini_name }
    _asm { mov ebx, ini }

    JMP_REG(ecx, 0x00681004);
}


/**
 *  Main function for patching the hooks.
 */
void CCINIClassExtension_Hooks()
{
    /**
     *  Inlined CCINIClass function hooks from here.
     */
    Patch_Jump(0x00680F07, &_WeaponTypeClass_Read_INI_Get_AnimType_List_Patch);

    Patch_Jump(0x0044ADC0, &CCINIClassFake::_Get_Owners);
    Patch_Jump(0x0044AE40, &CCINIClassFake::_Put_Owners);
}
