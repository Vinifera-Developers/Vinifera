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
#include "technotypeext_init.h"
#include "technotypeext.h"
#include "technotype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class TechnoTypeClassFake final : public TechnoTypeClass
{
    public:
        const WeaponInfoStruct &_Fetch_Weapon_Info(WeaponSlotType slot) const;
};


/**
 *  Reimplementation of TechnoTypeClass::Fetch_Weapon_Info.
 * 
 *  @author: CCHyper
 */
const WeaponInfoStruct &TechnoTypeClassFake::_Fetch_Weapon_Info(WeaponSlotType slot) const
{
    TechnoTypeClassExtension *technotypeext;
    technotypeext = TechnoTypeClassExtensions.find(this);

    /**
     *  Call the reimplementation of Fetch_Weapon_Info().
     */
    if (technotypeext) {
        return technotypeext->Fetch_Weapon_Info(slot);

    /**
     *  Call the original function.
     */
    } else {
        return TechnoTypeClass::Fetch_Weapon_Info(slot);
    }
}


/**
 *  Main function for patching the hooks.
 */
void TechnoTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TechnoTypeClassExtension_Init();

    Patch_Call(0x0040F2C8, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x0040F2DE, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x00436C37, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x00436C71, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x00436C8A, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x00443D37, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x00487023, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x0048702F, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x0064F047, &TechnoTypeClassFake::_Fetch_Weapon_Info);
    Patch_Call(0x00655DEA, &TechnoTypeClassFake::_Fetch_Weapon_Info);
}
