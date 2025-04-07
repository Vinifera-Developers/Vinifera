/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FOOTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended FootClass.
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
#include "footext_hooks.h"

#include "aircrafttracker.h"
#include "foot.h"
#include "technoext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_inline.h"
#include "tibsun_globals.h"
#include "tactical.h"
#include "textprint.h"
#include "clipline.h"
#include "convert.h"
#include "house.h"
#include "iomap.h"
#include "rules.h"
#include "rulesext.h"
#include "session.h"
#include "unit.h"
#include "unitext.h"
#include "unittype.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "ionstorm.h"
#include "levitatelocomotion.h"
#include "radarevent.h"
#include "uicontrol.h"
#include "vinifera_globals.h"
#include "vox.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class FootClassExt : public FootClass
{
public:
    void _Draw_Action_Line() const;
    void _Draw_NavComQueue_Lines() const;
    void _Death_Announcement(TechnoClass* source) const;
    Cell _Search_For_Tiberium(int rad, bool a2);
    bool _Unlimbo(Coordinate& coord, Dir256 dir);
    bool _Limbo();

private:
    void _Draw_Line(Coordinate& start_coord, Coordinate& end_coord, bool is_dashed, bool is_thick, bool is_dropshadow, unsigned line_color, unsigned drop_color, int rate) const;
};


/**
 *  Draws an action line with the given parameters.
 *
 *  @author: CCHyper, ZivDero
 */
void FootClassExt::_Draw_Line(Coordinate& start_coord, Coordinate& end_coord, bool is_dashed, bool is_thick, bool is_dropshadow, unsigned line_color, unsigned drop_color, int rate) const
{
    int point_size = 3;
    Point2D point_offset(-1, -1);

    if (is_thick) {
        point_size = 4;
        point_offset = Point2D(-2, -2);
    }

    /**
     *  Convert the world coord to screen pixel.
     */
    Point2D start_point;
    Point2D end_point;
    TacticalMap->Coord_To_Pixel(start_coord, start_point);
    TacticalMap->Coord_To_Pixel(end_coord, end_point);

    /**
     *  Offset pixel position relative to tactical viewport.
     */
    start_point += Point2D(TacticalRect.X, TacticalRect.Y);
    end_point += Point2D(TacticalRect.X, TacticalRect.Y);

    /**
     *  Save the start and end points before we clip them to the viewport,
     *  so that when we draw start and end rectangles they don't show up
     *  on screen edges if they're off-screen.
     */
    Point2D start_point_unclipped = start_point;
    Point2D end_point_unclipped = end_point;

    /**
     *  Draw the queue line.
     */
    if (Clip_Line(start_point, end_point, TacticalRect)) {

        Point2D drop_start_point = start_point;
        Point2D drop_end_point = end_point;

        drop_start_point.Y += 1;
        drop_end_point.Y += 1;

        if (is_dashed) {

            /**
             *  4 pixels on, 4 off, 4 pixels on, 4 off.
             */
            static bool _pattern[] = { true, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false };

            /**
             *  Adjust the offset of the line pattern.
             */
            int time = timeGetTime();
            int offset = (-time / rate) & (std::size(_pattern) - 1);

            /**
             *  Draw the drop shadow line.
             */
            if (is_dropshadow) {

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                }

                CompositeSurface->Draw_Dashed_Line(TacticalRect, drop_start_point, drop_end_point, drop_color, _pattern, offset);

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                    CompositeSurface->Draw_Dashed_Line(TacticalRect, drop_start_point, drop_end_point, drop_color, _pattern, offset);
                }

            }

            /**
             *  Draw the dashed queue line.
             */
            CompositeSurface->Draw_Dashed_Line(TacticalRect, start_point, end_point, line_color, _pattern, offset);

            if (is_thick) {
                start_point.Y += 1;
                end_point.Y += 1;
                CompositeSurface->Draw_Dashed_Line(TacticalRect, start_point, end_point, line_color, _pattern, offset);
            }

        }
        else {

            /**
             *  Draw the drop shadow line.
             */
            if (is_dropshadow) {

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                }

                CompositeSurface->Draw_Line(drop_start_point, drop_end_point, drop_color);

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                    CompositeSurface->Draw_Line(drop_start_point, drop_end_point, drop_color);
                }

            }

            /**
             *  Draw the queue line.
             */
            CompositeSurface->Draw_Line(start_point, end_point, line_color);

            if (is_thick) {
                start_point.Y += 1;
                end_point.Y += 1;
                CompositeSurface->Draw_Line(start_point, end_point, line_color);
            }

        }

    }

    /**
     *  Draw the queue line start and end squares.
     */
    if (is_dropshadow) {

        const int drop_point_size = is_thick ? (point_size + 3) : (point_size + 2);
        const Point2D drop_point_offset = is_thick ? (point_offset + Point2D(-2, -2)) : (point_offset + Point2D(-1, -1));

        if (is_thick) {
            point_size -= 1;
        }

        Rect drop_start_point_rect = Intersect(TacticalRect, Rect(start_point_unclipped + drop_point_offset, drop_point_size, drop_point_size));
        CompositeSurface->Fill_Rect(drop_start_point_rect, drop_color);

        Rect drop_end_point_rect = Intersect(TacticalRect, Rect(end_point_unclipped + drop_point_offset, drop_point_size, drop_point_size));
        CompositeSurface->Fill_Rect(drop_end_point_rect, drop_color);
    }

    Rect start_point_rect = Intersect(TacticalRect, Rect(start_point_unclipped + point_offset, point_size, point_size));
    CompositeSurface->Fill_Rect(start_point_rect, line_color);

    Rect end_point_rect = Intersect(TacticalRect, Rect(end_point_unclipped + point_offset, point_size, point_size));
    CompositeSurface->Fill_Rect(end_point_rect, line_color);
}


/**
 *  Draws a line for the current NavCom queue.
 * 
 *  @author: CCHyper, ZivDero
 */
void FootClassExt::_Draw_NavComQueue_Lines() const
{
    if (!NavCom || !NavQueue.Count()) {
        return;
    }

    /**
     *  Fetch the line properties.
     */
    const bool is_dashed = UIControls->IsNavComQueueLineDashed;
    const bool is_thick = UIControls->IsNavComQueueLineThick;
    const bool is_dropshadow = UIControls->IsNavComQueueLineDropShadow;

    const unsigned line_color = DSurface::RGB_To_Pixel(
        UIControls->NavComQueueLineColor.R,
        UIControls->NavComQueueLineColor.G,
        UIControls->NavComQueueLineColor.B);

    const unsigned drop_color = DSurface::RGB_To_Pixel(
        UIControls->NavComQueueLineDropShadowColor.R,
        UIControls->NavComQueueLineDropShadowColor.G,
        UIControls->NavComQueueLineDropShadowColor.B);

    /**
     *  Fetch the queue line start and end coord.
     */
    AbstractClass * start = NavCom;
    AbstractClass * end = NavQueue[0];

    Coordinate start_coord;
    Coordinate end_coord;

    for (int i = 0; i < NavQueue.Count(); i++) {

        start_coord = start->Center_Coord();

        if (Map.In_Radar(Coord_Cell(start_coord)) && Map[start_coord].IsUnderBridge) {
            start_coord.Z = BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(start_coord);
        }

        end_coord = end->Center_Coord();

        if (Map.In_Radar(Coord_Cell(end_coord)) && Map[end_coord].IsUnderBridge) {
            end_coord.Z = BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(end_coord);
        }

        _Draw_Line(start_coord, end_coord, is_dashed, is_thick, is_dropshadow, line_color, drop_color, 128);

        start = NavQueue[i];
        end = NavQueue[i + 1];
    }
}


/**
 *  Reimplementation of FootClass::Draw_Action_Line().
 * 
 *  @author: CCHyper, ZivDero
 */
void FootClassExt::_Draw_Action_Line() const
{
    if (!TarCom && !NavCom) {
        return;
    }

    if (!UIControls->IsAlwaysShowActionLines && ActionLineTimer.Expired() && !WWKeyboard->Down(Options.KeyQueueMove1) && !WWKeyboard->Down(Options.KeyQueueMove2)) {
        return;
    }

    /**
     *  Fetch the line properties.
     */
    const bool tarcom_is_dashed = UIControls->IsTargetLineDashed;
    const bool tarcom_is_thick = UIControls->IsTargetLineThick;
    const bool tarcom_is_dropshadow = UIControls->IsTargetLineDropShadow;

    const bool navcom_is_dashed = UIControls->IsMovementLineDashed;
    const bool navcom_is_thick = UIControls->IsMovementLineThick;
    const bool navcom_is_dropshadow = UIControls->IsMovementLineDropShadow;

    const unsigned tarcom_color = DSurface::RGB_To_Pixel(
        UIControls->TargetLineColor.R,
        UIControls->TargetLineColor.G,
        UIControls->TargetLineColor.B);

    const unsigned tarcom_drop_color = DSurface::RGB_To_Pixel(
        UIControls->TargetLineDropShadowColor.R,
        UIControls->TargetLineDropShadowColor.G,
        UIControls->TargetLineDropShadowColor.B);

    const unsigned navcom_color = DSurface::RGB_To_Pixel(
        UIControls->MovementLineColor.R,
        UIControls->MovementLineColor.G,
        UIControls->MovementLineColor.B);

    const unsigned navcom_drop_color = DSurface::RGB_To_Pixel(
        UIControls->MovementLineDropShadowColor.R,
        UIControls->MovementLineDropShadowColor.G,
        UIControls->MovementLineDropShadowColor.B);

    /**
     *  Fetch the action line start and end coord.
     */
    Coordinate start_coord;
    Coordinate end_coord;

    if (TarCom) {

        start_coord = entry_28C();
        end_coord = func_638AF0();

        _Draw_Line(start_coord, end_coord, tarcom_is_dashed, tarcom_is_thick, tarcom_is_dropshadow, tarcom_color, tarcom_drop_color, 64);

    }

    if (NavCom) {

        start_coord = Get_Coord();

        AbstractClass * navtarget = field_260.Count() ? field_260.Fetch_Tail() : NavCom;
        end_coord = navtarget->Center_Coord();
        Cell target_cell = Coord_Cell(end_coord);

        if (Map.In_Radar(target_cell) && Map[end_coord].IsUnderBridge) {
            end_coord.Z = BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(end_coord);
        }

        _Draw_Line(start_coord, end_coord, navcom_is_dashed, navcom_is_thick, navcom_is_dropshadow, navcom_color, navcom_drop_color, 128);

        if (UIControls->IsShowNavComQueueLines) {
            _Draw_NavComQueue_Lines();
        }

    }
}


/**
 *  #issue-203
 *  
 *  Evaluates the value of Tiberium on a single cell.
 *  
 *  Author: Rampastring
 */
void _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(FootClass* this_ptr, Cell& cell_coords, Cell* besttiberiumcell, int* besttiberiumvalue, UnitClassExtension* unitext)
{
    if (this_ptr->Tiberium_Check(cell_coords)) {

        CellClass* cell = &Map[cell_coords];
        int tiberiumvalue = cell->Get_Tiberium_Value();

        /**
        *  #issue-203
        *
        *  Consider distance to refinery when selecting the next tiberium patch to harvest.
        *  Prefer the most resourceful tiberium patch, but if there's a tie, prefer one that's
        *  closer to our refinery. Original game only cares about the value.
        *
        *  @author: Rampastring
        */
        if (unitext && unitext->LastDockedBuilding && unitext->LastDockedBuilding->IsActive && !unitext->LastDockedBuilding->IsInLimbo) {
            tiberiumvalue *= 100;
            tiberiumvalue -= ::Distance(cell_coords, unitext->LastDockedBuilding->Get_Cell());
        }

        if (tiberiumvalue > *besttiberiumvalue)
        {
            *besttiberiumvalue = tiberiumvalue;
            *besttiberiumcell = cell_coords;
        }
    }
}


/**
 *  #issue-203
 * 
 *  Smarter replacement for the Search_For_Tiberium method.
 *  Makes harvesters consider the distance to their refinery when
 *  looking for the cell of tiberium to harvest.
 * 
 *  Author: Rampastring
 */
Cell FootClassExt::_Search_For_Tiberium(int rad, bool a2)
{
    if (!Owner_HouseClass()->Is_Human_Control() &&
        Fetch_RTTI() == RTTI_UNIT &&
        ((UnitClass*)this)->Class->IsToHarvest &&
        a2 &&
        Session.Type != GAME_NORMAL)
    {
        /**
         *  Use weighted tiberium-seeking algorithm for AI in multiplayer.
         */

        return Search_For_Tiberium_Weighted(rad);
    }

    Coordinate center_coord = Center_Coord();
    Cell cell_coords = Coord_Cell(center_coord);
    Cell unit_cell_coords = cell_coords;

    if (Map[unit_cell_coords].Land_Type() == LAND_TIBERIUM) {

        /**
         *  If we're already standing on tiberium, then we don't need to move anywhere.
         */

        return unit_cell_coords;
    }

    int besttiberiumvalue = -1;
    Cell besttiberiumcell = Cell(0, 0);

    UnitClassExtension* unitext = nullptr;
    if (Fetch_RTTI() == RTTI_UNIT) {
        unitext = Extension::Fetch<UnitClassExtension>(this);
    }

    /**
     *  Perform a ring search outward from the center.
     */
    for (int radius = 1; radius < rad; radius++) {
        for (int x = -radius; x <= radius; x++) {

            cell_coords = Cell(unit_cell_coords.X + x, unit_cell_coords.Y - radius);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);

            cell_coords = Cell(unit_cell_coords.X + x, unit_cell_coords.Y + radius);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);

            cell_coords = Cell(unit_cell_coords.X - radius, unit_cell_coords.Y + x);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);

            cell_coords = Cell(unit_cell_coords.X + radius, unit_cell_coords.Y + x);
            _Vinifera_FootClass_Search_For_Tiberium_Check_Tiberium_Value_Of_Cell(this, cell_coords, &besttiberiumcell, &besttiberiumvalue, unitext);
        }

        if (besttiberiumvalue != -1)
            break;
    }

    return besttiberiumcell;
}


/**
 *  #issue-593
 * 
 *  Implements IsCanPassiveAcquire for TechnoTypes when the unit is in MISSION_MOVE.
 * 
 *  @author: CCHyper
 */
static bool Foot_Target_Something_Nearby_Coord(FootClass *this_ptr, ThreatType threat) { return this_ptr->Target_Something_Nearby(this_ptr->Get_Coord(), threat); }
DECLARE_PATCH(_FootClass_Mission_Move_Can_Passive_Acquire_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    static TechnoClassExtension *technoclassext;

    technoclassext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    /**
     *  Can this unit passively acquire new targets?
     */
    if (!technoclassext->Can_Passive_Acquire()) {
        goto finish_mission_process;
    }

    /**
     *  Find a fresh target within my range.
     */
    Foot_Target_Something_Nearby_Coord(this_ptr, THREAT_RANGE);

finish_mission_process:
    JMP(0x004A104B);
}


/**
 *  #issue-593
 * 
 *  Implements IsCanPassiveAcquire for TechnoTypes when the unit is in MISSION_GUARD.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_FootClass_Mission_Guard_Can_Passive_Acquire_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    static TechnoClassExtension *technoclassext;

    technoclassext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    /**
     *  Can this unit passively acquire new targets?
     */
    if (!technoclassext->Can_Passive_Acquire()) {
        goto continue_check;
    }

    /**
     *  Find a fresh target within my range.
     */
    if (!Foot_Target_Something_Nearby_Coord(this_ptr, THREAT_RANGE)) {
        goto random_animate;
    }

continue_check:
    JMP(0x004A1AD6);

random_animate:
    JMP(0x004A1ACC);
}


/**
 *  #issue-593
 * 
 *  Implements IsCanPassiveAcquire for TechnoTypes when the unit is in MISSION_GUARD_AREA.
 * 
 *  @author: CCHyper
 */
static bool Foot_Target_Something_Nearby_Archive(FootClass *this_ptr, ThreatType threat) { return this_ptr->Target_Something_Nearby(this_ptr->ArchiveTarget->Center_Coord(), threat); }
DECLARE_PATCH(_FootClass_Mission_Guard_Area_Can_Passive_Acquire_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    static TechnoClassExtension *technoclassext;

    technoclassext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    /**
     *  Can this unit passively acquire new targets?
     */
    if (!technoclassext->Can_Passive_Acquire()) {
        goto tarcom_check;
    }

    /**
     *  Find a fresh target in my area using the backup target.
     */
    Foot_Target_Something_Nearby_Archive(this_ptr, THREAT_AREA);

tarcom_check:
    JMP(0x004A2C04);
}


/**
 *  #issue-421
 * 
 *  Implements IdleRate for TechnoTypes.
 * 
 *  @author: CCHyper
 */
static bool Locomotion_Is_Moving_Now(FootClass *this_ptr) { return this_ptr->Locomotion->Is_Moving_Now(); }
DECLARE_PATCH(_FootClass_AI_IdleRate_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    GET_REGISTER_STATIC(ILocomotion *, loco, edi);
    static TechnoTypeClassExtension *technotypeext;

    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(this_ptr->Techno_Type_Class());

    /**
     *  Stolen bytes/code.
     * 
     *  If the object is currently moving, check to see if its time to update its walk frame.
     */
    if (Locomotion_Is_Moving_Now(this_ptr) && !(Frame % this_ptr->Techno_Type_Class()->WalkRate)) {
        ++this_ptr->TotalFramesWalked;

    /**
     *  Otherwise, if the object is not currently moving, check to see if its time to update its idle frame.
     */
    } else if (technotypeext->IdleRate > 0) {
        if (!Locomotion_Is_Moving_Now(this_ptr) && !(Frame % technotypeext->IdleRate)) {
            ++this_ptr->TotalFramesWalked;
        }
    }

    _asm { mov edi, loco }      // Restore EDI register.

    JMP_REG(edx, 0x004A5A12);
}


/**
 *  #issue-404
 * 
 *  A object with "CloakStop" set has no effect on the cloaking behavior.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_FootClass_Is_Allowed_To_Recloak_Cloak_Stop_BugFix_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TechnoTypeClass *, technotype, eax);
    static ILocomotion *loco;

    /**
     *  Is this unit flagged to only re-cloak when not moving?
     */
    if (technotype->CloakStop) {

        loco = this_ptr->Locomotor_Ptr();

        /**
         *  If the object is currently moving, then return false.
         * 
         *  The original code here called Is_Moving_Now, which returned
         *  false when the locomotor was on a slope or rotating, which
         *  breaks the CloakStop mechanic.
         */
        if (loco->Is_Moving()) {
            goto return_false;
        }
    }

    /**
     *  The unit can re-cloak.
     */
return_true:
    JMP_REG(ecx, 0x004A6897);

    /**
     *  The unit is not allowed to re-cloak.
     */
return_false:
    JMP_REG(ecx, 0x004A689B);
}


/**
 *  Announces the death of a unit.
 *
 *  @author: 07/01/1995 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun
 */
void FootClassExt::_Death_Announcement(TechnoClass* source) const
{
    if (IsOwnedByPlayer) {

        const auto is_spawned = Extension::Fetch<TechnoTypeClassExtension>(Techno_Type_Class())->IsSpawned;
        if (!Techno_Type_Class()->IsInsignificant && !is_spawned) {

            RadarEventClass::LastEventCell = Coord_Cell(entry_50());
            Speak(VOX_UNIT_LOST);
        }
    }
}


/**
 *  FootClass::Unlimbo replacement.
 *
 *  @author: ZivDero
 */
bool FootClassExt::_Unlimbo(Coordinate& coord, Dir256 dir)
{
    /**
     *  Try to unlimbo the unit.
     */
    if (TechnoClass::Unlimbo(coord, dir)) {

        Locomotion->Unlimbo();

        bool off = false;
        if (IonStorm_Is_Active()) {
            if (Locomotion->Is_Ion_Sensitive()) {
                off = true;
            }
        }

        if (off) {
            Locomotion->Power_Off();
        }
        else {
            Locomotion->Power_On();
        }

        /**
         *  Instead of patching levitate locomotion to add to tracking, since levitate locomotion is
         *  always in flight let's add it right now.
         */
        if (Techno_Type_Class()->Locomotor == __uuidof(LevitateLocomotionClass)) {
            AircraftTracker->Track(this);
        }

        /**
         *  Mobile units are always revealed to the house that owns them.
         */
        Revealed(House);

        /**
         *  Start in a still (non-moving) state.
         */
        Path[0] = FACING_NONE;

        Cell cell = Coord_Cell(Coord);
        for (int face = FACING_FIRST; face < FACING_COUNT; face++) {
            Cell c = Adjacent_Cell(cell, FacingType(face));
            CellClass* cptr = &Map[c];
            cptr->AdjacentObjectCount++;
        }

        if (!In_Air()) {
            LastAdjacencyCell = cell;
        }

        ThreatAvoidanceCoefficient = Techno_Type_Class()->ThreatAvoidanceCoefficient;
        return true;
    }
    return false;
}


/**
 *  FootClass::Limbo replacement.
 *
 *  @author: ZivDero
 */
bool FootClassExt::_Limbo()
{
    if (!IsInLimbo) {
        Cell cell = LastAdjacencyCell;
        for (FacingType face = FACING_FIRST; face < FACING_COUNT; face++) {
            Cell newcell = Adjacent_Cell(cell, face);
            CellClass* cptr = &Map[newcell];
            cptr->AdjacentObjectCount--;
        }
        Stop_Driver();
        if (Locomotion != nullptr) {
            Locomotion->Mark_All_Occupation_Bits(MARK_UP);
        }

        /**
         *  Remove the object from the aircraft tracker.
         */
        const auto ext = Extension::Fetch<FootClassExtension>(this);
        if (ext->Get_Last_Flight_Cell() != CELL_NONE) {
            AircraftTracker->Untrack(this);
        }
    }
    return TechnoClass::Limbo();
}



/**
 *  Main function for patching the hooks.
 */
void FootClassExtension_Hooks()
{
    Patch_Jump(0x004A6866, &_FootClass_Is_Allowed_To_Recloak_Cloak_Stop_BugFix_Patch);
    Patch_Jump(0x004A59E1, &_FootClass_AI_IdleRate_Patch);
    Patch_Jump(0x004A2BE7, &_FootClass_Mission_Guard_Area_Can_Passive_Acquire_Patch);
    Patch_Jump(0x004A1AAE, &_FootClass_Mission_Guard_Can_Passive_Acquire_Patch);
    Patch_Jump(0x004A102F, &_FootClass_Mission_Move_Can_Passive_Acquire_Patch);
    Patch_Jump(0x004A6A40, &FootClassExt::_Draw_Action_Line);
    Patch_Jump(0x004A4D60, &FootClassExt::_Death_Announcement);
    Patch_Jump(0x004A76F0, &FootClassExt::_Search_For_Tiberium);
    Patch_Jump(0x004A2C70, &FootClassExt::_Unlimbo);
    Patch_Jump(0x004A5E80, &FootClassExt::_Limbo);
}
