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
#include "mouse.h"
#include "tibsun_globals.h"
#include "scenario.h"
#include "convert.h"
#include "voc.h"
#include "laserdraw.h"
#include "ebolt.h"
#include "buildingtype.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "extension_globals.h"
#include "rules.h"
#include "rulesext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <timeapi.h>

#include "hooker.h"
#include "hooker_macros.h"
#include "uicontrol.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class TacticalExt : public Tactical
{
public:
    void _Draw_Band_Box();
};


/**
 *  Reimplements Tactical::Draw_Band_Box.
 *  
 *  @author: CCHyper, ZivDero
 */
void TacticalExt::_Draw_Band_Box()
{
    if (Band.X || Band.Y)
    {
        int x = Band.X;
        int y = Band.Y;
        int width = Band.Width;
        int height = Band.Height;

        if (width < x) {
            std::swap(width, x);
        }

        if (height < y) {
            std::swap(height, y);
        }

        Rect band_rect(x, y, width - x + 1, height - y + 1);

        /**
         *  Is the map ambient dark? If so, we adjust the colour slightly.
         */
        if (UIControls->BandBoxTintTransparency > 0) {

            Rect tint_rect = band_rect;
            const unsigned trans = UIControls->BandBoxTintTransparency;

            /**
             *  Draw the rubber band tint rect.
             *
             *  Fill_Rect_Trans() doesn't not take a relative rect, so we need
             *  to need to align it with the TacticalRect manually.
             */
            tint_rect.Move(TacticalRect.X, TacticalRect.Y);

            RGBClass tint_dark = UIControls->BandBoxTintColors[0];
            RGBClass tint_light = UIControls->BandBoxTintColors[1];

            /**
             *  Interpolate between the two colors to find the correct tint
             *  for the current map ambient level.
             */
            const float adjust = static_cast<float>(Scen->AmbientCurrent) / 100.0f;
            const RGBClass tint_color = RGBClass::Interpolate(tint_dark, tint_light, adjust);

            LogicSurface->Fill_Rect_Trans(tint_rect, tint_color, trans);
        }

        /**
         *  Draw the drop shadow.
         */
        if (UIControls->IsBandBoxDropShadow) {

            Rect drop_rect = band_rect;
            drop_rect.X += 1;
            drop_rect.Y += 1;

            const unsigned drop_color = DSurface::RGB_To_Pixel(
                UIControls->BandBoxDropShadowColor.R,
                UIControls->BandBoxDropShadowColor.G,
                UIControls->BandBoxDropShadowColor.B);

            /**
             *  Draw the band box.
             */
            if (UIControls->IsBandBoxThick) {

                drop_rect.X += 1;
                drop_rect.Y += 1;

                LogicSurface->Draw_Rect(TacticalRect, drop_rect, drop_color);

                Rect thick_rect = drop_rect;
                thick_rect.X += 1;
                thick_rect.Y += 1;
                thick_rect.Width -= 2;
                thick_rect.Height -= 2;

                LogicSurface->Draw_Rect(TacticalRect, thick_rect, drop_color);

            }
            else {
                LogicSurface->Draw_Rect(TacticalRect, drop_rect, drop_color);
            }

        }

        /**
         *  Draw the custom rubber band rect.
         */
        const unsigned band_color = DSurface::RGB_To_Pixel(
            UIControls->BandBoxColor.R,
            UIControls->BandBoxColor.G,
            UIControls->BandBoxColor.B);

        LogicSurface->Draw_Rect(TacticalRect, band_rect, band_color);

        /**
         *  If the band box is thick, draw an extra outline.
         */
        if (UIControls->IsBandBoxThick) {
            Rect thick_rect = band_rect;
            thick_rect.X += 1;
            thick_rect.Y += 1;
            thick_rect.Width -= 2;
            thick_rect.Height -= 2;
            LogicSurface->Draw_Rect(TacticalRect, thick_rect, band_color);
        }
    }
}


/**
 *  #issue-315
 * 
 *  Set the waypoint number text for all theaters to be "White" (14).
 *  TEMPERATE was "White" (14) and SNOW was "Black" (12).
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Draw_Waypoint_Paths_Text_Color_Patch)
{
    _asm { mov eax, 14 }

    JMP_REG(ecx, 0x00616FEB);
}


/**
 *  #issue-348
 * 
 *  The animation speed of Waypoint lines is not normalised and subjective to
 *  the game speed setting. This patch adjusts the animation using the system
 *  timer and makes the animation speed consistent across all game speeds.
 * 
 *  @authors: CCHyper
 */
DECLARE_PATCH(_Tactical_Draw_Rally_Points_NormaliseLineAnimation_Patch)
{
    GET_STACK_STATIC8(bool, blit, esp, 0x70);
    LEA_STACK_STATIC(Point2D *, start_pos, esp, 0x1C);
    LEA_STACK_STATIC(Point2D *, end_pos, esp, 0x14);

    /**
     *  5 pixels on, 3 off, 5 pixels on, 3 off.
     */
    static bool _pattern[16] = { true, true, true, true, true, false, false, false, true, true, true, true, true, false, false, false };
    
    static int time;
    static int offset;
    static unsigned color;
    static unsigned color_black;

    /**
     *  Adjust the offset of the line pattern.
     */
    time = timeGetTime();
    offset = (-time / 32) & (ARRAYSIZE(_pattern)-1);

    color = DSurface::RGB_To_Pixel(0,255,0);
    color_black = DSurface::RGB_To_Pixel(0,0,0);

#if 0
    /**
     *  Draw the line line with the desired pattern.
     */
    LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);
#endif

    /**
     *  #issue-351
     * 
     *  Thicken the rally point lines so they are easier to see in contrast to the terrain.
     * 
     *  @authors: CCHyper
     */

    /**
     *  Draw the drop shadow line.
     */
    start_pos->Y += 2;
    end_pos->Y += 2;
    LogicSurface->entry_48(*start_pos, *end_pos, color_black, _pattern, offset, blit);

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

    --start_pos->Y;
    --end_pos->Y;
    LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

    JMP(0x00616EFD);
}


/**
 *  #issue-348
 * 
 *  The animation speed of Rally Point lines is not normalised and subjective to
 *  the game speed setting. This patch adjusts the animation using the system
 *  timer and makes the animation speed consistent across all game speeds.
 * 
 *  @authors: CCHyper
 */
DECLARE_PATCH(_Tactical_Draw_Waypoint_Paths_NormaliseLineAnimation_Patch)
{
    GET_REGISTER_STATIC(unsigned, color, eax);
    GET_STACK_STATIC8(bool, blit, esp, 0x90);
    LEA_STACK_STATIC(Point2D *, start_pos, esp, 0x34);
    LEA_STACK_STATIC(Point2D *, end_pos, esp, 0x3C);

    /**
     *  5 pixels on, 3 off, 5 pixels on, 3 off.
     */
    static bool _pattern[16] = { true, true, true, true, true, false, false, false, true, true, true, true, true, false, false, false };

    static int time;
    static int offset;
    static unsigned color_black;

    /**
     *  Adjust the offset of the line pattern (this animates a little slower than rally points).
     */
    time = timeGetTime();
    offset = (-time / 64) & (ARRAYSIZE(_pattern)-1);

    color_black = DSurface::RGB_To_Pixel(0,0,0);

#if 0
    /**
     *  Draw the line line with the desired pattern.
     */
    LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);
#endif

    /**
     *  #issue-351
     * 
     *  Thicken the waypoint path lines so they are easier to see in contrast to the terrain.
     * 
     *  @authors: CCHyper
     */

    /**
     *  Draw the drop shadow line.
     */
    start_pos->Y += 2;
    end_pos->Y += 2;
    LogicSurface->entry_48(*start_pos, *end_pos, color_black, _pattern, offset, blit);

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

    --start_pos->Y;
    --end_pos->Y;
    LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

    JMP(0x00617307);
}


/**
 *  #issue-351
 * 
 *  Thicken the waypoint path lines so they are easier to see in contrast to the terrain.
 * 
 *  @authors: CCHyper
 */
DECLARE_PATCH(_Tactical_Draw_Waypoint_Paths_DrawNormalLine_Patch)
{
    GET_REGISTER_STATIC(unsigned, color, eax);
    GET_STACK_STATIC8(bool, blit, esp, 0x90);
    LEA_STACK_STATIC(Point2D *, start_pos, esp, 0x34);
    LEA_STACK_STATIC(Point2D *, end_pos, esp, 0x3C);

    static unsigned color_black;

    color_black = DSurface::RGB_To_Pixel(0,0,0);

    /**
     *  Draw the drop shadow line.
     */
    start_pos->Y += 2;
    end_pos->Y += 2;
    LogicSurface->entry_4C(*start_pos, *end_pos, color_black);

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    LogicSurface->entry_4C(*start_pos, *end_pos, color);

    --start_pos->Y;
    --end_pos->Y;
    LogicSurface->entry_4C(*start_pos, *end_pos, color);

    JMP(0x00617307);
}


/**
 *  This patch intercepts the post effects rendering process for Tactical
 *  allowing us to draw any new effects/systems.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Render_Post_Effects_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, ebp);

    /**
     *  Stolen bytes/code.
     */
    LaserDrawClass::Draw_All();

    /**
     *  Draw any new post effects here.
     */
    TacticalMapExtension->Render_Post();

    JMP(0x00611AFE);
}


/**
 *  This patch intercepts the end of the rendering process for Tactical
 *  for drawing any overlay or graphics.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Render_Overlay_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, ebp);

    /**
     *  If the developer mode is active, draw the developer overlay.
     */
    if (Vinifera_DeveloperMode) {

        TacticalMapExtension->Draw_Debug_Overlay();

        if (Vinifera_Developer_FrameStep) {
            TacticalMapExtension->Draw_FrameStep_Overlay();
        }
    }

#ifndef NDEBUG
    /**
     *  Various developer only debugging.
     */
    //Tactical_Debug_Draw_Facings();
#endif

    /**
     *  Has custom screen text been set?
     */
    if (TacticalMapExtension->IsInfoTextSet) {

        /**
         *  Draw it to the screen.
         */
        TacticalMapExtension->Draw_Information_Text();
        
        /**
         *  Play the one time notification sound if defined.
         */
        if (TacticalMapExtension->InfoTextNotifySound != VOC_NONE) {
            Sound_Effect(TacticalMapExtension->InfoTextNotifySound, TacticalMapExtension->InfoTextNotifySoundVolume);
            TacticalMapExtension->InfoTextNotifySound = VOC_NONE;
        }
        
        /**
         *  If the screen timer has expired, disable drawing.
         */
        if (TacticalMapExtension->InfoTextTimer.Expired()) {
            TacticalMapExtension->InfoTextTimer.Stop();
            TacticalMapExtension->IsInfoTextSet = false;
            std::memset(TacticalMapExtension->InfoTextBuffer, 0, sizeof(TacticalMapExtension->InfoTextBuffer));
            TacticalMapExtension->InfoTextNotifySound = VOC_NONE;
            TacticalMapExtension->InfoTextPosition = TOP_LEFT;
        }       
    }

    /**
     *  Draw the version number on screen.
     * 
     *  @note: This must be last in the draw order!
     */
    TacticalMapExtension->Draw_Version_Number_Text();

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
 *  Enables drawing of rally points for Service Depots.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_Tactical_Draw_Rally_Points_Draw_For_Service_Depots)
{
    GET_REGISTER_STATIC(BuildingTypeClass*, buildingtype, ecx);
    static RTTIType tobuild;

    tobuild = buildingtype->ToBuild;
    if (tobuild == RTTI_UNITTYPE || tobuild == RTTI_INFANTRYTYPE || tobuild == RTTI_AIRCRAFTTYPE)
        goto draw_rally_point;

    if (buildingtype->CanUnitRepair)
        goto draw_rally_point;

    /**
     *  This building is not eligible for having a rally point,
     *  skip the drawing process.
     */
no_rally_point:
    JMP(0x00616EFD);

    /**
     *  Draw the potential rally point of the building.
     */
draw_rally_point:
    JMP(0x00616D28);
}


/**
 *  #issue-1050
 *
 *  Fixes a bug where the camera keeps following a followed object
 *  when a trigger or script tells it to center on a waypoint
 *  or a team.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_Tactical_Center_On_Location_Unfollow_Object_Patch)
{
    Map.Follow_This(nullptr);

    // Rebuild function epilogue
    _asm { pop  edi }
    _asm { pop  esi }
    _asm { pop  ebp }
    _asm { pop  ebx }
    _asm { add  esp, 8 }
    _asm { retn 8 }
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

    Patch_Jump(0x00611AF9, &_Tactical_Render_Post_Effects_Patch);
    Patch_Jump(0x00611BCB, &_Tactical_Render_Overlay_Patch);

    Patch_Jump(0x00616E9A, &_Tactical_Draw_Rally_Points_NormaliseLineAnimation_Patch);
    Patch_Jump(0x006172DB, &_Tactical_Draw_Waypoint_Paths_NormaliseLineAnimation_Patch);
    Patch_Jump(0x00617327, &_Tactical_Draw_Waypoint_Paths_DrawNormalLine_Patch);

    Patch_Jump(0x00616D0F, &_Tactical_Draw_Rally_Points_Draw_For_Service_Depots);

    Patch_Jump(0x0060F953, &_Tactical_Center_On_Location_Unfollow_Object_Patch);

    /**
     *  #issue-351
     * 
     *  Changes the waypoint number text to have a stroke/outline.
     * 
     *  @authors: CCHyper
     */
    Patch_Dword(0x006171C8+1, (TPF_CENTER|TPF_EFNT|TPF_FULLSHADOW));
    Patch_Jump(0x00616FDA, &_Tactical_Draw_Waypoint_Paths_Text_Color_Patch);
    Patch_Jump(0x00616560, &TacticalExt::_Draw_Band_Box);
}
