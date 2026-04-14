/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_RENDER_UTILS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Shared sidebar tooltip formatting implementation.
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

#include "sidebar_render_utils.h"

#include "sidebar_model.h"

#include "extension.h"
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
