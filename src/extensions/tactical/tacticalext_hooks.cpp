/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended Tactical class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "tacticalext_hooks.h"

#include "building.h"
#include "buildingtype.h"
#include "clipline.h"
#include "convert.h"
#include "debughandler.h"
#include "ebolt.h"
#include "extension_globals.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "housetype.h"
#include "laserdraw.h"
#include "mouse.h"
#include "object.h"
#include "optionsext.h"
#include "rulesext.h"
#include "scenario.h"
#include "sdlsurface.h"
#include "side.h"
#include "sideext.h"
#include "syringe.h"
#include "tactical.h"
#include "tacticalext.h"
#include "tacticalext_init.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_globals.h"
#include "uicontrol.h"
#include "vinifera_globals.h"
#include "voc.h"

#include <timeapi.h>


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
    bool _Clamp_To_Tactical_Rect(Point2D& pixel);
    HRESULT STDMETHODCALLTYPE _Save(IStream* stream, BOOL cleardirty);
    void _Draw_Screen_Text(char const* text);

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
 *  Changes the function that clamps a point to the tactical rect
 *  to prevent jitter when the map is smaller than the screen.
 *
 *  @authors: Belonit, ZivDero
 */
bool TacticalExt::_Clamp_To_Tactical_Rect(Point2D& pixel)
{
    int xmin = TacticalRect.Width / 2 - (CELL_PIXEL_W >> 1) * (Map.PlayRect.Width - 2 * Map.LocalRect.X);
    int xmax = std::max(xmin + CELL_PIXEL_W * Map.LocalRect.Width - TacticalRect.Width, xmin);

    int ymin = TacticalRect.Height / 2 + (CELL_PIXEL_H >> 1) * (Map.PlayRect.Width + 2 * Map.LocalRect.Y - 5);
    int ymax = std::max(ymin + CELL_PIXEL_H * (2 * Map.LocalRect.Height + 9) / 2 - TacticalRect.Height, ymin);

    bool clamped = false;

    if (pixel.Y < ymin) {
        pixel.Y = ymin;
        clamped = true;
    } else if (pixel.Y > ymax) {
        pixel.Y = ymax;
        clamped = true;
    }

    if (pixel.X < xmin) {
        pixel.X = xmin;
        clamped = true;
    } else if (pixel.X > xmax) {
        pixel.X = xmax;
        clamped = true;
    }

    return clamped;
}


/**
 *  Reimplements Tactical::Draw_Band_Box.
 *  
 *  @author: CCHyper, ZivDero
 */
void TacticalExt::_Draw_Band_Box()
{
    if (RubberBandStart != Point2D(0, 0))
    {
        int x = RubberBandStart.X;
        int y = RubberBandStart.Y;
        int width = RubberBandEnd.X;
        int height = RubberBandEnd.Y;

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
            tint_rect = tint_rect.Bias_To(TacticalRect);

            RGBClass tint_dark = UIControls->BandBoxTintColors[0];
            RGBClass tint_light = UIControls->BandBoxTintColors[1];

            /**
             *  Interpolate between the two colors to find the correct tint
             *  for the current map ambient level.
             */
            const float adjust = static_cast<float>(Scen->CurrentAmbientLight) / 100.0f;
            const RGBClass tint_color = RGBClass::Interpolate(tint_dark, tint_light, adjust);

            LogicalSurface->Fill_Rect_Trans(tint_rect, tint_color, trans);
        }

        /**
         *  Draw the drop shadow.
         */
        if (UIControls->IsBandBoxDropShadow) {

            Rect drop_rect = band_rect;
            drop_rect.X += 1;
            drop_rect.Y += 1;

            const unsigned drop_color = DSurface::Build_Hicolor_Pixel(
                UIControls->BandBoxDropShadowColor.R,
                UIControls->BandBoxDropShadowColor.G,
                UIControls->BandBoxDropShadowColor.B);

            /**
             *  Draw the band box.
             */
            if (UIControls->IsBandBoxThick) {

                drop_rect.X += 1;
                drop_rect.Y += 1;

                LogicalSurface->Draw_Rect(TacticalRect, drop_rect, drop_color);

                Rect thick_rect = drop_rect;
                thick_rect.X += 1;
                thick_rect.Y += 1;
                thick_rect.Width -= 2;
                thick_rect.Height -= 2;

                LogicalSurface->Draw_Rect(TacticalRect, thick_rect, drop_color);

            }
            else {
                LogicalSurface->Draw_Rect(TacticalRect, drop_rect, drop_color);
            }

        }

        /**
         *  Draw the custom rubber band rect.
         */
        const unsigned band_color = DSurface::Build_Hicolor_Pixel(
            UIControls->BandBoxColor.R,
            UIControls->BandBoxColor.G,
            UIControls->BandBoxColor.B);

        LogicalSurface->Draw_Rect(TacticalRect, band_rect, band_color);

        /**
         *  If the band box is thick, draw an extra outline.
         */
        if (UIControls->IsBandBoxThick) {
            Rect thick_rect = band_rect;
            thick_rect.X += 1;
            thick_rect.Y += 1;
            thick_rect.Width -= 2;
            thick_rect.Height -= 2;
            LogicalSurface->Draw_Rect(TacticalRect, thick_rect, band_color);
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
    if (obj->Owner_HouseClass() != nullptr && !obj->Owner_HouseClass()->IsPlayerControl) {
        return false;
    }

    /**
     *  Exclude objects that aren't a selectable combatant per rules.
     */
    if (obj->Is_Techno()) {
        return Extension::Fetch(obj->TClass)->IsFilterFromBandBoxSelection;
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
        if (CurrentObjects[i]->Is_Techno() && Extension::Fetch(CurrentObjects[i]->TClass)->IsFilterFromBandBoxSelection)
            return true;
    }

    return false;
}


/**
 *  Reimplements Tactical::Select_These to filter non-combatants.
 *
 *  @author: ZivDero
 */
void TacticalExt::_Select_These(Rect& rect, void (*select_callback)(ObjectClass* obj))
{
    SelectionContainsNonCombatants = Has_NonCombatants_Selected();
    SelectedCount = CurrentObjects.Count();
    FilterSelection = false;

    AllowVoice = false;
    
    if (rect.Is_Valid()) {

        /**
         *  Sweep through all selectable objects and select the ones within the
         *  bounding box.
         */
        for (int index = 0; index < DirtyObjectCount; index++) {
            SelectData& sel = SelectableObjects[index];
            ObjectClass* obj = sel.Object;

            if (obj == nullptr || !obj->IsActive) {
                continue;
            }

            Point2D pos = sel.Position - field_5C;
            if (!rect.Is_Point_Within(pos)) {
                continue;
            }

            if (select_callback == nullptr) {

                bool force = false;

                if (obj->RTTI == RTTI_BUILDING) {
                    BuildingTypeClass* type = static_cast<BuildingClass*>(obj)->Class;
                    if (type->UndeploysInto && !type->IsConstructionYard && !type->IsMobileWar) {
                        force = true;
                    }
                }

                /**
                 *  Only try to select objects that are owned by the player, are allowed to be
                 *  selected.
                 */
                HouseClass* hptr = obj->Owner_HouseClass();
                if (hptr != nullptr && hptr->Is_Player_Control() && obj->Class_Of()->IsSelectable && (obj->RTTI != RTTI_BUILDING || force)) {
                    if (obj->Select()) {
                        //AllowVoice = false;
                    }
                }
            } else {
                select_callback(sel.Object);
            }
        }
    }

    /**
     *  If player-controlled units are non-additively selected,
     *  remove non-combatants if they aren't the only types of units selected
     */
    if (FilterSelection) {
        Filter_Selection();
    }

    AllowVoice = true;

    /**
     *  Play the selection voiceline.
     */
    for (auto& obj : CurrentObjects) {
        if (obj->Is_Techno()) {
            static_cast<TechnoClass*>(obj)->Response_Select();
            break;
        }
    }
}


/**
 *  Band box selection predicate replacement.
 *
 *  @author: ZivDero
 */
static void Vinifera_Bandbox_Select(ObjectClass* obj)
{
    HouseClass* house = obj->Owner_HouseClass();
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
         && !Keyboard->Down(VK_ALT))
     {
         const auto ext = Extension::Fetch(techno->TClass);
         if (ext->IsFilterFromBandBoxSelection)
             return;
     }

    if (obj->Select())
    {
        AllowVoice = false;

        /**
         *  If this is a new selection, filter it at the end.
         */
        if (TacticalExt::SelectedCount == 0 && !Keyboard->Down(VK_ALT))
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

    const unsigned color = DSurface::Build_Hicolor_Pixel(0, 255, 0);
    const unsigned color_black = DSurface::Build_Hicolor_Pixel(0, 0, 0);

    /**
     *  Iterate all selected objects to see if we need to draw a rally point line for them.
     */
    for (int i = 0; i < CurrentObjects.Count(); i++)
    {
        const ObjectClass* obj = CurrentObjects[i];
        if (obj->RTTI == RTTI_BUILDING && obj->IsActive && obj->IsSelected && obj->Owner_HouseClass() == PlayerPtr)
        {
            const BuildingClass* bldg = static_cast<const BuildingClass*>(obj);
            /**
             *  We draw rally point for factories, as well as repair bays (Rampastring) and armories/hospitals (JoyfulShush).
             */
            if (bldg->Class->ToBuild == RTTI_UNITTYPE || bldg->Class->ToBuild == RTTI_INFANTRYTYPE || bldg->Class->ToBuild == RTTI_AIRCRAFTTYPE || bldg->Class->IsCanUnitRepair || bldg->Class->IsArmory || bldg->Class->IsHospital)
            {
                /**
                 *  ArchiveTarget contains the rally point cell, so it needs to be set.
                 */
                if (bldg->ArchiveTarget != nullptr && bldg->Get_Mission() != MISSION_DECONSTRUCTION)
                {
                    /**
                     *  The start of the line is just at the building's center.
                     */
                    Coord center_coord = bldg->Center_Coord();
                    Point2D start_pos = func_60F150(center_coord);
                    start_pos += Point2D(TacticalRect.X, TacticalRect.Y) - field_5C;

                    /**
                     *  Get the coordinate of the rally point and adjust it for cell height.
                     */
                    Coord rally_coord = bldg->ArchiveTarget->Center_Coord();

                    rally_coord.Z = Map.Get_Height_GL(rally_coord);
                    if (Map[rally_coord].IsUnderBridge)
                        rally_coord.Z += BRIDGE_LEPTON_HEIGHT;

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
                        LogicalSurface->entry_48(start_pos, end_pos, color_black, _pattern, offset, blit);
                    }

                    /**
                     *  Draw two lines, offset by one pixel from each other, giving the
                     *  impression that it is double the thickness.
                     */
                    --start_pos.Y;
                    --end_pos.Y;
                    if (Clip_Line(start_pos, end_pos, TacticalRect))
                    {
                        LogicalSurface->entry_48(start_pos, end_pos, color, _pattern, offset, blit);
                    }

                    --start_pos.Y;
                    --end_pos.Y;
                    if (Clip_Line(start_pos, end_pos, TacticalRect))
                    {
                        LogicalSurface->entry_48(start_pos, end_pos, color, _pattern, offset, blit);
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
DEFINE_HOOK(0x00616FDA, _Tactical_Draw_Waypoint_Paths_Text_Color_Patch, 0)
{
    R->EAX(14);

    return(0x00616FEB);
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
DEFINE_HOOK(0x006172DB, _Tactical_Draw_Waypoint_Paths_NormaliseLineAnimation_Patch, 0)
{
    GET(unsigned, color, EAX);
    GET_STACK(bool, blit, 0x90);
    REF_STACK(Point2D, start_pos, 0x34);
    REF_STACK(Point2D, end_pos, 0x3C);

    /**
     *  5 pixels on, 3 off, 5 pixels on, 3 off.
     */
    static bool _pattern[16] = { true, true, true, true, true, false, false, false, true, true, true, true, true, false, false, false };

    /**
     *  Adjust the offset of the line pattern (this animates a little slower than rally points).
     */
    int time = timeGetTime();
    int offset = (-time / 64) & (std::size(_pattern)-1);

    unsigned color_black = DSurface::Build_Hicolor_Pixel(0,0,0);

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
    start_pos.Y += 2;
    end_pos.Y += 2;
    if (Clip_Line(start_pos, end_pos, TacticalRect)) {
        LogicalSurface->entry_48(start_pos, end_pos, color_black, _pattern, offset, blit);
    }

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos.Y;
    --end_pos.Y;
    if (Clip_Line(start_pos, end_pos, TacticalRect)) {
        LogicalSurface->entry_48(start_pos, end_pos, color, _pattern, offset, blit);
    }

    --start_pos.Y;
    --end_pos.Y;
    if (Clip_Line(start_pos, end_pos, TacticalRect)) {
        LogicalSurface->entry_48(start_pos, end_pos, color, _pattern, offset, blit);
    }

    return(0x00617307);
}


/**
 *  #issue-351
 * 
 *  Thicken the waypoint path lines so they are easier to see in contrast to the terrain.
 * 
 *  @authors: CCHyper
 */
DEFINE_HOOK(0x00617327, _Tactical_Draw_Waypoint_Paths_DrawNormalLine_Patch, 0)
{
    GET(unsigned, color, EAX);
    GET_STACK(bool, blit, 0x90);
    REF_STACK(Point2D, start_pos, 0x34);
    REF_STACK(Point2D, end_pos, 0x3C);

    unsigned color_black = DSurface::Build_Hicolor_Pixel(0, 0, 0);

    /**
     *  Draw the drop shadow line.
     */
    start_pos.Y += 2;
    end_pos.Y += 2;
    if (Clip_Line(start_pos, end_pos, TacticalRect)) {
        LogicalSurface->entry_4C(start_pos, end_pos, color_black, false);
    }

    /**
     *  Draw two lines, offset by one pixel from each other, giving the
     *  impression that it is double the thickness.
     */
    --start_pos.Y;
    --end_pos.Y;
    if (Clip_Line(start_pos, end_pos, TacticalRect)) {
        LogicalSurface->entry_4C(start_pos, end_pos, color, false);
    }

    --start_pos.Y;
    --end_pos.Y;
    if (Clip_Line(start_pos, end_pos, TacticalRect)) {
        LogicalSurface->entry_4C(start_pos, end_pos, color, false);
    }

    return(0x00617307);
}


/**
 *  This patch intercepts the post effects rendering process for Tactical
 *  allowing us to draw any new effects/systems.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00611AF9, _Tactical_Render_Post_Effects_Patch, 0)
{
    GET(Tactical *, this_ptr, EBP);

    /**
     *  Stolen bytes/code.
     */
    LaserDrawClass::Draw_All();

    /**
     *  Draw any new post effects here.
     */
    TacticalMapExtension->Render_Post();

    return(0x00611AFE);
}


/**
 *  This patch intercepts the end of the rendering process for Tactical
 *  for drawing any overlay or graphics.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00611BCB, _Tactical_Render_Overlay_Patch, 0)
{
    GET(Tactical *, this_ptr, EBP);

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
            Voice_Sound_Effect(TacticalMapExtension->InfoTextNotifySound, TacticalMapExtension->InfoTextNotifySoundVolume);
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
     *  Has a templated text been set to be displayed?
     */
    if (TacticalMapExtension->IsTemplatedTextVisible) {

        /**
         *  Draw it to the screen.
         */
        TacticalMapExtension->Draw_Templated_Text();
    }

    /**
     *  Draw the VOX subtitle (if any) at the bottom of the tactical view.
     *  Drawn before the version-number text so the version stays on top in
     *  the rare case of a long subtitle reaching the corner.
     */
    TacticalMapExtension->Draw_Subtitle();

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

    return(0x00611BE4);
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
DEFINE_HOOK(0x0060F953, _Tactical_Center_On_Location_Unfollow_Object_Patch, 7)
{
    Map.Break_Follow_Mode();

    return 0;
}


/**
 *  Fills the remaining space of the Tactical screen with black.
 *
 *  @authors: Belonit, ZivDero
 */
DEFINE_HOOK(0x00611BBB, _Tactical_Render_Fill_With_Black_Patch, 6)
{
    const int max_width = TacticalRect.Width - Map.LocalRect.Width * CELL_PIXEL_W;
    if (max_width > 0) {
        Rect rect = {TacticalRect.X + TacticalRect.Width - max_width, TacticalRect.Y, max_width, TacticalRect.Height};
        CompositeSurface->Fill_Rect(rect, COLOR_TBLACK);
    }

    const int max_height = TacticalRect.Height - Map.LocalRect.Height * CELL_PIXEL_H - static_cast<int>(4.5 * CELL_PIXEL_H);
    if (max_height > 0) {
        Rect rect = {TacticalRect.X, TacticalRect.Y + TacticalRect.Height - max_height, TacticalRect.Width, max_height};
        CompositeSurface->Fill_Rect(rect, COLOR_TBLACK);
    }

    return 0;
}


/**
 *  Re-implementation of Tactical::Save to save CellRedrawCount into the extension for load purposes.
 *
 *  @author: ZivDero
 */
HRESULT STDMETHODCALLTYPE TacticalExt::_Save(IStream* stream, BOOL cleardirty)
{
    HRESULT result = Abstract_Save(stream, cleardirty);
    if (SUCCEEDED(result)) {

        /**
         *  Save the cell redraw count as we won't have access to it when loading the extension.
         */
        TacticalMapExtension->CellRedrawCount = CellRedrawCount;

        result = S_OK;
    }
    return result;
}


/**
 *  Patch in Tactical::Flag_Cell to use the new cell redraw array.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00616C07, _TacticalClass_Flag_Cell_New_Array_Patch, 0)
{
    GET_STACK(CellClass*, cell, 0x20);

    TacticalMapExtension->Flag_Cell(*cell);

    return(0x00616C2C);
}


/**
 *  Patches in the multitude of Tactical render procedures to use the
 *  new cell redraw array. Uses a macro for convenience.
 *
 *  @author: ZivDero
 */
#define CELL_REDRAW_POINTER_PATCH(PatchName, dst_reg, ret_reg, ret_addr) \
DECLARE_PATCH(PatchName)                                                 \
{                                                                        \
    static CellClass** array;                                            \
    array = TacticalMapExtension->CellRedraw;                            \
    _asm { mov dst_reg, array }                                          \
    JMP_REG(ret_reg, ret_addr);                                          \
}

CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender1_Patch, eax, edx, 0x0061015A);
CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender2_Patch, ebx, eax, 0x006102BF);
CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender3_Patch, ebp, eax, 0x0061051F);
CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender4_Patch, ebx, eax, 0x00610768);
CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender5_Patch, ebp, eax, 0x0061094C);
CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender6_Patch, eax, edx, 0x00610B09);
CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender7_Patch, ebp, eax, 0x00610D67);
CELL_REDRAW_POINTER_PATCH(_TacticalClass_SubRender8_Patch, ebp, eax, 0x00610FB7);


/**
 *  Tactical::Draw_Screen_Text re-implementation to fix direct DDraw surface
 *  access that is invalid because of SDL.
 *
 *  @author: ZivDero, tomsons26, Rampastring
 */
void TacticalExt::_Draw_Screen_Text(char const* text)
{
    if (Debug_Map) {
        return;
    }
    if (text == nullptr || !strlen(text)) {
        return;
    }
    if (CompositeSurface->Is_Direct_Draw()) {
        HDC hdc = static_cast<SDLSurface*>(CompositeSurface)->GetDC();
        Rect rect = TacticalRect;
        if (hdc != nullptr) {
            HFONT font = CreateFont(48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_RASTER_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SWISS | DEFAULT_PITCH, NULL);
            HGDIOBJ h = SelectObject(hdc, font);
            Point2D point = Point2D(TacticalRect.Width / 2, TacticalRect.Height / 2);
            SetBkMode(hdc, TRANSPARENT);
            SetTextAlign(hdc, TA_CENTER);

            // Draw a shadow
            SetTextColor(hdc, RGB(0, 0, 0));
            TextOut(hdc, rect.X + point.X + 3, rect.Y + point.Y + 3, text, strlen(text));

            RGBClass rgb = SCREEN_TEXT_DEFAULT_COLOR;
            if (PlayerPtr != nullptr && PlayerPtr->Class->Side != SIDE_NONE) {
                rgb = Extension::Fetch(Sides[PlayerPtr->Class->Side])->ScreenTextColor;
            }

            // Draw the main text
            SetTextColor(hdc, RGB(rgb.Get_Red(), rgb.Get_Green(), rgb.Get_Blue()));
            TextOut(hdc, rect.X + point.X, rect.Y + point.Y, text, strlen(text));

            // Cleanup
            SelectObject(hdc, h);
            DeleteObject(font);
            static_cast<SDLSurface*>(CompositeSurface)->ReleaseDC(hdc);
        }
    }
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

    Patch_Jump(0x00616C90, &TacticalExt::_Draw_Rally_Points);
    Patch_Jump(0x00616560, &TacticalExt::_Draw_Band_Box);
    Patch_Jump(0x00616940, &TacticalExt::_Select_These);
    Patch_Jump(0x00614EC0, &TacticalExt::_Clamp_To_Tactical_Rect);
    Patch_Jump(0x00617F80, &TacticalExt::_Save);
    Patch_Jump(0x00479150, &Vinifera_Bandbox_Select);
    Patch_Jump(0x00611C60, &TacticalExt::_Draw_Screen_Text);

    /**
     *  #issue-351
     * 
     *  Changes the waypoint number text to have a stroke/outline.
     * 
     *  @authors: CCHyper
     */
    Patch_Dword(0x006171C8+1, (TPF_CENTER|TPF_EFNT|TPF_FULLSHADOW));

    Patch_Jump(0x00617F54, 0x00617F79); // Skip swizzling cell redraw pointers in TacticalClass as we have our own array
    Patch_Jump(0x00610154, _TacticalClass_SubRender1_Patch);
    Patch_Jump(0x006102B9, _TacticalClass_SubRender2_Patch);
    Patch_Jump(0x00610519, _TacticalClass_SubRender3_Patch);
    Patch_Jump(0x00610762, _TacticalClass_SubRender4_Patch);
    Patch_Jump(0x00610946, _TacticalClass_SubRender5_Patch);
    Patch_Jump(0x00610B03, _TacticalClass_SubRender6_Patch);
    Patch_Jump(0x00610D61, _TacticalClass_SubRender7_Patch);
    Patch_Jump(0x00610FB1, _TacticalClass_SubRender8_Patch);
}
