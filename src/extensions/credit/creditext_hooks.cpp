/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended CreditClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "colorscheme.h"
#include "dsurface.h"
#include "hooker.h"
#include "house.h"
#include "housetype.h"
#include "rgb.h"
#include "scenarioext.h"
#include "sideext.h"
#include "syringe.h"
#include "tibsun_globals.h"


/**
 *  Modifies the color of the "Options" text based on the player's side.
 *
 *  @author: Rampastring, ZivDero
 */
DEFINE_HOOK(0x0060E5AE, _TabClass_Draw_It_Faction_Specific_Options_Button_Color_Scheme_Patch, 6)
{
    ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;
    ColorScheme* colorscheme = ColorSchemes[colorschemetype];
    R->EDX(colorscheme);

    return 0;
}


/**
 *  Modifies the color of the credits display based on the player's side.
 *
 *  @author: Rampastring, ZivDero
 */
DEFINE_HOOK(0x004714E6, _CreditClass_Graphic_Logic_Faction_Specific_Color_Scheme_Patch, 8)
{
    ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;
    ColorScheme* colorscheme = ColorSchemes[colorschemetype];
    R->EAX(colorscheme);

    return 0;
}


/**
 *  Draws the tooltip rectangle using a color based on the player's side.
 *
 *  @author: Rampastring, ZivDero
 */
void Draw_Tooltip_Rectangle(Surface* surface, Rect& drawrect)
{
    surface->Fill_Rect(drawrect, 0);

    const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->ToolTipColor;
    const ColorScheme* colorscheme = ColorSchemes[colorschemetype];

    RGBClass rgb = colorscheme->HSV.operator RGBClass();
    surface->Draw_Rect(drawrect, DSurface::Build_Hicolor_Pixel(rgb));
}


/**
 *  Patch to replace the call to draw the tooptip rectangle.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x0044E682, _CCToolTip_Draw_Faction_Specific_Color_Scheme_Rect_Patch, 0)
{
    GET(Surface*, surface, ESI);
    GET(Rect*, drawrect, EAX);

    Draw_Tooltip_Rectangle(surface, *drawrect);
    
    return 0x0044E6D4;
}


/**
 *  Modifies the color of the tooltip text color based on the player's side.
 *
 *  @author: Rampastring, ZivDero
 */
DEFINE_HOOK(0x0044E6F3, _CCToolTip_Draw_Faction_Specific_Color_Scheme_Text_Patch, 0)
{
    ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->ToolTipColor;
    ColorScheme* colorscheme = ColorSchemes[colorschemetype];
    R->EAX(colorscheme);

    return 0x0044E6F8;
}


/**
 *  Main function for patching the hooks.
 */
void CreditClassExtension_Hooks()
{
    
}
