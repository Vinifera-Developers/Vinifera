/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DISPLAYEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended DisplayClass.
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
#include "displayext_hooks.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_util.h"
#include "display.h"
#include "iomap.h"
#include "cell.h"
#include "building.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "techno.h"
#include "technotype.h"
#include "house.h"
#include "housetype.h"
#include "session.h"
#include "sessionext.h"
#include "wwmouse.h"
#include "mousetype.h"
#include "actiontype.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Sets the mouse cursor based on the action.
 *
 *  @author: CCHyper, ZivDero
 */
static void Display_Set_Mouse_Cursor(ActionType action, bool shadow, bool wsmall, CellClass *cellptr)
{
    MouseType mouse = MOUSE_NORMAL;

    if (shadow) {
        
        mouse = ActionTypeClass::As_Reference(action).Get_Shadow_Mouse();

        if (action == ACTION_NOMOVE) {
            if (CurrentObjects.Count()
                && CurrentObjects[0]->Is_Techno()
                && CurrentObjects[0]->Techno_Type_Class()->MoveToShroud) {

                mouse = ActionTypeClass::As_Reference(ACTION_MOVE).Get_Shadow_Mouse();
            }
        }

    } else {
        
        mouse = ActionTypeClass::As_Reference(action).Get_Mouse();

        if (action == ACTION_ATTACK) {
            if (cellptr
                && CurrentObjects.Count() == 1
                && CurrentObjects[0]->Is_Techno()
                && static_cast<TechnoClass*>(CurrentObjects[0])->In_Range_Of(cellptr)) {

                mouse = MOUSE_STAY_ATTACK;
            }
        }

    }

    Map.Set_Default_Mouse(mouse, wsmall);
}


/**
 *  Patch to set the mouse cursor based on the action.
 *
 *  @author: CCHyper, ZivDero
 */
DECLARE_PATCH(_DisplayClass_Mouse_Left_Up_Set_Mouse)
{
    GET_REGISTER_STATIC(ActionType, action, ebx);
    GET_STACK_STATIC8(bool, shadow, esp, 0x20);
    GET_STACK_STATIC(CellClass *, cellptr, esp, 0x10);
    GET_STACK_STATIC8(bool, wsmall, esp, 0x2C);

    _asm { pop ebp }

    Display_Set_Mouse_Cursor(action, shadow, wsmall, cellptr);

    //return
    JMP(0x004786C5);
}


/**
 *  #issue-171
 * 
 *  Adds game option to control if allies can build off each others bases.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_DisplayClass_Passes_Proximity_Passes_Check_Patch)
{
    //GET_REGISTER_STATIC(DisplayClass *, this_ptr, ?);   // No access to "this".
    GET_REGISTER_STATIC(BuildingClass *, base, eax);
    GET_STACK_STATIC(HousesType, house, esp, 0x38);
    //GET_STACK_STATIC8(bool, passes, esp, 0x3C);
    static BuildingTypeClassExtension *buildingtypeext;
    static HouseClass *hptr;

    /**
     *  Store the proximity check result.
     */
    #define passes() _asm { mov byte ptr [esp+0x3C], 1 }

    hptr = Houses[house];

    /**
     *  Stolen bytes/code.
     * 
     *  Ensure the building is considered eligible for adjacency checks.
     */
    if (base->House->ID == house && base->Class->IsBase) {
        passes();
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

                buildingtypeext = Extension::Fetch<BuildingTypeClassExtension>(base->Class);
                if (buildingtypeext->IsEligibleForAllyBuilding) {
#ifndef NDEBUG
                    //DEV_DEBUG_INFO("Ally \"%s's\" building \"%s\" is eligible for building off.\n", base->House->IniName, base->Name());
#endif
                    passes();
                }
            }
        }
    }

    #undef passes

continue_scan:
    JMP(0x00476308);
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
DECLARE_PATCH(_DisplayClass_Mouse_Left_Release_PlaceAnywhere_BugFix_Patch)
{
    GET_REGISTER_STATIC(DisplayClass *, this_ptr, ebx);

    static Point2D mouse_pos;

    /**
     *  Find out where the mouse cursor is, if its over the sidebar
     *  then invalidate the proximity checks, fixing the glitch.
     */
    mouse_pos.X = WWMouse->Get_Mouse_X();
    mouse_pos.Y = WWMouse->Get_Mouse_Y();
    if (mouse_pos.X >= (TacticalRect.Width-1)) {
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
    JMP(0x00478A30);

    /**
     *  Create PLACE event.
     */
place_it:
    JMP(0x00478990);
}


/**
 *  We can't allocate instance on the stack in inline patches, so this
 *  fetches the mouse coords and assigned them to a global which we can
 *  then use after a call is made to this function without any issues.
 * 
 *  @author: CCHyper
 */
static Cell _tmpcell;
static Coordinate _tmpcoord;
static void Get_Mouse_Cursor_Coords()
{
    _tmpcell = Get_Cell_Under_Mouse();
    _tmpcoord = Get_Coord_Under_Mouse();

    /**
     *  Fixup Z position based on cell height.
     */
    _tmpcoord.Z = Map.Get_Cell_Height(_tmpcoord);
}

/**
 *  Patch to return the mouse coords if the developer option is enabled.
 * 
 *  @see: CursorPositionCommandClass.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_DisplayClass_Help_Text_GetCursorPosition_Patch)
{
    GET_REGISTER_STATIC(DisplayClass *, this_ptr, ebx);
    LEA_STACK_STATIC(Coordinate *, coordinate, esp, 0x2C);
    static char _cursor_position_buffer[128];

    if (Vinifera_Developer_ShowCursorPosition) {

        /**
         *  We need handle this out of this functions stack.
         */
        Get_Mouse_Cursor_Coords();

        /**
         *  Format the buffer with the cell and coord of the
         *  current mouse cursor position.
         */
        std::snprintf(_cursor_position_buffer, sizeof(_cursor_position_buffer),
            " Cell: %d,%d  Coord: %d,%d,%d ",
            _tmpcell.X, _tmpcell.Y, _tmpcoord.X, _tmpcoord.Y, _tmpcoord.Z);
        
        _asm { mov eax, offset _cursor_position_buffer }
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
    JMP(0x0047AFDA);

    /**
     *  Returns TXT_SHADOW.
     */
txt_shadow:
    JMP(0x0047AFC7);

    /**
     *  Function return, expects buffer or string pointer in EAX register.
     */
return_label:
    JMP_REG(ecx, 0x0047AFD1);
}


/**
 *  #issue-71
 *
 *  Replace the old waypoint count in a loop.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_DisplayClass_47A790_Patch)
{
    GET_REGISTER_STATIC(int, i, edi)

        if (i < NEW_WAYPOINT_COUNT)
        {
            JMP(0x0047A7FC);
        }
        else
        {
            JMP(0x0047A85B);
        }
}


/**
 *  Main function for patching the hooks.
 */
void DisplayClassExtension_Hooks()
{
    Patch_Jump(0x0047AFA6, &_DisplayClass_Help_Text_GetCursorPosition_Patch);
    Patch_Jump(0x00478974, &_DisplayClass_Mouse_Left_Release_PlaceAnywhere_BugFix_Patch);
    Patch_Jump(0x004762E4, &_DisplayClass_Passes_Proximity_Passes_Check_Patch);
    
    Patch_Jump(0x004782CA, &_DisplayClass_Mouse_Left_Up_Set_Mouse);

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

    /**
     *  #issue-71
     *
     *  Increases the amount of available waypoints (see ScenarioClassExtension for implementation).
     *
     *  @author: ZivDero
     */
    Patch_Jump(0x0047A856, &_DisplayClass_47A790_Patch);
}
