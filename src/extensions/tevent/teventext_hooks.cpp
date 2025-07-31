/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEVENTEXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended TEventClass.
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
#include "teventext_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "tevent.h"
#include "scenario.h"
#include "scenarioext.h"
#include "voc.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "house.h"
#include "object.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "building.h"
#include "session.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "mouse.h"
#include "rules.h"
#include "team.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TEventClass)
{
public:
    bool _Operator_Parens_Intercept(TEventType event, HouseClass const* house, ObjectClass const* object, CDTimerClass<FrameTimerClass> &timer, bool& is_perm, TechnoClass * source);
    bool _Is_Temporal() const;
    bool _Has_Memory() const;
};


/**
 *  Intercept for TEventClass::operator() to add the
 *  execution of our new TEVents.
 *
 *  @author: ZivDero
 */
bool TEventClassExt::_Operator_Parens_Intercept(TEventType event, HouseClass const* house, ObjectClass const* object, CDTimerClass<FrameTimerClass>& timer, bool& is_perm, TechnoClass* source)
{
    /*
    **  Triggers based on the game's global environment such as time or
    **  global flags are triggered only when the appropriate condition
    **  is true.
    */
    switch (Event) {
    case TEVENT_AMBIENT_LESS_THAN:
        return Scen->CurrentAmbientLight <= Data.Value;
        break;

    case TEVENT_AMBIENT_GREATER_THAN:
        return Scen->CurrentAmbientLight >= Data.Value;
        break;

    case TEVENT_GAME_TIME:
        if (Data.Value > Frame / TICKS_PER_SECOND) {
            return false;
        } else {
            return true;
        }
        break;

    case TEVENT_GLOBAL_SET: {
        bool value;
        Scen->Get_Global_Value(Data.Value, value);
        return value;
    }

    case TEVENT_GLOBAL_CLEAR: {
        bool value;
        Scen->Get_Global_Value(Data.Value, value);
        return!value;
    }

    case TEVENT_LOCAL_SET: {
        bool value;
        Scen->Get_Local_Value(Data.Value, value);
        return value;
    }

    case TEVENT_LOCAL_CLEAR: {
        bool value;
        Scen->Get_Local_Value(Data.Value, value);
        return !value;
    }

    case TEVENT_MISSION_TIMER_EXPIRED:
        if (!Scen->MissionTimer.Is_Active() || Scen->MissionTimer != 0) return false;
        return true;

    case TEVENT_TIME:
    case TEVENT_RANDOM_TIME:
        if (timer != 0) return false;
        return true;

    }

    /*
    **  Don't trigger this event if the parameters mean nothing. Typical of
    **  this would be for events related to time or other outside influences.
    */
    if (Event == TEVENT_NONE) {
        return false;
    }

    /*
    **  If this is not the event for this trigger, just return. This is only
    **  necessary to check for those trigger events that are presumed to be
    **  true just by the fact that this routine is called with the appropriate
    **  event identifier.
    */
    if (Event != TEVENT_ATTACKED_BY &&
        Event != TEVENT_PLAYER_ENTERED &&
        Event != TEVENT_CROSS_HORIZONTAL &&
        Event != TEVENT_CROSS_VERTICAL &&
        Event != TEVENT_NEAR_WAYPOINT &&
        Event != TEVENT_ENTERS_ZONE &&
        Event != TEVENT_BUILD &&
        Event != TEVENT_BUILD_UNIT &&
        Event != TEVENT_BUILD_INFANTRY &&
        Event != TEVENT_BUILD_AIRCRAFT &&
        Is_Temporal()) {
        if (event != Event || Debug_Map) {
            return false;
        }
    }

    /*
    **  The cell entry trigger event is only tripped when an object of the
    **  matching ownership has entered the cell in question. All other
    **  conditions will not trigger the event.
    */
    if (Event == TEVENT_PLAYER_ENTERED || Event == TEVENT_CROSS_HORIZONTAL || Event == TEVENT_CROSS_VERTICAL || Event == TEVENT_ENTERS_ZONE) {
        if (event != Event) return false;
        if (!object || (Data.House != HOUSE_NONE && object->Owner() != House_From_HousesType(Data.House)->HeapID)) return false;
        is_perm = true;
        return true;
    } else if (Event == TEVENT_NEAR_WAYPOINT) {
        if (event != Event) return false;
        assert(object != NULL);
        Coord waypoint_location(Scen->Waypoint_Coord(Data.Value));
        if (object->Distance(waypoint_location) > CELL_LEPTON_W * 5) {
            return false;
        }
        return true;
    } else if (Event == TEVENT_ATTACKED_BY) {
        if (event != Event) return false;
        if (source == nullptr || Data.House != source->House->HeapID) {
            return false;
        }
    }

    /*
    **  The following trigger events are not considered to have sprung
    **  merely by fact that this routine has been called. These trigger
    **  events must be verified manually by examining the house that
    **  they are assigned to.
    */
    int index;
    if (house != nullptr) {
        int count;
        switch (Event) {
            /*
            **  Check to see if a team of the appropriate type has left the map.
            */
        case TEVENT_LEAVES_MAP:
            for (index = 0; index < Teams.Count(); index++) {
                TeamClass* ptr = Teams[index];
                if (ptr->Class == Team && ptr->Is_Empty() && ptr->IsLeaveMap) {
                    is_perm = true;
                    break;
                }
            }
            if (index == Teams.Count()) return false;
            break;

            /*
            **  Credits must be equal or greater to the value specified.
            */
        case TEVENT_CREDITS:
            if (const_cast<HouseClass*>(house)->Available_Money() < Data.Value) return false;
            break;

        case TEVENT_CREDITS_BELOW:
            if (const_cast<HouseClass*>(house)->Available_Money() > Data.Value) return false;
            break;

            /*
            **  Ensure that there are no more factories left.
            */
        case TEVENT_NOFACTORIES:
            if (house->CurBuildings > 0) {
                for (int index = 0; index < Buildings.Count(); index++) {
                    BuildingClass* ptr = Buildings[index];
                    if (ptr != nullptr && !ptr->IsInLimbo && ptr->House == house && ptr->Class->ToBuild != RTTI_NONE) {
                        return false;
                    }
                }
            }
            break;

            /*
            **  A civilian must have been evacuated.
            */
        case TEVENT_EVAC_CIVILIAN:
            if (!house->IsCivEvacuated) return false;
            break;

            /*
            **  Verify that the structure has been built.
            */
        case TEVENT_BUILDING_EXISTS:
            if (house->BQuantity.Value(Data.Structure) == 0) return false;
            is_perm = true;
            break;

            /*
            **  Verify that the structure has been built.
            */
        case TEVENT_BUILD:
            if (house->JustBuiltStructure != Data.Structure) return false;
            is_perm = true;
            break;

            /*
            **  Verify that the unit has been built.
            */
        case TEVENT_BUILD_UNIT:
            if (house->JustBuiltUnit != Data.Unit) return false;
            is_perm = true;
            break;

            /*
            **  Verify that the infantry has been built.
            */
        case TEVENT_BUILD_INFANTRY:
            if (house->JustBuiltInfantry != Data.Infantry) return false;
            is_perm = true;
            break;

            /*
            **  Verify that the aircraft has been built.
            */
        case TEVENT_BUILD_AIRCRAFT:
            if (house->JustBuiltAircraft != Data.Aircraft) return false;
            is_perm = true;
            break;

            /*
            **  Verify that the specified number of buildings have been destroyed.
            */
        case TEVENT_NBUILDINGS_DESTROYED:
            if (house->BuildingsLost < Data.Value) return false;
            break;

            /*
            **  Verify that the specified number of units have been destroyed.
            */
        case TEVENT_NUNITS_DESTROYED:
            if (house->UnitsLost < Data.Value) return false;
            break;

        default:
            break;
        }
    }

    house = House_From_HousesType(Data.House);
    if (house != nullptr) {
        switch (Event) {
        case TEVENT_LOW_POWER:
            if (house->Power_Fraction() >= 1) return false;
            break;

        case TEVENT_THIEVED:
            if (!house->IsThieved) return false;
            break;

            /*
            **  Verify that the house has been discovered.
            */
        case TEVENT_HOUSE_DISCOVERED:
            if (!house->IsDiscovered) return false;
            break;

            /*
            **  Verify that all buildings have been destroyed.
            */
        case TEVENT_BUILDINGS_DESTROYED:
            if (house->CurBuildings > 0) return false;
            break;

            /*
            **  Verify that all units have been destroyed -- with some
            **  exceptions.
            */
        case TEVENT_UNITS_DESTROYED:
            if (house->CurUnits > 0 || house->CurInfantry > 0) return false;
            break;

            /*
            **  Verify that all buildings and units have been destroyed.
            */
        case TEVENT_ALL_DESTROYED:
            if (house->CurBuildings > 0 || house->CurUnits > 0 || house->CurInfantry > 0) return false;
            break;

        default:
            break;
        }
    }

    return true;
}


/**
 *  Is trigger based on temporal event rather than game state?
 *
 *  @author: ZivDero
 */
bool TEventClassExt::_Is_Temporal() const
{
    switch (Event) {
    case TEVENT_PICKUP_CRATE:
    case TEVENT_PICKUP_CRATE_ANY:
    case TEVENT_PLAYER_ENTERED:
    case TEVENT_SPIED:
    case TEVENT_THIEVED:
    case TEVENT_DISCOVERED:
    case TEVENT_ATTACKED:
    case TEVENT_ATTACKED_BY:
    case TEVENT_DESTROYED:
    case TEVENT_DESTROYED_ANY:
    case TEVENT_EVAC_CIVILIAN:
    case TEVENT_BUILD:
    case TEVENT_BUILD_UNIT:
    case TEVENT_BUILD_INFANTRY:
    case TEVENT_BUILD_AIRCRAFT:
    case TEVENT_LEAVES_MAP:
    case TEVENT_ENTERS_ZONE:
    case TEVENT_CROSS_HORIZONTAL:
    case TEVENT_CROSS_VERTICAL:
    case TEVENT_SELECTED:
    case TEVENT_NEAR_WAYPOINT:
    case TEVENT_ENEMY_IN_SPOTLIGHT:
    case TEVENT_FIRST_DAMAGED:
    case TEVENT_ENTER_YELLOW:
    case TEVENT_ENTER_RED:
    case TEVENT_FIRST_DAMAGED_ANY:
    case TEVENT_ENTER_YELLOW_ANY:
    case TEVENT_ENTER_RED_ANY:
    case TEVENT_PARALYZED:
    case TEVENT_ENEMY_IN_SPOTLIGHT_REPEATING:
    case TEVENT_LIMPED:
        return true;

    }
    return false;
}


/**
 *  Does this trigger have memory?
 *  For it to have memory means it remembers if it's ever been fired.
 *  For example, with memory TEVENT_BRIDGE_DESTROYED would mean "Has this bridge ever been destroyed".
 *  Without, it activates every time a bridge has been destroyed.
 *
 *  @author: ZivDero
 */
bool TEventClassExt::_Has_Memory() const
{
    switch (Event) {
    case TEVENT_ATTACKED:
    case TEVENT_ATTACKED_BY:
    case TEVENT_PLAYER_ENTERED:
    case TEVENT_PARALYZED:
    case TEVENT_ENEMY_IN_SPOTLIGHT_REPEATING:
        return false;
    }
    return true;
}


/**
 *  Main function for patching the hooks.
 */
void TEventClassExtension_Hooks()
{
    Patch_Jump(0x00642310, &TEventClassExt::_Operator_Parens_Intercept);
    Patch_Jump(0x00642E20, &TEventClassExt::_Is_Temporal);
    Patch_Jump(0x00642E80, &TEventClassExt::_Has_Memory);
}
