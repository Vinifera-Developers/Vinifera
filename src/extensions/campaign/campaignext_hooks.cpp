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
#include "campaignext_hooks.h"
#include "campaignext_init.h"
#include "campaign.h"
#include "campaignext.h"
#include "addon.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  #issue-723
 * 
 *  Patches in support for checking IsDebugOnly when loading campaigns.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Choose_Campaign_Debug_Only_Patch)
{
    GET_REGISTER_STATIC(CampaignClass *, campaign, esi);
    GET_REGISTER_STATIC(int, index, edi);
    static CampaignClassExtension *campaignext;

    campaignext = CampaignClassExtensions.find(campaign);

    /**
     *  Is this a debug campaign? Make sure the developer mode is enabled
     *  first before allowing it to continue availability checks.
     */
    if (campaignext) {
        if (campaignext->IsDebugOnly && !Vinifera_DeveloperMode) {
            DEBUG_INFO("  Skipping Debug-Only Campaign [%d] - %s\n", index, campaign->Description);
            goto skip_no_print;
        }
    }
    
    /**
     *  Are there any addon modes enabled? Check to make sure its the required one.
     */
    if (Addon_Enabled(ADDON_ANY)) {
        if (campaign->RequiredAddon == ADDON_NONE) {
            goto skip_campaign;
        }
        if (!Addon_Enabled(campaign->RequiredAddon)) {
            goto skip_campaign;
        }

    /**
     *  We are in the normal Tiberian Sun mode, but if the campaign has a
     *  required addon set, skip it.
     */
    } else if (campaign->RequiredAddon != ADDON_NONE) {
        goto skip_campaign;
    }

    /**
     *  Add the campaign to the dialog list.
     */
add_campaign:
    DEBUG_INFO("  Adding Campaign [%d] - %s\n", index, campaign->Description);
add_no_print:
    _asm { mov esi, campaign }
    _asm { add esi, 0x268 } // campaign->Description
    _asm { mov edi, index }
    JMP_REG(ecx, 0x004E33D1);

    /**
     *  Skip this campaign.
     */
skip_campaign:
    DEBUG_GAME("  Skipping Campaign [%d] - %s\n", index, campaign->Description);
skip_no_print:
    JMP(0x004E33E6);
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

    Patch_Jump(0x004E337D, &_Choose_Campaign_Debug_Only_Patch);
    Patch_Byte_Range(0x004E3377, 0x90, 3); // Removes "or ecx, 0x0FFFFFFFF"
}
