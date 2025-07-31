/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TechnoTypeClass.
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
#include "technotypeext_hooks.h"
#include "technotypeext.h"
#include "objecttypeext.h"
#include "technotype.h"
#include "house.h"
#include "rules.h"
#include "tibsun_defines.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "extension_globals.h"
#include "hooker.h"
#include "rulesext.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TechnoTypeClass)
{
public:
    int _Max_Pips() const;
};


/**
 *  Reimplements TechnoTypeClass::Max_Pips.
 *
 *  @author: ZivDero
 */
int TechnoTypeClassExt::_Max_Pips() const
{
    int max_pips = 0;
    if (PipScale - 1 < RuleExtension->MaxPips.Count())
        max_pips = RuleExtension->MaxPips[PipScale - 1];

    // Negative values are not allowed
    if (max_pips < 0)
        return 0;

    switch (PipScale)
    {
    case PIP_AMMO:
        return std::clamp(MaxAmmo, 0, max_pips);

    case PIP_PASSENGERS:
        return std::clamp(MaxPassengers, 0, max_pips);

    case PIP_TIBERIUM:
    case PIP_POWER:
    case PIP_CHARGE:
    default:
        return max_pips;
    }
}


/**
 *  Main function for patching the hooks.
 */
void TechnoTypeClassExtension_Hooks()
{
    Patch_Jump(0x0063D460, &TechnoTypeClassExt::_Max_Pips);
}
