/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Shared sidebar tooltip formatting implementation.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sidebar_render_utils.h"

#include "extension.h"
#include "sidebar_model.h"
#include "supertype.h"
#include "supertypeext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"

#include <cstdio>

/**
 *  Formats sidebar tooltip text for a build item. Ported from the
 *  extended sidebar tooltip hook.
 *
 *  @author: ZivDero, Rampastring
 */
const char* Format_Cameo_Tooltip(const BuildItem& item)
{
    static char buffer[512];

    if (item.Type == RTTI_SPECIAL) {
        const SuperWeaponTypeClass* swtype = SuperWeaponTypes[item.ID];
        if (swtype == nullptr) {
            return nullptr;
        }

        const SuperWeaponTypeClassExtension* swtypeext = Extension::Fetch(swtype);
        const char* description = swtypeext->Description;

        if (description[0] == '\0') {
            return swtype->Full_Name();
        }

        std::snprintf(buffer, sizeof(buffer), "%s@@%s", swtype->Full_Name(), description);
        return buffer;
    }

    const TechnoTypeClass* ttype = Fetch_Techno_Type(item.Type, item.ID);
    if (ttype == nullptr) {
        return nullptr;
    }

    const TechnoTypeClassExtension* technotypeext = Extension::Fetch(ttype);
    const char* description = technotypeext->Description;

    if (description[0] == '\0') {
        std::snprintf(buffer, sizeof(buffer), "%s@$%d", ttype->Full_Name(), ttype->Cost_Of(PlayerPtr));
    } else {
        std::snprintf(buffer, sizeof(buffer), "%s@$%d@@%s", ttype->Full_Name(), ttype->Cost_Of(PlayerPtr), description);
    }

    return buffer;
}
