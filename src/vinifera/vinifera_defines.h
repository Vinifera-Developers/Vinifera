/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Vinifera defines and constants.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "session.h"
#include "tibsun_defines.h"


/**
 *  Enable debug printing of the swizzle remapping process.
 * 
 *  WARNING: This will take cause the save/load process to take up to 10 minutes!
 */
#ifndef NDEBUG
//#define VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING 1
#endif

/**
 *  CLSIDs for all new locomotors.
 */
#define CLSID_TEST_LOCOMOTOR                "501DEF92-C7ED-448E-8FEB-7908DCE73377"
#define CLSID_ROCKET_LOCOMOTOR              "B7B49766-E576-11d3-9BD9-00104B972FE8"
#define CLSID_NEWJUMPJET_LOCOMOTOR          "92612C46-F71F-11D1-AC9F-006008055BB5" // This is the same as vanilla JumpjetLocomotion. This way, we silently replace it.


/**
 *  CLSIDs for new classes.
 */
#define UUID_ARMORTYPE                      "EE8D505F-12BB-4313-AEDC-4AEA30A5BA03"
#define UUID_ROCKETTYPE                     "FAE72300-A93C-476C-A6DB-CB2B62ADCECD"
#define UUID_SPAWN_MANAGER                  "157ADEE5-D344-48B9-811B-3FA01EF3CCD4"
#define UUID_PREREQUISITE_GROUP             "C0D8765A-FD0C-476D-B4DD-C8D061C323EE"


/**
 *  UUIDs for all extension classes.
 */
#define UUID_UNIT_EXTENSION                 "17621513-3BDA-4FBD-A591-1A0B6DA0F4B9"
#define UUID_AIRCRAFT_EXTENSION             "04B6C8D5-6D12-41C3-BF4B-B52F25928CF3"
#define UUID_AIRCRAFTTYPE_EXTENSION         "2985D76F-00B8-4F6B-A89E-AE8149F31203"
#define UUID_ANIM_EXTENSION                 "4099F805-F5BC-40C2-A465-BB9C66DFE130"
#define UUID_ANIMTYPE_EXTENSION             "00F9418E-171E-4B6E-83B1-D32840622DAC"
#define UUID_BUILDING_EXTENSION             "B63BC5ED-23DF-4F98-8A87-E6CD79C19DB4"
#define UUID_BUILDINGTYPE_EXTENSION         "F5DF6BE6-86F8-4DDB-9FF8-C353A40043A9"
#define UUID_BULLET_EXTENSION               "866098FE-A846-4300-8F40-7AE058F80A9C"
#define UUID_BULLETTYPE_EXTENSION           "B0A3AD67-07D3-4D40-8F06-41BF1202489E"
#define UUID_CAMPAIGN_EXTENSION             "59BBAC71-C9F5-48DC-B624-D253F50B7A78"
#define UUID_CELL_EXTENSION                 "3D2CB3B7-8873-4055-A775-F44B24460F53"
#define UUID_FACTORY_EXTENSION              "8565FEDD-1C81-4C5C-B026-F3CBAD0D00BC"
#define UUID_HOUSE_EXTENSION                "7F10C6F0-F2C4-4DA4-A703-76F720E49212"
#define UUID_HOUSETYPE_EXTENSION            "9146FAFE-9352-4E88-A660-AE720D80DF1C"
#define UUID_INFANTRY_EXTENSION             "641151C0-8622-4453-9C2E-65110F31C147"
#define UUID_INFANTRYTYPE_EXTENSION         "6A07DC7A-CCEC-4211-A194-06728659B0FF"
#define UUID_ISOTILE_EXTENSION              "CFAD340B-63F1-45A0-BC52-32BEFC682201"
#define UUID_ISOTILETYPE_EXTENSION          "A0E9C134-2FF9-429B-9E32-67C843A69969"
#define UUID_LIGHT_EXTENSION                "B66943F4-02C8-472C-A641-247105DCFA26"
#define UUID_OVERLAY_EXTENSION              "A728F930-574E-403A-8E0C-1FEDFAB3432F"
#define UUID_OVERLAYTYPE_EXTENSION          "909D8654-24FF-4578-8DE4-EBBD4FB4400A"
#define UUID_PARTICLE_EXTENSION             "5F6578B0-E093-4EB1-B2F6-18AC29FEAA5A"
#define UUID_PARTICLETYPE_EXTENSION         "530B0567-0BAE-4EA6-A09C-491A62ED34DC"
#define UUID_PARTICLESYSTEM_EXTENSION       "D6956628-11A0-46F7-B192-B10C88AE3AE7"
#define UUID_PARTICLESYSTEMTYPE_EXTENSION   "D73A1DEE-D9E3-4695-90A9-26C4F1F62D59"
#define UUID_SCRIPT_EXTENSION               "B5E5B269-7622-4424-B1EA-D74738D437CE"
#define UUID_SCRIPTTYPE_EXTENSION           "AA9F56E7-1D25-4A0C-9381-A12FE3EECC06"
#define UUID_SIDE_EXTENSION                 "5B3BA576-710E-4895-A64E-8F0A26F7C4CC"
#define UUID_SMUDGE_EXTENSION               "F611D411-1F93-43BD-91E3-8775451A5BD2"
#define UUID_SMUDGETYPE_EXTENSION           "A5901451-8B24-49B4-A985-7B088E331633"
#define UUID_SPECIAL_EXTENSION              "05844DB1-CF6A-4CC7-86BB-0D26D4CADB1C"
#define UUID_SUPERWEAPONTYPE_EXTENSION      "DE57E079-BDC6-44C7-B543-50FB496E03F5"
#define UUID_TASKFORCE_EXTENSION            "00F924D9-D9BA-4377-9003-EA87A4806852"
#define UUID_TEAM_EXTENSION                 "0EB26DF4-63C4-4F78-A31D-44092C029AF0"
#define UUID_TEAMTYPE_EXTENSION             "3D99E462-372C-4839-8786-4A9B5B7DF5F7"
#define UUID_TERRAIN_EXTENSION              "20894849-0FA8-4605-9932-33E0A6B6AD34"
#define UUID_TERRAINTYPE_EXTENSION          "063C2121-A2B0-40BC-A99E-0980861AF5FF"
#define UUID_TRIGGER_EXTENSION              "1C97F13C-6436-481E-80EE-2C561C54D520"
#define UUID_TRIGGERTYPE_EXTENSION          "B315CE9A-3E3D-4D1A-AAE1-AEE337FFB449"
#define UUID_UNITTYPE_EXTENSION             "6582C5FD-00D2-4FB3-AB4D-3AA4CB07BA33"
#define UUID_VOXELANIM_EXTENSION            "35BADA82-EC34-48D1-A0C4-FA63D00050E0"
#define UUID_VOXELANIMTYPE_EXTENSION        "EB378E30-D869-422C-9B4D-6B35B1843721"
#define UUID_WAVE_EXTENSION                 "1CAC2D6C-8427-46EF-B34E-9679A586FBC8"
#define UUID_TAG_EXTENSION                  "C792A1A6-0E33-45A4-9F06-EB65C320B0FE"
#define UUID_TAGTYPE_EXTENSION              "829A522D-2E52-4AD0-A85A-AA5CBBC26B58"
#define UUID_TIBERIUM_EXTENSION             "304CB21E-6D4F-4AFF-803A-795D050F5764"
#define UUID_ACTION_EXTENSION               "84EC9941-BFE0-4E12-A2D8-511B92CAD3AE"
#define UUID_EVENT_EXTENSION                "D65EB592-69B6-4EBA-A2FC-DADA0879DAE7"
#define UUID_WEAPONTYPE_EXTENSION           "EDDB6074-03E4-4DF4-B883-DD48F583506A"
#define UUID_WARHEADTYPE_EXTENSION          "DC9AD11A-AB41-42AC-A7FC-C7AF81D12017"
#define UUID_WAYPOINT_EXTENSION             "C9838D5D-706C-4946-8511-EFC464C919CD"
#define UUID_ABSTRACT_EXTENSION             "957275D3-8C3E-43C8-AB21-37FFA70D8E8B"
#define UUID_TUBE_EXTENSION                 "AFEF972F-A12F-4B8F-8C65-4EE55A079C04"
#define UUID_LIGHTSOURCE_EXTENSION          "AAA984DB-720E-42F6-A9FF-C7A8C121C578"
#define UUID_EMPULSE_EXTENSION              "9B06BCF6-0EAC-4D9A-9B13-1112EABFF0CC"
#define UUID_TACTICALMAP_EXTENSION          "31EA713F-A141-4160-AB07-906674887839"
#define UUID_SUPERWEAPON_EXTENSION          "661ED23D-FDB0-46BC-B435-CD8BC0DDE87F"
#define UUID_AITRIGGER_EXTENSION            "08BE496C-282C-4E4F-9AA2-36950F7C5215"
#define UUID_AITRIGGERTYPE_EXTENSION        "9C1B8527-6DC1-420B-A948-CAA81589E624"
#define UUID_NEURON_EXTENSION               "4599C976-F74F-431C-A63D-E1FD6B36480F"
#define UUID_FOGGEDOBJECT_EXTENSION         "7D9C5263-465F-42CE-AD81-5C057B52226F"
#define UUID_ALPHASHAPE_EXTENSION           "4C8171D5-E7A7-43D1-80F3-0C285CF6B352"
#define UUID_VEINHOLEMONSTER_EXTENSION      "4AD76F43-090A-44BF-BB1A-5BFDE52BC842"


/**
 *  The maximum amount of waypoints available for a scenario to use.
 */
#define NEW_WAYPOINT_COUNT SHRT_MAX // "AVLG"


/**
 *  Extension of the RTTIType enum.
 */
enum ExtRTTIType
{
    EXT_RTTI_PAD = RTTI_VEINHOLEMONSTER, // The last RTTIType

    /**
     *  Add new ExtRTTITypes from here.
     */
    EXT_RTTI_SPAWN_MANAGER,

    /**
     *  The new total ExtRTTITypes count.
     */
    EXT_RTTI_COUNT,

    /**
     *  The first ExtRTTITypes.
     */
    EXT_RTTI_FIRST = EXT_RTTI_PAD + 1
};
DEFINE_ENUMERATION_OPERATORS(ExtRTTIType);


enum TargetZoneScanType
{
    TZST_SAME,
    TZST_ANY,
    TZST_INRANGE
};


/**
 *  Production flags that are used for factory selection.
 */
enum ProductionFlags
{
    PRODFLAG_NONE = 0,
    PRODFLAG_NAVAL = 1 << 0,
    PRODFLAG_DEFENSE = 1 << 1
};
DEFINE_ENUMERATION_OPERATORS(ProductionFlags);


/**
 *  Prerequisite group enum.
 */
enum PrerequisiteGroupType
{
    PREREQ_GROUP_FIRST = 0,

    PREREQ_GROUP_NONE = -1
};
DEFINE_ENUMERATION_OPERATORS(PrerequisiteGroupType);


/**
 *  Extension of the TActionType enum.
 */
typedef enum ExtTActionType
{
    EXT_TACTION_PAD = TACTION_TALK_BUBBLE, // The last TActionType

    /**
     *  Add new ExtTActionTypes from here.
     */
    EXT_TACTION_GIVE_CREDITS,
    EXT_TACTION_ENABLE_SHORT_GAME,
    EXT_TACTION_DISABLE_SHORT_GAME,
    EXT_TACTION_CREATE_BUILDING_AT,
    EXT_TACTION_HOUSE_DESTROY_ALL,
    EXT_TACTION_MAKE_ELITE,
    EXT_TACTION_ENABLE_ALLYREVEAL,
    EXT_TACTION_DISABLE_ALLYREVEAL,
    EXT_TACTION_CREATE_AUTOSAVE,
    EXT_TACTION_DELETE_OBJECT,
    EXT_TACTION_ALL_ASSIGN_MISSION,
    EXT_TACTION_MAKE_ALLY_ONE_WAY,
    EXT_TACTION_MAKE_ENEMY_ONE_WAY,
    EXT_TACTION_MODIFY_GLOBAL_CONSTANT,
    EXT_TACTION_MODIFY_GLOBAL_GLOBAL,
    EXT_TACTION_MODIFY_GLOBAL_LOCAL,
    EXT_TACTION_INCREMENT_GLOBAL,
    EXT_TACTION_DECREMENT_GLOBAL,
    EXT_TACTION_MODIFY_LOCAL_CONSTANT,
    EXT_TACTION_MODIFY_LOCAL_GLOBAL,
    EXT_TACTION_MODIFY_LOCAL_LOCAL,
    EXT_TACTION_INCREMENT_LOCAL,
    EXT_TACTION_DECREMENT_LOCAL,
    EXT_TACTION_RANDOM_NUMBER_GLOBAL,
    EXT_TACTION_RANDOM_NUMBER_LOCAL,
    EXT_TACTION_PRINT_GLOBAL,
    EXT_TACTION_PRINT_LOCAL,
    EXT_TACTION_ENABLE_TEMPLATED_TEXT,
    EXT_TACTION_DISABLE_TEMPLATED_TEXT,
    EXT_TACTION_ADJUST_HOUSE_MODIFIER,
    EXT_TACTION_APPLY_IRON_CURTAIN,
    EXT_TACTION_STOP_SOUNDS_AT,
    EXT_TACTION_ATTACH_SOUND,
    EXT_TACTION_DETACH_SOUND,

    /**
     *  The new total ExtTActionType count.
     */
    EXT_TACTION_COUNT,

    /**
     *  The first ExtTActionType.
     */
    EXT_TACTION_FIRST = EXT_TACTION_PAD + 1
} ExtTActionType;


/**
 *  Extension of the TActionType enum.
 */
typedef enum ExtTEventType
{
    EXT_TEVENT_PAD = TEVENT_LIMPED, // The last TEventType

    /**
     *  Add new ExtTEventTypes from here.
     */
    EXT_TEVENT_COMPARE_GLOBAL_WITH_CONSTANT,
    EXT_TEVENT_COMPARE_GLOBAL_WITH_GLOBAL,
    EXT_TEVENT_COMPARE_GLOBAL_WITH_LOCAL,
    EXT_TEVENT_GLOBAL_EQUALS_CONSTANT,
    EXT_TEVENT_GLOBAL_EQUALS_GLOBAL,
    EXT_TEVENT_GLOBAL_EQUALS_LOCAL,
    EXT_TEVENT_GLOBAL_GREATER_THAN_CONSTANT,
    EXT_TEVENT_GLOBAL_GREATER_THAN_GLOBAL,
    EXT_TEVENT_GLOBAL_GREATER_THAN_LOCAL,
    EXT_TEVENT_GLOBAL_LESS_THAN_CONSTANT,
    EXT_TEVENT_GLOBAL_LESS_THAN_GLOBAL,
    EXT_TEVENT_GLOBAL_LESS_THAN_LOCAL,
    EXT_TEVENT_COMPARE_LOCAL_WITH_CONSTANT,
    EXT_TEVENT_COMPARE_LOCAL_WITH_GLOBAL,
    EXT_TEVENT_COMPARE_LOCAL_WITH_LOCAL,
    EXT_TEVENT_LOCAL_EQUALS_CONSTANT,
    EXT_TEVENT_LOCAL_EQUALS_GLOBAL,
    EXT_TEVENT_LOCAL_EQUALS_LOCAL,
    EXT_TEVENT_LOCAL_GREATER_THAN_CONSTANT,
    EXT_TEVENT_LOCAL_GREATER_THAN_GLOBAL,
    EXT_TEVENT_LOCAL_GREATER_THAN_LOCAL,
    EXT_TEVENT_LOCAL_LESS_THAN_CONSTANT,
    EXT_TEVENT_LOCAL_LESS_THAN_GLOBAL,
    EXT_TEVENT_LOCAL_LESS_THAN_LOCAL,
    EXT_TEVENT_BUILDING_DOES_NOT_EXIST,

    /**
     *  The new total ExtTEventType count.
     */
    EXT_TEVENT_COUNT,

    /**
     *  The first ExtTEventType.
     */
    EXT_TEVENT_FIRST = EXT_TEVENT_PAD + 1
} ExtTEventType;


/**
 *  Extension of the EventType enum.
 */
enum ExtEventType {
    EXT_EVENT_PAD = EVENT_LATENCYFUDGE, // The last EventType

    /**
     *  Add new ExtEventTypes from here.
     */
    EXT_EVENT_PLAYER_OPTIONS,
    EXT_EVENT_RESPONSE_TIME2,

    /**
     *  The new total ExtEventTypes count.
     */
    EXT_EVENT_COUNT,

    /**
     *  The first ExtEventTypes.
     */
    EXT_EVENT_FIRST = EXT_EVENT_PAD + 1
};
DEFINE_ENUMERATION_OPERATORS(ExtEventType);


/**
 *  Extension of the ActionType enum.
 */
enum ExtActionType {
    EXT_ACTION_PAD = ACTION_ATTACK_SUPPORT, // The last ActionType

    /**
     *  Add new ExtEventTypes from here.
     */
    EXT_ACTION_PLACE_BEACON,
    EXT_ACTION_PLACE_BEACON_1,
    EXT_ACTION_PLACE_BEACON_2,
    EXT_ACTION_PLACE_BEACON_3,
    EXT_ACTION_PLACE_BEACON_4,
    EXT_ACTION_PLACE_BEACON_5,
    EXT_ACTION_PLACE_BEACON_6,
    EXT_ACTION_PLACE_BEACON_7,
    EXT_ACTION_SELECT_BEACON,

    /**
     *  The new total ExtEventTypes count.
     */
    EXT_ACTION_COUNT,

    /**
     *  The first ExtEventTypes.
     */
    EXT_ACTION_FIRST = EXT_ACTION_PAD + 1
};
DEFINE_ENUMERATION_OPERATORS(ExtActionType);

/**
 *  New global packet types.
 */
enum ExtNetCommandType {
    EXT_NET_BEACON_PLACE = NET_PROPOSE_KICK + 1,
    EXT_NET_BEACON_DELETE,
    EXT_NET_BEACON_TEXT,
    EXT_NET_LOAD_GAME,
    EXT_NET_HOST_ANNOUNCE,      // Sent by the game host to let the other players know who the host is.
    EXT_NET_DESYNC_HEARTBEAT,   // Sent periodically while the desync dialog is open; keeps connections alive and detects departures.
    EXT_NET_DESYNC_CONTINUE,    // The host's decision to continue the game without the desynced players.
    EXT_NET_MOVIE_SKIP_VOTE,    // A player's request to skip the currently playing fullscreen movie.
};

/**
 *  Extended struct for new global packet types.
 */
#pragma pack(1)
struct ExtGlobalPacketType {
    ExtNetCommandType Command;
    char Name[MPLAYER_NAME_MAX];
    char Serial[SERIAL_MAX];
    union {
        struct {
            CoordStruct Position;
            char House;
            int Number;
        } PlaceBeacon;
        struct {
            char House;
            int Number;
        } DeleteBeacon;
        struct {
            char Text[256];
            int Number;
            char House;
        } BeaconText;
        struct {
            char Buf[370];
            char Scope[30];
            PlayerColorType Color;
            unsigned long NameCRC;
        } Message;
        struct {
            int ID;
        } SaveInfo;
        struct {
            char HouseID;
            char IsHost;
        } Heartbeat;
        struct {
            unsigned long Context;
            unsigned long Movie;
            unsigned long Instance;
        } MovieSkipVote;
        char padding[455 - sizeof(Command) - sizeof(Name) - sizeof(Serial)];
    };
};
#pragma pack()

static_assert(sizeof(ExtGlobalPacketType) == sizeof(GlobalPacketType), "ExtGlobalPacketType size is wrong!");

enum ExtThreatType {
    EXT_THREAT_HARVESTERS = 0x8000 // Limit scan to harvesters only
};
DEFINE_ENUMERATION_OPERATORS(ExtThreatType);
DEFINE_ENUMERATION_BITWISE_OPERATORS(ExtThreatType);

/**
 *  Extension of the ActionType enum.
 */
enum ExtQuarryType {
    EXT_QUARRY_PAD = QUARRY_POWER,            // The last QuarryType

    /**
     *  Add new ExtActionTypes from here.
     */
    EXT_QUARRY_HARVESTERS,   // Attack harvesters only (no refineries).

    /**
     *  The new total ExtActionType count.
     */
    EXT_QUARRY_COUNT,

    /**
     *  The first ExtActionType.
     */
    EXT_QUARRY_FIRST = 0
};
DEFINE_ENUMERATION_OPERATORS(ExtQuarryType);

/**
 *  Extension of the Difficulty enum.
 */
enum ExtDiffType {
    EXT_DIFF_PAD = DIFF_HARD, // The last DiffType

    /**
     *  Add new ExtDiffTypes from here.
     */
    EXT_DIFF_ULTIMATELY_EASY,
    EXT_DIFF_EXTREMELY_EASY,
    EXT_DIFF_BRUTALLY_EASY,
    EXT_DIFF_VERY_EASY,

    /**
     *  The new total ExtDiffType count.
     */
    EXT_DIFF_COUNT,

    /**
     *  The first ExtDiffType.
     */
    EXT_DIFF_FIRST = EXT_DIFF_PAD + 1
};
DEFINE_ENUMERATION_OPERATORS(ExtDiffType);

/**
 *  Returns the name for a DiffType value. Reflects how the bonuses affect *this* house.
 *  For viewing CDifficulty or showing how "tough" an AI is, use CDifficulty_Name instead.
 */
inline const char *Difficulty_Name(DiffType d)
{
    switch (static_cast<int>(d)) {
    case EXT_DIFF_ULTIMATELY_EASY:  return "Ultimately Easy";
    case EXT_DIFF_EXTREMELY_EASY:   return "Extremely Easy";
    case EXT_DIFF_BRUTALLY_EASY:    return "Brutally Easy";
    case EXT_DIFF_VERY_EASY:        return "Very Easy";
    case DIFF_EASY:                 return "Easy";
    case DIFF_NORMAL:               return "Medium";
    case DIFF_HARD:                 return "Hard";
    default:                        return "?";
    }
}

/**
 *  Returns a short "AI Difficulty" name for a DiffType value.
 *  Reflects how "tough" an AI is, so the bonuses are reversed compared to Difficulty_Name.
 */
inline const char *CDifficulty_Name(DiffType d)
{
    switch (static_cast<int>(d)) {
    case EXT_DIFF_ULTIMATELY_EASY:  return "Ultimate";
    case EXT_DIFF_EXTREMELY_EASY:   return "Extreme";
    case EXT_DIFF_BRUTALLY_EASY:    return "Brutal";
    case EXT_DIFF_VERY_EASY:        return "Very Hard";
    case DIFF_EASY:                 return "Hard";
    case DIFF_NORMAL:               return "Medium";
    case DIFF_HARD:                 return "Easy";
    default:                        return "?";
    }
}


/**
 *  Extension of the HousesType enum.
 */
enum ExtHousesType {
    EXT_HOUSE_SPAWN1 = 50,
    EXT_HOUSE_SPAWN2,
    EXT_HOUSE_SPAWN3,
    EXT_HOUSE_SPAWN4,
    EXT_HOUSE_SPAWN5,
    EXT_HOUSE_SPAWN6,
    EXT_HOUSE_SPAWN7,
    EXT_HOUSE_SPAWN8
};
DEFINE_ENUMERATION_OPERATORS(ExtHousesType);


/**
 *  Extension of the SpecialDialogType enum.
 */
typedef enum ExtSpecialDialogType {
    EXT_SDLG_PAD = SDLG_SPECIAL,

    /**
     *  Add new ExtSpecialDialogTypes from here.
     */
    EXT_SDLG_LOAD
} ExtSpecialDialogType;
