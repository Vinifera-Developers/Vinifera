/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PREREQUISITEGROUP_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for Prerequisite Group class.
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

#include "prerequisitegroup_hooks.h"

#include "ccini.h"
#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "houseext.h"
#include "prerequisitegroup.h"
#include "typelist.h"
#include "vinifera_globals.h"


/**
 *  Re-implementation of prerequisite parsing for prerequisite groups.
 *
 *  @author: ZivDero
 */
TypeList<int> Get_Prerequisites(CCINIClass const& ini, char const* section, char const* entry, TypeList<int> defvalue)
{
    char buffer[512];

    if (ini.Get_String(section, entry, "", buffer, sizeof(buffer))) {
        TypeList<int> list;
        char* token = std::strtok(buffer, ",");
        while (token != nullptr && *token != '\0') {

            PrerequisiteGroupType group = PrerequisiteGroupClass::From_Name(token);
            if (group != PREREQ_GROUP_NONE) {
                list.Add(PrerequisiteGroupClass::Encode(group));
            } else {
                int building = BuildingTypeClass::From_Name(token);
                if (building != STRUCT_NONE) {
                    list.Add(building);
                }
            }

            token = std::strtok(nullptr, ",");
        }
        return list;
    }

    return defvalue;
}


/**
 *  Patch to check new prerequisite groups in HouseClass::Can_Build.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Can_Build_Prereq_Groups_Patch)
{
    GET_REGISTER_STATIC(int, prereq, eax);
    GET_REGISTER_STATIC(HouseClass*, house, ebp);

    _asm pushad

    if (Extension::Fetch(house)->Has_Prerequisite(prereq)) {

        _asm popad
        JMP(0x004BBFD4);
    }

    _asm popad
    JMP(0x004BBFEE);
}


/**
 *  Main function for patching the hooks.
 */
void PrerequisiteGroup_Hooks()
{
    Patch_Jump(0x0044CB30, &Get_Prerequisites);
    Patch_Jump(0x004BBD3E, &_HouseClass_Can_Build_Prereq_Groups_Patch);
}
