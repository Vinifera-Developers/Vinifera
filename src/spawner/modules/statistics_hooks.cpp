/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          STATISTICS_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for statistics collection.
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

#include "statistics_hooks.h"

#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "packet.h"
#include "scenario.h"
#include "session.h"
#include "spawner.h"
#include "field.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "scenarioext.h"
#include "tibsun_globals.h"


static bool Is_Spawner_Write_Statistics()
{
    if (Vinifera_SpawnerActive)
    {
        return Vinifera_SpawnerConfig->WriteStatistics
            && Session.Type == GAME_IPX;
    }

    return false;
}


static bool Is_Statistics_Enabled()
{
    return Is_Spawner_Write_Statistics() || Session.Type == GAME_INTERNET;
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
class PacketClassExt : public PacketClass
{
public:
    char* _Create_Comms_Packet(int& size);
    void _Add_Field_SCEN_ACCN_HASH(FieldClass* field);
    void _Add_Field_Player_Data(FieldClass* field);
};


/**
 *  Write statistics to a file for the client.
 *
 *  @author: ZivDero
 */
char* PacketClassExt::_Create_Comms_Packet(int& size)
{
    char* result = Create_Comms_Packet(size);

    if (Is_Spawner_Write_Statistics())
    {
        CCFileClass stats_file("stats.dmp");
        if (stats_file.Open(FILE_ACCESS_WRITE))
        {
            stats_file.Write(result, size);
            stats_file.Close();
        }
    
        GameStatisticsPacketSent = true;
    }

    return result;
}


/**
 *  Add some scenario-related fields to the statistics packet.
 *
 *  @author: ZivDero
 */
void PacketClassExt::_Add_Field_SCEN_ACCN_HASH(FieldClass* field)
{
    if (Is_Spawner_Write_Statistics())
    {
        PacketClass::Add_Field(new FieldClass("SCEN", Vinifera_SpawnerConfig->UIMapName));
        PacketClass::Add_Field(new FieldClass("ACCN", PlayerPtr->IniName));
        PacketClass::Add_Field(new FieldClass("HASH", Vinifera_SpawnerConfig->MapHash));

        return;
    }

    PacketClass::Add_Field(field);
}


/**
 *  Add some player-related fields to the statistics packet.
 *
 *  @author: ZivDero
 */
void PacketClassExt::_Add_Field_Player_Data(FieldClass* field)
{
    // This is the global string "NAM?"
    // The game replaces "?" with the player's ID before this call,
    // so we can grab it from there.
    // It should be also be the house ID.
    static auto& field_player_handle = Make_Global<char[5]>(0x0070FCF4);

    if (Is_Spawner_Write_Statistics())
    {
        const char id = field_player_handle[3] - '0';

        const HouseClass* house = Houses[id];
        const HouseClassExtension* house_ext = Extension::Fetch<HouseClassExtension>(house);

        if (house == PlayerPtr)
        {
            PacketClass::Add_Field(new FieldClass("MYID", static_cast<unsigned long>(id)));
            PacketClass::Add_Field(new FieldClass("NKEY", static_cast<unsigned long>(0)));
            PacketClass::Add_Field(new FieldClass("SKEY", static_cast<unsigned long>(0)));
        }

        static char field_player_allies[] = "ALY?";
        field_player_allies[3] = id;
        PacketClass::Add_Field(new FieldClass(field_player_allies, static_cast<unsigned long>(house->Allies)));

        static char field_player_spawn[] = "BSP?";
        field_player_spawn[3] = id;
        PacketClass::Add_Field(new FieldClass(field_player_spawn, static_cast<unsigned long>(ScenExtension->StartingPositions[id])));

        static char field_player_observer[] = "SPC?";
        field_player_observer[3] = id;
        PacketClass::Add_Field(new FieldClass(field_player_observer, static_cast<unsigned long>(house_ext->IsObserver)));
    }

    PacketClass::Add_Field(field);
}


/**
 *  Numerous patches to enable statistics collection.
 *
 *  @author: ZivDero
 */

DECLARE_PATCH(_CellClass_Goodie_Check_SendStatistics)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x00457E83);
    }

    JMP(0x00457E95);
}


DECLARE_PATCH(_HouseClass_Tracking_Add_SendStatistics1)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x004C2218);
    }

    JMP(0x004C22FA);
}


DECLARE_PATCH(_HouseClass_Tracking_Add_SendStatistics2)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x004C2262);
    }

    JMP(0x004C22FA);
}


DECLARE_PATCH(_HouseClass_Tracking_Add_SendStatistics3)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x004C22A8);
    }

    JMP(0x004C22FA);
}


DECLARE_PATCH(_HouseClass_Tracking_Add_SendStatistics4)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x004C22EE);
    }

    JMP(0x004C22FA);
}


DECLARE_PATCH(_Print_MP_Stats_Check)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x00463542);
    }

    JMP(0x0046371F);
}


DECLARE_PATCH(_Kick_Player_Now_SendStatistics)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x005B433C);
    }

    JMP(0x005B439F);
}


DECLARE_PATCH(_Queue_AI_Multiplayer_SendStatistics)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x005B1EA0);
    }

    JMP(0x005B1F21);
}


DECLARE_PATCH(_Main_Loop_SendStatistics1)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x00509229);
    }

    JMP(0x0050924B);
}


DECLARE_PATCH(_Main_Loop_SendStatistics2)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x00509283);
    }

    JMP(0x005092A5);
}


DECLARE_PATCH(_Execute_DoList_SendStatistics1)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x005B4FB9);
    }

    JMP(0x005B500C);
}


DECLARE_PATCH(_Execute_DoList_SendStatistics2)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x005B4FDE);
    }

    JMP(0x005B500C);
}


DECLARE_PATCH(_Main_Game_Start_Timer)
{
    if (Is_Statistics_Enabled())
    {
        JMP(0x00462A2F);
    }

    JMP(0x00462A46);
}


/**
 *  Don't send statistics for observers.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Send_Statistics_Packet_Send_AI_Dont_Send_Observers)
{
    GET_REGISTER_STATIC(HouseClass**, house, edx);
    static HouseClassExtension* house_ext;

    _asm pushad

    if (Is_Spawner_Write_Statistics())
    {
        house_ext = Extension::Fetch<HouseClassExtension>(*house);
        if ((*house)->Class->IsMultiplayPassive || house_ext->IsObserver)
        {
            _asm popad
            JMP_REG(ecx, 0x006098EC);
        }
    }
    else // Vanilla condition
    {
        if (!(*house)->IsHuman)
        {
            _asm popad
            JMP_REG(ecx, 0x006098EC);
        }
    }

    _asm popad
    JMP_REG(ecx, 0x006098E6);
}


/**
 *  Main function for patching the hooks.
 */
void Statistics_Hooks()
{
    Patch_Call(0x0060A797, &PacketClassExt::_Create_Comms_Packet);
    Patch_Jump(0x00457E7A, &_CellClass_Goodie_Check_SendStatistics);
    Patch_Jump(0x004C220B, &_HouseClass_Tracking_Add_SendStatistics1);
    Patch_Jump(0x004C2255, &_HouseClass_Tracking_Add_SendStatistics2);
    Patch_Jump(0x004C229F, &_HouseClass_Tracking_Add_SendStatistics3);
    Patch_Jump(0x004C22E5, &_HouseClass_Tracking_Add_SendStatistics4);
    Patch_Jump(0x0046353C, &_Print_MP_Stats_Check);
    Patch_Jump(0x005B4333, &_Kick_Player_Now_SendStatistics);
    Patch_Jump(0x005B1E94, &_Queue_AI_Multiplayer_SendStatistics);
    Patch_Jump(0x00509220, &_Main_Loop_SendStatistics1);
    Patch_Jump(0x0050927A, &_Main_Loop_SendStatistics2);
    Patch_Jump(0x005B4FAE, &_Execute_DoList_SendStatistics1);
    Patch_Jump(0x005B4FD3, &_Execute_DoList_SendStatistics2);
    Patch_Jump(0x00462A26, &_Main_Game_Start_Timer);
    Patch_Call(0x0060982A, &PacketClassExt::_Add_Field_SCEN_ACCN_HASH);
    Patch_Call(0x00609DA6, &PacketClassExt::_Add_Field_Player_Data);
    //Patch_Jump(0x006098DA, &_Send_Statistics_Packet_Send_AI_Dont_Send_Observers); // Crashes. We write whether the player is the observer in the statistics anyway
    Patch_Jump(0x0060A79C, 0x0060A7C6); // Skip call to some WOL utility to send the packet
}
