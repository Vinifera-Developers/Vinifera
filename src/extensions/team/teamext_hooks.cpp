/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEAMEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TeamClass.
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
#include "teamext_hooks.h"
#include "teamext_init.h"
#include "team.h"
#include "cell.h"
#include "foot.h"
#include "footext.h"
#include "house.h"
#include "rulesext.h"
#include "tag.h"
#include "taskforce.h"
#include "technotypeext.h"
#include "weapontype.h"
#include "extension.h"
#include "iomap.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "building.h"

#include "hooker.h"
#include "scripttype.h"
#include "syringe.h"
#include "teamtype.h"
#include "vinifera_defines.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TeamClass)
{
public:
    void _TMission_ATTACK(ScriptMissionClass * mission, bool a1);
};


/**
 *  #issue-196
 * 
 *  Fixes incorrect cell calculation for the MOVECELL script.
 * 
 *  The original code used outdated code from Red Alert to calculate
 *  the cell position on the map.
 * 
 *  @author: CCHyper (based on research by E1Elite)
 */
DEFINE_HOOK(0x00622B2C, _TeamClass_AI_MoveCell_FixCellCalc_Patch, 0)
{
    GET_STACK(unsigned, argument, 0x24);

    /**
     *  Get the cell X and Y position from the script argument.
     */
    Cell tmpcell;
    if (NewINIFormat < 4) {
        tmpcell.X = argument % 256;
        tmpcell.Y = argument / 256;
    } else {
        tmpcell.X = argument % 1000;
        tmpcell.Y = argument / 1000;
    }

    /**
     *  Fetch the map cell. Added pointer check to make sure the
     *  script didn't have an invalid position.
     */
    CellClass* cell = &Map[tmpcell];
    if (!cell) {
        goto coordinate_move;
    }

    /**
     *  The Assign_Mission_Target call pushes EAX into the stack
     *  for the cell argument.
     */
    R->EAX(cell);

assign_mission_target:
    return 0x00622B5F;

coordinate_move:
    return 0x00622B19;
}


/**
 *  #issue-71
 *
 *  Increases the amount of available waypoints (see ScenarioClassExtension for implementation).
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00625886, _TeamClass_TMission_PATROL_WaypointMax, 0)
{
    GET(ScriptMissionClass*, mission, EAX);

    if (mission->Data.Value < NEW_WAYPOINT_COUNT) {
        return 0x0062588C;
    }

    return 0x00625894;
}


/**
 *  TeamClassExt::TMission_ATTACK re-implementation.
 *
 *  @author: tomsons26, ZivDero, modifications by Rampastring
 */
void TeamClassExt::_TMission_ATTACK(ScriptMissionClass* mission, bool)
{
    if (MissionTarget == nullptr && Member != nullptr) {

        /*
        **	Pick a team leader that has a weapon. Only in the case of no
        **	team members having any weapons, will a member without a weapon
        **	be chosen.
        */
        FootClass const* candidate = Fetch_A_Leader();

        /*
        **	Have the team leader pick what the next team target will be.
        */
        switch (mission->Data.Quarry) {
        case QUARRY_ANYTHING:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_NORMAL, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_BUILDINGS:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_BUILDINGS, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_HARVESTERS:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_TIBERIUM, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_INFANTRY:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_INFANTRY, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_VEHICLES:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_VEHICLES, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_FACTORIES:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_FACTORIES, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_DEFENSE:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_BASE_DEFENSE, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_THREAT:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_NORMAL, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case QUARRY_POWER:
            Assign_Mission_Target(candidate->Greatest_Threat(THREAT_POWER, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        case EXT_QUARRY_HARVESTERS:
            Assign_Mission_Target(candidate->Greatest_Threat((ThreatType)EXT_THREAT_HARVESTERS, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
            break;

        default:
            break;
        }
        if (MissionTarget == nullptr || !Ammo_Check()) IsNextMission = true;
    }
    if (MissionTarget == nullptr || !Ammo_Check()) IsNextMission = true;

    Coordinate_Attack();
}


enum TargetPropertyType
{
    TPROPERTY_LEAST_THREAT,
    TPROPERTY_GREATEST_THREAT,
    TPROPERTY_NEAREST,
    TPROPERTY_FARTHEST,

    TPROPERTY_COUNT,
};


/*
**  Fixes a bug where the AI does not ignore buildings in limbo when selecting a BwP target.
**  Also makes BwP scan respect TargetZoneScanType.
**
**  @author: tomsons26/ZivDero for original code, Rampastring for fixing the aforementioned bug
**           and implementing TargetZoneScanType functionality.
*/
BuildingClass* _Pick_Building_With_Property(BuildingTypeClass* type, HouseClass* house, FootClass* unit, TargetPropertyType prop, bool only_enemy)
{
    int best_same_dist = -1;
    BuildingClass* best_same_ptr = nullptr;
    int best_dist = -1;
    BuildingClass* best_ptr = nullptr;

    TargetZoneScanType tzst = Extension::Fetch(unit->TClass)->TargetZoneScan;
    int ourzone = Map.Get_Cell_Zone(unit->Center_Coord().As_Cell(), unit->TClass->MZone, true);

    for (int index = 0; index < Buildings.Count(); index++) {

        BuildingClass* ptr = Buildings[index];

        if (!ptr->IsActive || ptr->IsInLimbo || !ptr->IsDown) {
            continue;
        }

        HouseClass* hptr = ptr->House;

        bool same_house = hptr == house;

        if (ptr->Class == type && (same_house || !unit->House->Is_Ally(ptr->House))) {

            if (tzst == TargetZoneScanType::TZST_SAME) {
                int targetzone = Map.Get_Cell_Zone(ptr->Center_Coord().As_Cell(), unit->TClass->MZone, false);
                if (targetzone != ourzone) {
                    continue;
                }
            }
            else if (tzst == TargetZoneScanType::TZST_INRANGE)
            {
                // If the zone is different, only allow targeting if we can reach the target from our zone.

                int targetzone = Map.Get_Cell_Zone(ptr->Center_Coord().As_Cell(), unit->TClass->MZone, false);

                if (ourzone != targetzone) 
                {
                    Cell nearbycell = Map.Nearby_Location(ptr->Center_Coord().As_Cell(),
                        unit->TClass->Speed,
                        /*Phobos has -1 here*/ ourzone,
                        unit->TClass->MZone,
                        false, Point2D(1, 1), true, false, false, unit->TClass->Speed != SPEED_FLOAT);

                    if (nearbycell == CELL_NONE) {
                        // We couldn't find a valid cell to reach the target from
                        continue;
                    }

                    int distance = ::Distance(nearbycell, ptr->Center_Coord().As_Cell());

                    WeaponSlotType weaponslot = unit->What_Weapon_Should_I_Use(ptr);
                    auto weaponinfo = unit->Get_Weapon(weaponslot);
                    if (weaponinfo->Weapon == nullptr) {
                        continue;
                    }

                    if (distance * CELL_LEPTON_W >= weaponinfo->Weapon->Range) {
                        continue;
                    }
                }
            }

            int dist = -1;

            switch (prop) {
            case TPROPERTY_LEAST_THREAT:
                dist = INT_MAX - Map.Cell_Threat(ptr->Center_Coord().As_Cell(), unit->House);
                break;

            case TPROPERTY_GREATEST_THREAT:
                dist = Map.Cell_Threat(ptr->Center_Coord().As_Cell(), unit->House);
                break;

            case TPROPERTY_NEAREST:
                dist = INT_MAX - ptr->Get_Coord().Distance_To(unit->Get_Coord());
                break;

            case TPROPERTY_FARTHEST:
                dist = ptr->Get_Coord().Distance_To(unit->Get_Coord());
                break;

            }

            if (dist > best_same_dist && same_house) {
                best_same_ptr = ptr;
                best_same_dist = dist;
            }
            if (dist > best_dist) {
                best_ptr = ptr;
                best_dist = dist;
            }
        }
    }

    if (best_same_ptr) {
        return best_same_ptr;
    }

    if (!only_enemy) {
        return best_ptr;
    }

    return nullptr;
}


/**
 *  Main function for patching the hooks.
 */
void TeamClassExtension_Hooks()
{
    TeamClassExtension_Init();

    Patch_Jump(0x00625B90, &TeamClassExt::_TMission_ATTACK);
    Patch_Jump(0x006271F0, &_Pick_Building_With_Property);
}
