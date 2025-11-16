/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BEACON_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for beacons.
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

#include "beacon_hooks.h"

#include "beacon.h"
#include "building.h"
#include "extension.h"
#include "foot.h"
#include "hooker.h"
#include "house.h"
#include "mouse.h"
#include "radar.h"
#include "radarevent.h"
#include "syringe.h"
#include "tacticalext.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"


/**
 *  Patch to redraw the radar when a beacon anim needs redraw.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005BC4BB, _RadarClass_Render_Radar_Redraw_Beacons_Patch, 0)
{
    GET(RadarClass*, this_ptr, ESI);

    if (this_ptr->BackgroundUpdateStack.Count() > 0 ||
        BeaconManager.Is_To_Redraw_Radar()) {

        return 0x005BC4C9;
    }

    return 0x005BCBBC;
}


/**
 *  Draws beacons on the radar.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005BC83F, _RadarClass_Render_Radar_Draw_Beacons_Patch, 0)
{
    RadarEventClass::Draw_Events();
    BeaconManager.Draw_On_Radar(Map.RadarSurface, Map.RadarSurface->Get_Rect());

    return 0x005BC844;
}


/**
 *  Deletes the house's beacons when the house is defeated.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004BF5CC, _HouseClass_MPlayer_Defeated_Delete_Beacons_Patch, 10)
{
    GET(HouseClass*, this_ptr, EBX);

    BeaconManager.Delete_Owned_Beacons(this_ptr->HeapID);

    return 0;
}

/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class DisplayClassExt : public DisplayClass
{
public:
    void _Sell_Mode_Control(int control);
    void _Power_Mode_Control(int control);
    void _Repair_Mode_Control(int control);
    void _Mouse_Right_Release(Point2D const& point);
};


/**
 *  Replacement for DisplayClass::Sell_Mode_Control.
 *  Disables beacon placement mode when sell mode is entered.
 *
 *  @author: ZivDero, tomsons26
 */
void DisplayClassExt::_Sell_Mode_Control(int control)
{
    bool mode = IsSellMode;
    switch (control) {
    case 0:
        mode = false;
        break;

    case -1:
        mode = IsSellMode == false;
        break;

    case 1:
        mode = true;
        break;
    }

    if (mode != IsSellMode && !PendingObject) {
        Set_Default_Mouse(MOUSE_NORMAL, false);
        IsRepairMode = false;
        IsPowerMode = false;
        IsWaypointMode = false;
        TacticalMapExtension->IsBeaconPlacementMode = false;
        if (mode && PlayerPtr->CurBuildings > 0) {
            IsSellMode = true;
            Unselect_All();
        } else {
            IsSellMode = false;
            Revert_Mouse_Shape();
        }
    }
}


/**
 *  Replacement for DisplayClass::Power_Mode_Control.
 *  Disables beacon placement mode when power mode is entered.
 *
 *  @author: ZivDero, tomsons26
 */
void DisplayClassExt::_Power_Mode_Control(int control)
{
    bool mode = IsPowerMode;
    switch (control) {
    case 0:
        mode = false;
        break;

    case -1:
        mode = IsPowerMode == false;
        break;

    case 1:
        mode = true;
        break;
    }

    if (mode != IsPowerMode && !PendingObject) {
        Set_Default_Mouse(MOUSE_NORMAL, false);
        IsRepairMode = false;
        IsSellMode = false;
        IsWaypointMode = false;
        TacticalMapExtension->IsBeaconPlacementMode = false;
        if (mode && PlayerPtr->CurBuildings > 0) {
            IsPowerMode = true;
            Unselect_All();
        } else {
            IsPowerMode = false;
            Revert_Mouse_Shape();
        }
    }
}


/**
 *  Replacement for DisplayClass::Repair_Mode_Control.
 *  Disables beacon placement mode when repair mode is entered.
 *
 *  @author: ZivDero, tomsons26
 */
void DisplayClassExt::_Repair_Mode_Control(int control)
{
    bool mode = IsRepairMode;
    switch (control) {
    case 0:
        mode = false;
        break;

    case -1:
        mode = IsRepairMode == false;
        break;

    case 1:
        mode = true;
        break;
    }

    if (mode != IsRepairMode && !PendingObject) {
        IsSellMode = false;
        IsPowerMode = false;
        IsWaypointMode = false;
        TacticalMapExtension->IsBeaconPlacementMode = false;
        Set_Default_Mouse(MOUSE_NORMAL, false);
        if (mode && PlayerPtr->CurBuildings > 0) {
            IsRepairMode = true;
            Unselect_All();
        } else {
            IsRepairMode = false;
            Revert_Mouse_Shape();
        }
    }
}


/**
 *  Replacement for DisplayClass::Mouse_Right_Release.
 *  Disables beacon placement mode on RMB.
 *
 *  @author: ZivDero, tomsons26
 */
void DisplayClassExt::_Mouse_Right_Release(Point2D const& point)
{
    if (PendingObjectPtr && PendingObjectPtr->Is_Techno()) {
        PendingObjectPtr = nullptr;
        PendingObject = nullptr;
        PendingHouse = HOUSE_NONE;
        Set_Cursor_Shape(nullptr);
    } else {
        if (IsRepairMode) {
            Repair_Mode_Control(0);
        } else {
            if (IsSellMode) {
                Sell_Mode_Control(0);
            } else {
                if (IsPowerMode) {
                    Power_Mode_Control(0);
                } else {
                    if (TargettingType != SUPER_NONE) {
                        TargettingType = SUPER_NONE;
                    } else {
                        if (IsWaypointMode) {
                            Waypoint_Mode_Control(0);
                        } else {
                            if (TacticalMapExtension->IsBeaconPlacementMode) {
                                TacticalMapExtension->Beacon_Mode_Control(0);
                            } else {
                                Unselect_All();
                            }
                        }
                    }
                }
            }
        }
    }

    Set_Default_Mouse(MOUSE_NORMAL, Map.IsSmall);
}


/**
 *  Disables beacon placement mode when waypoint mode is entered.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004795CF, _DisplayClass_Waypoint_Mode_Control_BeaconMode_Patch, 7)
{
    TacticalMapExtension->IsBeaconPlacementMode = false;

    return 0;
}


/**
 *  Acts on the various beacon actions.
 *
 *  @author: ZivDero
 */
void Place_Beacon(Cell const& cell, ActionType action)
{
    if (!PlayerPtr->IsDefeated) {
        Coord coord = cell.As_Coord();
        coord.Z = Map[coord].Height * LEVEL_LEPTON_H;
        BeaconManager.Place_Beacon(PlayerPtr->HeapID, coord, -1, BeaconManagerClass::Beacon_Text(action));
    }
    TacticalMapExtension->Beacon_Mode_Control(0);
    Map.Set_Default_Mouse(MOUSE_NORMAL, false);
}

void Select_Beacon(Cell const& cell)
{
    Coord coord = cell.As_Coord();
    coord.Z = Map[coord].Height * LEVEL_LEPTON_H;
    BeaconManager.Select_Beacon(coord);
}

DEFINE_HOOK(0x00478C6D, _DisplayClass_Mouse_Left_Release_Beacon_Patch, 0)
{
    GET(ActionType, action, EDI);
    GET(Cell const*, cell, EBP);

    if (BeaconManagerClass::Is_Beacon_Placement_Action(action)) {
        Place_Beacon(*cell, action);
        return 0x004790D1;
    } if (action == EXT_ACTION_SELECT_BEACON) {
        Select_Beacon(*cell);
        return 0x00478B1E;
    } else if (action == ACTION_LOOP_WAYPOINT_PATH) {
        return 0x00478C72;
    } else {
        return 0x00478CE5;
    }
}


/**
 *  Sets the beacon selection action when in beacon mode.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005E8A1B, _ScrollClass_What_Action_Select_Beacon_Patch, 0)
{
    GET(ObjectClass*, object, ESI);
    GET(TechnoClass*, techno, EDI);
    GET(ActionType, action, EBP);
    GET_STACK(Cell*, cell, 0x20);

    if (object && object->Class_Of() && object->Class_Of()->IsSelectable && (object->RTTI != RTTI_BUILDING || !static_cast<BuildingClass*>(object)->IsFogged) && (techno == nullptr || !techno->IsALoaner)) {
        action = ACTION_SELECT;
    }

    Coord coord = cell->As_Coord();
    coord.Z = Map[*cell].Height * LEVEL_LEPTON_H;
    if (BeaconManager.Beacon_At(coord) != nullptr) {
        action = static_cast<ActionType>(EXT_ACTION_SELECT_BEACON);
    }

    R->EBP(action);

    return 0x005E8A6A;
}


/**
 *  Sets the beacon placement action when in beacon mode.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005E8C41, _ScrollClass_What_Action_Place_Beacon_Patch, 0)
{
    if (TacticalMapExtension->IsBeaconPlacementMode) {
        ActionType action = BeaconManagerClass::Pick_Beacon_Placement_Action();
        R->EBP(action);
    }
    if (Map.IsSellMode) {
        return 0x005E8C4E;
    } else {
        return 0x005E8D7F;
    }
}


/**
 *  Main function for patching the hooks.
 */
void Beacon_Hooks()
{
    Patch_Jump(0x004794E0, &DisplayClassExt::_Sell_Mode_Control);
    Patch_Jump(0x00479690, &DisplayClassExt::_Power_Mode_Control);
    Patch_Jump(0x00479730, &DisplayClassExt::_Repair_Mode_Control);
    Patch_Jump(0x00477D30, &DisplayClassExt::_Mouse_Right_Release);
}

