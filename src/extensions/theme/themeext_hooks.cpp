/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEMEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ThemeClass.
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
#include "themeext_hooks.h"
#include "themeext_init.h"
#include "themeext.h"
#include "theme.h"
#include "tibsun_globals.h"
#include "house.h"
#include "housetype.h"
#include "session.h"
#include "scenario.h"
#include "addon.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class ThemeClassExt : public ThemeClass
{
    public:
        bool _Is_Allowed(ThemeType index) const;
};


/**
 *  Checks to see if the specified theme is legal.
 * 
 *  @author: 07/04/1996 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
bool ThemeClassExt::_Is_Allowed(ThemeType index) const
{
    if (index == THEME_QUIET || index == THEME_PICK_ANOTHER) {
        return true;
    }
    
    if (index >= Themes.Count()) {
        return true;
    }

    /**
     *  If the theme is not present, then it certainly isn't allowed.
     */
    if (!Themes[index]->Available) {
        return false;
    }

    /**
     *  Only normal themes (playable during battle) are considered allowed.
     */
    if (!Themes[index]->Normal) {
        return false;
    }

    /**
     *  #issue-764
     * 
     *  If this theme requires an addon, make sure that addon is active.
     * 
     *  @author: CCHyper
     */
    ThemeControlExtension *themectrlext = Extension::List::Fetch<ThemeClass::ThemeControl, ThemeControlExtension>(Themes[index], ThemeControlExtensions);
    if (themectrlext->RequiredAddon != ADDON_NONE) {
        if (!Is_Addon_Enabled(themectrlext->RequiredAddon)) {
            return false;
        }
    }

    /**
     *  If the theme is not allowed to be played by the player's house, then don't allow
     *  it. If the player's house hasn't yet been determined, then presume this test
     *  passes.
     */
    SideType owner = Themes[index]->Owner;
    if (PlayerPtr != nullptr && owner != SIDE_NONE && PlayerPtr->Class->Side != owner) {
        return false;
    }

    /**
     *  If the scenario doesn't allow this theme yet, then return the failure flag. The
     *  scenario check only makes sense for solo play.
     */
    if (Session.Type == GAME_NORMAL && Scen->Scenario < Themes[index]->Scenario) {
        return false;
    }

    /**
     *  Since all tests passed, return with the "is allowed" flag.
     */
    return true;
}


/**
 *  Main function for patching the hooks.
 */
void ThemeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    ThemeClassExtension_Init();

    Patch_Jump(0x00644300, &ThemeClassExt::_Is_Allowed);
}
