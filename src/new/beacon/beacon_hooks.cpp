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
#include "hooker_macros.h"
#include "house.h"
#include "mouse.h"
#include "radar.h"
#include "radarevent.h"
#include "tacticalext.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"


//
DECLARE_PATCH(_RadarClass_Render_Radar_Redraw_Beacons_Patch)
{
    GET_REGISTER_STATIC(RadarClass*, this_ptr, esi);

    if (this_ptr->BackgroundUpdateStack.Count() > 0 ||
        BeaconManager.Is_To_Redraw_Radar()) {

        JMP(0x005BC4C9);
    }

    JMP(0x005BCBBC);
}

void Draw_Radar_Beacons()
{
    BeaconManager.Draw_On_Radar(Map.RadarSurface, Map.RadarSurface->Get_Rect());
}


DECLARE_PATCH(_RadarClass_Render_Radar_Draw_Beacons_Patch)
{
    RadarEventClass::Draw_Events();
    Draw_Radar_Beacons();

    JMP(0x005BC844);
}


DECLARE_PATCH(_HouseClass_MPlayer_Defeated_Delete_Beacons_Patch)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, ebx);
    GET_REGISTER_STATIC(MouseClass*, map, ecx);

    Session.ObiWan = true;
    BeaconManager.Delete_Owned_Beacons(this_ptr->HeapID);
    
    _asm mov ecx, map
    JMP(0x004BF5D6);
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


DECLARE_PATCH(_DisplayClass_Waypoint_Mode_Control_BeaconMode_Patch)
{
    GET_REGISTER_STATIC(DisplayClass*, this_ptr, esi);

    this_ptr->IsPowerMode = false;
    TacticalMapExtension->IsBeaconPlacementMode = false;

    JMP(0x004795D6);
}

void Place_Beacon(Cell* cell, ActionType action)
{
    if (!PlayerPtr->IsDefeated) {
        Coord coord = cell->As_Coord();
        coord.Z = Map[coord].Height * LEVEL_LEPTON_H;
        BeaconManager.Place_Beacon(PlayerPtr->HeapID, coord, -1, BeaconManagerClass::Beacon_Text(action));
    }
    TacticalMapExtension->Beacon_Mode_Control(0);
    Map.Set_Default_Mouse(MOUSE_NORMAL, false);
}

void Select_Beacon(Cell* cell)
{
    Coord coord = cell->As_Coord();
    coord.Z = Map[coord].Height * LEVEL_LEPTON_H;
    BeaconManager.Select_Beacon(coord);
}

DECLARE_PATCH(_DisplayClass_Mouse_Left_Release_Beacon_Patch)
{
    GET_REGISTER_STATIC(ActionType, action, edi)
    GET_REGISTER_STATIC(Cell*, cell, ebp);

    if (BeaconManagerClass::Is_Beacon_Placement_Action(action)) {
        Place_Beacon(cell, action);
        JMP(0x004790D1);
    } if (action == EXT_ACTION_SELECT_BEACON) {
        Select_Beacon(cell);
        JMP(0x00478B1E);
    } else if (action == ACTION_LOOP_WAYPOINT_PATH) {
        JMP(0x00478C72);
    } else {
        JMP(0x00478CE5);
    }
}


void Process_Select(ObjectClass* object, TechnoClass* techno, ActionType& action, Cell* cell)
{
    if (object && object->Class_Of() && object->Class_Of()->IsSelectable && (object->RTTI != RTTI_BUILDING || !static_cast<BuildingClass*>(object)->IsFogged) && (techno == nullptr || !techno->IsALoaner)) {
        action = ACTION_SELECT;
    }

    Coord coord = cell->As_Coord();
    coord.Z = Map[*cell].Height * LEVEL_LEPTON_H;
    if (BeaconManager.Beacon_At(coord) != nullptr) {
        action = static_cast<ActionType>(EXT_ACTION_SELECT_BEACON);
    }
}


DECLARE_PATCH(_ScrollClass_What_Action_Select_Beacon_Patch)
{
    GET_REGISTER_STATIC(ObjectClass*, object, esi);
    GET_REGISTER_STATIC(TechnoClass*, techno, edi);
    GET_REGISTER_STATIC(ActionType, action, ebp);
    GET_STACK_STATIC(Cell*, cell, esp, 0x20);

    Process_Select(object, techno, action, cell);

    _asm mov ebp, action
    JMP(0x005E8A6A);
}


DECLARE_PATCH(_ScrollClass_What_Action_Place_Beacon_Patch)
{
    if (TacticalMapExtension->IsBeaconPlacementMode) {
        static ActionType action;
        action = BeaconManagerClass::Pick_Beacon_Placement_Action();
        _asm mov ebp, action
    }
    if (Map.IsSellMode) {
        JMP(0x005E8C4E);
    } else {
        JMP(0x005E8D7F);
    }
}


/**
 *  Main function for patching the hooks.
 */
void Beacon_Hooks()
{
    Patch_Jump(0x005BC4BB, &_RadarClass_Render_Radar_Redraw_Beacons_Patch);
    Patch_Jump(0x005BC83F, &_RadarClass_Render_Radar_Draw_Beacons_Patch);
    Patch_Jump(0x004BF5CC, &_HouseClass_MPlayer_Defeated_Delete_Beacons_Patch);
    Patch_Jump(0x004794E0, &DisplayClassExt::_Sell_Mode_Control);
    Patch_Jump(0x00479690, &DisplayClassExt::_Power_Mode_Control);
    Patch_Jump(0x00479730, &DisplayClassExt::_Repair_Mode_Control);
    Patch_Jump(0x00477D30, &DisplayClassExt::_Mouse_Right_Release);
    Patch_Jump(0x004795CF, &_DisplayClass_Waypoint_Mode_Control_BeaconMode_Patch);
    Patch_Jump(0x00478C6D, &_DisplayClass_Mouse_Left_Release_Beacon_Patch);
    Patch_Jump(0x005E8A1B, &_ScrollClass_What_Action_Select_Beacon_Patch);
    Patch_Jump(0x005E8C41, &_ScrollClass_What_Action_Place_Beacon_Patch);
}
