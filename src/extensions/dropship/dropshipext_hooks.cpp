/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended Dropship loadout.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "dropshipext_hooks.h"

#include "armortype.h"
#include "colorscheme.h"
#include "dropship.h"
#include "dsurface.h"
#include "gscreen.h"
#include "hooker.h"
#include "syringe.h"
#include "textprint.h"
#include "theme.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "wwmouse.h"


/**
 *  #issue-107
 *
 *  Patches the Dropship screen to display new armor types' names.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0048706A, _Dropship_Draw_Info_Text_ArmorName_Patch, 0)
{
    GET(ArmorType, armor, EDX);
    GET(char*, dest, ECX)

    std::sprintf(dest, "Armor: %s", ArmorTypeClass::Name_From(armor));

    R->ESP(R->ESP() - 0xC); // Fix up the stack from the removed printf call.
    return 0x0048707D;
}


/**
 *  #issue-262
 * 
 *  In certain cases, the mouse might not be shown on the Dropship Loadout menu.
 *  This patch fixes that by showing the mouse regardless of its current state.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005DB3BB, _Start_Scenario_Dropship_Loadout_Show_Mouse_Patch, 0)
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

    MouseCursor->Release_Mouse();
    Show_Mouse();

    Dropship_Loadout();

    Hide_Mouse();
    MouseCursor->Capture_Mouse();

    if (Theme.Still_Playing()) {
        Theme.Stop(true); // Smoothly fade out the track.
    }

    return 0x005DB3C0;
}


/**
 *  #issue-285
 * 
 *  Draws help text on the dropship loadout menu.
 * 
 *  @author: CCHyper
 */
static void Draw_Dropship_Loadout_Help_Text(Surface *surface)
{
    #define TEXT_PRESS_SPACE "Press SPACE to start the mission"

    if (!surface) {
        return;
    }

    Rect surfrect = surface->Get_Rect();

    TextPrintType style = (TPF_CENTER|TPF_FULLSHADOW|TPF_6PT_GRAD);
    ColorScheme *color_white = Fetch_Scheme_By_Name("White");
    ColorType back_color = COLOR_TBLACK;

    Point2D text_pos;
    text_pos.X = surfrect.Width/2;
    text_pos.Y = (surfrect.Height/2)+185;

    Fancy_Text_Print(TEXT_PRESS_SPACE, *surface, surfrect, text_pos, color_white, back_color, style);
}

DEFINE_HOOK(0x004868FB, _Dropship_Loadout_Help_Text_Patch, 6)
{
    Draw_Dropship_Loadout_Help_Text(HiddenSurface);

    /**
     *  Draws the version text over the menu background.
     */
    Vinifera_Draw_Version_Text(HiddenSurface);

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void DropshipExtension_Hooks()
{

}
