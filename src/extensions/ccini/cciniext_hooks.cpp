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
};


/**
 *  Fetch a list of AnimTypes.
 * 
 *  @author: CCHyper
 */
TypeList<AnimTypeClass *> CCINIClassFake::Get_AnimType_List(const char *section, const char *entry, const TypeList<AnimTypeClass *> defvalue)
{
    char buffer[128];

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
 *  Main function for patching the hooks.
 */
void CCINIClassExtension_Hooks()
{
}
