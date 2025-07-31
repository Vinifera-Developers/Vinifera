/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CARGOEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended CargoClass.
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
#include "cargoext_hooks.h"
#include "tibsun_globals.h"
#include "cargo.h"
#include "foot.h"
#include "scenario.h"
#include "session.h"
#include "techno.h"

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
static class CargoClassExt : public CargoClass
{
public:
    void _Attach_One(FootClass* object);
};


/**
 *  #issue-1093
 *
 *  CargoClass::Attach can attach multiple objects to the
 *  cargo depending on the linked list of units (ObjectClass.Next).
 *  The function is "dual-use" and used for both linking reinforcement
 *  teams into a transport and for attaching individual units into transports.
 *
 *  Because ObjectClass.Next is also used for tracking occupiers of a
 *  single cell, this dual-use behaviour can result in the
 *  transport getting attached to its own cargo in edge cases
 *  where a cell has multiple units.
 *  This bug later causes the transport to disappear when it unloads its cargo.
 *
 *  This function only attaches the given object to the cargo,
 *  making it possible to fix the mentioned bug.
 *
 *  Authors: 10/31/1994 JLB: CargoClass::Attach from Red Alert 1 source code
 *           08/17/2024 Rampastring: Changes to only attach one object.
 */
void CargoClassExt::_Attach_One(FootClass* object) {

    // If there is no object, then no action is necessary.
    if (object == NULL) return;

    object->Limbo();

    // Attach any existing cargo hold object to follow the given object.
    object->Next = CargoHold;

    // Assign the object pointer as the first object attached to this cargo hold.
    CargoHold = object;
    Quantity++;
}


/**
 *  Main function for patching the hooks.
 */
void CargoClassExtension_Hooks()
{
    Patch_Call(0x00651617, &CargoClassExt::_Attach_One);
    Patch_Call(0x004D39FB, &CargoClassExt::_Attach_One);
    Patch_Call(0x004D3A82, &CargoClassExt::_Attach_One);
    Patch_Call(0x0065431A, &CargoClassExt::_Attach_One);
}
