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
#include "session.h"
#include "spawner.h"
#include "mouse.h"

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
    bool _Is_Observer() const;
    bool _Is_Ally_Or_Observer(const HouseClassExt* house) const;
    void _Update_Radars();
    bool _Has_Player_Allies() const;
};


/**
 *  Helper function that returns if the house is a observer.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Is_Observer() const
{
    HouseClassExtension* ext = Extension::Fetch<HouseClassExtension>(this);
    return ext->IsObserver;
}


/**
 *  Helper function that returns if the house is allied to the other house, or is a observer.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Is_Ally_Or_Observer(const HouseClassExt* house) const
{
    return Is_Ally(house) || _Is_Observer() || house->_Is_Observer();
}


/**
 *  Helper function that returns if the player has any allies.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Has_Player_Allies() const
{
    const char* SPECIAL = "Special";

    unsigned int allies = Allies;

    // Special is allied to everyone, so we need to exclude it from the allies list to get the real picture
    int special_house_id = As_Pointer(HouseTypeClass::From_Name(SPECIAL))->Get_Heap_ID();
    allies &= ~(1 << special_house_id);

    return allies != 0;
}


/**
 *  Enable the radar for observers.
 *
 *  @author: ZivDero
 */
static bool observer_radar_enabled = false;
void HouseClassExt::_Update_Radars()
{
    Update_Radars();

    if (this == Vinifera_ObserverPtr && !observer_radar_enabled) {
        Map.IsRadarAvailable = true;
        Map.RadarClass::Radar_Activate(1);
        observer_radar_enabled = true;
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
    if (Vinifera_SpawnerActive && PlayerPtr == Vinifera_ObserverPtr) {
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
    if (Vinifera_SpawnerActive && PlayerPtr == Vinifera_ObserverPtr) {
        return;
    }

    DisplayClass::Encroach_Fog();
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
class MapClassExt : public MapClass
{
public:
    void _Reveal_The_Map();
};


/**
 *  Don't reveal the map in coach mode.
 *
 *  @author: ZivDero
 */
void MapClassExt::_Reveal_The_Map()
{
    if (Vinifera_SpawnerActive && Vinifera_SpawnerConfig->CoachMode && static_cast<HouseClassExt*>(PlayerPtr)->_Has_Player_Allies()) {
        return;
    }

    MapClass::Reveal_The_Map();
}


/**
 *  Don't process the radar for observers.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Radar_Outage_Observers)
{
    GET_STACK_STATIC8(bool, tactical_availability, esp, 0x4);
    GET_REGISTER_STATIC(HouseClassExt*, house, esi);

    if (house != Vinifera_ObserverPtr) {
        Map.RadarClass::Toggle_Radar(tactical_availability);
    }

    // Return
    JMP(0x004C9693);
}


/**
 *  Main function for patching the hooks.
 */
void Observer_Hooks()
{
    Patch_Call(0x00506D7B, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x00507291, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x00619AE9, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x0061B985, &DisplayClassExt::_Encroach_Shadow_Observer);
    Patch_Call(0x00506DFC, &DisplayClassExt::_Encroach_Fog_Observer);
    Patch_Call(0x00507309, &DisplayClassExt::_Encroach_Fog_Observer);
    Patch_Jump(0x004C9684, &_HouseClass_Radar_Outage_Observers);
    Patch_Call(0x0043852B, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x00438540, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x00633E85, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x00633E9F, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x0062C6CE, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x0062CA26, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x00428A23, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x0047B0BB, &HouseClassExt::_Is_Ally_Or_Observer);
    Patch_Call(0x004BC608, &HouseClassExt::_Update_Radars);
    Patch_Call(0x004BF5D6, &MapClassExt::_Reveal_The_Map);
}
