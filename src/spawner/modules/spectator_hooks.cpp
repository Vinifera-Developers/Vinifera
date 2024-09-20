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
    bool _Is_Ally(const HouseClassExt* house) const;
    void _Update_Radars();
};


bool HouseClassExt::_Is_Spectator() const
{
    if (Spawner::Active)
    {
        return Spawner::GetConfig()->Houses[Get_Heap_ID()].IsSpectator;
    }

    return false;
}


bool HouseClassExt::_Is_Ally(const HouseClassExt* house) const
{
    return HouseClass::Is_Ally(house) || (Spawner::Active && (_Is_Spectator() || house->_Is_Spectator()));
}


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


void DisplayClassExt::_Encroach_Shadow_Spectator()
{
    if (Spawner::Active && ((HouseClassExt*)PlayerPtr)->_Is_Spectator())
    {
        return;
    }

    DisplayClass::Encroach_Shadow();
}


void DisplayClassExt::_Encroach_Fog_Spectator()
{
    if (Spawner::Active && ((HouseClassExt*)PlayerPtr)->_Is_Spectator())
    {
        return;
    }

    DisplayClass::Encroach_Fog();
}


DECLARE_PATCH(_HouseClass_MPlayer_Defeated_Dont_Count_Spectators)
{
    GET_REGISTER_STATIC(HouseClassExt*, hptr, eax);

    if (Spawner::Active && hptr != PlayerPtr && Session.Type != GAME_SKIRMISH && hptr->_Is_Spectator())
    {
        JMP(0x004BF74A);
    }

    vanilla_code:
    if (hptr->IsDefeated)
    {
        _asm mov eax, hptr
        JMP_REG(ebp, 0x004BF75D);
    }

    _asm mov eax, hptr
    JMP_REG(ebp, 0x004BF724)
}


DECLARE_PATCH(_HouseClass_Radar_Outage_Spectators)
{
    GET_REGISTER_STATIC(BOOL, tactical_availability, edx);
    GET_REGISTER_STATIC(HouseClassExt*, house, esi);

    if (!Spawner::Active || (house == PlayerPtr && !house->_Is_Spectator()))
    {
        Map.RadarClass::Toggle_Radar(tactical_availability);
    }

    // Return
    JMP(0x004C9693);
}


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
    Patch_Call(0x0043852B, &HouseClassExt::_Is_Ally);
    Patch_Call(0x00438540, &HouseClassExt::_Is_Ally);
    Patch_Call(0x00633E85, &HouseClassExt::_Is_Ally);
    Patch_Call(0x00633E9F, &HouseClassExt::_Is_Ally);
    Patch_Call(0x0062C6CE, &HouseClassExt::_Is_Ally);
    Patch_Call(0x0062CA26, &HouseClassExt::_Is_Ally);
    Patch_Call(0x00428A23, &HouseClassExt::_Is_Ally);
    Patch_Call(0x004BC608, &HouseClassExt::_Update_Radars);
}