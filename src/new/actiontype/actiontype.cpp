/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ACTIONTYPE.H
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         Mouse cursor controls and overrides.
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
 *                 If not, see <http://www.gnu.org/licenses/ }.
 *
 ******************************************************************************/
#include "actiontype.h"
#include "mousetype.h"
#include "ccini.h"
#include "vinifera_globals.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  These are the ASCII names for the action types.
 */
static const char* ActionNames[EXT_ACTION_COUNT] = {
    "None",                     // ACTION_NONE

    "Move",                     // ACTION_MOVE
    "NoMove",                   // ACTION_NOMOVE
    "Enter",                    // ACTION_ENTER
    "Self",                     // ACTION_SELF
    "Attack",                   // ACTION_ATTACK
    "Harvest",                  // ACTION_HARVEST
    "Select",                   // ACTION_SELECT
    "ToggleSelect",             // ACTION_TOGGLE_SELECT
    "Capture",                  // ACTION_CAPTURE
    "Repair",                   // ACTION_REPAIR
    "Sell",                     // ACTION_SELL
    "SellUnit",                 // ACTION_SELL_UNIT
    "NoSell",                   // ACTION_NO_SELL
    "NoRepair",                 // ACTION_NO_REPAIR
    "Sabotage",                 // ACTION_SABOTAGE
    "Tote",                     // ACTION_TOTE
    "DontUse2",                 // ACTION_PARA_INFANTRY
    "DontUse3",                 // ACTION_PARA_SABOTEUR
    "Nuke",                     // ACTION_NUKE_BOMB
    "DontUse4",                 // ACTION_AIR_STRIKE
    "DontUse5",                 // ACTION_CHRONOSPHERE
    "DontUse6",                 // ACTION_CHRONO2
    "DontUse7",                 // ACTION_IRON_CURTAIN
    "DontUse8",                 // ACTION_SPY_MISSION
    "GuardArea",                // ACTION_GUARD_AREA
    "Heal",                     // ACTION_HEAL
    "Damage",                   // ACTION_DAMAGE
    "GRepair",                  // ACTION_GREPAIR
    "NoDeploy",                 // ACTION_NO_DEPLOY
    "NoEnter",                  // ACTION_NO_ENTER
    "NoGRepair",                // ACTION_NO_GREPAIR
    "TogglePower",              // ACTION_TOGGLE_POWER
    "NoTogglePower",            // ACTION_NO_TOGGLE_POWER
    "EnterTunnel",              // ACTION_ENTER_TUNNEL
    "NoEnterTunnel",            // ACTION_NO_ENTER_TUNNEL
    "EMPulse",                  // ACTION_EMPULSE
    "IonCannon",                // ACTION_ION_CANNON
    "EMPulseRange",             // ACTION_EMPULSE_RANGE
    "ChemBomb",                 // ACTION_CHEM_BOMB
    "PlaceWaypoint",            // ACTION_PLACE_WAYPOINT
    "NoPlaceWaypoint",          // ACTION_NO_PLACE_WAYPOINT
    "EnterWaypointMode",        // ACTION_ENTER_WAYPOINT_MODE
    "FollowWaypoint",           // ACTION_FOLLOW_WAYPOINT
    "SelectWaypoint",           // ACTION_SELECT_WAYPOINT
    "LoopWaypointPath",         // ACTION_LOOP_WAYPOINT_PATH
    "DragWaypoint",             // ACTION_DRAG_WAYPOINT
    "AttackWaypoint",           // ACTION_ATTACK_WAYPOINT
    "EnterWaypoint",            // ACTION_ENTER_WAYPOINT
    "PatrolWaypoint",           // ACTION_PATROL_WAYPOINT
    "DropPod",                  // ACTION_DROP_POD
    "RallyToPoint",             // ACTION_RALLY_TO_POINT        // Was "Rally To Point"
    "AttackSupport",            // ACTION_ATTACK_SUPPORT        // Was "Attack Support"
    "PlaceBeacon",              // EXT_ACTION_PLACE_BEACON
    "PlaceBeacon1",             // EXT_ACTION_PLACE_BEACON_1
    "PlaceBeacon2",             // EXT_ACTION_PLACE_BEACON_2
    "PlaceBeacon3",             // EXT_ACTION_PLACE_BEACON_3
    "PlaceBeacon4",             // EXT_ACTION_PLACE_BEACON_4
    "PlaceBeacon5",             // EXT_ACTION_PLACE_BEACON_5
    "PlaceBeacon6",             // EXT_ACTION_PLACE_BEACON_6
    "PlaceBeacon7",             // EXT_ACTION_PLACE_BEACON_7
    "SelectBeacon"              // EXT_ACTION_SELECT_BEACON
};


/**
 *  This array of structures is used to control the cursors used by various actions.
 */
struct ActionControlType {
    const char * Name;
    MouseType Mouse;
    MouseType ShadowMouse;
};

#define NO_MOUSE_SHAPE MOUSE_NORMAL
static ActionControlType ActionControl[EXT_ACTION_COUNT] = {
    ActionControlType {ActionNames[ACTION_NONE],                    MOUSE_NORMAL,               MOUSE_NORMAL},              // ACTION_NONE
    ActionControlType {ActionNames[ACTION_MOVE],                    MOUSE_CAN_MOVE,             MOUSE_CAN_MOVE},            // ACTION_MOVE
    ActionControlType {ActionNames[ACTION_NOMOVE],                  MOUSE_NO_MOVE,              MOUSE_NO_MOVE},             // ACTION_NOMOVE
    ActionControlType {ActionNames[ACTION_ENTER],                   MOUSE_ENTER,                NO_MOUSE_SHAPE},            // ACTION_ENTER
    ActionControlType {ActionNames[ACTION_SELF],                    MOUSE_DEPLOY,               NO_MOUSE_SHAPE},            // ACTION_SELF
    ActionControlType {ActionNames[ACTION_ATTACK],                  MOUSE_CAN_ATTACK,           MOUSE_CAN_MOVE},            // ACTION_ATTACK
    ActionControlType {ActionNames[ACTION_HARVEST],                 MOUSE_CAN_ATTACK,           NO_MOUSE_SHAPE},            // ACTION_HARVEST
    ActionControlType {ActionNames[ACTION_SELECT],                  MOUSE_CAN_SELECT,           NO_MOUSE_SHAPE},            // ACTION_SELECT
    ActionControlType {ActionNames[ACTION_TOGGLE_SELECT],           MOUSE_CAN_SELECT,           NO_MOUSE_SHAPE},            // ACTION_TOGGLE_SELECT
    ActionControlType {ActionNames[ACTION_CAPTURE],                 MOUSE_ENTER,                NO_MOUSE_SHAPE},            // ACTION_CAPTURE
    ActionControlType {ActionNames[ACTION_REPAIR],                  MOUSE_REPAIR,               MOUSE_NO_REPAIR},           // ACTION_REPAIR
    ActionControlType {ActionNames[ACTION_SELL],                    MOUSE_SELL_BACK,            MOUSE_NO_SELL_BACK},        // ACTION_SELL
    ActionControlType {ActionNames[ACTION_SELL_UNIT],               MOUSE_SELL_UNIT,            MOUSE_NO_SELL_BACK},        // ACTION_SELL_UNIT
    ActionControlType {ActionNames[ACTION_NO_SELL],                 MOUSE_NO_SELL_BACK,         MOUSE_NO_SELL_BACK},        // ACTION_NO_SELL
    ActionControlType {ActionNames[ACTION_NO_REPAIR],               MOUSE_NO_REPAIR,            MOUSE_NO_REPAIR},           // ACTION_NO_REPAIR
    ActionControlType {ActionNames[ACTION_SABOTAGE],                MOUSE_DEMOLITIONS,          NO_MOUSE_SHAPE},            // ACTION_SABOTAGE
    ActionControlType {ActionNames[ACTION_TOTE],                    MOUSE_TOTE,                 MOUSE_NO_TOTE},             // ACTION_TOTE
    ActionControlType {ActionNames[ACTION_PARA_INFANTRY],           NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_PARA_INFANTRY
    ActionControlType {ActionNames[ACTION_PARA_SABOTEUR],           NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_PARA_SABOTEUR
    ActionControlType {ActionNames[ACTION_NUKE_BOMB],               MOUSE_NUCLEAR_BOMB,         MOUSE_NUCLEAR_BOMB},        // ACTION_NUKE_BOMB
    ActionControlType {ActionNames[ACTION_AIR_STRIKE],              NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_AIR_STRIKE
    ActionControlType {ActionNames[ACTION_CHRONOSPHERE],            NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_CHRONOSPHERE
    ActionControlType {ActionNames[ACTION_CHRONO2],                 NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_CHRONO2
    ActionControlType {ActionNames[ACTION_IRON_CURTAIN],            NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_IRON_CURTAIN
    ActionControlType {ActionNames[ACTION_SPY_MISSION],             NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_SPY_MISSION
    ActionControlType {ActionNames[ACTION_GUARD_AREA],              MOUSE_AREA_GUARD,           MOUSE_AREA_GUARD},          // ACTION_GUARD_AREA
    ActionControlType {ActionNames[ACTION_HEAL],                    MOUSE_HEAL,                 MOUSE_HEAL},                // ACTION_HEAL
    ActionControlType {ActionNames[ACTION_DAMAGE],                  MOUSE_ENTER,                NO_MOUSE_SHAPE},            // ACTION_DAMAGE
    ActionControlType {ActionNames[ACTION_GREPAIR],                 MOUSE_GREPAIR,              NO_MOUSE_SHAPE},            // ACTION_GREPAIR
    ActionControlType {ActionNames[ACTION_NO_DEPLOY],               MOUSE_NO_DEPLOY,            MOUSE_NO_DEPLOY},           // ACTION_NO_DEPLOY
    ActionControlType {ActionNames[ACTION_NO_ENTER],                MOUSE_NO_ENTER,             MOUSE_NO_ENTER},            // ACTION_NO_ENTER
    ActionControlType {ActionNames[ACTION_NO_GREPAIR],              MOUSE_NO_REPAIR,            MOUSE_NO_REPAIR},           // ACTION_NO_GREPAIR
    ActionControlType {ActionNames[ACTION_TOGGLE_POWER],            MOUSE_TOGGLE_POWER,         MOUSE_NO_TOGGLE_POWER},     // ACTION_TOGGLE_POWER
    ActionControlType {ActionNames[ACTION_NO_TOGGLE_POWER],         MOUSE_NO_TOGGLE_POWER,      MOUSE_NO_TOGGLE_POWER},     // ACTION_NO_TOGGLE_POWER
    ActionControlType {ActionNames[ACTION_ENTER_TUNNEL],            MOUSE_ENTER,                NO_MOUSE_SHAPE},            // ACTION_ENTER_TUNNEL
    ActionControlType {ActionNames[ACTION_NO_ENTER_TUNNEL],         NO_MOUSE_SHAPE,             MOUSE_NO_ENTER},            // ACTION_NO_ENTER_TUNNEL
    ActionControlType {ActionNames[ACTION_EMPULSE],                 MOUSE_EM_PULSE,             MOUSE_EM_PULSE},            // ACTION_EMPULSE
    ActionControlType {ActionNames[ACTION_ION_CANNON],              MOUSE_AIR_STRIKE,           MOUSE_AIR_STRIKE},          // ACTION_ION_CANNON
    ActionControlType {ActionNames[ACTION_EMPULSE_RANGE],           MOUSE_EM_PULSE_RANGE,       MOUSE_EM_PULSE_RANGE},      // ACTION_EMPULSE_RANGE
    ActionControlType {ActionNames[ACTION_CHEM_BOMB],               MOUSE_CHEMBOMB,             MOUSE_CHEMBOMB},            // ACTION_CHEM_BOMB
    ActionControlType {ActionNames[ACTION_PLACE_WAYPOINT],          MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // ACTION_PLACE_WAYPOINT
    ActionControlType {ActionNames[ACTION_NO_PLACE_WAYPOINT],       MOUSE_NO_PLACE_WAYPOINT,    MOUSE_NO_PLACE_WAYPOINT},   // ACTION_NO_PLACE_WAYPOINT
    ActionControlType {ActionNames[ACTION_ENTER_WAYPOINT_MODE],     MOUSE_ENTER_WAYPOINT_MODE,  MOUSE_ENTER_WAYPOINT_MODE}, // ACTION_ENTER_WAYPOINT_MODE
    ActionControlType {ActionNames[ACTION_FOLLOW_WAYPOINT],         MOUSE_FOLLOW_WAYPOINT,      MOUSE_FOLLOW_WAYPOINT},     // ACTION_FOLLOW_WAYPOINT
    ActionControlType {ActionNames[ACTION_SELECT_WAYPOINT],         MOUSE_SELECT_WAYPOINT,      MOUSE_SELECT_WAYPOINT},     // ACTION_SELECT_WAYPOINT
    ActionControlType {ActionNames[ACTION_LOOP_WAYPOINT_PATH],      MOUSE_LOOP_WAYPOINT_PATH,   MOUSE_LOOP_WAYPOINT_PATH},  // ACTION_LOOP_WAYPOINT_PATH
    ActionControlType {ActionNames[ACTION_DRAG_WAYPOINT],           NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_DRAG_WAYPOINT
    ActionControlType {ActionNames[ACTION_ATTACK_WAYPOINT],         MOUSE_ATTACK_WAYPOINT,      MOUSE_ATTACK_WAYPOINT},     // ACTION_ATTACK_WAYPOINT
    ActionControlType {ActionNames[ACTION_ENTER_WAYPOINT],          MOUSE_ENTER_WAYPOINT,       MOUSE_ENTER_WAYPOINT},      // ACTION_ENTER_WAYPOINT
    ActionControlType {ActionNames[ACTION_PATROL_WAYPOINT],         MOUSE_PATROL_WAYPOINT,      MOUSE_PATROL_WAYPOINT},     // ACTION_PATROL_WAYPOINT
    ActionControlType {ActionNames[ACTION_DROP_POD],                MOUSE_AIR_STRIKE,           MOUSE_AIR_STRIKE},          // ACTION_DROP_POD
    ActionControlType {ActionNames[ACTION_RALLY_TO_POINT],          MOUSE_CAN_MOVE,             NO_MOUSE_SHAPE},            // ACTION_RALLY_TO_POINT
    ActionControlType {ActionNames[ACTION_ATTACK_SUPPORT],          NO_MOUSE_SHAPE,             NO_MOUSE_SHAPE},            // ACTION_ATTACK_SUPPORT
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON],        MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON_1],      MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON_1
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON_2],      MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON_2
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON_3],      MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON_3
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON_4],      MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON_4
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON_5],      MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON_6
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON_6],      MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON_7
    ActionControlType {ActionNames[EXT_ACTION_PLACE_BEACON_7],      MOUSE_PLACE_WAYPOINT,       MOUSE_PLACE_WAYPOINT},      // EXT_ACTION_PLACE_BEACON_8
    ActionControlType {ActionNames[EXT_ACTION_SELECT_BEACON],       MOUSE_SELECT_WAYPOINT,      MOUSE_SELECT_WAYPOINT}      // EXT_ACTION_SELECT_BEACON
};


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
ActionTypeClass::ActionTypeClass(const char* name, MouseType mouse, MouseType shadow_mouse) :
    Name(name),
    Mouse(mouse),
    ShadowMouse(shadow_mouse)
{
    ActionTypes.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ActionTypeClass::ActionTypeClass(const NoInitClass &noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
ActionTypeClass::~ActionTypeClass()
{
    ActionTypes.Delete(this);
}


/**
 *  Performs one time initialization of the action type class.
 *
 *  @warning: Do not change this function, otherwise it will break support
 *            with the original game!
 *
 *  @author: CCHyper
 */
void ActionTypeClass::One_Time()
{
    /**
     *  Create the default action type controls.
     */
    for (ActionType action = ACTION_NONE; action < std::size(ActionControl); ++action) {
        ActionTypeClass* actiontype = new ActionTypeClass(ActionControl[action].Name, ActionControl[action].Mouse, ActionControl[action].ShadowMouse);
        ASSERT(actiontype != nullptr);
    }
}


/**
 *  Reads action controls from the INI file.
 *  
 *  @author: CCHyper
 */
bool ActionTypeClass::Read_INI(CCINIClass &ini)
{
    static char const * const ACTION = "ActionTypes";

    if (!ini.Is_Present(ACTION)) {
        return false;
    }

    char buffer[1024];
    char *tok = nullptr;
    int value = 0;

    int entry_count = ini.Entry_Count(ACTION);
    for (int index = 0; index < entry_count; ++index) {

        const char *entry_name = ini.Get_Entry(ACTION, index);

        /**
         *  Load the properties for this mouse type.
         */
        int readlen = ini.Get_String(ACTION, entry_name, buffer, sizeof(buffer));
        ASSERT_FATAL(readlen > 0);

        ActionTypeClass *actiontype = Find_Or_Make(entry_name);

        actiontype->Name = entry_name;

        tok = std::strtok(buffer, ",");
        actiontype->Mouse = MouseTypeClass::From_Name(tok);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse Mouse for %s!", actiontype->Name.c_str());

        tok = std::strtok(nullptr, ",");
        actiontype->ShadowMouse = MouseTypeClass::From_Name(tok);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse ShadowMouse for %s!", actiontype->Name.c_str());

        DEV_DEBUG_INFO("Action: Name: %s Mouse: %s ShadowMouse: %s\n", actiontype->Name.c_str(), MouseTypeClass::Name_From(actiontype->Mouse), MouseTypeClass::Name_From(actiontype->ShadowMouse));
    }

    return true;
}


#ifndef NDEBUG
/**
 *  Writes out the default action control values.
 *
 *  @author: CCHyper
 */
bool ActionTypeClass::Write_Default_INI(CCINIClass &ini)
{
    static char const * const ACTION = "ActionTypes";

    char buffer[1024];

    for (ActionType action = ACTION_NONE; action < std::size(ActionControl); ++action) {
        auto const& control = ActionControl[action];
        std::snprintf(buffer, sizeof(buffer), "%s,%s", MouseTypeClass::Name_From(control.Mouse), MouseTypeClass::Name_From(control.ShadowMouse));
        ini.Put_String(ACTION, control.Name, buffer);
    }

    return true;
}
#endif


/**
 *  Retrieves the action type for given name.
 * 
 *  @author: CCHyper
 */
ActionType ActionTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (std::string_view(name) == "<none>" || std::string_view(name) == "none") {
        return ACTION_NONE;
    }

    if (name != nullptr) {
        for (ActionType index = ACTION_NONE; index < ActionTypes.Count(); ++index) {
            if (ActionTypes[index]->Name == name) {
                return index;
            }
        }
    }

    return ACTION_NONE;
}


/**
 *  Returns name for given action control type.
 * 
 *  @author: CCHyper
 */
const char *ActionTypeClass::Name_From(ActionType type)
{
    return (type >= ACTION_NONE && type < ActionTypes.Count() ? ActionTypes[type]->Name.c_str() : "<none>");
}


/**
 *  Find or create a action type of the name specified.
 * 
 *  @author: CCHyper
 */
ActionTypeClass *ActionTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    if (std::string_view(name) == "<none>" || std::string_view(name) == "none") {
        return nullptr;
    }

    for (ActionType index = ACTION_NONE; index < ActionTypes.Count(); ++index) {
        if (ActionTypes[index]->Name == name) {
            return ActionTypes[index];
        }
    }

    ActionTypeClass *ptr = new ActionTypeClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}
