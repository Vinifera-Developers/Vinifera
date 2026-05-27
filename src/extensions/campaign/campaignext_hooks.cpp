/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended CampaignClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
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
        DEBUG_INFO("  Skipping Debug-Only Campaign [{}] - {}\n", index, campaign->Description);
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
    DEBUG_INFO("  Adding Campaign [{}] - {}\n", index, campaign->Description);
add_no_print:
    R->ESI(&campaign->Description);
    R->EDI(index);
    return 0x004E33D1;

    /**
     *  Skip this campaign.
     */
skip_campaign:
    DEBUG_INFO("  Skipping Campaign [{}] - {}\n", index, campaign->Description);
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
