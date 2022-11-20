/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DROPSHIPEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended Dropship loadout.
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
#include "dropshipext_hooks.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "dropship.h"
#include "dsurface.h"
#include "gscreen.h"
#include "theme.h"
#include "colorscheme.h"
#include "textprint.h"
#include "armortype.h"
#include "fatal.h"
#include "wwmouse.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-107
 * 
 *  x
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Dropship_Draw_Info_Text_ArmorName_Patch)
{
    GET_REGISTER_STATIC(ArmorType, armor, edx);
    static const char* armor_name;

    armor_name = ArmorTypeClass::Name_From(armor);
    _asm { mov eax, armor_name }

    JMP_REG(edx, 0x00487071);
}


/**
 *  #issue-262
 * 
 *  In certain cases, the mouse might not be shown on the Dropship Loadout menu.
 *  This patch fixes that by showing the mouse regardless of its current state.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Start_Scenario_Dropship_Loadout_Show_Mouse_Patch)
{
    /**
     *  issue-284
     * 
     *  Play a background theme during the loadout menu.
     * 
     *  @author: CCHyper
     */
    if (!Theme.Still_Playing()) {

        /**
         *  If DSHPLOAD is defined in THEME.INI, play that, otherwise default
         *  to playing the TS Maps theme.
         */
        ThemeType theme = Theme.From_Name("DSHPLOAD");
        if (theme == THEME_NONE) {
            theme = Theme.From_Name("MAPS");
        }

        Theme.Play_Song(theme);
    }

    WWMouse->Release_Mouse();
    WWMouse->Show_Mouse();

    Dropship_Loadout();

    WWMouse->Hide_Mouse();
    WWMouse->Capture_Mouse();

    if (Theme.Still_Playing()) {
        Theme.Stop(true); // Smoothly fade out the track.
    }

    JMP(0x005DB3C0);
}


/**
 *  #issue-285
 * 
 *  Draws help text on the dropship loadout menu.
 * 
 *  @author: CCHyper
 */
static void Draw_Dropship_Loadout_Help_Text(XSurface *surface)
{
    #define TEXT_PRESS_SPACE "Press SPACE to start the mission"

    if (!surface) {
        return;
    }

    Rect surfrect = surface->Get_Rect();

    TextPrintType style = (TPF_CENTER|TPF_FULLSHADOW|TPF_6PT_GRAD);
    ColorScheme *color_white = ColorScheme::As_Pointer("White");
    ColorType back_color = COLOR_TBLACK;

    Point2D text_pos;
    text_pos.X = surfrect.Width/2;
    text_pos.Y = (surfrect.Height/2)+185;

    Fancy_Text_Print(TEXT_PRESS_SPACE, surface, &surfrect, &text_pos, color_white, back_color, style);
}

DECLARE_PATCH(_Dropship_Loadout_Help_Text_Patch)
{
    Draw_Dropship_Loadout_Help_Text(HiddenSurface);

    /**
     *  Draws the version text over the menu background.
     */
    Vinifera_Draw_Version_Text(HiddenSurface);

    /**
     *  Stolen bytes/code.
     */
original_code:
    GScreenClass::Blit(true, HiddenSurface);

    _asm { mov ebx, Scen }
    _asm { mov ebx, [ebx] } // Second dereference required due to the global reference in TS++.

    JMP(0x00486910);
}


/**
 *  Main function for patching the hooks.
 */
void DropshipExtension_Hooks()
{
    Patch_Jump(0x004868FB, &_Dropship_Loadout_Help_Text_Patch);
    Patch_Jump(0x005DB3BB, &_Start_Scenario_Dropship_Loadout_Show_Mouse_Patch);
    Patch_Jump(0x0048706A, &_Dropship_Draw_Info_Text_ArmorName_Patch);
}
