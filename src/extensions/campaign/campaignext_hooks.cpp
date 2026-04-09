/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CAMPAIGNEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended CampaignClass.
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

#include "always.h"

#include "campaignext_hooks.h"

#include "addon.h"
#include "campaign.h"
#include "campaignext.h"
#include "campaignext_init.h"
#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "syringe.h"


/**
 *  #issue-723
 *
 *  Patches in support for checking IsDebugOnly when loading campaigns.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E337D, _Choose_Campaign_Debug_Only_Patch, 0)
{
    GET(CampaignClass *, campaign, ESI);
    GET(int, index, EDI);

    auto campaignext = Extension::Fetch(campaign);

    /**
     *  Is this a debug campaign? Make sure the developer mode is enabled
     *  first before allowing it to continue availability checks.
     */
    if (campaignext->IsDebugOnly && !Vinifera_DeveloperMode) {
        DEBUG_INFO("  Skipping Debug-Only Campaign [%d] - %s\n", index, campaign->Description);
        goto skip_no_print;
    }
    
    /**
     *  Are there any addon modes enabled? Check to make sure its the required one.
     */
    if (Addon_Enabled(ADDON_ANY)) {
        if (campaign->RequiredAddon == ADDON_BASE_GAME) {
            goto skip_campaign;
        }
        if (!Addon_Enabled(campaign->RequiredAddon)) {
            goto skip_campaign;
        }

    /**
     *  We are in the normal Tiberian Sun mode, but if the campaign has a
     *  required addon set, skip it.
     */
    } else if (campaign->RequiredAddon != ADDON_BASE_GAME) {
        goto skip_campaign;
    }

    /**
     *  Add the campaign to the dialog list.
     */
add_campaign:
    DEBUG_INFO("  Adding Campaign [%d] - %s\n", index, campaign->Description);
add_no_print:
    R->ESI(&campaign->Description);
    R->EDI(index);
    return 0x004E33D1;

    /**
     *  Skip this campaign.
     */
skip_campaign:
    DEBUG_GAME("  Skipping Campaign [%d] - %s\n", index, campaign->Description);
skip_no_print:
    return 0x004E33E6;
}


/**
 *  Main function for patching the hooks.
 */
void CampaignClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    CampaignClassExtension_Init();

    Patch_Byte_Range(0x004E3377, 0x90, 3); // Removes "or ecx, 0x0FFFFFFFF"
}
