/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_DEFINES.H
 *
 *  @authors       CCHyper
 *
 *  @brief         Vinifera defines and constants.
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
#pragma once

#include "always.h"
#include "tibsun_defines.h"


/**
 *  If defined, this will replace the SwizzleManagerClass with our own new implementation.
 */
#define VINIFERA_USE_NEW_SWIZZLE_MANAGER 1

/**
 *  Enable debug printing of the swizzle remapping process.
 * 
 *  WARNING: This will take cause the save/load process to take up to 10 minutes!
 */
#ifndef NDEBUG
//#define VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING 1
#endif

/**
 *  Enable debug printing of class extension creation and destruction process.
 * 
 *  WARNING: This will slow the game down when many instances are created at once.
 */
#ifndef NDEBUG
//#define VINIFERA_ENABLE_EXTENSION_DEBUG_PRINTING 1
#endif


/**
 *  CLSIDs for all new locomotors.
 */
#define CLSID_TEST_LOCOMOTOR                "501DEF92-C7ED-448E-8FEB-7908DCE73377"
#define CLSID_ROCKET_LOCOMOTOR              "B7B49766-E576-11d3-9BD9-00104B972FE8"


/**
 *  CLSIDs for new classes.
 */
#define UUID_ARMORTYPE                      "EE8D505F-12BB-4313-AEDC-4AEA30A5BA03"
#define UUID_ROCKETTYPE                     "FAE72300-A93C-476C-A6DB-CB2B62ADCECD"
#define UUID_SPAWN_MANAGER                  "157ADEE5-D344-48B9-811B-3FA01EF3CCD4"


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


typedef enum ViniferaRTTIType
{
    RTTI_SPAWN_MANAGER = RTTI_COUNT,

    VINIFERA_RTTI_COUNT
};
DEFINE_ENUMERATION_OPERATORS(ViniferaRTTIType);


typedef enum ViniferaDiffType : int
{
    DIFF_VERY_EASY = DIFF_COUNT,
    DIFF_EXTREMELY_EASY,

    VINIFERA_DIFF_COUNT
};
