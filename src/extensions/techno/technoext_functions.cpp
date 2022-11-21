/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT_FUNCTIONS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the supporting functions for the extended TechnoClass.
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
#include "technoext_functions.h"
#include "technoext.h"
#include "technotypeext.h"
#include "techno.h"
#include "tibsun_globals.h"
#include "ebolt.h"
#include "target.h"
#include "tactical.h"
#include "extension.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Creates a electric bolt zap from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
EBoltClass *TechnoClassExtension_Electric_Zap(TechnoClass *this_ptr, TARGET target, int which, const WeaponTypeClass *weapontype, Coordinate &source_coord)
{
    ASSERT(this_ptr != nullptr);

    EBoltClass *ebolt = new EBoltClass;
    if (!ebolt) {
        return nullptr;
    }

    int z_adj = 0;

    if (Is_Target_Building(target)) {
        Coordinate source = this_ptr->Render_Coord();

        Point2D p1 = TacticalMap->func_60F150(source);
        Point2D p2 = TacticalMap->func_60F150(source_coord);

        z_adj = p2.Y - p1.Y;

        if (z_adj > 0) {
            z_adj = 0;
        }
    }

    Coordinate target_coord = Is_Target_Object(target) ?
        reinterpret_cast<ObjectClass *>(target)->Target_Coord() : // #TODO: Should be Target_As_Object.
        target->entry_5C();

    /**
     *  Spawn the electric bolt.
     */
    ebolt->Create(source_coord, target_coord, z_adj);

    return ebolt;
}


/**
 *  Creates an instance of the electric bolt from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
EBoltClass *TechnoClassExtension_Electric_Bolt(TechnoClass *this_ptr, TARGET target)
{
    ASSERT(this_ptr != nullptr);

    WeaponSlotType which = this_ptr->What_Weapon_Should_I_Use(target);
    const WeaponTypeClass *weapontype = this_ptr->Get_Weapon(which)->Weapon;
    Coordinate fire_coord = this_ptr->Fire_Coord(which);

    EBoltClass *ebolt = TechnoClassExtension_Electric_Zap(this_ptr, target, which, weapontype, fire_coord);
    if (ebolt) {
        if (this_ptr->IsActive) {

            TechnoClassExtension *technoext = Extension::Fetch<TechnoClassExtension>(this_ptr);

            /**
             *  Remove existing electric bolt from the object.
             */
            if (technoext->ElectricBolt) {
                technoext->ElectricBolt->Flag_To_Delete();
                technoext->ElectricBolt = nullptr;
            }

            if (!technoext->ElectricBolt) {
                technoext->ElectricBolt = ebolt;
                technoext->ElectricBolt->Set_Properties(this_ptr, weapontype, which);
            }
        }
    }

    return ebolt;
}
