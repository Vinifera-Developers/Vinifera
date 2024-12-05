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
#include "optionsext.h"
#include "object.h"
#include "house.h"
#include "technotype.h"
#include "building.h"
#include "buildingtype.h"

#include <timeapi.h>

#include "clipline.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "technotypeext.h"
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
    void _Select_These(Rect& rect, void (*selection_func)(ObjectClass* obj));
    void _Draw_Rally_Points(bool blit);


public:

    /**
     *  Static variables for selection filtering, need to be static
     *  so that the selection predicate can use them.
     */
    static bool SelectionContainsNonCombatants;
    static int SelectedCount;
    static bool FilterSelection;
};

bool TacticalExt::SelectionContainsNonCombatants = false;
int TacticalExt::SelectedCount = 0;
bool TacticalExt::FilterSelection = false;


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
 *  Helper function.
 *  Checks whether a specific object should be filtered
 *  out from selection if the selection includes combatants.
 */
static bool Should_Exclude_From_Selection(ObjectClass* obj)
{
    /**
     *  Don't exclude objects that we don't own.
     */
    if (obj->Owning_House() != nullptr && !obj->Owning_House()->IsPlayerControl) {
        return false;
    }

    /**
     *  Exclude objects that aren't a selectable combatant per rules.
     */
    if (obj->Is_Techno()) {
        return Extension::Fetch<TechnoTypeClassExtension>(obj->Techno_Type_Class())->IsFilterFromBandBoxSelection;
    }

    return false;
}


/**
 *  Filters the selection from any non-combatants.
 *  
 *  @author: Petroglyph (Remaster), Rampastring, ZivDero
 */
static void Filter_Selection()
{
    if (!OptionsExtension->FilterBandBoxSelection) {
        return;
    }

    bool any_to_exclude = false;
    bool all_to_exclude = true;

    for (int i = 0; i < CurrentObjects.Count(); i++) {
        const bool exclude = Should_Exclude_From_Selection(CurrentObjects[i]);
        any_to_exclude |= exclude;
        all_to_exclude &= exclude;
    }

    if (any_to_exclude && !all_to_exclude) {
        for (int i = 0; i < CurrentObjects.Count(); i++) {
            if (Should_Exclude_From_Selection(CurrentObjects[i])) {

                const int count_before = CurrentObjects.Count();
                CurrentObjects[i]->Unselect();
                const int count_after = CurrentObjects.Count();
                if (count_after < count_before) {
                    i--;
                }
            }
        }
    }
}


/**
 *  Checks if the player has currently any non-combatants selected.
 *
 *  @author: ZivDero
 */
static bool Has_NonCombatants_Selected()
{
    for (int i = 0; i < CurrentObjects.Count(); i++)
    {
        if (CurrentObjects[i]->Is_Techno() && Extension::Fetch<TechnoTypeClassExtension>(CurrentObjects[i]->Techno_Type_Class())->IsFilterFromBandBoxSelection)
            return true;
    }

    return false;
}


/**
 *  Reimplements Tactical::Select_These to filter non-combatants.
 *
 *  @author: ZivDero
 */
void TacticalExt::_Select_These(Rect& rect, void (*selection_func)(ObjectClass* obj))
{
    SelectionContainsNonCombatants = Has_NonCombatants_Selected();
    SelectedCount = CurrentObjects.Count();
    FilterSelection = false;

    AllowVoice = true;

    if (rect.Width > 0 && rect.Height > 0 && DirtyObjectCount > 0)
    {
        for (int i = 0; i < DirtyObjectCount; i++)
        {
            const auto dirty = DirtyObjects[i];
            if (dirty.Object && dirty.Object->IsActive)
            {
                Point2D position = dirty.Position - field_5C;
                if (rect.Is_Within(position))
                {
                    if (selection_func)
                    {
                        selection_func(dirty.Object);
                    }
                    else
                    {
                        bool is_selectable_building = false;
                        if (dirty.Object->What_Am_I() == RTTI_BUILDING)
                        {
                            const auto bclass = static_cast<BuildingClass*>(dirty.Object)->Class;
                            if (bclass->UndeploysInto && !bclass->IsConstructionYard && !bclass->IsMobileWar)
                            {
                                is_selectable_building = true;
                            }
                        }

                        HouseClass* owner = dirty.Object->Owning_House();
                        if (owner && owner->Is_Player_Control())
                        {
                            if (dirty.Object->Class_Of()->IsSelectable)
                            {
                                if (dirty.Object->What_Am_I() != RTTI_BUILDING || is_selectable_building)
                                {
                                    if (dirty.Object->Select())
                                        AllowVoice = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     *  If player-controlled units are non-additively selected,
     *  remove non-combatants if they aren't the only types of units selected
     */
    if (FilterSelection)
        Filter_Selection();

    AllowVoice = true;
}


/**
 *  Band box selection predicate replacement.
 *
 *  @author: ZivDero
 */
static void Vinifera_Bandbox_Select(ObjectClass* obj)
{
    HouseClass* house = obj->Owning_House();
    BuildingClass* building = Target_As_Building(obj);

    /**
     *  Don't select objects that we don't own.
     */
    if (!house || !house->Is_Player_Control())
        return;

    /**
     *  Don't select objects that aren't selectable.
     */
    if (!obj->Class_Of()->IsSelectable)
        return;

    /**
     *  Don't select buildings, unless it undeploys and is something other than
     *  a construction yard or a war factory (for example, a deploying artillery).
     */
    if (building && (!building->Class->UndeploysInto || building->Class->IsConstructionYard || building->Class->IsMobileWar))
        return;

    /**
     *  Don't select limboed objects.
     */
    if (obj->IsInLimbo)
        return;

    /**
     *  If this is a Techno that's not a combatant, and the selection isn't new and doesn't
     *  already contain non-combatants, don't select it.
     */
     const TechnoClass* techno = Target_As_Techno(obj);
     if (techno && OptionsExtension->FilterBandBoxSelection
         && TacticalExt::SelectedCount > 0 && !TacticalExt::SelectionContainsNonCombatants
         && !WWKeyboard->Down(VK_ALT))
     {
         const auto ext = Extension::Fetch<TechnoTypeClassExtension>(techno->Techno_Type_Class());
         if (ext->IsFilterFromBandBoxSelection)
             return;
     }

    if (obj->Select())
    {
        AllowVoice = false;

        /**
         *  If this is a new selection, filter it at the end.
         */
        if (TacticalExt::SelectedCount == 0 && !WWKeyboard->Down(VK_ALT))
            TacticalExt::FilterSelection = true;
    }
}


/**
 *  Rally point line drawing routine replacement.
 *
 *  @author: ZivDero
 */
void TacticalExt::_Draw_Rally_Points(bool blit)
{
    /**
     *  5 pixels on, 3 off, 5 pixels on, 3 off.
     */
    static bool _pattern[16] = { true, true, true, true, true, false, false, false, true, true, true, true, true, false, false, false };

    /**
     *  #issue-348
     *
     *  The animation speed of Rally Point lines is not normalised and subjective to
     *  the game speed setting. This adjusts the animation using the system
     *  timer and makes the animation speed consistent across all game speeds.
     *
     *  @authors: CCHyper
     */
    const int time = timeGetTime();
    const int offset = (-time / 32) & (std::size(_pattern) - 1);

    const unsigned color = DSurface::RGB_To_Pixel(0, 255, 0);
    const unsigned color_black = DSurface::RGB_To_Pixel(0, 0, 0);

    /**
     *  Iterate all selected objects to see if we need to draw a rally point line for them.
     */
    for (int i = 0; i < CurrentObjects.Count(); i++)
    {
        const ObjectClass* obj = CurrentObjects[i];
        if (obj->Kind_Of() == RTTI_BUILDING && obj->IsActive && obj->IsSelected && obj->Owning_House() == PlayerPtr)
        {
            const BuildingClass* bldg = static_cast<const BuildingClass*>(obj);
            /**
             *  We draw rally point for factories, as well as repair bays (Rampastring).
             */
            if (bldg->Class->ToBuild == RTTI_UNITTYPE || bldg->Class->ToBuild == RTTI_INFANTRYTYPE || bldg->Class->ToBuild == RTTI_AIRCRAFTTYPE || bldg->Class->CanUnitRepair)
            {
                /**
                 *  ArchiveTarget contains the rally point cell, so it needs to be set.
                 */
                if (Target_Legal(bldg->ArchiveTarget) && bldg->Get_Mission() != MISSION_DECONSTRUCTION)
                {
                    /**
                     *  The start of the line is just at the building's center.
                     */
                    Coordinate center_coord = bldg->Center_Coord();
                    Point2D start_pos = func_60F150(center_coord);
                    start_pos += Point2D(TacticalRect.X, TacticalRect.Y) - field_5C;

                    /**
                     *  Get the coordinate of the rally point and adjust it for cell height.
                     */
                    Coordinate rally_coord = bldg->ArchiveTarget->Center_Coord();

                    rally_coord.Z = Map.Get_Cell_Height(rally_coord);
                    if (Map[rally_coord].IsBridge)
                        rally_coord.Z += BridgeCellHeight;

                    Point2D end_pos = func_60F0F0(Point2D(rally_coord.X, rally_coord.Y)) / 256;
                    end_pos.Y -= Z_Lepton_To_Pixel(rally_coord.Z);
                    end_pos += Point2D(TacticalRect.X, TacticalRect.Y) - field_5C;

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
                    start_pos.Y += 2;
                    end_pos.Y += 2;
                    if (Clip_Line(start_pos, end_pos, TacticalRect))
                    {
                        LogicSurface->entry_48(start_pos, end_pos, color_black, _pattern, offset, blit);
                    }

                    /**
                     *  Draw two lines, offset by one pixel from each other, giving the
                     *  impression that it is double the thickness.
                     */
                    --start_pos.Y;
                    --end_pos.Y;
                    if (Clip_Line(start_pos, end_pos, TacticalRect))
                    {
                        LogicSurface->entry_48(start_pos, end_pos, color, _pattern, offset, blit);
                    }

                    --start_pos.Y;
                    --end_pos.Y;
                    if (Clip_Line(start_pos, end_pos, TacticalRect))
                    {
                        LogicSurface->entry_48(start_pos, end_pos, color, _pattern, offset, blit);
                    }
                }
            }
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
    if (Clip_Line(*start_pos, *end_pos, TacticalRect))
    {
        LogicSurface->entry_48(*start_pos, *end_pos, color_black, _pattern, offset, blit);
    }

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    if (Clip_Line(*start_pos, *end_pos, TacticalRect))
    {
        LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);
    }

    --start_pos->Y;
    --end_pos->Y;
    if (Clip_Line(*start_pos, *end_pos, TacticalRect))
    {
        LogicSurface->entry_48(*start_pos, *end_pos, color, _pattern, offset, blit);
    }

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
    if (Clip_Line(*start_pos, *end_pos, TacticalRect))
    {
        LogicSurface->entry_4C(*start_pos, *end_pos, color_black);
    }

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos->Y;
    --end_pos->Y;
    if (Clip_Line(*start_pos, *end_pos, TacticalRect))
    {
        LogicSurface->entry_4C(*start_pos, *end_pos, color);
    }

    --start_pos->Y;
    --end_pos->Y;
    if (Clip_Line(*start_pos, *end_pos, TacticalRect))
    {
        LogicSurface->entry_4C(*start_pos, *end_pos, color);
    }

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

    Patch_Jump(0x00616C90, &TacticalExt::_Draw_Rally_Points);

    Patch_Jump(0x006172DB, &_Tactical_Draw_Waypoint_Paths_NormaliseLineAnimation_Patch);
    Patch_Jump(0x00617327, &_Tactical_Draw_Waypoint_Paths_DrawNormalLine_Patch);

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

    Patch_Jump(0x00616940, &TacticalExt::_Select_These);
    Patch_Jump(0x00479150, &Vinifera_Bandbox_Select);
}
