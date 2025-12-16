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
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "ccini.h"
#include "housetype.h"
#include "theatertype.h"
#include "armortype.h"
#include "actiontype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "syringe.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class CCINIClassExt : public CCINIClass
{
public:
    long _Get_Owners(const char* section, const char* entry, const long defvalue);
    bool _Put_Owners(const char* section, const char* entry, long value);

    TheaterType _Get_TheaterType(const char* section, const char* entry, const TheaterType defvalue);
    bool _Put_TheaterType(const char* section, const char* entry, TheaterType value);

    ArmorType _Get_ArmorType(const char* section, const char* entry, const ArmorType defvalue);
    bool _Put_ArmorType(const char* section, const char* entry, ArmorType value);

    ActionType _Get_ActionType(const char* section, const char* entry, const ActionType defvalue);
};


/**
 *  Fetch the owners (list of house bits).
 */
long CCINIClassExt::_Get_Owners(const char *section, const char *entry, const long defvalue)
{
    /**
     *  #issue-372
     * 
     *  Increases the buffer size from 128 to 2048.
     * 
     *  @author: CCHyper
     */
    //char buffer[128];
    char buffer[2048];

    long ownable = defvalue;

    if (CCINIClass::Get_String(section, entry, "", buffer, sizeof(buffer)) > 0) {

        //DEV_DEBUG_INFO("Get_Owners(\"%s\",\"%s\") - \"%s\"\n", section, entry, buffer);

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
bool CCINIClassExt::_Put_Owners(const char *section, const char *entry, long value)
{
    /**
     *  #issue-372
     * 
     *  Increases the buffer size from 128 to 2048.
     * 
     *  @author: CCHyper
     */
    //char buffer[128];
    char buffer[2048];

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
            std::strcat(buffer, HouseTypes[house]->Name());
        }
    }

    if (buffer[0] != '\0') {

        //DEV_DEBUG_INFO("Put_Owners(\"%s\",\"%s\") - \"%s\"\n", section, entry, buffer);

        return CCINIClass::Put_String(section, entry, buffer);
    }

    return true;
}


/**
 *  Reimplementation of CCINIClass::Get_TheaterType to support TheaterTypeClass.
 *  
 *  @author: CCHyper
 */
TheaterType CCINIClassExt::_Get_TheaterType(const char *section, const char *entry, const TheaterType defvalue)
{
    char buffer[2048];

    if (CCINIClass::Get_String(section, entry, "", buffer, sizeof(buffer))) {
        return TheaterTypeClass::From_Name(buffer);
    }

    return defvalue;
}


/**
 *  Reimplementation of CCINIClass::Put_TheaterType to support TheaterTypeClass.
 *  
 *  @author: CCHyper
 */
bool CCINIClassExt::_Put_TheaterType(const char *section, const char *entry, TheaterType value)
{
    return CCINIClass::Put_String(section, entry, TheaterTypeClass::Name_From(value));
}


/**
 *  Reimplementation of CCINIClass::Get_ActionType to support ActionTypeClass.
 *  
 *  @author: CCHyper
 */
ActionType CCINIClassExt::_Get_ActionType(const char *section, const char *entry, const ActionType defvalue)
{
    char buffer[2048];

    if (CCINIClass::Get_String(section, entry, "", buffer, sizeof(buffer))) {
        return ActionTypeClass::From_Name(buffer);
    }

    return defvalue;
}


/**
 *  Fetches the armor type from the INI database.
 *
 *  @author: CCHyper
 */
ArmorType CCINIClassExt::_Get_ArmorType(const char *section, const char *entry, const ArmorType defvalue)
{
    char buffer[1024];

    if (INIClass::Get_String(section, entry, nullptr, buffer, sizeof(buffer)) > 0) {
        return ArmorTypeClass::From_Name(buffer);
    }

    return defvalue;
}


/**
 *  Store the armor type to the INI database.
 *
 *  @author: CCHyper
 */
bool CCINIClassExt::_Put_ArmorType(const char *section, const char *entry, ArmorType value)
{
    return Put_String(section, entry, ArmorTypeClass::Name_From(value));
}


/**
 *  Main function for patching the hooks.
 */
void CCINIClassExtension_Hooks()
{
    Patch_Jump(0x0044ADC0, &CCINIClassExt::_Get_Owners);
    Patch_Jump(0x0044AE40, &CCINIClassExt::_Put_Owners);
    Patch_Jump(0x0044B310, &CCINIClassExt::_Get_TheaterType);
    Patch_Jump(0x0044B360, &CCINIClassExt::_Put_TheaterType);
    Patch_Jump(0x0044AF50, &CCINIClassExt::_Get_ArmorType);
    Patch_Jump(0x0044AFA0, &CCINIClassExt::_Put_ArmorType);

    // Put this here as it was only called in INIClass::Get_ArmorType.
    Patch_Jump(0x00681320, &ArmorTypeClass::From_Name);

    Patch_Jump(0x0044AC20, &CCINIClassExt::_Get_ActionType);
}
