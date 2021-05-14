/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains various hooks that do not fit elsewhere.
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
#include "vinifera_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "vinifera_util.h"
#include "vinifera_functions.h"
#include "dsurface.h"
#include "wwmouse.h"
#include "vinifera_gitinfo.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Draws the version text on the main menu.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Version_Text_Draw_Patch)
{
    GET_REGISTER_STATIC(XSurface *, surface, ecx);

    Vinifera_Draw_Version_Text(surface);

    _asm { ret }
}


/**
 *  Draws the version text over the loading screen background.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ProgressClass_Load_Screen_Version_Text_Patch)
{
    Vinifera_Draw_Version_Text(HiddenSurface);

    /**
     *  Stolen bytes/code.
     */
original_code:
    _asm { mov eax, [HiddenSurface] }
    _asm { mov edx, [eax] } // Second dereference required due to the global reference in TS++.

    JMP(0x005ADFC4);
}


/**
 *  Draws the version text over the loading screen background.
 * 
 *  @note: This has to be after the New menu initialisation, otherwise the menu
 *         title page will draw over the text.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Game_Loading_Screen_Version_Text_Patch)
{
    /**
     *  Flag as pre-init, as we need to draw this differently.
     */
    Vinifera_Draw_Version_Text(PrimarySurface, true);

    /**
     *  Stolen bytes/code.
     */
original_code:
    Call_Back();

    JMP(0x004E0852);
}


/**
 *  Draws the version text over the menu background.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Load_Title_Page_Version_Text_Patch)
{
    Vinifera_Draw_Version_Text(HiddenSurface, true);

    _asm { ret }
}


/**
 *  Patch in the Vinifera command line parser.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WinMain_Parse_Command_Line)
{
    GET_REGISTER_STATIC(int, argc, edi);
    static char **argv; _asm { lea eax, [ebp-0x178] } _asm { mov argv, eax }

    /**
     *  Parse_Command_Line could return 
     */
    if (!Parse_Command_Line(argc, argv) || !Vinifera_Parse_Command_Line(argc, argv)) {
        JMP(0x00601A3B); // Failure.
    } else {
        JMP(0x00601085);
    }
}


/**
 *  Patch in the main Vinifera startup function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WinMain_Vinifera_Startup)
{
    if (Vinifera_Startup()) {
        JMP(0x005FFC41);
    }

    /**
     *  Something went wrong!
     */

    DEBUG_ERROR("Failed to initialise Vinifera systems!\n");

    _asm { mov esi, EXIT_FAILURE }
    JMP(0x00601A6B);
}


void Vinifera_Hooks()
{
    /**
     *  Draw the build version info on the bottom on the screen.
     */
    Patch_Jump(0x004E53C0, &_Version_Text_Draw_Patch);
    Patch_Jump(0x005ADFBE, &_ProgressClass_Load_Screen_Version_Text_Patch);
    Patch_Jump(0x004E084D, &_Init_Game_Loading_Screen_Version_Text_Patch);
    Patch_Jump(0x004E3B7A, &_Load_Title_Page_Version_Text_Patch);

    /**
     *  Add in Vinifera startup hooks.
     */
    Patch_Jump(0x00601070, &_WinMain_Parse_Command_Line);
    Patch_Jump(0x005FF81C, &_WinMain_Vinifera_Startup);
}
