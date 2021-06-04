/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTICALEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended Tactical class.
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
#include "tacticalext_hooks.h"
#include "tacticalext_init.h"
#include "tacticalext.h"
#include "tactical.h"
#include "dsurface.h"
#include "textprint.h"
#include "wwfont.h"
#include "scenario.h"
#include "colorscheme.h"
#include "fatal.h"
#include "vinifera_util.h"
#include "vinifera_gitinfo.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Draws the developer mode overlay.
 * 
 *  @authors: CCHyper
 */
static void Tactical_Draw_Debug_Overlay()
{
    RGBClass rgb_black(0,0,0);
    unsigned color_black = DSurface::RGBA_To_Pixel(0, 0, 0);
    ColorScheme *text_color = ColorScheme::As_Pointer("White");

    int padding = 2;

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "[%s]", strupr(Scen->ScenarioName));

    /**
     * Fetch the text occupy area.
     */
    Rect scenario_text_rect;
    GradFont6Ptr->String_Pixel_Rect(buffer, &scenario_text_rect);

    /**
     *  Fill the background area.
     */
    Rect fill_rect;
    fill_rect.X = 160; // Width of Options tab, so we draw from there.
    fill_rect.Y = 0;
    fill_rect.Width = scenario_text_rect.Width+(padding+1);
    fill_rect.Height = 16; // Tab bar height
    CompositeSurface->Fill_Rect(fill_rect, color_black);

    /**
     *  Move rects into position.
     */
    scenario_text_rect.X = fill_rect.X+padding;
    scenario_text_rect.Y = 0;
    scenario_text_rect.Width += padding;
    scenario_text_rect.Height += 3;

    /**
     *  Draw the scenario name.
     */
    Fancy_Text_Print(buffer, CompositeSurface, &CompositeSurface->Get_Rect(),
        &Point2D(scenario_text_rect.X, scenario_text_rect.Y), text_color, COLOR_TBLACK, TextPrintType(TPF_6PT_GRAD|TPF_NOSHADOW));
}


/**
 *  This patch intercepts the end of the rendering process for Tactical.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Render_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, ebp);

    /**
     *  If the developer mode is active, draw the developer overlay.
     */
    if (Vinifera_DeveloperMode) {
        Tactical_Draw_Debug_Overlay();
    }

#ifndef RELEASE
    /**
     *  Draw the version number on screen for non-release builds.
     * 
     *  @note: This must be last in the draw order!
     */
    Vinifera_Draw_Version_Text(CompositeSurface);
#endif

    /**
     *  Stolen bytes/code.
     */
original_code:
    this_ptr->Draw_Screen_Text(this_ptr->ScreenText);

    this_ptr->field_D30 = false;
    this_ptr->IsToRedraw = false;

    JMP(0x00611BE4);
}


/**
 *  Main function for patching the hooks.
 */
void TacticalExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TacticalExtension_Init();

    Patch_Jump(0x00611BCB, &_Tactical_Render_Patch);
}
