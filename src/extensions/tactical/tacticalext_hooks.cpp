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
#include "voc.h"
#include "laserdraw.h"
#include "ebolt.h"
#include  "house.h"
#include "buildingtype.h"
#include "unittype.h"
#include "unit.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "extension_globals.h"
#include "optionsext.h"
#include "session.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <timeapi.h>

#include "hooker.h"
#include "hooker_macros.h"


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
    TempSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);
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
    TempSurface->entry_48(*start_pos, *end_pos, color_black, _pattern, offset, blit);

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    TempSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

    --start_pos->Y;
    --end_pos->Y;
    TempSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

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
    TempSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);
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
    TempSurface->entry_48(*start_pos, *end_pos, color_black, _pattern, offset, blit);

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    TempSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

    --start_pos->Y;
    --end_pos->Y;
    TempSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);

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
    TempSurface->entry_4C(*start_pos, *end_pos, color_black);

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    TempSurface->entry_4C(*start_pos, *end_pos, color);

    --start_pos->Y;
    --end_pos->Y;
    TempSurface->entry_4C(*start_pos, *end_pos, color);

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

#ifndef RELEASE
    /**
     *  Draw the version number on screen for non-release builds.
     *
     *  @note: This must be last in the draw order!
     */
    Vinifera_Draw_Version_Text(CompositeSurface);
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
     *  Stolen bytes/code.
     */
original_code:
    this_ptr->Draw_Screen_Text(this_ptr->ScreenText);

    this_ptr->field_D30 = false;
    this_ptr->IsToRedraw = false;

    JMP(0x00611BE4);
}


/**
 *  #issue-966
 *
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
 *  Helper function.
 *  Checks whether a specific object should be filtered
 *  out from selection if the selection includes combatants.
 */
bool Should_Exclude_From_Selection(ObjectClass* obj)
{
    /**
     *  Don't exclude objects that we don't own.
     */
    if (obj->Owning_House() != nullptr && !obj->Owning_House()->IsPlayerControl) {
        return false;
    }

    if (obj->What_Am_I() == RTTI_UNIT) {
        UnitTypeClass* unittype = reinterpret_cast<UnitClass*>(obj)->Class;

        /**
         *  Exclude units that harvest either Tiberium or Tiberium weeds,
         *  and units that can deploy into factories.
         *
         *  However, never exclude units that have a weapon! (DTA's Enforcer
         *  can deploy into a Barracks, yet still has a weapon)
         */
        if (unittype->IsToHarvest || unittype->IsToVeinHarvest ||
            (unittype->DeploysInto != nullptr && unittype->DeploysInto->ToBuild != RTTI_NONE))
        {
            if (unittype->Fetch_Weapon_Info(WEAPON_SLOT_PRIMARY).Weapon == nullptr) {
                return true;
            }
        }
    }

    return false;
}


void Filter_Selection()
{
    if (!OptionsExtension->FilterBandBoxSelection) {
        return;
    }

    /**
     *  This is a bit tricky. Optimally, when Shift is hold, the game should
     *  decide what to do depending on what units were already selected
     *  prior to the selection.
     *
     *  Scenario 1: the player has selected only combatant units. In this case,
     *  shift-selection should exclude non-combatants.
     *
     *  Scenario 2: the player has selected one or more non-combatant units,
     *  exlusively or in addition to combatants. In this case, shift-selection
     *  should include non-combatants as it's already a mixed group.
     *
     *  However, the C&C RC implementation only filters out units AFTER they
     *  have been selected. Meaning we don't know what units were selected prior
     *  to the selection. Knowing that would require a somewhat more complex hack,
     *  with either a reimplementation of the selection algorithm or hooks at the
     *  beginning and at the end of it.
     *
     *  The best we can do for now is not to filter if the user is holding Shift.
     *  Actually looks like this is what the C&C RC also does.
     */
    if (WWKeyboard->Down(KN_LSHIFT)) {
        return;
    }

    bool any_to_exclude = false;
    bool all_to_exclude = true;

    for (int i = 0; i < CurrentObjects.Count(); i++) {
        bool exclude = Should_Exclude_From_Selection(CurrentObjects[i]);
        any_to_exclude |= exclude;
        all_to_exclude &= exclude;
    }

    if (any_to_exclude && !all_to_exclude) {
        for (int i = 0; i < CurrentObjects.Count(); i++) {
            if (Should_Exclude_From_Selection(CurrentObjects[i])) {

                /**
                 *  Petroglyph added a count check here.
                 *  Maybe they had trouble with the feature?
                 *  Let's also add it just to be sure we won't end up
                 *  in an infinite loop.
                 */
                int count_before = CurrentObjects.Count();
                CurrentObjects[i]->Unselect();
                int count_after = CurrentObjects.Count();
                if (count_after < count_before) {
                    i--;
                }
            }
        }
    }
}


/**
 *  #issue-825
 *
 *  Excludes harvesters and deployable factories from band-box selection
 *  when the selection also includes combatant units.
 *
 *  Ported over from Red Alert Remastered source code.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_Tactical_Select_These_Unselect_NonCombatants_Patch)
{
    Filter_Selection();

    /**
     *  Stolen bytes / code.
     */
    AllowVoice = true;
    JMP_REG(ecx, 0x00616A7A);
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

    // Rampastring: DTA players have seen occasional crashes when the game draws rally point lines.
    // Disable this for now to see whether the problem goes away.
    // Patch_Jump(0x00616E9A, &_Tactical_Draw_Rally_Points_NormaliseLineAnimation_Patch);
    Patch_Jump(0x006172DB, &_Tactical_Draw_Waypoint_Paths_NormaliseLineAnimation_Patch);
    Patch_Jump(0x00617327, &_Tactical_Draw_Waypoint_Paths_DrawNormalLine_Patch);

    Patch_Jump(0x00616D0F, &_Tactical_Draw_Rally_Points_Draw_For_Service_Depots);

    Patch_Jump(0x00616A73, &_Tactical_Select_These_Unselect_NonCombatants_Patch);

    /**
     *  #issue-351
     * 
     *  Changes the waypoint number text to have a stroke/outline.
     * 
     *  @authors: CCHyper
     */
    Patch_Dword(0x006171C8+1, (TPF_CENTER|TPF_EFNT|TPF_FULLSHADOW));
    Patch_Jump(0x00616FDA, &_Tactical_Draw_Waypoint_Paths_Text_Color_Patch);
}
