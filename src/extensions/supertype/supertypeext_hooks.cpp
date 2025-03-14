/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SUPERTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended SuperWeaponTypeClass.
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
#include "supertypeext_hooks.h"
#include "supertypeext_init.h"
#include "supertypeext.h"
#include "supertype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "cell.h"
#include "object.h"
#include "building.h"
#include "buildingtype.h"
#include "extension.h"
#include "hooker.h"
#include "mouse.h"
#include "weapontype.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class SuperWeaponTypeClassExt : public SuperWeaponTypeClass
{
public:
    ActionType _What_Action(Cell& cell, ObjectClass* target) const;
};


/**
 *  Returns the action used by this Super Weapon.
 *
 *  @author: ZivDero
 */
ActionType SuperWeaponTypeClassExt::_What_Action(Cell& cell, ObjectClass* object) const
{
    if (ActsLike == SPECIAL_EM_PULSE)
    {
        const auto swtype_ext = Extension::Fetch<SuperWeaponTypeClassExtension>(this);

        AbstractClass * target_ptr;
        Cell target_cell;

        if (object)
        {
            target_ptr = object;
            target_cell = object->Get_Cell();
        }
        else
        {
            target_ptr = &Map[cell];
            target_cell = cell;
        }

        unsigned min_distance = INT_MAX;
        BuildingClass* closest_cannon = nullptr;

        for (int i = 0; i < Buildings.Count(); i++)
        {
            BuildingClass* building = Buildings[i];
            if (building->Class->IsEMPulseCannon)
            {
                if (building->House == PlayerPtr && building->Is_Powered_On() && building->Distance(target_ptr) < min_distance)
                {
                    min_distance = building->Distance(target_ptr);
                    closest_cannon = building;
                }
            }
        }

        if (!closest_cannon)
            return swtype_ext->ActionOutOfRange;

        int range = closest_cannon->Get_Weapon()->Weapon->Range / 256;
        Cell cannon_cell = closest_cannon->Get_Cell();

        return (target_cell - cannon_cell).Length2() > range * range ? swtype_ext->ActionOutOfRange : Action;
    }

    return Action;
}


/**
 *  Main function for patching the hooks.
 */
void SuperWeaponTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SuperWeaponTypeClassExtension_Init();

    Patch_Jump(0x0060D6C0, &SuperWeaponTypeClassExt::_What_Action);
}
