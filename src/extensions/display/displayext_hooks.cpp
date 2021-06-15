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
#include "wwmouse.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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
    LEA_STACK_STATIC(Cell *, cell, esp, 0x2C);
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
    if (Map[*cell].IsVisible && MainWindow) {
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
 *  Main function for patching the hooks.
 */
void DisplayClassExtension_Hooks()
{
    Patch_Jump(0x0047AFA6, &_DisplayClass_Help_Text_GetCursorPosition_Patch);
    Patch_Jump(0x00478974, &_DisplayClass_Mouse_Left_Release_PlaceAnywhere_BugFix_Patch);

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
}
