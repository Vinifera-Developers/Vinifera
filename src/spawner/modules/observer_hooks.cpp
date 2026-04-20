/*******************************************************************************
/*                     O P E N  S O U R C E  --  T S + +                      **
/*******************************************************************************
 *
 *  @project       TS++
 *
 *  @file          OBSERVER_HOOKS.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         Contains the hooks for observer mode.
 *
 *  @license       TS++ is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 TS++ is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "observer_hooks.h"

#include "display.h"
#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "mouse.h"
#include "session.h"
#include "spawner.h"

/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class HouseClassExt : public HouseClass
{
public:
    bool _Is_Ally_Or_Observer(const HouseClassExt* house) const;
    void _Recalc_Radar_Availability();
};


/**
 *  Helper function that returns if the house is allied to the other house, or if the player's house is the observer.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Is_Ally_Or_Observer(const HouseClassExt* house) const
{
    return Is_Ally(house) || Extension::Fetch(PlayerPtr)->IsObserver;
}


/**
 *  Enable the radar and reveal the map for observers.
 *
 *  @author: ZivDero
 */
void HouseClassExt::_Recalc_Radar_Availability()
{
    if (this == PlayerPtr && Extension::Fetch(PlayerPtr)->IsObserver) {
        if (!Map.Is_Radar_Existing()) {
            Map.Toggle_Radar(true);
            Map.Reveal_The_Map();
        }
    } else {
        Recalc_Radar_Availability();
    }
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class DisplayClassExt : public DisplayClass
{
public:
    void _Encroach_Shadow_Observer();
    void _Encroach_Fog_Observer();
};


/**
 *  Don't encroach shadow for observers.
 *
 *  @author: ZivDero
 */
void DisplayClassExt::_Encroach_Shadow_Observer()
{
    if (Extension::Fetch(PlayerPtr)->IsObserver) {
        return;
    }

    DisplayClass::Encroach_Shadow();
}


/**
 *  Don't encroach fog for observers.
 *
 *  @author: ZivDero
 */
void DisplayClassExt::_Encroach_Fog_Observer()
{
    if (Extension::Fetch(PlayerPtr)->IsObserver) {
        return;
    }

    DisplayClass::Encroach_Fog();
}


/**
 *  Main function for patching the hooks.
 */
void Observer_Hooks()
{
    Patch_Call(0x004BC608, &HouseClassExt::_Recalc_Radar_Availability);
    Patch_Call(0x00506D7B, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x00507291, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x00619AE9, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x0061B985, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x00506DFC, &DisplayClassExt::_Encroach_Fog_Observer);
    Patch_Call(0x00507309, &DisplayClassExt::_Encroach_Fog_Observer);
    Patch_Call(0x0043852B, &HouseClassExt::_Is_Ally_Or_Observer); // BuildingClass::Visual_Character
    Patch_Call(0x00438540, &HouseClassExt::_Is_Ally_Or_Observer); // BuildingClass::Visual_Character
    Patch_Call(0x00633E85, &HouseClassExt::_Is_Ally_Or_Observer); // TechnoClass::Visual_Character
    Patch_Call(0x00633E9F, &HouseClassExt::_Is_Ally_Or_Observer); // TechnoClass::Visual_Character
    Patch_Call(0x0062C6CE, &HouseClassExt::_Is_Ally_Or_Observer); // TechnoClass::Draw_Health_Bar
    Patch_Call(0x0062CA26, &HouseClassExt::_Is_Ally_Or_Observer); // TechnoClass::Draw_Health_Bar
    Patch_Call(0x0047B0BB, &HouseClassExt::_Is_Ally_Or_Observer); // DisplayClass::Help_Text
}
