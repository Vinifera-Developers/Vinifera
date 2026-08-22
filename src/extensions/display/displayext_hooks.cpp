/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended DisplayClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "displayext_hooks.h"

#include "actiontype.h"
#include "building.h"
#include "buildingtypeext.h"
#include "cell.h"
#include "display.h"
#include "extension.h"
#include "hooker.h"
#include "house.h"
#include "iomap.h"
#include "layer.h"
#include "rules.h"
#include "session.h"
#include "sessionext.h"
#include "syringe.h"
#include "tactical.h"
#include "techno.h"
#include "technoext.h"
#include "technotype.h"
#include "tibsun_globals.h"
#include "tibsun_util.h"
#include "vinifera_globals.h"
#include "wwmouse.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class DisplayClassExt final : public DisplayClass
{
    public:
        ObjectClass * _Next_Object(ObjectClass * object) const;
        ObjectClass * _Prev_Object(ObjectClass * object) const;
        void _Compute_Start_Pos();
        void _Constrained_Look(Coord const& center, LEPTON distance);
};


/**
 *  Reimplementation of DisplayClass::Next_Object.
 * 
 *  Searches for next object on display.
 * 
 *  @author: 06/20/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
ObjectClass * DisplayClassExt::_Next_Object(ObjectClass * object) const
{
    static const LayerType _layers[] = {
        
        /**
         *  #issue-785
         * 
         *  Adds underground layer to display search.
         */
        LAYER_UNDERGROUND,

        LAYER_GROUND, LAYER_AIR, LAYER_TOP,
    };

    ObjectClass * firstobj = nullptr;
    bool foundmatch = false;

    if (object == nullptr) {
        foundmatch = true;
    }
    for (int index = 0; index < std::size(_layers); ++index) {
        LayerType layer = _layers[index];

        for (unsigned uindex = 0; uindex < (unsigned)Layer[layer].Count(); uindex++) {
            ObjectClass * obj = Layer[layer][uindex];

            /**
             *  Verify that the object can be selected by and is owned by the player.
             */
            if (obj != nullptr && obj->Is_Players_Army()) {
                if (firstobj == nullptr) firstobj = obj;
                if (foundmatch) return obj;
                if (object == obj) foundmatch = true;
            }
        }
    }
    return firstobj;
}


/**
 *  Reimplementation of DisplayClass::Next_Object.
 * 
 *  Searches for the previous object on the map.
 * 
 *  @author: 08/24/1995 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
ObjectClass * DisplayClassExt::_Prev_Object(ObjectClass * object)  const
{
    static const LayerType _layers[] = {
        LAYER_TOP, LAYER_AIR, LAYER_GROUND,

        /**
         *  #issue-785
         * 
         *  Adds underground layer to display search.
         */
        LAYER_UNDERGROUND,
    };

    ObjectClass * firstobj = nullptr;
    bool foundmatch = false;

    if (object == nullptr) {
        foundmatch = true;
    }
    for (int index = 0; index < std::size(_layers); ++index) {
        LayerType layer = _layers[index];

        for (int uindex = Layer[layer].Count()-1; uindex >= 0; uindex--) {
            ObjectClass * obj = Layer[layer][uindex];

            /**
             *  Verify that the object can be selected by and is owned by the player.
             */
            if (obj != nullptr && obj->Is_Players_Army()) {
                if (firstobj == nullptr) firstobj = obj;
                if (foundmatch) return obj;
                if (object == obj) foundmatch = true;
            }
        }
    }
    return firstobj;
}


/**
 *  Computes player's start pos from unit coords.
 *
 *  @author: 02/28/1995 JLB - Red Alert Source COde
 *           29/10/2024 ZivDero - Adjustments for Tiberian Sun
 */
void DisplayClassExt::_Compute_Start_Pos()
{
    /**
     *  Find the summation coordinate for all the player's units, infantry,
     *  and buildings.
     */
    Coord coord(0, 0, 0);
    long num = 0;

    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* technop = Technos[i];
        if (!technop->IsInLimbo && technop->IsOwnedByPlayer) {
            coord += technop->Get_Coord();
            num++;
        }
    }

    /**
     *  Divide the coordinate by 'num' to compute the average value.
     */
    coord.Z = 0;
    if (num != 0) {
        coord /= num;
    }

    /**
     *  If the player has no units (i. e. is an observer), use their house's center cell.
     */
    else {
        coord = PlayerPtr->Center;
    }

    Scen->Views[0] = Scen->Views[1] = Scen->Views[2] = Scen->Views[3] = Cell(coord);
    Scen->AltHome = Scen->Home;

    if (TacticalMap != nullptr) {
        TacticalMap->Set_Tactical_Position(coord);
    }
}


/**
 *  Sets the mouse cursor based on the action.
 *
 *  @author: CCHyper, ZivDero
 */
DEFINE_HOOK(0x004782CF, _DisplayClass_Mouse_Left_Up_Set_Mouse, 0)
{
    GET(ActionType, action, EBX);
    GET_STACK(bool, shadow, 0x1C);
    GET_STACK(CellClass*, cellptr, 0xC);
    GET_STACK(bool, wsmall, 0x28);

    MouseType mouse = MOUSE_NORMAL;

    if (shadow) {
        mouse = ActionTypes[action]->Get_Shadow_Mouse();
        if (action == ACTION_NOMOVE) {
            if (CurrentObjects.Count() && CurrentObjects[0]->Is_Techno() && CurrentObjects[0]->TClass->MoveToShroud) {
                mouse = ActionTypes[ACTION_MOVE]->Get_Shadow_Mouse();
            }
        }
    } else {
        mouse = ActionTypes[action]->Get_Mouse();
        if (action == ACTION_ATTACK) {
            if (cellptr && CurrentObjects.Count() == 1 && CurrentObjects[0]->Is_Techno() && static_cast<TechnoClass*>(CurrentObjects[0])->In_Range_Of(cellptr)) {
                mouse = MOUSE_STAY_ATTACK;
            }
        }
    }

    Map.Set_Default_Mouse(mouse, wsmall);

    // return;
    return 0x004786C5;
}


/**
 *  #issue-171
 * 
 *  Adds game option to control if allies can build off each others bases.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004762E4, _DisplayClass_Passes_Proximity_Passes_Check_Patch, 0)
{
    GET(BuildingClass *, base, EAX);
    GET_STACK(HousesType, house, 0x38);
    REF_STACK(bool, passes, 0x3C);

    HouseClass* hptr = Houses[house];

    /**
     *  Stolen bytes/code.
     * 
     *  Ensure the building is considered eligible for adjacency checks.
     */
    if (base->House->HeapID == house && base->Class->IsBase) {
        passes = true;
    }

    /**
     *  If the build-off-ally option is enabled, ensure the building is
     *  owned by an ally house and is eligible for adjacent building before
     *  passing the check.
     * 
     *  #NOTE: This feature is only available for multiplayer games.
     */
    if (Session.Type != GAME_NORMAL) {
        if (SessionExtension && SessionExtension->ExtOptions.IsBuildOffAlly) {

            if (base->House != hptr && base->House->Is_Ally(hptr)) {

                BuildingTypeClassExtension* buildingtypeext = Extension::Fetch(base->Class);
                if (buildingtypeext->IsEligibleForAllyBuilding) {
                    passes = true;
                }
            }
        }
    }

continue_scan:
    return 0x00476308;
}


/**
 *  #issue-344
 * 
 *  This patch fixes a bug/glitch where the user can place a building
 *  anywhere on the map by moving the mouse over the sidebar while the
 *  proximity checks have been passed.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00478974, _DisplayClass_Mouse_Left_Release_PlaceAnywhere_BugFix_Patch, 0)
{
    GET(DisplayClass *, this_ptr, EBX);

    /**
     *  Find out where the mouse cursor is, if its over the sidebar
     *  then invalidate the proximity checks, fixing the glitch.
     */
    Point2D mouse_pos = Get_Mouse_Point();
    if (mouse_pos.X >= TacticalRect.Width-1) {
        this_ptr->IsProximityCheck = false;
        this_ptr->IsShroudCheck = false;
        goto unable_to_deploy;
    }

    /**
     *  Stolen bytes/code here.
     */

    /**
     *  Try to place the pending object onto the map.
     */
    if (this_ptr->IsProximityCheck && this_ptr->IsShroudCheck) {
        goto place_it;
    }

    /**
     *  Cannot deploy here.
     */
unable_to_deploy:
    return 0x00478A30;

    /**
     *  Create PLACE event.
     */
place_it:
    return 0x00478990;
}


/**
 *  Patch to return the mouse coords if the developer option is enabled.
 * 
 *  @see: CursorPositionCommandClass.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0047AFA6, _DisplayClass_Help_Text_GetCursorPosition_Patch, 0)
{
    LEA_STACK(Coord *, coordinate, 0x2C);
    static char _cursor_position_buffer[128];

    if (Vinifera_Developer_ShowCursorPosition) {

        Cell tmpcell = Get_Cell_Under_Mouse();
        Coord tmpcoord = Get_Coord_Under_Mouse();

        /**
         *  Fixup Z position based on cell height.
         */
        tmpcoord.Z = Map.Get_Height_GL(tmpcoord);

        /**
         *  Format the buffer with the cell and coord of the
         *  current mouse cursor position.
         */
        std::snprintf(_cursor_position_buffer, sizeof(_cursor_position_buffer),
            " Cell: %d,%d  Coord: %d,%d,%d ",
            tmpcell.X, tmpcell.Y, tmpcoord.X, tmpcoord.Y, tmpcoord.Z);

        R->EAX(_cursor_position_buffer);
        goto return_label;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    if (!Map[*coordinate].IsMapped && MainWindow) {
        goto txt_shadow;
    }

    /**
     *  Continue the function flow.
     */
continue_function:
    return 0x0047AFDA;

    /**
     *  Returns TXT_SHADOW.
     */
txt_shadow:
    return 0x0047AFC7;

    /**
     *  Function return, expects buffer or string pointer in EAX register.
     */
return_label:
    return 0x0047AFD1;
}


/**
 *  #issue-71
 *
 *  Replace the old waypoint count in a loop.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0047A856, _DisplayClass_47A790_Patch, 0)
{
    GET(int, i, EDI);

    if (i < NEW_WAYPOINT_COUNT) {
        return 0x0047A7FC;
    } else {
        return 0x0047A85B;
    }
}


/**
 *  Patches DisplayClass::Mouse_Left_Release to skip the 'Active_Click' call when the action
 *  is ACTION_TOGGLE_SELECT (adding to selection). In most cases, this was unnecessary and did nothing.
 * 
 *  However, in the case of medics, since they can click on themselves to switch to Area Guard,
 *  this caused adding them to selection to immediately move them to Area Guard, while also flashing.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x00478BC7, _Display_Class_Mouse_Left_Release_Toggle_Select_Patch, 5)
{
    GET(ActionType, action, EDI);

    if (action == ACTION_TOGGLE_SELECT) {
        return 0x00478BD8;
    }

    return 0;
}


/**
 *  Reimplements DisplayClass::Constrained_Look
 *  Adds support for veterancy bonuses for sight range
 *
 *  @author: JoyfulShush
 */
void DisplayClassExt::_Constrained_Look(Coord const& center, LEPTON distance)
{
    for (int index = 0; index < Layer[LAYER_GROUND].Count(); index++) {
        TechnoClass* techno = dynamic_cast<TechnoClass*>(Layer[LAYER_GROUND][index]);
        if (techno != NULL) {
            auto techno_ext = Extension::Fetch(techno);
            if (techno->House->Is_Player_Control()) {
                if (techno->IsDiscoveredByPlayer && Distance(center, techno->Center_Coord()) <= (techno_ext->Get_Sight_Range() * CELL_LEPTON_W) + distance) {
                    techno->Look();
                }
            } else {
                if (techno->RTTI == RTTI_BUILDING && Rule->IsAllyReveal && techno->House->Is_Ally(PlayerPtr) && Distance(techno->Center_Coord(), center) <= (techno_ext->Get_Sight_Range() * CELL_LEPTON_W) + distance) {
                    techno->Look();
                }
            }
        }
    }
}


/**
 *  Main function for patching the hooks.
 */
void DisplayClassExtension_Hooks()
{
    /**
     *  #issue-76
     * 
     *  Extend the IsoMapPack5 decoding size buffer.
     * 
     *  When large maps with lots of terrain have over 9750 lines in the
     *  IsoMapPack5 section, the game is unable to decode further lines and fills
     *  the bottom left area of the map with clear tiles.
     * 
     *  These patches increase the buffer size to 3 times the original size.
     * 
     *  @author: CCHyper (based on research by E1Elite)
     */
    #define ISOMAPPACK_BUFF_WIDTH 1024
    #define ISOMAPPACK_BUFF_HEIGHT 768
    Patch_Dword(0x0047A0B5+1, ISOMAPPACK_BUFF_WIDTH);
    Patch_Dword(0x0047A0BA+1, ISOMAPPACK_BUFF_HEIGHT);
    Patch_Dword(0x0047A0C8+1, ISOMAPPACK_BUFF_WIDTH*ISOMAPPACK_BUFF_HEIGHT*sizeof(unsigned short));

    Patch_Byte_Range(0x0047C0A2, 0x90, 13); // Patch out logging "Display: Abort_Drag_Select()"

    Patch_Jump(0x00477390, &DisplayClassExt::_Next_Object);
    Patch_Jump(0x00477430, &DisplayClassExt::_Prev_Object);
    Patch_Jump(0x004793A0, &DisplayClassExt::_Compute_Start_Pos);
    Patch_Jump(0x0047AAF0, &DisplayClassExt::_Constrained_Look);
}
