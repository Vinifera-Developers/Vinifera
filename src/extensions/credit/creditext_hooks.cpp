/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CREDITEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended CreditClass.
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

#include "tibsun_globals.h"

#include "colorscheme.h"
#include "dsurface.h"
#include "extension_globals.h"
#include "house.h"
#include "housetype.h"
#include "rgb.h"
#include "scenarioext.h"
#include "sideext.h"


#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Modifies the color of the "Options" text based on the player's side.
 */
DECLARE_PATCH(_TabClass_Draw_It_Faction_Specific_Options_Button_Color_Scheme_Patch)
{
    static ColorSchemeType colorschemetype;
    static ColorScheme* colorscheme;

    colorschemetype = Extension::Fetch<SideClassExtension>(Sides[PlayerPtr->Class->Side])->UIColor;
    colorscheme = ColorSchemes[colorschemetype];

    _asm mov edx, colorscheme 
    _asm mov ecx, LogicSurface
    _asm mov ecx, [ecx]
    JMP(0x0060E5B4);
}


/**
 *  Modifies the color of the credits display based on the player's side.
 */
DECLARE_PATCH(_CreditClass_Graphic_Logic_Faction_Specific_Color_Scheme_Patch)
{
    _asm push ecx
    _asm push 4108h
    _asm push 0

    static ColorSchemeType colorschemetype;
    static ColorScheme* colorscheme;

    colorschemetype = Extension::Fetch<SideClassExtension>(Sides[PlayerPtr->Class->Side])->UIColor;
    colorscheme = ColorSchemes[colorschemetype];

    _asm mov eax, colorscheme
    JMP_REG(ecx, 0x004714F0);
}


void Draw_Tooltip_Rectangle(DSurface* surface, Rect& drawrect)
{
    surface->Fill_Rect(drawrect, 0);

    const ColorSchemeType colorschemetype = Extension::Fetch<SideClassExtension>(Sides[PlayerPtr->Class->Side])->ToolTipColor;
    const ColorScheme* colorscheme = ColorSchemes[colorschemetype];

    RGBClass rgb = colorscheme->HSV.operator RGBClass();
    surface->Draw_Rect(drawrect, DSurface::RGB_To_Pixel(rgb));
}


DECLARE_PATCH(_CCToolTip_Draw_Faction_Specific_Color_Scheme_Rect_Patch)
{
    GET_REGISTER_STATIC(DSurface*, surface, esi);
    GET_REGISTER_STATIC(Rect*, drawrect, eax);

    Draw_Tooltip_Rectangle(surface, *drawrect);
    
    JMP(0x0044E6D4);
}


DECLARE_PATCH(_CCToolTip_Draw_Faction_Specific_Color_Scheme_Text_Patch)
{
    static ColorSchemeType colorschemetype;
    static ColorScheme* colorscheme;

    colorschemetype = Extension::Fetch<SideClassExtension>(Sides[PlayerPtr->Class->Side])->ToolTipColor;
    colorscheme = ColorSchemes[colorschemetype];

    _asm mov eax, colorscheme
    JMP_REG(ecx, 0x0044E6F8);
}


/**
 *  Main function for patching the hooks.
 */
void CreditClassExtension_Hooks()
{
    Patch_Jump(0x0060E5AE, &_TabClass_Draw_It_Faction_Specific_Options_Button_Color_Scheme_Patch);
    Patch_Jump(0x004714E6, &_CreditClass_Graphic_Logic_Faction_Specific_Color_Scheme_Patch);

    Patch_Jump(0x0044E682, &_CCToolTip_Draw_Faction_Specific_Color_Scheme_Rect_Patch);
    Patch_Jump(0x0044E6F3, &_CCToolTip_Draw_Faction_Specific_Color_Scheme_Text_Patch);
}