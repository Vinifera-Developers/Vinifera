/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for Prerequisite Group class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "prerequisitegroup_hooks.h"

#include "ccini.h"
#include "extension.h"
#include "hooker.h"
#include "houseext.h"
#include "prerequisitegroup.h"
#include "syringe.h"
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
DEFINE_HOOK(0x004BBD3E, _HouseClass_Can_Build_Prereq_Groups_Patch, 0)
{
    GET(int, prereq, EAX);
    GET(HouseClass*, house, EBP);

    if (Extension::Fetch(house)->Has_Prerequisite(prereq)) {
        return 0x004BBFD4;
    }

    return 0x004BBFEE;
}


/**
 *  Main function for patching the hooks.
 */
void PrerequisiteGroup_Hooks()
{
    Patch_Jump(0x0044CB30, &Get_Prerequisites);
}
