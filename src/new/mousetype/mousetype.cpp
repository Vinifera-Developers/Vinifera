/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MOUSETYPE.CPP
 *
 *  @author        CCHyper
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
#include "mousetype.h"
#include "ccini.h"
#include "vinifera_globals.h"
#include "asserthandler.h"
#include "debughandler.h"


MouseType MouseTypeClass::CanMoveMouse = MOUSE_NORMAL;
MouseType MouseTypeClass::NoMoveMouse = MOUSE_NORMAL;
MouseType MouseTypeClass::CanAttackMouse = MOUSE_NORMAL;
MouseType MouseTypeClass::StayAttackMouse = MOUSE_NORMAL;


/**
 *  These are the ASCII names for the mouse control types.
 */
const char *MouseTypeClass::MouseNames[MOUSE_COUNT] = {
    "Normal",               // MOUSE_NORMAL

    "ScrollN",              // MOUSE_N
    "ScrollNE",             // MOUSE_NE
    "ScrollE",              // MOUSE_E
    "ScrollSE",             // MOUSE_SE
    "ScrollS",              // MOUSE_S
    "ScrollSW",             // MOUSE_SW
    "ScrollW",              // MOUSE_W
    "ScrollNW",             // MOUSE_NW
    "NoScrollN",            // MOUSE_NO_N
    "NoScrollNE",           // MOUSE_NO_NE
    "NoScrollE",            // MOUSE_NO_E
    "NoScrollSE",           // MOUSE_NO_SE
    "NoScrollS",            // MOUSE_NO_S
    "NoScrollSW",           // MOUSE_NO_SW
    "NoScrollW",            // MOUSE_NO_W
    "NoScrollNW",           // MOUSE_NO_NW

    "CanSelect",            // MOUSE_CAN_SELECT
    "CanMove",              // MOUSE_CAN_MOVE
    "NoMove",               // MOUSE_NO_MOVE
    "StayAttack",           // MOUSE_STAY_ATTACK
    "CanAttack",            // MOUSE_CAN_ATTACK
    "AreaGuard",            // MOUSE_AREA_GUARD
    "Tote",                 // MOUSE_TOTE
    "NoTote",               // MOUSE_NO_TOTE
    "Enter",                // MOUSE_ENTER
    "NoEnter",              // MOUSE_NO_ENTER
    "Deploy",               // MOUSE_DEPLOY
    "NoDeploy",             // MOUSE_NO_DEPLOY
    "Undeploy",             // MOUSE_UNDEPLOY
    "Sell",                 // MOUSE_SELL_BACK
    "SellUnit",             // MOUSE_SELL_UNIT
    "NoSell",               // MOUSE_NO_SELL_BACK
    "GRepair",              // MOUSE_GREPAIR
    "Repair",               // MOUSE_REPAIR
    "NoRepair",             // MOUSE_NO_REPAIR
    "Waypoint",             // MOUSE_WAYPOINT
    "PlaceWaypoint",        // MOUSE_PLACE_WAYPOINT
    "NoPlaceWaypoint",      // MOUSE_NO_PLACE_WAYPOINT
    "SelectWaypoint",       // MOUSE_SELECT_WAYPOINT
    "EnterWaypointMode",    // MOUSE_ENTER_WAYPOINT_MODE
    "FollowWaypoint",       // MOUSE_FOLLOW_WAYPOINT
    "ToteWaypoint",         // MOUSE_WAYPOINT_TOTE
    "RepairWaypoint",       // MOUSE_WAYPOINT_REPAIR
    "AttackWaypoint",       // MOUSE_ATTACK_WAYPOINT
    "EnterWaypoint",        // MOUSE_ENTER_WAYPOINT
    "LoopWaypointPath",     // MOUSE_LOOP_WAYPOINT_PATH
    "AirStrike",            // MOUSE_AIR_STRIKE
    "ChemBomb",             // MOUSE_CHEMBOMB
    "Demolitions",          // MOUSE_DEMOLITIONS
    "NuclearBomb",          // MOUSE_NUCLEAR_BOMB
    "TogglePower",          // MOUSE_TOGGLE_POWER
    "NoTogglePower",        // MOUSE_NO_TOGGLE_POWER
    "Heal",                 // MOUSE_HEAL
    "EMPulse",              // MOUSE_EM_PULSE
    "EMPulseRange",         // MOUSE_EM_PULSE_RANGE

    "ScrollCoast",          // MOUSE_SCROLL_COASTING
    "ScrollCoastN",         // MOUSE_SCROLL_COASTING_N
    "ScrollCoastNE",        // MOUSE_SCROLL_COASTING_NE
    "ScrollCoastE",         // MOUSE_SCROLL_COASTING_E
    "ScrollCoastSE",        // MOUSE_SCROLL_COASTING_SE
    "ScrollCoastS",         // MOUSE_SCROLL_COASTING_S
    "ScrollCoastSW",        // MOUSE_SCROLL_COASTING_SW
    "ScrollCoastW",         // MOUSE_SCROLL_COASTING_W
    "ScrollCoastNW",        // MOUSE_SCROLL_COASTING_NW

    "PatrolWaypoint",       // MOUSE_PATROL_WAYPOINT
};


/**
 *  This array of structures is used to control the mouse animation
 *  sequences.
 */
MouseTypeClass MouseTypeClass::MouseControl[MOUSE_COUNT] = {
    MouseTypeClass ( 0,     1,   0,    1,    1,   0,   { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MIN }, { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MIN } ),         // MOUSE_NORMAL,

    MouseTypeClass ( 2,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MIN }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MIN } ),         // MOUSE_N,
    MouseTypeClass ( 3,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MIN }, { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MIN } ),         // MOUSE_NE,
    MouseTypeClass ( 4,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MAX,   MOUSE_HOTSPOT_CENTER } ),      // MOUSE_E,
    MouseTypeClass ( 5,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MAX }, { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MAX } ),         // MOUSE_SE,
    MouseTypeClass ( 6,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MAX }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MAX } ),         // MOUSE_S,
    MouseTypeClass ( 7,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MAX }, { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MAX } ),         // MOUSE_SW,
    MouseTypeClass ( 8,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MIN,   MOUSE_HOTSPOT_CENTER } ),      // MOUSE_W,
    MouseTypeClass ( 9,     1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MIN }, { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MIN } ),         // MOUSE_NW,
    MouseTypeClass ( 10,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MIN }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MIN } ),         // MOUSE_NO_N,
    MouseTypeClass ( 11,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MIN }, { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MIN } ),         // MOUSE_NO_NE,
    MouseTypeClass ( 12,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MAX,   MOUSE_HOTSPOT_CENTER } ),      // MOUSE_NO_E,
    MouseTypeClass ( 13,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MAX }, { MOUSE_HOTSPOT_MAX,      MOUSE_HOTSPOT_MAX } ),         // MOUSE_NO_SE,
    MouseTypeClass ( 14,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MAX }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_MAX } ),         // MOUSE_NO_S,
    MouseTypeClass ( 15,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MAX }, { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MAX } ),         // MOUSE_NO_SW,
    MouseTypeClass ( 16,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MIN,   MOUSE_HOTSPOT_CENTER } ),      // MOUSE_NO_W,
    MouseTypeClass ( 17,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MIN }, { MOUSE_HOTSPOT_MIN,      MOUSE_HOTSPOT_MIN } ),         // MOUSE_NO_NW,

    MouseTypeClass ( 18,   13,   4,   -1,   13,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_CAN_SELECT,
    MouseTypeClass ( 31,   10,   4,   42,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_CAN_MOVE,
    MouseTypeClass ( 41,    1,   0,   52,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_MOVE,
    MouseTypeClass ( 53,    5,   4,   63,    5,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_STAY_ATTACK,
    MouseTypeClass ( 58,    5,   4,   63,    5,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_CAN_ATTACK,
    MouseTypeClass ( 68,    5,   4,   73,    5,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_AREA_GUARD,
    MouseTypeClass ( 78,   10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_TOTE,
    MouseTypeClass ( 88,    1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_TOTE,
    MouseTypeClass ( 89,   10,   4,  100,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_ENTER,
    MouseTypeClass ( 99,    1,   0,   63,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_ENTER,
    MouseTypeClass ( 110,   9,   4,   -1,    9,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_DEPLOY,
    MouseTypeClass ( 119,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_DEPLOY,
    MouseTypeClass ( 120,   9,   4,   -1,    9,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_UNDEPLOY,
    MouseTypeClass ( 129,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SELL_BACK,
    MouseTypeClass ( 139,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SELL_UNIT,
    MouseTypeClass ( 149,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_SELL_BACK,
    MouseTypeClass ( 150,  20,   4,   -1,   20,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_GREPAIR,          // Engineer entering friendly building to heal it.
    MouseTypeClass ( 170,  20,   4,   -1,   20,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_REPAIR,           // Engineer entering building to damage it.
    MouseTypeClass ( 190,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_REPAIR,
    MouseTypeClass ( 191,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_WAYPOINT,
    MouseTypeClass ( 201,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_PLACE_WAYPOINT,
    MouseTypeClass ( 211,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_PLACE_WAYPOINT,
    MouseTypeClass ( 212,   7,   4,   -1,    7,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SELECT_WAYPOINT,
    MouseTypeClass ( 219,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_ENTER_WAYPOINT_MODE,
    MouseTypeClass ( 229,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_FOLLOW_WAYPOINT,
    MouseTypeClass ( 239,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_WAYPOINT_TOTE,
    MouseTypeClass ( 249,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_WAYPOINT_REPAIR,
    MouseTypeClass ( 259,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_ATTACK_WAYPOINT,
    MouseTypeClass ( 269,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_ENTER_WAYPOINT,
    MouseTypeClass ( 356,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_LOOP_WAYPOINT_PATH,
    MouseTypeClass ( 279,  20,   4,   -1,   20,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_AIR_STRIKE,
    MouseTypeClass ( 299,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_CHEMBOMB,
    MouseTypeClass ( 309,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_DEMOLITIONS,
    MouseTypeClass ( 319,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NUCLEAR_BOMB,
    MouseTypeClass ( 329,  16,   2,   -1,   16,   2,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_TOGGLE_POWER,
    MouseTypeClass ( 345,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_NO_TOGGLE_POWER,
    MouseTypeClass ( 346,  10,   4,   42,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_HEAL,
    MouseTypeClass ( 357,  20,   3,   -1,   20,   3,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_EM_PULSE,
    MouseTypeClass ( 377,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_EM_PULSE_RANGE,

    MouseTypeClass ( 378,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING,
    MouseTypeClass ( 379,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_N,
    MouseTypeClass ( 380,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_NE,
    MouseTypeClass ( 381,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_E,
    MouseTypeClass ( 382,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_SE,
    MouseTypeClass ( 383,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_S,
    MouseTypeClass ( 384,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_SW,
    MouseTypeClass ( 385,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_W,
    MouseTypeClass ( 386,   1,   0,   -1,    1,   0,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } ),   // MOUSE_SCROLL_COASTING_NW,

    MouseTypeClass ( 387,  10,   4,   -1,   10,   4,   { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER,   MOUSE_HOTSPOT_CENTER } )    // MOUSE_PATROL_WAYPOINT,
};


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
MouseTypeClass::MouseTypeClass(const char *name) :
    Name(name),
    StartFrame(0),
    FrameCount(0),
    FrameRate(0),
    SmallFrame(0),
    SmallFrameCount(0),
    SmallFrameRate(0),
    Hotspot(0,0),
    SmallHotspot(0,0)
{
    MouseTypes.Add(this);
}


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
MouseTypeClass::MouseTypeClass(int start_frame, int frame_count, int frame_rate, int small_frame, int small_frame_count, int small_frame_rate, Point2D hotspot, Point2D small_hotspot) :
    Name(),
    StartFrame(start_frame),
    FrameCount(frame_count),
    FrameRate(frame_rate),
    SmallFrame(small_frame),
    SmallFrameCount(small_frame_count),
    SmallFrameRate(small_frame_rate),
    Hotspot(hotspot),
    SmallHotspot(small_hotspot)
{
    MouseTypes.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
MouseTypeClass::MouseTypeClass(const NoInitClass &noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
MouseTypeClass::~MouseTypeClass()
{
    MouseTypes.Delete(this);
}


/**
 *  Performs one time initialization of the mouse type class.
 *
 *  @warning: Do not change this function, otherwise it will break support
 *            with the original game!
 *
 *  @author: CCHyper
 */
void MouseTypeClass::One_Time()
{
    /**
     *  Create the default mouse type controls.
     */
    for (MouseType mouse = MOUSE_NORMAL; mouse < MOUSE_COUNT; ++mouse) {

        MouseTypeClass *mousectrl = new MouseTypeClass(
            MouseControl[mouse].StartFrame,
            MouseControl[mouse].FrameCount,
            MouseControl[mouse].FrameRate,
            MouseControl[mouse].SmallFrame,
            MouseControl[mouse].SmallFrameCount,
            MouseControl[mouse].SmallFrameRate,
            MouseControl[mouse].Hotspot,
            MouseControl[mouse].SmallHotspot);

        mousectrl->Name = MouseNames[mouse];

        ASSERT(mousectrl != nullptr);
    }
}


/**
 *  Reads mouse controls from the INI file.
 *  
 *  @author: CCHyper
 */
bool MouseTypeClass::Read_INI(CCINIClass &ini)
{
    static char const * const MOUSE = "MouseTypes";

    if (!ini.Is_Present(MOUSE)) {
        return false;
    }

    char buffer[1024];
    char *tok = nullptr;
    int value = 0;

    int entry_count = ini.Entry_Count(MOUSE);
    for (int index = 0; index < entry_count; ++index) {

        const char *entry_name = ini.Get_Entry(MOUSE, index);

        if (index <= MOUSE_COUNT) {
            ASSERT_FATAL_PRINT(Wstring(entry_name) == Wstring(MouseNames[index]), "Read %s, expected %s!", entry_name, MouseNames[index]);
        }

        /**
         *  Load the properties for this mouse type.
         */
        int readlen = ini.Get_String(MOUSE, entry_name, buffer, sizeof(buffer));
        ASSERT_FATAL(readlen > 0);

        MouseTypeClass *mousectrl = Find_Or_Make(entry_name);
        ASSERT(mousectrl != nullptr);

        mousectrl->Name = entry_name;

        tok = std::strtok(buffer, ",");
        mousectrl->StartFrame = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse StartFrame for %s!", mousectrl->Name.Peek_Buffer());

        tok = std::strtok(nullptr, ",");
        mousectrl->FrameCount = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse FrameCount for %s!", mousectrl->Name.Peek_Buffer());

        tok = std::strtok(nullptr, ",");
        mousectrl->FrameRate = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse FrameRate for %s!", mousectrl->Name.Peek_Buffer());

        tok = std::strtok(nullptr, ",");
        mousectrl->SmallFrame = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse SmallFrame for %s!", mousectrl->Name.Peek_Buffer());

        tok = std::strtok(nullptr, ",");
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse SmallFrameCount for %s!", mousectrl->Name.Peek_Buffer());
        mousectrl->SmallFrameCount = std::strtol(tok, nullptr, 10);

        tok = std::strtok(nullptr, ",");
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse SmallFrameRate for %s!", mousectrl->Name.Peek_Buffer());
        mousectrl->SmallFrameRate = std::strtol(tok, nullptr, 10);

        tok = std::strtok(nullptr, ",");
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse HotspotX for %s!", mousectrl->Name.Peek_Buffer());
        if (!strcmpi(tok, "left")) {
            value = MOUSE_HOTSPOT_MIN;
        } else if (!strcmpi(tok, "center")) {
            value = MOUSE_HOTSPOT_CENTER;
        } else if (!strcmpi(tok, "right")) {
            value = MOUSE_HOTSPOT_MAX;
        } else {
            value = std::strtol(tok, nullptr, 10);
        }
        mousectrl->Hotspot.X = value;

        tok = std::strtok(nullptr, ",");
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse HotspotY for %s!", mousectrl->Name.Peek_Buffer());
        if (!strcmpi(tok, "top")) {
            value = MOUSE_HOTSPOT_MIN;
        } else if (!strcmpi(tok, "middle")) {
            value = MOUSE_HOTSPOT_CENTER;
        } else if (!strcmpi(tok, "bottom")) {
            value = MOUSE_HOTSPOT_MAX;
        } else {
            value = std::strtol(tok, nullptr, 10);
        }
        mousectrl->Hotspot.Y = value;
        
        mousectrl->SmallHotspot.X = mousectrl->Hotspot.X;
        mousectrl->SmallHotspot.Y = mousectrl->Hotspot.Y;

    }

    /**
     *  
     */
    CanMoveMouse = From_Name("CanMove");
    ASSERT_FATAL(CanMoveMouse != MOUSE_NORMAL);
    NoMoveMouse = From_Name("NoMove");
    ASSERT_FATAL(NoMoveMouse != MOUSE_NORMAL);
    CanAttackMouse = From_Name("CanAttack");
    ASSERT_FATAL(CanAttackMouse != MOUSE_NORMAL);
    StayAttackMouse = From_Name("StayAttack");
    ASSERT_FATAL(StayAttackMouse != MOUSE_NORMAL);

    return true;
}


#ifndef NDEBUG
/**
 *  Writes out the default mouse control values.
 *
 *  @author: CCHyper
 */
bool MouseTypeClass::Write_Default_INI(CCINIClass &ini)
{
    static char const * const MOUSE = "MouseTypes";

    char buffer[1024];

    for (MouseType mouse = MOUSE_NORMAL; mouse < MOUSE_COUNT; ++mouse) {

        MouseTypeClass &mousectrl = MouseControl[mouse];

        const char *hotspot_x = nullptr;
        const char *hotspot_y = nullptr;
        const char *smallhotspot_x = nullptr;
        const char *smallhotspot_y = nullptr;

        switch (mousectrl.Hotspot.X) {
            case MOUSE_HOTSPOT_MIN:
                hotspot_x = "left";
                break;
            case MOUSE_HOTSPOT_CENTER:
                hotspot_x = "center";
                break;
            case MOUSE_HOTSPOT_MAX:
                hotspot_x = "right";
                break;
            default:
                DEBUG_ERROR("Mouse: Invalid hotspot X for %s!\n", mousectrl.Name.Peek_Buffer());
                return false;
        };

        switch (mousectrl.Hotspot.Y) {
            case MOUSE_HOTSPOT_MIN:
                hotspot_y = "top";
                break;
            case MOUSE_HOTSPOT_CENTER:
                hotspot_y = "middle";
                break;
            case MOUSE_HOTSPOT_MAX:
                hotspot_y = "bottom";
                break;
            default:
                DEBUG_ERROR("Mouse: Invalid hotspot Y for %s!\n", mousectrl.Name.Peek_Buffer());
                return false;
        };

        switch (mousectrl.SmallHotspot.X) {
            case MOUSE_HOTSPOT_MIN:
                smallhotspot_x = "left";
                break;
            case MOUSE_HOTSPOT_CENTER:
                smallhotspot_x = "center";
                break;
            case MOUSE_HOTSPOT_MAX:
                smallhotspot_x = "right";
                break;
            default:
                DEBUG_ERROR("Mouse: Invalid hotspot X for %s!\n", mousectrl.Name.Peek_Buffer());
                return false;
        };

        switch (mousectrl.SmallHotspot.Y) {
            case MOUSE_HOTSPOT_MIN:
                smallhotspot_y = "top";
                break;
            case MOUSE_HOTSPOT_CENTER:
                smallhotspot_y = "middle";
                break;
            case MOUSE_HOTSPOT_MAX:
                smallhotspot_y = "bottom";
                break;
            default:
                DEBUG_ERROR("Mouse: Invalid hotspot Y for %s!\n", mousectrl.Name.Peek_Buffer());
                return false;
        };

        std::snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d,%d,%d,%s,%s,%s,%s",
                                    mousectrl.StartFrame,
                                    mousectrl.FrameCount,
                                    mousectrl.FrameRate,
                                    mousectrl.SmallFrame,
                                    mousectrl.SmallFrameCount,
                                    mousectrl.SmallFrameRate,
                                    hotspot_x,
                                    hotspot_y,
                                    smallhotspot_x,
                                    smallhotspot_y);

        ini.Put_String(MOUSE, mousectrl.Name.Peek_Buffer(), buffer);
    }

    return true;
}
#endif


/**
 *  Converts a mouse number into a mouse control object pointer.
 * 
 *  @author: CCHyper
 */
const MouseTypeClass *MouseTypeClass::As_Pointer(MouseType type)
{
    //ASSERT(type >= MOUSE_NORMAL && type < MouseTypes.Count());
    return type >= MOUSE_NORMAL && type < MouseTypes.Count() ? MouseTypes[type] : nullptr;
}


/**
 *  Converts a mouse name into a mouse control object pointer.
 * 
 *  @author: CCHyper
 */
const MouseTypeClass *MouseTypeClass::As_Pointer(const char *name)
{
    return As_Pointer(From_Name(name));
}


/**
 *  Converts a mouse number into a mouse control object reference.
 * 
 *  @author: CCHyper
 */
const MouseTypeClass &MouseTypeClass::As_Reference(MouseType type)
{
    ASSERT(type >= MOUSE_NORMAL && type < MouseTypes.Count());
    return *MouseTypes[type];
}


/**
 *  Converts a mouse name into a mouse control object reference.
 * 
 *  @author: CCHyper
 */
const MouseTypeClass &MouseTypeClass::As_Reference(const char *name)
{
    return As_Reference(From_Name(name));
}


/**
 *  Retrieves the mouse type for given name.
 * 
 *  @author: CCHyper
 */
MouseType MouseTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (Wstring(name) == "<none>" || Wstring(name) == "none") {
        return MOUSE_NORMAL;
    }

    if (name != nullptr) {
        for (MouseType index = MOUSE_NORMAL; index < MouseTypes.Count(); ++index) {
            if (MouseTypes[index]->Name.Peek_Buffer() == name) {
                return index;
            }
        }
    }

    return MOUSE_NORMAL;
}


/**
 *  Returns name for given mouse control type.
 * 
 *  @author: CCHyper
 */
const char *MouseTypeClass::Name_From(MouseType type)
{
    return (type >= MOUSE_NORMAL && type < MouseTypes.Count() ? MouseTypes[type]->Name.Peek_Buffer() : "<none>");
}


/**
 *  Find or create a mouse type of the name specified.
 * 
 *  @author: CCHyper
 */
MouseTypeClass *MouseTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    if (Wstring(name) == "<none>" || Wstring(name) == "none") {
        return nullptr;
    }

    for (MouseType index = MOUSE_NORMAL; index < MouseTypes.Count(); ++index) {
        if (MouseTypes[index]->Name == name) {
            return MouseTypes[index];
        }
    }

    MouseTypeClass *ptr = new MouseTypeClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}
