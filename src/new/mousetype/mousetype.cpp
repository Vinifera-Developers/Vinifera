/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Mouse cursor controls and overrides.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "mousetype.h"

#include "asserthandler.h"
#include "ccini.h"
#include "debughandler.h"
#include "vinifera_globals.h"


/**
 *  These are the ASCII names for the mouse control types.
 */
static const char* MouseNames[MOUSE_COUNT] = {
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
struct MouseControlType
{
    const char * Name;
    int StartFrame;
    int FrameCount;
    int FrameRate;
    int SmallFrame;
    int SmallFrameCount;
    int SmallFrameRate;
    Point2D Hotspot;
    Point2D SmallHotspot;
};

static MouseControlType MouseControl[MOUSE_COUNT] = {
    MouseControlType {MouseNames[MOUSE_NORMAL],              0,   1,  0, 1,   1,  0, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MIN    }, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MIN    }}, // MOUSE_NORMAL,
  
    MouseControlType {MouseNames[MOUSE_N],                   2,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MIN    }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MIN    }}, // MOUSE_N,
    MouseControlType {MouseNames[MOUSE_NE],                  3,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MIN    }, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MIN    }}, // MOUSE_NE,
    MouseControlType {MouseNames[MOUSE_E],                   4,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_CENTER }}, // MOUSE_E,
    MouseControlType {MouseNames[MOUSE_SE],                  5,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MAX    }, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MAX    }}, // MOUSE_SE,
    MouseControlType {MouseNames[MOUSE_S],                   6,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MAX    }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MAX    }}, // MOUSE_S,
    MouseControlType {MouseNames[MOUSE_SW],                  7,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MAX    }, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MAX    }}, // MOUSE_SW,
    MouseControlType {MouseNames[MOUSE_W],                   8,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_CENTER }}, // MOUSE_W,
    MouseControlType {MouseNames[MOUSE_NW],                  9,   1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MIN    }, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MIN    }}, // MOUSE_NW,
    MouseControlType {MouseNames[MOUSE_NO_N],                10,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MIN    }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MIN    }}, // MOUSE_NO_N,
    MouseControlType {MouseNames[MOUSE_NO_NE],               11,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MIN    }, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MIN    }}, // MOUSE_NO_NE,
    MouseControlType {MouseNames[MOUSE_NO_E],                12,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_E,
    MouseControlType {MouseNames[MOUSE_NO_SE],               13,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MAX    }, { MOUSE_HOTSPOT_MAX,    MOUSE_HOTSPOT_MAX    }}, // MOUSE_NO_SE,
    MouseControlType {MouseNames[MOUSE_NO_S],                14,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MAX    }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_MAX    }}, // MOUSE_NO_S,
    MouseControlType {MouseNames[MOUSE_NO_SW],               15,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MAX    }, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MAX    }}, // MOUSE_NO_SW,
    MouseControlType {MouseNames[MOUSE_NO_W],                16,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_W,
    MouseControlType {MouseNames[MOUSE_NO_NW],               17,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MIN    }, { MOUSE_HOTSPOT_MIN,    MOUSE_HOTSPOT_MIN    }}, // MOUSE_NO_NW,
  
    MouseControlType {MouseNames[MOUSE_CAN_SELECT],          18,  13, 4, -1,  13, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_CAN_SELECT,
    MouseControlType {MouseNames[MOUSE_CAN_MOVE],            31,  10, 4, 42,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_CAN_MOVE,
    MouseControlType {MouseNames[MOUSE_NO_MOVE],             41,  1,  0, 52,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_MOVE,
    MouseControlType {MouseNames[MOUSE_STAY_ATTACK],         53,  5,  4, 63,  5,  4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_STAY_ATTACK,
    MouseControlType {MouseNames[MOUSE_CAN_ATTACK],          58,  5,  4, 63,  5,  4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_CAN_ATTACK,
    MouseControlType {MouseNames[MOUSE_AREA_GUARD],          68,  5,  4, 73,  5,  4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_AREA_GUARD,
    MouseControlType {MouseNames[MOUSE_TOTE],                78,  10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_TOTE,
    MouseControlType {MouseNames[MOUSE_NO_TOTE],             88,  1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_TOTE,
    MouseControlType {MouseNames[MOUSE_ENTER],               89,  10, 4, 100, 10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_ENTER,
    MouseControlType {MouseNames[MOUSE_NO_ENTER],            99,  1,  0, 63,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_ENTER,
    MouseControlType {MouseNames[MOUSE_DEPLOY],              110, 9,  4, -1,  9,  4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_DEPLOY,
    MouseControlType {MouseNames[MOUSE_NO_DEPLOY],           119, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_DEPLOY,
    MouseControlType {MouseNames[MOUSE_UNDEPLOY],            120, 9,  4, -1,  9,  4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_UNDEPLOY,
    MouseControlType {MouseNames[MOUSE_SELL_BACK],           129, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SELL_BACK,
    MouseControlType {MouseNames[MOUSE_SELL_UNIT],           139, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SELL_UNIT,
    MouseControlType {MouseNames[MOUSE_NO_SELL_BACK],        149, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_SELL_BACK,
    MouseControlType {MouseNames[MOUSE_GREPAIR],             150, 20, 4, -1,  20, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_GREPAIR,             // Engineer entering friendly building to heal it.
    MouseControlType {MouseNames[MOUSE_REPAIR],              170, 20, 4, -1,  20, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_REPAIR,              // Engineer entering building to damage it.
    MouseControlType {MouseNames[MOUSE_NO_REPAIR],           190, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_REPAIR,
    MouseControlType {MouseNames[MOUSE_WAYPOINT],            191, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_WAYPOINT,
    MouseControlType {MouseNames[MOUSE_PLACE_WAYPOINT],      201, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_PLACE_WAYPOINT,
    MouseControlType {MouseNames[MOUSE_NO_PLACE_WAYPOINT],   211, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_PLACE_WAYPOINT,
    MouseControlType {MouseNames[MOUSE_SELECT_WAYPOINT],     212, 7,  4, -1,  7,  4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SELECT_WAYPOINT,
    MouseControlType {MouseNames[MOUSE_ENTER_WAYPOINT_MODE], 219, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_ENTER_WAYPOINT_MODE,
    MouseControlType {MouseNames[MOUSE_FOLLOW_WAYPOINT],     229, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_FOLLOW_WAYPOINT,
    MouseControlType {MouseNames[MOUSE_WAYPOINT_TOTE],       239, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_WAYPOINT_TOTE,
    MouseControlType {MouseNames[MOUSE_WAYPOINT_REPAIR],     249, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_WAYPOINT_REPAIR,
    MouseControlType {MouseNames[MOUSE_ATTACK_WAYPOINT],     259, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_ATTACK_WAYPOINT,
    MouseControlType {MouseNames[MOUSE_ENTER_WAYPOINT],      269, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_ENTER_WAYPOINT,
    MouseControlType {MouseNames[MOUSE_LOOP_WAYPOINT_PATH],  356, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_LOOP_WAYPOINT_PATH,
    MouseControlType {MouseNames[MOUSE_AIR_STRIKE],          279, 20, 4, -1,  20, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_AIR_STRIKE,
    MouseControlType {MouseNames[MOUSE_CHEMBOMB],            299, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_CHEMBOMB,
    MouseControlType {MouseNames[MOUSE_DEMOLITIONS],         309, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_DEMOLITIONS,
    MouseControlType {MouseNames[MOUSE_NUCLEAR_BOMB],        319, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NUCLEAR_BOMB,
    MouseControlType {MouseNames[MOUSE_TOGGLE_POWER],        329, 16, 2, -1,  16, 2, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_TOGGLE_POWER,
    MouseControlType {MouseNames[MOUSE_NO_TOGGLE_POWER],     345, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_NO_TOGGLE_POWER,
    MouseControlType {MouseNames[MOUSE_HEAL],                346, 10, 4, 42,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_HEAL,
    MouseControlType {MouseNames[MOUSE_EM_PULSE],            357, 20, 3, -1,  20, 3, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_EM_PULSE,
    MouseControlType {MouseNames[MOUSE_EM_PULSE_RANGE],      377, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_EM_PULSE_RANGE,  
  
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING],     378, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_N],   379, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_N,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_NE],  380, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_NE,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_E],   381, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_E,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_SE],  382, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_SE,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_S],   383, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_S,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_SW],  384, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_SW,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_W],   385, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_W,
    MouseControlType {MouseNames[MOUSE_SCROLL_COASTING_NW],  386, 1,  0, -1,  1,  0, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}, // MOUSE_SCROLL_COASTING_NW,
  
    MouseControlType {MouseNames[MOUSE_PATROL_WAYPOINT],     387, 10, 4, -1,  10, 4, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }, { MOUSE_HOTSPOT_CENTER, MOUSE_HOTSPOT_CENTER }}  // MOUSE_PATROL_WAYPOINT,
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
MouseTypeClass::MouseTypeClass(const char* name, int start_frame, int frame_count, int frame_rate, int small_frame, int small_frame_count, int small_frame_rate, Point2D hotspot, Point2D small_hotspot) :
    Name(name),
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

        MouseTypeClass *mousetype = new MouseTypeClass(
            MouseControl[mouse].Name,
            MouseControl[mouse].StartFrame,
            MouseControl[mouse].FrameCount,
            MouseControl[mouse].FrameRate,
            MouseControl[mouse].SmallFrame,
            MouseControl[mouse].SmallFrameCount,
            MouseControl[mouse].SmallFrameRate,
            MouseControl[mouse].Hotspot,
            MouseControl[mouse].SmallHotspot);

        ASSERT(mousetype != nullptr);
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

        /**
         *  Load the properties for this mouse type.
         */
        int readlen = ini.Get_String(MOUSE, entry_name, "", buffer, sizeof(buffer));
        ASSERT_FATAL(readlen > 0);

        MouseTypeClass *mousectrl = Find_Or_Make(entry_name);
        ASSERT(mousectrl != nullptr);

        mousectrl->Name = entry_name;

        tok = std::strtok(buffer, ",");
        mousectrl->StartFrame = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse StartFrame for {}!", mousectrl->Name);

        tok = std::strtok(nullptr, ",");
        mousectrl->FrameCount = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse FrameCount for {}!", mousectrl->Name);

        tok = std::strtok(nullptr, ",");
        mousectrl->FrameRate = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse FrameRate for {}!", mousectrl->Name);

        tok = std::strtok(nullptr, ",");
        mousectrl->SmallFrame = std::strtol(tok, nullptr, 10);
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse SmallFrame for {}!", mousectrl->Name);

        tok = std::strtok(nullptr, ",");
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse SmallFrameCount for {}!", mousectrl->Name);
        mousectrl->SmallFrameCount = std::strtol(tok, nullptr, 10);

        tok = std::strtok(nullptr, ",");
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse SmallFrameRate for {}!", mousectrl->Name);
        mousectrl->SmallFrameRate = std::strtol(tok, nullptr, 10);

        tok = std::strtok(nullptr, ",");
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse HotspotX for {}!", mousectrl->Name);
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
        ASSERT_FATAL_PRINT(tok != nullptr, "Unable to parse HotspotY for {}!", mousectrl->Name);
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
        auto const& control = MouseControl[mouse];

        const char *hotspot_x = nullptr;
        const char *hotspot_y = nullptr;
        const char *smallhotspot_x = nullptr;
        const char *smallhotspot_y = nullptr;

        switch (control.Hotspot.X) {
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
                DEBUG_ERROR("Mouse: Invalid hotspot X for {}!\n", control.Name);
                return false;
        }

        switch (control.Hotspot.Y) {
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
                DEBUG_ERROR("Mouse: Invalid hotspot Y for {}!\n", control.Name);
                return false;
        }

        switch (control.SmallHotspot.X) {
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
                DEBUG_ERROR("Mouse: Invalid hotspot X for {}!\n", control.Name);
                return false;
        }

        switch (control.SmallHotspot.Y) {
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
                DEBUG_ERROR("Mouse: Invalid hotspot Y for {}!\n", control.Name);
                return false;
        }

        std::snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d,%d,%d,%s,%s,%s,%s",
                                    control.StartFrame,
                                    control.FrameCount,
                                    control.FrameRate,
                                    control.SmallFrame,
                                    control.SmallFrameCount,
                                    control.SmallFrameRate,
                                    hotspot_x,
                                    hotspot_y,
                                    smallhotspot_x,
                                    smallhotspot_y);

        ini.Put_String(MOUSE, control.Name, buffer);
    }

    return true;
}
#endif


/**
 *  Retrieves the mouse type for given name.
 *
 *  @author: CCHyper
 */
MouseType MouseTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (std::string_view(name) == "<none>" || std::string_view(name) == "none") {
        return MOUSE_NORMAL;
    }

    if (name != nullptr) {
        for (MouseType index = MOUSE_NORMAL; index < MouseTypes.Count(); ++index) {
            if (MouseTypes[index]->Name == name) {
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
    return (type >= MOUSE_NORMAL && type < MouseTypes.Count() ? MouseTypes[type]->Name.c_str() : "<none>");
}


/**
 *  Find or create a mouse type of the name specified.
 *
 *  @author: CCHyper
 */
MouseTypeClass *MouseTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    if (std::string_view(name) == "<none>" || std::string_view(name) == "none") {
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
