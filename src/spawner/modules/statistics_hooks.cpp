/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for statistics collection.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "statistics_hooks.h"

#include "extension.h"
#include "field.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "packet.h"
#include "scenario.h"
#include "scenarioext.h"
#include "session.h"
#include "sessionext.h"
#include "spawner.h"
#include "syringe.h"
#include "tibsun_globals.h"


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

    if (SessionExtension->Are_Extra_Statistics_Enabled()) {
        CCFileClass stats_file("stats.dmp");
        if (stats_file.Open(FILE_ACCESS_WRITE)) {
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
    if (SessionExtension->Are_Extra_Statistics_Enabled()) {
        PacketClass::Add_Field(new FieldClass("SCEN", const_cast<char*>(SessionExtension->SpawnerInfo.StatsMapName.c_str())));
        PacketClass::Add_Field(new FieldClass("ACCN", const_cast<char*>(PlayerPtr->IniName.c_str())));
        PacketClass::Add_Field(new FieldClass("HASH", const_cast<char*>(SessionExtension->SpawnerInfo.StatsMapHash.c_str())));
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

    if (SessionExtension->Are_Extra_Statistics_Enabled()) {
        const char id = field_player_handle[3] - '0';

        const HouseClass* house = Houses[id];
        const HouseClassExtension* house_ext = Extension::Fetch(house);

        if (house == PlayerPtr) {
            PacketClass::Add_Field(new FieldClass("MYID", static_cast<unsigned long>(id)));
            PacketClass::Add_Field(new FieldClass("NKEY", static_cast<unsigned long>(0)));
            PacketClass::Add_Field(new FieldClass("SKEY", static_cast<unsigned long>(0)));
        }

        static char field_player_allies[] = "ALY?";
        field_player_allies[3] = id;
        PacketClass::Add_Field(new FieldClass(field_player_allies, static_cast<unsigned long>(house->Allies)));

        static char field_player_spawn[] = "BSP?";
        field_player_spawn[3] = id;
        PacketClass::Add_Field(new FieldClass(field_player_spawn, static_cast<unsigned long>(house_ext->SpawnWaypoint)));

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
DEFINE_HOOK(0x0046353C, _Print_MP_Stats_Check, 0)
{
    if (SessionExtension->Are_Statistics_Enabled()) {
        return 0x00463542;
    }

    return 0x0046371F;
}


DEFINE_HOOK(0x005B4333, _Kick_Player_Now_SendStatistics, 0)
{
    if (SessionExtension->Are_Statistics_Enabled()) {
        return 0x005B433C;
    }

    return 0x005B439F;
}


DEFINE_HOOK(0x005B1E94, _Queue_AI_Multiplayer_SendStatistics, 0)
{
    if (SessionExtension->Are_Statistics_Enabled()) {
        return 0x005B1EA0;
    }

    return 0x005B1F21;
}


DEFINE_HOOK(0x00509220, _Main_Loop_SendStatistics1, 0)
{
    if (SessionExtension->Are_Statistics_Enabled()) {
        return 0x00509229;
    }

    return 0x0050924B;
}


DEFINE_HOOK(0x0050927A, _Main_Loop_SendStatistics2, 0)
{
    if (SessionExtension->Are_Statistics_Enabled()) {
        return 0x00509283;
    }

    return 0x005092A5;
}


DEFINE_HOOK(0x00462A26, _Main_Game_Start_Timer, 0)
{
    if (SessionExtension->Are_Statistics_Enabled()) {
        return 0x00462A2F;
    }

    return 0x00462A46;
}


/**
 *  Don't collect statistics for observers.
 *
 *  @note: YRpp spawner also writes non-multiplay-passive AI players, seems weird.
 *  Skipped that for now.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x006098DC, _Send_Statistics_Packet_Send_AI_Dont_Send_Observers, 6)
{
    GET(HouseClass*, house, EAX);

    if (Extension::Fetch(house)->IsObserver) {
        return 0x006098EC; // skip
    }
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void Statistics_Hooks()
{
    Patch_Call(0x0060A797, &PacketClassExt::_Create_Comms_Packet);
    Patch_Call(0x0060982A, &PacketClassExt::_Add_Field_SCEN_ACCN_HASH);
    Patch_Call(0x00609DA6, &PacketClassExt::_Add_Field_Player_Data);
    Patch_Jump(0x0060A79C, 0x0060A7C6); // Skip call WOL NetUtil->RequestGameresSend
}
