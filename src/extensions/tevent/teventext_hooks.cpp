/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TEventClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "teventext_hooks.h"

#include "asserthandler.h"
#include "building.h"
#include "hooker.h"
#include "house.h"
#include "houseext.h"
#include "mouse.h"
#include "object.h"
#include "rulesext.h"
#include "scenario.h"
#include "scenarioext.h"
#include "syringe.h"
#include "tag.h"
#include "team.h"
#include "teamtype.h"
#include "tevent.h"
#include "teventext.h"
#include "teventext_init.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "vinifera_defines.h"
#include "voc.h"

// warning C4063: case '#' is not a valid value for switch of enum 'TActionType'
#pragma warning(disable : 4063)

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
    void _Read_INI();
    void _Build_INI_Entry(char* ptr) const;
};


/**
 *  An enum for the various ways variables can be compared.
 */
enum ComparisonType
{
    COMP_GREATER,
    COMP_LESS,
    COMP_EQ,
    COMP_NEQ,
    COMP_GREATER_OR_EQ,
    COMP_LESS_OR_EQ,
    COMP_AND,
    COMP_OR,
    COMP_XOR
};


/**
 *  Comparison helper.
 *
 *  @author: ZivDero
 */
static bool Compare(int lhs, int rhs, ComparisonType type)
{
    switch (type) {
    case COMP_GREATER:
        return lhs > rhs;
    case COMP_LESS:
        return lhs < rhs;
    case COMP_EQ:
        return lhs == rhs;
    case COMP_NEQ:
        return lhs != rhs;
    case COMP_GREATER_OR_EQ:
        return lhs >= rhs;
    case COMP_LESS_OR_EQ:
        return lhs <= rhs;
    case COMP_AND:
        return (lhs & rhs) != 0;
    case COMP_OR:
        return (lhs | rhs) != 0;
    case COMP_XOR:
        return (lhs ^ rhs) != 0;
    default:
        return false;
    }
}


/**
 *  Compares a variable with a constant.
 *
 *  @author: ZivDero
 */
static bool Compare_With_Constant(int left_index, bool left_global, int right, ComparisonType comp)
{
    int left;
    if (!(left_global ? ScenExtension->Get_Global_Value(left_index, left) : ScenExtension->Get_Local_Value(left_index, left))) {
        return false;
    }
    return Compare(left, right, comp);
}


/**
 *  Compares a variable with another variable.
 *
 *  @author: ZivDero
 */
static bool Compare_With_Variable(int left_index, bool left_global, int right_index, bool right_global, ComparisonType comp)
{
    int left;
    if (!(left_global ? ScenExtension->Get_Global_Value(left_index, left) : ScenExtension->Get_Local_Value(left_index, left))) {
        return false;
    }
    int right;
    if (!(right_global ? ScenExtension->Get_Global_Value(right_index, right) : ScenExtension->Get_Local_Value(right_index, right))) {
        return false;
    }
    return Compare(left, right, comp);
}


/**
 *  Intercept for TEventClass::operator() to add the
 *  execution of our new TEvents.
 *
 *  @author: ZivDero
 */
bool TEventClassExt::_Operator_Parens_Intercept(TEventType event, HouseClass const* house, ObjectClass const* object, CDTimerClass<FrameTimerClass>& timer, bool& is_perm, TechnoClass* source)
{
    auto& extension = *Extension::Fetch(this);

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

    case EXT_TEVENT_COMPARE_GLOBAL_WITH_CONSTANT: {
        int left_index = Data.Value;
        ComparisonType comp = static_cast<ComparisonType>(extension.Data2.Value);
        int right = extension.Data3.Value;

        return Compare_With_Constant(left_index, true, right, comp);
    }

    case EXT_TEVENT_COMPARE_GLOBAL_WITH_GLOBAL: {
        int left_index = Data.Value;
        ComparisonType comp = static_cast<ComparisonType>(extension.Data2.Value);
        int right_index = extension.Data3.Value;

        return Compare_With_Variable(left_index, true, right_index, true, comp);
    }

    case EXT_TEVENT_COMPARE_GLOBAL_WITH_LOCAL: {
        int left_index = Data.Value;
        ComparisonType comp = static_cast<ComparisonType>(extension.Data2.Value);
        int right_index = extension.Data3.Value;

        return Compare_With_Variable(left_index, true, right_index, false, comp);
    }

    case EXT_TEVENT_GLOBAL_EQUALS_CONSTANT: {
        int left_index = Data.Value;
        int right = extension.Data2.Value;

        return Compare_With_Constant(left_index, true, right, COMP_EQ);
    }

    case EXT_TEVENT_GLOBAL_EQUALS_GLOBAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, true, right_index, true, COMP_EQ);
    }

    case EXT_TEVENT_GLOBAL_EQUALS_LOCAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, true, right_index, false, COMP_EQ);
    }

    case EXT_TEVENT_GLOBAL_GREATER_THAN_CONSTANT: {
        int left_index = Data.Value;
        int right = extension.Data2.Value;

        return Compare_With_Constant(left_index, true, right, COMP_GREATER);
    }

    case EXT_TEVENT_GLOBAL_GREATER_THAN_GLOBAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, true, right_index, true, COMP_GREATER);
    }

    case EXT_TEVENT_GLOBAL_GREATER_THAN_LOCAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, true, right_index, false, COMP_GREATER);
    }

    case EXT_TEVENT_GLOBAL_LESS_THAN_CONSTANT: {
        int left_index = Data.Value;
        int right = extension.Data2.Value;

        return Compare_With_Constant(left_index, true, right, COMP_LESS);
    }

    case EXT_TEVENT_GLOBAL_LESS_THAN_GLOBAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, true, right_index, true, COMP_LESS);
    }

    case EXT_TEVENT_GLOBAL_LESS_THAN_LOCAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, true, right_index, false, COMP_LESS);
    }

    case EXT_TEVENT_COMPARE_LOCAL_WITH_CONSTANT: {
        int left_index = Data.Value;
        ComparisonType comp = static_cast<ComparisonType>(extension.Data2.Value);
        int right = extension.Data3.Value;

        return Compare_With_Constant(left_index, false, right, comp);
    }

    case EXT_TEVENT_COMPARE_LOCAL_WITH_GLOBAL: {
        int left_index = Data.Value;
        ComparisonType comp = static_cast<ComparisonType>(extension.Data2.Value);
        int right_index = extension.Data3.Value;

        return Compare_With_Variable(left_index, false, right_index, true, comp);
    }

    case EXT_TEVENT_COMPARE_LOCAL_WITH_LOCAL: {
        int left_index = Data.Value;
        ComparisonType comp = static_cast<ComparisonType>(extension.Data2.Value);
        int right_index = extension.Data3.Value;

        return Compare_With_Variable(left_index, false, right_index, false, comp);
    }

    case EXT_TEVENT_LOCAL_EQUALS_CONSTANT: {
        int left_index = Data.Value;
        int right = extension.Data2.Value;

        return Compare_With_Constant(left_index, false, right, COMP_EQ);
    }

    case EXT_TEVENT_LOCAL_EQUALS_GLOBAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, false, right_index, true, COMP_EQ);
    }

    case EXT_TEVENT_LOCAL_EQUALS_LOCAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, false, right_index, false, COMP_EQ);
    }

    case EXT_TEVENT_LOCAL_GREATER_THAN_CONSTANT: {
        int left_index = Data.Value;
        int right = extension.Data2.Value;

        return Compare_With_Constant(left_index, false, right, COMP_GREATER);
    }

    case EXT_TEVENT_LOCAL_GREATER_THAN_GLOBAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, false, right_index, true, COMP_GREATER);
    }

    case EXT_TEVENT_LOCAL_GREATER_THAN_LOCAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, false, right_index, false, COMP_GREATER);
    }

    case EXT_TEVENT_LOCAL_LESS_THAN_CONSTANT: {
        int left_index = Data.Value;
        int right = extension.Data2.Value;

        return Compare_With_Constant(left_index, false, right, COMP_LESS);
    }

    case EXT_TEVENT_LOCAL_LESS_THAN_GLOBAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, false, right_index, true, COMP_LESS);
    }

    case EXT_TEVENT_LOCAL_LESS_THAN_LOCAL: {
        int left_index = Data.Value;
        int right_index = extension.Data2.Value;

        return Compare_With_Variable(left_index, false, right_index, false, COMP_LESS);
    }
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
        if (!object || !object->Is_Techno()) return false;
        if ((Data.House > HOUSE_NONE && object->Owner() != HouseClassExtension::House_From_HousesType(Data.House)->HeapID)) return false;
        if ((Data.House == HOUSE_ANY_HUMAN && !reinterpret_cast<const TechnoClass*>(object)->House->Is_Human_Player())) return false;
        is_perm = true;
        return true;
    } else if (Event == TEVENT_NEAR_WAYPOINT) {
        if (event != Event) return false;
        assert(object != NULL);
        Coord waypoint_location(Scen->Waypoint_Coord(Data.Value));
        if (object->Distance(waypoint_location) > RuleExtension->ComesNearWaypointDistance) {
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
    if (house != nullptr) {
        switch (Event) {
            /*
            **  Check to see if a team of the appropriate type has left the map.
            */
        case TEVENT_LEAVES_MAP:
            int index;
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
                for (int i = 0; i < Buildings.Count(); i++) {
                    BuildingClass* ptr = Buildings[i];
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
            **  Verify that the structure exists.
            */
        case TEVENT_BUILDING_EXISTS:
            if (house->ActiveBQuantity.Value(Data.Structure) == 0) return false;
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

            /*
            **  Verify that the structure does not exist.
            */
        case EXT_TEVENT_BUILDING_DOES_NOT_EXIST:
            if (house->ActiveBQuantity.Value(Data.Structure) > 0) return false;
            is_perm = true;
            break;

        default:
            break;
        }
    }

    house = HouseClassExtension::House_From_HousesType(Data.House);
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
    case TEVENT_DESTROYED_ANY_X:
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
    case TEVENT_BRIDGE_DESTROYED:
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
 *  Parses the INI text for this event's data.
 *
 *  @author: ZivDero
 */
void TEventClassExt::_Read_INI()
{
    auto& extension = *Extension::Fetch(this);

    Data.Value = 0;
    Event = static_cast<TEventType>(std::atoi(std::strtok(nullptr, ",")));
    int code = std::atoi(std::strtok(nullptr, ","));
    char* text = std::strtok(nullptr, ",");
    int val = std::atoi(text);

    char* fourth_arg = nullptr;
    char* fifth_arg = nullptr;

    switch (code) {
    default:
        break;
    case 4: // 5 args
        fourth_arg = std::strtok(nullptr, ",");
        fifth_arg = std::strtok(nullptr, ",");
        break;
    case 3: // 4 args
    case 2:
        fourth_arg = std::strtok(nullptr, ",");
        break;
    }

    switch (code) {

        /**
         *  Single argument, number.
         */
    case 0:
        Data.Value = val;
        break;

        /**
         *  Single argument, team.
         */
    case 1:
        Team = TeamTypes[TeamTypeClass::From_Name(text)];
        break;

        /**
         *  Two arguments, number and INI name.
         */
    case 2:
        Data.Value = val;
        std::strncpy(extension.IniNameArgument, fourth_arg, sizeof(extension));
        break;

        /**
         *  Three arguments, numbers.
         */
    case 4:
        extension.Data3.Value = std::atoi(fifth_arg);
        // fall through

        /**
         *  Two arguments, numbers.
         */
    case 3:
        extension.Data2.Value = std::atoi(fourth_arg);
        Data.Value = val;
        break;
    }
}


int Event_Need_Code(TEventType type)
{
    switch (type) {
    case TEVENT_LEAVES_MAP:
        return 1;

        // Events that require three arguments
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_CONSTANT:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_GLOBAL:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_LOCAL:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_CONSTANT:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_GLOBAL:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_LOCAL:
        return 4; // NeedThreeArgs

        // Events that require two arguments
    case EXT_TEVENT_GLOBAL_EQUALS_CONSTANT:
    case EXT_TEVENT_GLOBAL_EQUALS_GLOBAL:
    case EXT_TEVENT_GLOBAL_EQUALS_LOCAL:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_CONSTANT:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_GLOBAL:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_LOCAL:
    case EXT_TEVENT_GLOBAL_LESS_THAN_CONSTANT:
    case EXT_TEVENT_GLOBAL_LESS_THAN_GLOBAL:
    case EXT_TEVENT_GLOBAL_LESS_THAN_LOCAL:
    case EXT_TEVENT_LOCAL_EQUALS_CONSTANT:
    case EXT_TEVENT_LOCAL_EQUALS_GLOBAL:
    case EXT_TEVENT_LOCAL_EQUALS_LOCAL:
    case EXT_TEVENT_LOCAL_GREATER_THAN_CONSTANT:
    case EXT_TEVENT_LOCAL_GREATER_THAN_GLOBAL:
    case EXT_TEVENT_LOCAL_GREATER_THAN_LOCAL:
    case EXT_TEVENT_LOCAL_LESS_THAN_CONSTANT:
    case EXT_TEVENT_LOCAL_LESS_THAN_GLOBAL:
    case EXT_TEVENT_LOCAL_LESS_THAN_LOCAL:
        return 3; // NeedTwoArgs
    }

    return 0;
}


/**
 *  Builds the ini text for this event.
 *
 *  @author: ZivDero
 */
void TEventClassExt::_Build_INI_Entry(char* ptr) const
{
    auto& extension = *Extension::Fetch(this);
    ptr += std::strlen(ptr);
    int code = Event_Need_Code(Event);

    switch (code) {
    case 0:
        std::sprintf(ptr, "%d,%d,%d", Event, code, Data.Value);
        break;

    case 1:
        std::sprintf(ptr, "%d,%d,%s", Event, code, Team->IniName.c_str());
        break;

    case 2:
        std::sprintf(ptr, "%d,%d,%d,%s", Event, code, Data.Value, extension.IniNameArgument);
        break;

    case 3:
        std::sprintf(ptr, "%d,%d,%d,%d", Event, code, Data.Value, extension.Data2.Value);
        break;

    case 4:
        std::sprintf(ptr, "%d,%d,%d,%d,%d", Event, code, Data.Value, extension.Data2.Value, extension.Data3.Value);
        break;
    }
}


/**
 *  What can this event attach to?
 *
 *  @author: ZivDero
 */
AttachType _Attaches_To(TEventType event)
{
    AttachType attach = ATTACH_NONE;

    switch (event) {
    case TEVENT_CROSS_HORIZONTAL:
    case TEVENT_CROSS_VERTICAL:
    case TEVENT_ENTERS_ZONE:
    case TEVENT_PLAYER_ENTERED:
    case TEVENT_ANY:
    case TEVENT_DISCOVERED:
    case TEVENT_BRIDGE_DESTROYED:
    case TEVENT_NONE:
        attach |= ATTACH_CELL;
        break;

    default:
        break;
    }

    switch (event) {
    case TEVENT_FIRST_DAMAGED_ANY:
    case TEVENT_ENTER_YELLOW_ANY:
    case TEVENT_ENTER_RED_ANY:
    case TEVENT_FIRST_DAMAGED:
    case TEVENT_ENTER_YELLOW:
    case TEVENT_ENTER_RED:
    case TEVENT_SPIED:
    case TEVENT_PLAYER_ENTERED:
    case TEVENT_DISCOVERED:
    case TEVENT_DESTROYED:
    case TEVENT_DESTROYED_ANY:
    case TEVENT_ATTACKED:
    case TEVENT_ATTACKED_BY:
    case TEVENT_ANY:
    case TEVENT_NONE:
    case TEVENT_SELECTED:
    case TEVENT_NEAR_WAYPOINT:
    case TEVENT_ENEMY_IN_SPOTLIGHT:
    case TEVENT_PICKUP_CRATE:
    case TEVENT_PARALYZED:
    case TEVENT_ENEMY_IN_SPOTLIGHT_REPEATING:
    case TEVENT_LIMPED:
        attach |= ATTACH_OBJECT;
        break;

    default:
        break;
    }

    switch (event) {
    case TEVENT_ENTERS_ZONE:
    case TEVENT_ANY:
        attach |= ATTACH_MAP;
        break;

    default:
        break;
    }

    switch (event) {
    case TEVENT_LOW_POWER:
    case TEVENT_EVAC_CIVILIAN:
    case TEVENT_BUILDING_EXISTS:
    case TEVENT_BUILD:
    case TEVENT_BUILD_UNIT:
    case TEVENT_BUILD_INFANTRY:
    case TEVENT_BUILD_AIRCRAFT:
    case TEVENT_NOFACTORIES:
    case TEVENT_BUILDINGS_DESTROYED:
    case TEVENT_NBUILDINGS_DESTROYED:
    case TEVENT_UNITS_DESTROYED:
    case TEVENT_NUNITS_DESTROYED:
    case TEVENT_ALL_DESTROYED:
    case TEVENT_HOUSE_DISCOVERED:
    case TEVENT_CREDITS:
    case TEVENT_CREDITS_BELOW:
    case TEVENT_THIEVED:
    case TEVENT_ANY:
    case EXT_TEVENT_BUILDING_DOES_NOT_EXIST:
        attach |= ATTACH_HOUSE;
        break;

    default:
        break;
    }

    switch (event) {
    case TEVENT_GAME_TIME:
    case TEVENT_TIME:
    case TEVENT_RANDOM_TIME:
    case TEVENT_GLOBAL_SET:
    case TEVENT_GLOBAL_CLEAR:
    case TEVENT_LOCAL_SET:
    case TEVENT_LOCAL_CLEAR:
    case TEVENT_MISSION_TIMER_EXPIRED:
    case TEVENT_ANY:
    case TEVENT_AMBIENT_LESS_THAN:
    case TEVENT_AMBIENT_GREATER_THAN:
    case TEVENT_LEAVES_MAP:
    case TEVENT_PICKUP_CRATE_ANY:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_CONSTANT:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_GLOBAL:
    case EXT_TEVENT_COMPARE_GLOBAL_WITH_LOCAL:
    case EXT_TEVENT_GLOBAL_EQUALS_CONSTANT:
    case EXT_TEVENT_GLOBAL_EQUALS_GLOBAL:
    case EXT_TEVENT_GLOBAL_EQUALS_LOCAL:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_CONSTANT:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_GLOBAL:
    case EXT_TEVENT_GLOBAL_GREATER_THAN_LOCAL:
    case EXT_TEVENT_GLOBAL_LESS_THAN_CONSTANT:
    case EXT_TEVENT_GLOBAL_LESS_THAN_GLOBAL:
    case EXT_TEVENT_GLOBAL_LESS_THAN_LOCAL:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_CONSTANT:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_GLOBAL:
    case EXT_TEVENT_COMPARE_LOCAL_WITH_LOCAL:
    case EXT_TEVENT_LOCAL_EQUALS_CONSTANT:
    case EXT_TEVENT_LOCAL_EQUALS_GLOBAL:
    case EXT_TEVENT_LOCAL_EQUALS_LOCAL:
    case EXT_TEVENT_LOCAL_GREATER_THAN_CONSTANT:
    case EXT_TEVENT_LOCAL_GREATER_THAN_GLOBAL:
    case EXT_TEVENT_LOCAL_GREATER_THAN_LOCAL:
    case EXT_TEVENT_LOCAL_LESS_THAN_CONSTANT:
    case EXT_TEVENT_LOCAL_LESS_THAN_GLOBAL:
    case EXT_TEVENT_LOCAL_LESS_THAN_LOCAL:
        attach |= ATTACH_GENERAL;
        break;

    default:
        break;
    }

    return attach;
}


/**
 *  Spring all the new local and global events in LogicClass::AI.
 *  This patch is after `if (Scen->IsGlobalChanged)`.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00506B9D, _LogicClass_AI_GlobalChanged_Patch, 6)
{
    GET(TagClass*, tag, ESI);

    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_COMPARE_GLOBAL_WITH_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_COMPARE_GLOBAL_WITH_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_COMPARE_GLOBAL_WITH_LOCAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_EQUALS_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_EQUALS_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_EQUALS_LOCAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_GREATER_THAN_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_GREATER_THAN_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_GREATER_THAN_LOCAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_LESS_THAN_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_LESS_THAN_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_GLOBAL_LESS_THAN_LOCAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_COMPARE_LOCAL_WITH_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_COMPARE_LOCAL_WITH_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_COMPARE_LOCAL_WITH_LOCAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_EQUALS_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_EQUALS_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_EQUALS_LOCAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_GREATER_THAN_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_GREATER_THAN_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_GREATER_THAN_LOCAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_LESS_THAN_CONSTANT))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_LESS_THAN_GLOBAL))) goto cont;
    if (tag->Spring(static_cast<TEventType>(EXT_TEVENT_LOCAL_LESS_THAN_LOCAL))) goto cont;

    return 0;

    cont:
    return 0x00506C5E; // continue;
}


/**
 *  Main function for patching the hooks.
 */
void TEventClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TEventClassExtension_Init();

    Patch_Jump(0x00642310, &TEventClassExt::_Operator_Parens_Intercept);
    Patch_Jump(0x00642E20, &TEventClassExt::_Is_Temporal);
    Patch_Jump(0x00642E80, &TEventClassExt::_Has_Memory);
    Patch_Jump(0x00642A60, &TEventClassExt::_Read_INI);
    Patch_Jump(0x00642A10, &TEventClassExt::_Build_INI_Entry);
    Patch_Jump(0x00642B90, &_Attaches_To);
}
