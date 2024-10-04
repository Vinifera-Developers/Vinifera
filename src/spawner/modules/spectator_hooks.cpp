/*******************************************************************************
/*                     O P E N  S O U R C E  --  T S + +                      **
/*******************************************************************************
 *
 *  @project       TS++
 *
 *  @file          SPECTATOR_HOOKS.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         Contains the hooks for spectator mode.
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

#include "spectator_hooks.h"

#include "display.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
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
    bool _Is_Spectator() const;
    bool _Is_Coach() const;
    bool _Is_Ally_Or_Spectator(const HouseClassExt* house) const;
    void _Update_Radars();
    bool _Has_Player_Allies() const;
};


/**
 *  Helper function that returns if the house is a spectator.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Is_Spectator() const
{
    if (Spawner::Active)
    {
        return Spawner::Get_Config()->Houses[Get_Heap_ID()].IsSpectator;
    }

    return false;
}


/**
 *  Helper function that returns if the house is a coach.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Is_Coach() const
{
    if (Spawner::Active)
    {
        if (Spawner::Get_Config()->CoachMode)
        {
            return _Is_Spectator() && _Has_Player_Allies();
        }
    }

    return false;
}


/**
 *  Helper function that returns if the house is allied to the other house, or is a spectator.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Is_Ally_Or_Spectator(const HouseClassExt* house) const
{
    bool is_ally = HouseClass::Is_Ally(house);

    if (Spawner::Active)
    {
        if (is_ally)
            return is_ally;

        if (Spawner::Get_Config()->CoachMode)
        {
            return _Is_Coach() || house->_Is_Coach();
        }

        return _Is_Spectator() || house->_Is_Spectator();
    }

    return is_ally;
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
 *  Enable the radar for spectators.
 *
 *  @author: ZivDero
 */
static bool spectator_radar_enabled = false;
void HouseClassExt::_Update_Radars()
{
    Update_Radars();

    if (Spawner::Active)
    {
        if (this == PlayerPtr && !spectator_radar_enabled && _Is_Spectator())
        {
            Map.IsRadarAvailable = true;
            Map.RadarClass::Radar_Activate(1);
            spectator_radar_enabled = true;
        }
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
    void _Encroach_Shadow_Spectator();
    void _Encroach_Fog_Spectator();
};


/**
 *  Don't encroach shadow for spectators.
 *
 *  @author: ZivDero
 */
void DisplayClassExt::_Encroach_Shadow_Spectator()
{
    if (Spawner::Active && reinterpret_cast<HouseClassExt*>(PlayerPtr)->_Is_Spectator() && !reinterpret_cast<HouseClassExt*>(PlayerPtr)->_Is_Coach())
    {
        return;
    }

    DisplayClass::Encroach_Shadow();
}


/**
 *  Don't encroach fog for spectators.
 *
 *  @author: ZivDero
 */
void DisplayClassExt::_Encroach_Fog_Spectator()
{
    if (Spawner::Active && reinterpret_cast<HouseClassExt*>(PlayerPtr)->_Is_Spectator() && !reinterpret_cast<HouseClassExt*>(PlayerPtr)->_Is_Coach())
    {
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
    if (Spawner::Active && Spawner::Get_Config()->CoachMode && reinterpret_cast<HouseClassExt*>(PlayerPtr)->_Is_Coach())
    {
        return;
    }

    MapClass::Reveal_The_Map();
}


/**
 *  Don't count spectators as defeated players.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_MPlayer_Defeated_Dont_Count_Spectators)
{
    GET_REGISTER_STATIC(HouseClassExt*, hptr, eax);
    _asm pushad

    if (Spawner::Active && hptr != PlayerPtr && Session.Type != GAME_SKIRMISH && hptr->_Is_Spectator())
    {
        _asm popad
        JMP(0x004BF74A);
    }

    // Vanilla code
    if (!hptr->IsDefeated && !hptr->Class->IsMultiplayPassive)
    {
        _asm popad
        JMP_REG(ebp, 0x004BF730);
    }

    _asm popad
    JMP_REG(ebp, 0x004BF75D)
}


/**
 *  Don't process the radar for spectators.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Radar_Outage_Spectators)
{
    GET_STACK_STATIC8(bool, tactical_availability, esp, 0x4);
    GET_REGISTER_STATIC(HouseClassExt*, house, esi);

    if (!Spawner::Active || (house == PlayerPtr && !house->_Is_Spectator()))
    {
        Map.RadarClass::Toggle_Radar(tactical_availability);
    }

    // Return
    JMP(0x004C9693);
}


/**
 *  Reveal the map for spectators.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_RadarClass_Compute_Radar_Image)
{
    if (Spawner::Active)
    {
        spectator_radar_enabled = false;
        if (reinterpret_cast<HouseClassExt*>(PlayerPtr)->_Is_Spectator() && !reinterpret_cast<HouseClassExt*>(PlayerPtr)->_Is_Coach() && PlayerPtr->IsDefeated)
        {
            Session.ObiWan = true;
            Map.Reveal_The_Map();
        }
    }

    // Function epilogue
    _asm
    {
        pop edi
        pop esi
        pop ebp
        add esp, 0x14
        ret
    }
}


/**
 *  Main function for patching the hooks.
 */
void Spectator_Hooks()
{
    Patch_Call(0x00506D7B, &DisplayClassExt::_Encroach_Shadow_Spectator);
    Patch_Call(0x00507291, &DisplayClassExt::_Encroach_Shadow_Spectator);
    Patch_Call(0x00619AE9, &DisplayClassExt::_Encroach_Shadow_Spectator);
    Patch_Call(0x0061B985, &DisplayClassExt::_Encroach_Shadow_Spectator);
    Patch_Call(0x00506DFC, &DisplayClassExt::_Encroach_Fog_Spectator);
    Patch_Call(0x00507309, &DisplayClassExt::_Encroach_Fog_Spectator);
    Patch_Jump(0x004BF71B, &_HouseClass_MPlayer_Defeated_Dont_Count_Spectators);
    Patch_Jump(0x004C9684, &_HouseClass_Radar_Outage_Spectators);
    Patch_Call(0x0043852B, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x00438540, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x00633E85, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x00633E9F, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x0062C6CE, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x0062CA26, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x00428A23, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x0047B0BB, &HouseClassExt::_Is_Ally_Or_Spectator);
    Patch_Call(0x004BC608, &HouseClassExt::_Update_Radars);
    Patch_Call(0x004BF5D6, &MapClassExt::_Reveal_The_Map);
    Patch_Jump(0x005B9CFE, &_RadarClass_Compute_Radar_Image);
}
