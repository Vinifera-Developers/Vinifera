/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended hotkey command class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "commandext.h"

#include "aircraft.h"
#include "aircrafttype.h"
#include "anim.h"
#include "animtype.h"
#include "armortype.h"
#include "asserthandler.h"
#include "audio_theme.h"
#include "base.h"
#include "battleui.h"
#include "beacon.h"
#include "building.h"
#include "buildingtype.h"
#include "bullettype.h"
#include "combat.h"
#include "debug_overlay.h"
#include "debughandler.h"
#include "dsurface.h"
#include "event.h"
#include "eventext.h"
#include "extension.h"
#include "factory.h"
#include "fatal.h"
#include "filepng.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "infantry.h"
#include "infantrytype.h"
#include "ionblast.h"
#include "ionstorm.h"
#include "language.h"
#include "minidump.h"
#include "miscutil.h"
#include "mouse.h"
#include "overlaytype.h"
#include "particlesystype.h"
#include "particletype.h"
#include "queue.h"
#include "rockettype.h"
#include "rules.h"
#include "scenario.h"
#include "scenario_overlay.h"
#include "scenarioext.h"
#include "session.h"
#include "sidebar_tabbed_view.h"
#include "smudgetype.h"
#include "super.h"
#include "tactical.h"
#include "tacticalext.h"
#include "tag.h"
#include "tagtype.h"
#include "technotypeext.h"
#include "terraintype.h"
#include "theme.h"
#include "tiberium.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "tibsun_util.h"
#include "trigger.h"
#include "triggertype.h"
#include "unit.h"
#include "unittype.h"
#include "vinifera_globals.h"
#include "voxelanim.h"
#include "voxelanimtype.h"
#include "warheadtype.h"
#include "waypointpath.h"
#include "weapontype.h"
#include "winutil.h"
#include "wwcrc.h"
#include "wwmouse.h"

#include "mapext_hooks.h"
#include <algorithm>
#include <map>


/**
 *  Handy defines for handling any adjustments.
 */
#define CATEGORY_DEVELOPER "Developer"


/**
 *  Skips to the previous available music track allowed.
 *
 *  @author: CCHyper
 */
static bool Prev_Theme_Command()
{
    ThemeType theme = Theme.What_Is_Playing();

    /**
     *  Iterate backward from the current theme and find the next available
     *  music track we can play.
     */
    while (theme >= THEME_FIRST) {

        --theme;

        if (theme < THEME_FIRST) {
            theme = ThemeType(Theme.Max_Themes());
        }

        if (Theme.Is_Allowed(theme)) {
            break;
        }

    }

    /**
     *  Queue the track for playback. We need to stop the track first
     *  otherwise Queue_Song() will fade the track out.
     */
    Theme.Stop(false);
    Theme.Queue_Song(theme);

    /**
     *  Print the chosen music track name on the screen.
     */
    TacticalMapExtension->InfoTextTimer.Stop();

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "Now Playing: %s", Theme.Full_Name(theme));

    TacticalMapExtension->Set_Info_Text(buffer);
    TacticalMapExtension->IsInfoTextSet = true;

    TacticalMapExtension->InfoTextPosition = InfoTextPosType::BOTTOM_LEFT;

    //TacticalMapExtension->InfoTextNotifySound = Rule->OptionsChanged;
    //TacticalMapExtension->InfoTextNotifySoundVolume = 0.5f;

    TacticalMapExtension->InfoTextTimer = SECONDS_TO_MILLISECONDS(4);
    TacticalMapExtension->InfoTextTimer.Start();

    return true;
}


/**
 *  Skips to the next available music track allowed.
 * 
 *  @author: CCHyper
 */
static bool Next_Theme_Command()
{
    ThemeType theme = Theme.What_Is_Playing();

    /**
     *  Iterate forward from the current theme and find the next available
     *  music track we can play.
     */
    while (theme < ThemeType(Theme.Max_Themes())) {

        ++theme;

        if (theme >= ThemeType(Theme.Max_Themes())) {
            theme = ThemeType(THEME_FIRST);
        }

        if (Theme.Is_Allowed(theme)) {
            break;
        }

    }

    /**
     *  Queue the track for playback. We need to stop the track first
     *  otherwise Queue_Song() will fade the track out.
     */
    Theme.Stop();
    Theme.Queue_Song(theme);

    /**
     *  Print the chosen music track name on the screen.
     */
    TacticalMapExtension->InfoTextTimer.Stop();

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "Now Playing: %s", Theme.Full_Name(theme));

    TacticalMapExtension->Set_Info_Text(buffer);
    TacticalMapExtension->IsInfoTextSet = true;
    
    TacticalMapExtension->InfoTextPosition = InfoTextPosType::BOTTOM_LEFT;

    //TacticalMapExtension->InfoTextNotifySound = Rule->OptionsChanged;
    //TacticalMapExtension->InfoTextNotifySoundVolume = 0.5f;

    TacticalMapExtension->InfoTextTimer = SECONDS_TO_MILLISECONDS(4);
    TacticalMapExtension->InfoTextTimer.Start();

    return true;
}



/**
 *  #issue-167
 * 
 *  Writes a PNG screenshot of the current screen buffer.
 * 
 *  @author: CCHyper
 */
const char *PNGScreenCaptureCommandClass::Get_Name() const
{
    return "ScreenCapture";
}

const char *PNGScreenCaptureCommandClass::Get_UI_Name() const
{
    return "Screen Capture";
}

const char *PNGScreenCaptureCommandClass::Get_Category() const
{
    return "Interface";
}

const char *PNGScreenCaptureCommandClass::Get_Description() const
{
    return "Takes a snapshot of the game screen (Saved as 'SCRN_<date-time>.PNG.)";
}

bool PNGScreenCaptureCommandClass::Process()
{
    if (!IsWindow(MainWindow)) {
        return false;
    }

    RECT crect;
    if (!GetClientRect(MainWindow, &crect)) {
        return false;
    }

    POINT tl_point;
    tl_point.x = crect.left;
    tl_point.y = crect.top;
    if (!ClientToScreen(MainWindow, &tl_point)) {
        return false;
    }

    POINT br_point;
    br_point.x = crect.right;
    br_point.y = crect.bottom;
    if (!ClientToScreen(MainWindow, &br_point)) {
        return false;
    }

    int w = std::min((int)crect.right+1, HiddenSurface->Get_Width());
    int h = std::min((int)crect.bottom+1, HiddenSurface->Get_Height());

    Rect src(tl_point.x, tl_point.y, w, h);
    Rect dest(0, 0, HiddenSurface->Get_Width(), HiddenSurface->Get_Height());

    /**
     *  We don't want the mouse to appear in screenshots!
     */
    Hide_Mouse();

    /**
     *  Blit primary surface to the hidden.
     */
    bool blit = HiddenSurface->Blit_From(dest, *VisibleSurface, src);
    ASSERT(blit);

    /**
     *  Now show the mouse again.
     */
    Show_Mouse();

    char buffer[256];

#if 0
    /**
     *  Find a free filename slot.
     */
    for (unsigned i = 0; i <= 9999; ++i) {
        std::snprintf(buffer, sizeof(buffer), "SCRN%04d.PNG", i);
        if (!RawFileClass(buffer).Is_Available()) {
            break;
        }
    }
#endif

    /**
     *  Generate a unique filename with the current timestamp.
     */
    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;
    Get_Full_Time(day, month, year, hour, min, sec);
    std::snprintf(buffer, sizeof(buffer), "SCRN_%02u-%02u-%04u_%02u-%02u-%02u.PNG", day, month, year, hour, min, sec);

    /**
     *  #issue-195
     * 
     *  Output screenshots to its own sub-directory.
     * 
     *  @author: CCHyper
     */
    char fullpath_buffer[PATH_MAX];
    std::snprintf(fullpath_buffer, sizeof(fullpath_buffer), "%s\\%s", Vinifera_ScreenshotDirectory, buffer);

    /**
     *  We found a free filename, now write the buffer to a PNG file.
     */
    bool success = Write_PNG_File(&RawFileClass(fullpath_buffer), *HiddenSurface, &GamePalette);

    if (success) {
        DEBUG_INFO("PNG screenshot \"{}\" written sucessfully.\n", buffer);
    } else {
        DEBUG_ERROR("Failed to write PNG screenshot \"{}\"!\n", buffer);
    }

    return success;
}


/**
 *  Replacement for DeleteWaypointCommandClass.
 *
 *  @author: ZivDero
 */
const char* DeleteCommandClass::Get_Name() const
{
    return "DeleteWaypoint"; // kept as DeleteWaypoint to preserve keyboard.ini compatibility
}

const char* DeleteCommandClass::Get_UI_Name() const
{
    return "Delete";
}

const char* DeleteCommandClass::Get_Category() const
{
    return "Interface";
}

const char* DeleteCommandClass::Get_Description() const
{
    return "Deletes the selected waypoint or beacon.";
}

bool DeleteCommandClass::Process()
{
    if (Map.DraggedWaypoint) {
        char waypoint_number;
        PathType path_type = PATH_NONE;
        PlayerPtr->Fetch_Waypoint_Data(Map.DraggedWaypoint, path_type, waypoint_number);
        PlayerPtr->Ensure_Path(path_type);
        PlayerPtr->Paths[path_type]->Delete_Waypoint(waypoint_number);
        Map.DraggedWaypoint = nullptr;

        for (int i = Foots.Count() - 1; i >= 0; i--) {
            FootClass* foot = Foots[i];
            if (foot->House == PlayerPtr && foot->CurrentPath == path_type && foot->NextWaypoint > waypoint_number) {
                foot->NextWaypoint--;
            }
        }

        Show_Mouse();
    }

    BeaconManager.Delete_Beacon(HOUSE_NONE, -1);

    return true;
}


/**
 *  Replacement for SelectSameTypeCommandClass.
 *
 *  @author: JoyfulShush
 */
const char* SelectSameTypeImprovedCommandClass::Get_Name() const
{
    return "SelectType";
}

const char* SelectSameTypeImprovedCommandClass::Get_UI_Name() const
{
    return "Select Same Type";
}

const char* SelectSameTypeImprovedCommandClass::Get_Category() const
{
    return "Selection";
}

const char* SelectSameTypeImprovedCommandClass::Get_Description() const
{
    return "Selects all units of the same type as currently selected.";
}

/*
 *  Improves the Select Same Type command in the following ways:
 *  1. No longer deselects units that are out of the screen when running the command.
 *  2. Rather than calling 'TacticalMap->Select_These' for each type, runs it once for all types.
 *  3. When processed twice in a small amount of time, selects the units of those types in the entire map rather than just the current tactical view.
 *  4. Ignores selected technos that do not belong to the player.
 * 
 *  @author: JoyfulShush
 */
bool SelectSameTypeImprovedCommandClass::Process()
{   
    SelectionTypes.clear();
    DWORD current_time = timeGetTime();
    DWORD previous_execution_time = LastExecutionTime;

    LastExecutionTime = current_time;
    
    for (int i = 0; i < CurrentObjects.Count(); i++) {
        auto current_object = CurrentObjects[i];
        auto techno_class = current_object->Techno_Type_Class();

        if (current_object->Is_Techno() && !current_object->As_Techno()->House->Is_Player_Control()) {
            continue;
        }

        if (!SelectionTypes.contains(techno_class))  {
            SelectionTypes.insert(techno_class);
        }
    }

    if (SelectionTypes.size() > 0) {
        if (previous_execution_time != 0 && current_time - previous_execution_time < 500) {
            Map_Select_These(Process_Callback);
        } else {
            TacticalMap->Select_These(TacticalRect, Process_Callback);
        }
    }

    return true;
}


/*
 *  For each object being checked by the game, decide if the techno should be selected when running the Select Same Type command.
 *
 *  @author: JoyfulShush
 */
void SelectSameTypeImprovedCommandClass::Process_Callback(ObjectClass* object_ptr) 
{
    if (object_ptr == nullptr) return;
    if (!object_ptr->Is_Techno()) return;
    if (!object_ptr->IsDown) return;
    
    auto techno = object_ptr->As_Techno();
    auto techno_class = techno->Techno_Type_Class();

    if (techno->IsSelected) return;
    if (!SelectionTypes.contains(techno_class)) return;
    if (!techno->House->Is_Player_Control()) return;

    techno->Select();
}


/**
 *  #issue-112
 * 
 *  Enter the manual placement mode when a building is complete
 *  and pending placement on the sidebar.
 * 
 *  @author: CCHyper (based on research by dkeeton)
 */
const char *ManualPlaceCommandClass::Get_Name() const
{
    return "ManualPlace";
}

const char *ManualPlaceCommandClass::Get_UI_Name() const
{
    return "Place Building";
}

const char *ManualPlaceCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ManualPlaceCommandClass::Get_Description() const
{
    return "Enter the manual placement mode when a building is complete and pending on the sidebar.";
}

bool ManualPlaceCommandClass::Process()
{
    if (PlayerPtr)
    {
        /**
         *  Fetch the house's factory associated with producing buildings.
         */
        FactoryClass* factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(RTTI_BUILDING, PRODFLAG_NONE);
        if (!factory)
            return false;

        /**
         *  If this object is still being built, then bail.
         */
        if (!factory->Has_Completed()) {
            return false;
        }

        TechnoClass* pending = factory->Get_Object();

        /**
         *  If by some rare chance the product is not a building, then bail.
         */
        if (pending->RTTI != RTTI_BUILDING)
            return false;

        BuildingClass* pending_bptr = reinterpret_cast<BuildingClass*>(pending);

        /**
         *  Are we already trying to place this building? No need to re-enter placement mode...
         */
        if (Map.PendingObjectPtr == pending_bptr)
            return false;

        /**
         *  Fetch the factory building that can build this object.
         */
        BuildingClass* builder = pending_bptr->Who_Can_Build_Me();
        if (!builder)
            return false;

        /**
         *  Abort targeting the SW, so that once we place the building we don't go back to a superweapon cursor.
         */
        Map.TargettingType = SUPER_NONE;

        /**
         *  Go into placement mode.
         */
        PlayerPtr->Manual_Place(builder, pending_bptr);

        return true;
    }

    return false;
}


/**
 *  #issue-168
 * 
 *  Reproduces the last structure that was built.
 * 
 *  @author: CCHyper (based on research by dkeeton)
 */
const char *RepeatLastBuildingCommandClass::Get_Name() const
{
    return "RepeatLastBuilding";
}

const char *RepeatLastBuildingCommandClass::Get_UI_Name() const
{
    return "Repeat Last Building";
}

const char *RepeatLastBuildingCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *RepeatLastBuildingCommandClass::Get_Description() const
{
    return "Queue the last structure that was built.";
}

bool RepeatLastBuildingCommandClass::Process()
{
    if (!PlayerPtr) {
        return false;
    }

    /**
     *  Fetch the house's factory associated with producing building. This is
     *  done to make sure the house still has a factory.
     */
    if (!Extension::Fetch(PlayerPtr)->Factory_Count(RTTI_BUILDING, PRODFLAG_NONE)) {
        DEV_DEBUG_WARNING("RepeatLastBuildingCommandClass - Unable to fetch primary factory!\n");
        return false;
    }
    
    /**
     *  Nothing built? Nothing to reproduce...
     */
    StructType building = PlayerPtr->JustBuiltStructure;
    if (building == STRUCT_NONE) {
        return false;
    }

    /**
     *  Don't allow queuing of multiple structures.
     */
    if (Extension::Fetch(PlayerPtr)->Fetch_Factory(RTTI_BUILDING, PRODFLAG_NONE) &&
        Extension::Fetch(PlayerPtr)->Fetch_Factory(RTTI_BUILDING, PRODFLAG_NONE)->Get_Object()) {
        return false;
    }

    const BuildingTypeClass *buildingtype = BuildingTypes[building];
    if (!buildingtype) {
        return false;
    }

    /**
     *  Is the item currently available to build on the sidebar?
     */
    if (!BattleUI.Get_Sidebar().Is_On_Sidebar(RTTI_BUILDINGTYPE, building)) {
        return false;
    }

    DEBUG_INFO("RepeatLastBuildingCommandClass - \"{}\"\n", buildingtype->Full_Name());

    OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_PRODUCE, RTTI_BUILDINGTYPE, building, TechnoTypeClassExtension::Get_Production_Flags(RTTI_BUILDINGTYPE, building)).As_Event());

    return true;
}


/**
 *  #issue-168
 * 
 *  Reproduces the last infantry that was built.
 * 
 *  @author: CCHyper (based on research by dkeeton)
 */
const char *RepeatLastInfantryCommandClass::Get_Name() const
{
    return "RepeatLastInfantry";
}

const char *RepeatLastInfantryCommandClass::Get_UI_Name() const
{
    return "Repeat Last Infantry";
}

const char *RepeatLastInfantryCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *RepeatLastInfantryCommandClass::Get_Description() const
{
    return "Queue the last infantry that was built.";
}

bool RepeatLastInfantryCommandClass::Process()
{
    if (!PlayerPtr) {
        return false;
    }

    /**
     *  Fetch the house's factory associated with producing infantry. This is
     *  done to make sure the house still has a factory.
     */
    if (!Extension::Fetch(PlayerPtr)->Factory_Count(RTTI_INFANTRY, PRODFLAG_NONE)) {
        DEV_DEBUG_WARNING("RepeatLastInfantryCommandClass - Unable to fetch primary factory!\n");
        return false;
    }
    
    /**
     *  Nothing built? Nothing to reproduce...
     */
    InfantryType infantry = PlayerPtr->JustBuiltInfantry;
    if (infantry == INFANTRY_NONE) {
        return false;
    }

    const InfantryTypeClass *infantrytype = InfantryTypes[infantry];
    if (!infantrytype) {
        return false;
    }
    
    /**
     *  Is the item currently available to build on the sidebar?
     */
    if (!BattleUI.Get_Sidebar().Is_On_Sidebar(RTTI_INFANTRYTYPE, infantry)) {
        return false;
    }

    DEBUG_INFO("RepeatLastInfantryCommandClass - \"{}\"\n", infantrytype->Full_Name());

    OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_PRODUCE, RTTI_INFANTRYTYPE, infantry, TechnoTypeClassExtension::Get_Production_Flags(RTTI_INFANTRYTYPE, infantry)).As_Event());

    return true;
}


/**
 *  #issue-168
 * 
 *  Reproduces the last unit that was built.
 * 
 *  @author: CCHyper (based on research by dkeeton)
 */
const char *RepeatLastUnitCommandClass::Get_Name() const
{
    return "RepeatLastUnit";
}

const char *RepeatLastUnitCommandClass::Get_UI_Name() const
{
    return "Repeat Last Vehicle";
}

const char *RepeatLastUnitCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *RepeatLastUnitCommandClass::Get_Description() const
{
    return "Queue the last vehicle that was built.";
}

bool RepeatLastUnitCommandClass::Process()
{
    if (!PlayerPtr) {
        return false;
    }

    /**
     *  Fetch the house's factory associated with producing unit. This is
     *  done to make sure the house still has a factory.
     */
    if (!Extension::Fetch(PlayerPtr)->Factory_Count(RTTI_UNIT, PRODFLAG_NONE)) {
        DEV_DEBUG_WARNING("RepeatLastUnitCommandClass - Unable to fetch primary factory!\n");
        return false;
    }
    
    /**
     *  Nothing built? Nothing to reproduce...
     */
    UnitType unit = PlayerPtr->JustBuiltUnit;
    if (unit == UNIT_NONE) {
        return false;
    }

    const UnitTypeClass *unittype = UnitTypes[unit];
    if (!unittype) {
        return false;
    }
    
    /**
     *  Is the item currently available to build on the sidebar?
     */
    if (!BattleUI.Get_Sidebar().Is_On_Sidebar(RTTI_UNITTYPE, unit)) {
        return false;
    }

    DEBUG_INFO("RepeatLastUnitCommandClass - \"{}\"\n", unittype->Full_Name());

    OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_PRODUCE, RTTI_UNITTYPE, unit, TechnoTypeClassExtension::Get_Production_Flags(RTTI_UNITTYPE, unit)).As_Event());

    return true;
}


/**
 *  #issue-168
 * 
 *  Reproduces the last aircraft that was built.
 * 
 *  @author: CCHyper (based on research by dkeeton)
 */
const char *RepeatLastAircraftCommandClass::Get_Name() const
{
    return "RepeatLastAircraft";
}

const char *RepeatLastAircraftCommandClass::Get_UI_Name() const
{
    return "Repeat Last Aircraft";
}

const char *RepeatLastAircraftCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *RepeatLastAircraftCommandClass::Get_Description() const
{
    return "Queue the last aircraft that was built.";
}

bool RepeatLastAircraftCommandClass::Process()
{
    if (!PlayerPtr) {
        return false;
    }

    /**
     *  Fetch the house's factory associated with producing aircraft. This is
     *  done to make sure the house still has a factory.
     */
    if (!Extension::Fetch(PlayerPtr)->Factory_Count(RTTI_AIRCRAFT, PRODFLAG_NONE)) {
        DEV_DEBUG_WARNING("RepeatLastAircraftCommandClass - Unable to fetch primary factory!\n");
        return false;
    }
    
    /**
     *  Nothing built? Nothing to reproduce...
     */
    AircraftType aircraft = PlayerPtr->JustBuiltAircraft;
    if (aircraft == AIRCRAFT_NONE) {
        return false;
    }

    const AircraftTypeClass *aircrafttype = AircraftTypes[aircraft];
    if (!aircrafttype) {
        return false;
    }
    
    /**
     *  Is the item currently available to build on the sidebar?
     */
    if (!BattleUI.Get_Sidebar().Is_On_Sidebar(RTTI_AIRCRAFTTYPE, aircraft)) {
        return false;
    }

    DEBUG_INFO("RepeatLastAircraftCommandClass - \"{}\"\n", aircrafttype->Full_Name());

    OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_PRODUCE, RTTI_AIRCRAFTTYPE, aircraft, TechnoTypeClassExtension::Get_Production_Flags(RTTI_AIRCRAFTTYPE, aircraft)).As_Event());

    return true;
}


/**
 *  Skip to the previous playable music track.
 * 
 *  @author: CCHyper
 */
const char *PrevThemeCommandClass::Get_Name() const
{
    return "PrevTheme";
}

const char *PrevThemeCommandClass::Get_UI_Name() const
{
    return "Music: Previous Track";
}

const char *PrevThemeCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *PrevThemeCommandClass::Get_Description() const
{
    return "Play the previous music track in the jukebox.";
}

bool PrevThemeCommandClass::Process()
{
    Prev_Theme_Command();

    return true;
}


/**
 *  Skip to the next playable music track.
 * 
 *  @author: CCHyper
 */
const char *NextThemeCommandClass::Get_Name() const
{
    return "NextTheme";
}

const char *NextThemeCommandClass::Get_UI_Name() const
{
    return "Music: Next Track";
}

const char *NextThemeCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *NextThemeCommandClass::Get_Description() const
{
    return "Play the next music track in the jukebox.";
}

bool NextThemeCommandClass::Process()
{
    Next_Theme_Command();

    return true;
}


/**
 *  Scroll tactical map to the north-east.
 * 
 *  @author: CCHyper
 */
const char *ScrollNECommandClass::Get_Name() const
{
    return "ScrollNorthEast";
}

const char *ScrollNECommandClass::Get_UI_Name() const
{
    return "Scroll North-East";
}

const char *ScrollNECommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollNECommandClass::Get_Description() const
{
    return "Scroll tactical map to the north-east.";
}

bool ScrollNECommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_NE, dist);

    return true;
}


/**
 *  Scroll tactical map to the south-east.
 * 
 *  @author: CCHyper
 */
const char *ScrollSECommandClass::Get_Name() const
{
    return "ScrollSouthEast";
}

const char *ScrollSECommandClass::Get_UI_Name() const
{
    return "Scroll South-East";
}

const char *ScrollSECommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollSECommandClass::Get_Description() const
{
    return "Scroll tactical map to the south-east.";
}

bool ScrollSECommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_SE, dist);

    return true;
}


/**
 *  Scroll tactical map to the south-west.
 * 
 *  @author: CCHyper
 */
const char *ScrollSWCommandClass::Get_Name() const
{
    return "ScrollSouthWest";
}

const char *ScrollSWCommandClass::Get_UI_Name() const
{
    return "Scroll South-West";
}

const char *ScrollSWCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollSWCommandClass::Get_Description() const
{
    return "Scroll tactical map to the south-west.";
}

bool ScrollSWCommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_SW, dist);

    return true;
}


/**
 *  Scroll tactical map to the north-west.
 * 
 *  @author: CCHyper
 */
const char *ScrollNWCommandClass::Get_Name() const
{
    return "ScrollNorthWest";
}

const char *ScrollNWCommandClass::Get_UI_Name() const
{
    return "Scroll North-West";
}

const char *ScrollNWCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollNWCommandClass::Get_Description() const
{
    return "Scroll tactical map to the north-west.";
}

bool ScrollNWCommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_NW, dist);

    return true;
}


/**
 *  Jump the tactical map camera to the west edge of the map.
 * 
 *  @author: CCHyper
 */
const char *JumpCameraWestCommandClass::Get_Name() const
{
    return "JumpCameraWest";
}

const char *JumpCameraWestCommandClass::Get_UI_Name() const
{
    return "Jump Camera West";
}

const char *JumpCameraWestCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *JumpCameraWestCommandClass::Get_Description() const
{
    return "Jump the tactical map camera to the west edge of the map.";
}

bool JumpCameraWestCommandClass::Process()
{
    /**
     *  Find the largest distance on the map.
     */
    int dist = Cell_To_Lepton(Map.PlayRect.Width <= Map.PlayRect.Height ? Map.PlayRect.Height : Map.PlayRect.Width);

    Map.Scroll_Map(FACING_W, dist);

    return true;
}


/**
 *  Jump the tactical map camera to the east edge of the map.
 * 
 *  @author: CCHyper
 */
const char *JumpCameraEastCommandClass::Get_Name() const
{
    return "JumpCameraEast";
}

const char *JumpCameraEastCommandClass::Get_UI_Name() const
{
    return "Jump Camera East";
}

const char *JumpCameraEastCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *JumpCameraEastCommandClass::Get_Description() const
{
    return "Jump the tactical map camera to the east edge of the map.";
}

bool JumpCameraEastCommandClass::Process()
{
    /**
     *  Find the largest distance on the map.
     */
    int dist = Cell_To_Lepton(Map.PlayRect.Width <= Map.PlayRect.Height ? Map.PlayRect.Height : Map.PlayRect.Width);

    Map.Scroll_Map(FACING_E, dist);

    return true;
}


/**
 *  Jump the tactical map camera to the north edge of the map.
 * 
 *  @author: CCHyper
 */
const char *JumpCameraNorthCommandClass::Get_Name() const
{
    return "JumpCameraNorth";
}

const char *JumpCameraNorthCommandClass::Get_UI_Name() const
{
    return "Jump Camera North";
}

const char *JumpCameraNorthCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *JumpCameraNorthCommandClass::Get_Description() const
{
    return "Jump the tactical map camera to the north edge of the map.";
}

bool JumpCameraNorthCommandClass::Process()
{
    /**
     *  Find the largest distance on the map.
     */
    int dist = Cell_To_Lepton(Map.PlayRect.Width <= Map.PlayRect.Height ? Map.PlayRect.Height : Map.PlayRect.Width);

    Map.Scroll_Map(FACING_N, dist);

    return true;
}


/**
 *  Jump the tactical map camera to the south edge of the map.
 * 
 *  @author: CCHyper
 */
const char *JumpCameraSouthCommandClass::Get_Name() const
{
    return "JumpCameraSouth";
}

const char *JumpCameraSouthCommandClass::Get_UI_Name() const
{
    return "Jump Camera South";
}

const char *JumpCameraSouthCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *JumpCameraSouthCommandClass::Get_Description() const
{
    return "Jump the tactical map camera to the south edge of the map.";
}

bool JumpCameraSouthCommandClass::Process()
{
    /**
     *  Find the largest distance on the map.
     */
    int dist = Cell_To_Lepton(Map.PlayRect.Width <= Map.PlayRect.Height ? Map.PlayRect.Height : Map.PlayRect.Width);

    Map.Scroll_Map(FACING_S, dist);

    return true;
}


/**
 *  Toggles the visibility of the special weapon timers on the tactical view.
 * 
 *  @author: CCHyper
 */
const char *ToggleSuperTimersCommandClass::Get_Name() const
{
    return "ToggleSuperTimers";
}

const char *ToggleSuperTimersCommandClass::Get_UI_Name() const
{
    return "Toggle Special Timers";
}

const char *ToggleSuperTimersCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ToggleSuperTimersCommandClass::Get_Description() const
{
    return "Toggles the visibility of the special weapon timers on the tactical view.";
}

bool ToggleSuperTimersCommandClass::Process()
{
    if (Session.Type == GAME_NORMAL) {
        return false;
    }

    Vinifera_ShowSuperWeaponTimers = !Vinifera_ShowSuperWeaponTimers;

    return true;
}


/**
 *  Switches the sidebar to the Building Tab.
 *
 *  @author: ZivDero
 */
const char* SetStructureTabCommandClass::Get_Name() const
{
    return "StructureTab";
}

const char* SetStructureTabCommandClass::Get_UI_Name() const
{
    return "Select Building Tab";
}

const char* SetStructureTabCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char* SetStructureTabCommandClass::Get_Description() const
{
    return "Switch the command bar to the Building Tab and select the completed building if any.";
}

bool SetStructureTabCommandClass::Process()
{
    const TabbedSidebarView::SidebarTabType newtab = TabbedSidebarView::SIDEBAR_TAB_STRUCTURE;
    bool result = BattleUI.Get_Sidebar().Change_Tab(newtab);

    /**
     *  Enter the manual placement mode when a building is complete
     *  and pending placement on the sidebar.
     *
     *  @author: CCHyper (based on research by dkeeton)
     */
    if (PlayerPtr)
    {
        /**
         *  Fetch the house's factory associated with producing buildings.
         */
        FactoryClass* factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(RTTI_BUILDING, PRODFLAG_NONE);
        if (!factory)
            return result;

        /**
         *  If this object is still being built, then bail.
         */
        if (!factory->Has_Completed()) {
            return result;
        }

        TechnoClass* pending = factory->Get_Object();

        /**
         *  If by some rare chance the product is not a building, then bail.
         */
        if (pending->RTTI != RTTI_BUILDING)
            return result;

        BuildingClass* pending_bptr = reinterpret_cast<BuildingClass*>(pending);

        /**
         *  Are we already trying to place this building? No need to re-enter placement mode...
         */
        if (Map.PendingObjectPtr == pending_bptr)
            return result;

        /**
         *  Fetch the factory building that can build this object.
         */
        BuildingClass* builder = pending_bptr->Who_Can_Build_Me();
        if (!builder)
            return result;

        /**
         *  Abort targeting the SW, so that once we place the building we don't go back to a superweapon cursor.
         */
        Map.TargettingType = SUPER_NONE;

        /**
         *  Go into placement mode.
         */
        PlayerPtr->Manual_Place(builder, pending_bptr);
    }

    return result;
}


/**
 *  Switches the sidebar to the Infantry Tab.
 *
 *  @author: ZivDero
 */
const char* SetInfantryTabCommandClass::Get_Name() const
{
    return "InfantryTab";
}

const char* SetInfantryTabCommandClass::Get_UI_Name() const
{
    return "Select Infantry Tab";
}

const char* SetInfantryTabCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char* SetInfantryTabCommandClass::Get_Description() const
{
    return "Switch the command bar to the Infantry Tab.";
}

bool SetInfantryTabCommandClass::Process()
{
    const TabbedSidebarView::SidebarTabType newtab = TabbedSidebarView::SIDEBAR_TAB_INFANTRY;
    return BattleUI.Get_Sidebar().Change_Tab(newtab);
}


/**
 *  Switches the sidebar to the Vehicle Tab.
 *
 *  @author: ZivDero
 */
const char* SetUnitTabCommandClass::Get_Name() const
{
    return "UnitTab";
}

const char* SetUnitTabCommandClass::Get_UI_Name() const
{
    return "Select Vehicles Tab";
}

const char* SetUnitTabCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char* SetUnitTabCommandClass::Get_Description() const
{
    return "Switch the command bar to the Vehicle Tab.";
}

bool SetUnitTabCommandClass::Process()
{
    const TabbedSidebarView::SidebarTabType newtab = TabbedSidebarView::SIDEBAR_TAB_UNIT;
    return BattleUI.Get_Sidebar().Change_Tab(newtab);
}


/**
 *  Switches the sidebar to the Special Tab.
 *
 *  @author: ZivDero
 */
const char* SetSpecialTabCommandClass::Get_Name() const
{
    return "SpecialTab";
}

const char* SetSpecialTabCommandClass::Get_UI_Name() const
{
    return "Select Specials Tab";
}

const char* SetSpecialTabCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char* SetSpecialTabCommandClass::Get_Description() const
{
    return "Switch the command bar to the Special Tab.";
}

bool SetSpecialTabCommandClass::Process()
{
    const TabbedSidebarView::SidebarTabType newtab = TabbedSidebarView::SIDEBAR_TAB_SPECIAL;
    return BattleUI.Get_Sidebar().Change_Tab(newtab);
}


/**
 *  Produces a memory dump on request.
 * 
 *  @author: CCHyper
 */
const char *MemoryDumpCommandClass::Get_Name() const
{
    return "MemoryDump";
}

const char *MemoryDumpCommandClass::Get_UI_Name() const
{
    return "Memory Dump";
}

const char *MemoryDumpCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *MemoryDumpCommandClass::Get_Description() const
{
    return "Produces a mini-dump of the memory for analysis.";
}

bool MemoryDumpCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    DEBUG_INFO("About to produce memory dump...\n");

    GenerateFullCrashDump = false; // We don't need a full memory dump.
    NonFatalMinidump = true;
    MinidumpUseCurrentTime = true;

    Create_Mini_Dump(nullptr, Get_Module_File_Name());

    return true;
}


/**
 *  Dumps all the current game objects as CRCs to the log output.
 * 
 *  @author: CCHyper
 */
const char *DumpHeapCRCCommandClass::Get_Name() const
{
    return "DumpHeapCRC";
}

const char *DumpHeapCRCCommandClass::Get_UI_Name() const
{
    return "Dump Heap CRCs";
}

const char *DumpHeapCRCCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DumpHeapCRCCommandClass::Get_Description() const
{
    return "Dumps all the current game objects as CRCs to the log output.";
}

/**
 *  Handy macro for defining the logging the heaps CRCs.
 * 
 *  @author: CCHyper, Rampastring
 */
#define LOG_CRC(class_name, heap_name) \
    { \
        DEBUG_INFO(#class_name ":\n"); \
        if (!heap_name.Count()) { \
            DEBUG_INFO("  EMPTY\n"); \
        } else { \
            CRCEngine crc; \
            for (unsigned i = 0; i < heap_name.Count(); ++i) { \
                class_name *ptr = heap_name[i]; \
                if (ptr != nullptr) { \
                    ptr->Object_CRC(crc); \
                    if (ptr->RTTI == RTTI_INFANTRY || ptr->RTTI == RTTI_UNIT || ptr->RTTI == RTTI_BUILDING || ptr->RTTI == RTTI_AIRCRAFT) {                                                                                                                                                                    \
                        TechnoClass* techno = (TechnoClass*)ptr;                                                                                                                                                                                                                                               \
                        DEBUG_INFO("  {:04}\tName: {}\tCRC: 0x{:08X}\tOwner: {} ({}) (Class: {})\tCoord: {},{},{}\n", i, ptr->Name(), crc.CRC_Value(), techno->House->IniName, (int)techno->House->HeapID, techno->House->Class->IniName, techno->Position.X, techno->Position.Y, techno->Position.Z); \
                    } else {                                                                                                                                                                                                                                                                                   \
                        DEBUG_INFO("  {:04}\tName: {}\tCRC: 0x{:08X}\n", i, ptr->Name(), crc.CRC_Value());                                                                                                                                                                                                        \
                    } \
                } else { \
                    DEBUG_INFO("  {:04}\tFAILED!\n", i); \
                } \
            } \
        } \
        DEBUG_INFO("\n"); \
    }

bool DumpHeapCRCCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    DEBUG_INFO("\nAbout to dump heap CRC's...\n\n");

    LOG_CRC(UnitClass, Units);
    LOG_CRC(InfantryClass, Infantry);
    LOG_CRC(BuildingClass, Buildings);
    LOG_CRC(AircraftClass, Aircrafts);

    LOG_CRC(UnitTypeClass, UnitTypes);
    LOG_CRC(InfantryTypeClass, InfantryTypes);
    LOG_CRC(BuildingTypeClass, BuildingTypes);
    LOG_CRC(AircraftTypeClass, AircraftTypes);

    LOG_CRC(WeaponTypeClass, Weapons);
    LOG_CRC(WarheadTypeClass, Warheads);

    /**
     *  Color Schemes.
     */
    {
        DEBUG_INFO("ColorSchemes :\n");
        if (!ColorSchemes.Count()) {
            DEBUG_INFO("  EMPTY\n");
        } else {
            CRCEngine crc;
            for (unsigned i = 0; i < ColorSchemes.Count(); ++i) {
                ColorScheme *ptr = ColorSchemes[i];
                if (ptr != nullptr) {
                    DEBUG_INFO("  {:04}\tName: {}\tfield_310: {}\n", i, ptr->Name, ptr->field_310);
                } else {
                    DEBUG_INFO("  {:04}\tFAILED!\n", i);
                }
            }
        }
        DEBUG_INFO("\n");
    }
    
    DEBUG_INFO("\nFinished!\n\n");

    return true;
}


/**
 *  Produces a log dump of active trigger instances.
 *
 *  @author: Rampastring
 */
const char* DumpTriggersCommandClass::Get_Name() const
{
    return "DumpTriggers";
}

const char* DumpTriggersCommandClass::Get_UI_Name() const
{
    return "Dump Trigger Info";
}

const char* DumpTriggersCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char* DumpTriggersCommandClass::Get_Description() const
{
    return "Dumps all existing triggers, tags, and local and global variables to the log output.";
}

bool DumpTriggersCommandClass::Process()
{
    DEBUG_INFO("\nAbout to dump trigger information...\n\n");

    for (int i = 0; i < Triggers.Count(); i++)
    {
        TriggerClass* trigger = Triggers[i];

        DEBUG_INFO("Trigger {}: {}\n", i, trigger->Class->GivenName);
        DEBUG_INFO("    IsToDie: {}\n", trigger->IsToDie);
        DEBUG_INFO("    TrippedFlags: {}\n", trigger->TrippedFlags);
        DEBUG_INFO("    IsActive: {}\n", trigger->IsActive);

        while (trigger->LinkedTo != nullptr) {
            trigger = trigger->LinkedTo;

            DEBUG_INFO("    LinkedTo: {}\n", trigger->Class->GivenName);
            DEBUG_INFO("        IsToDie: {}\n", trigger->IsToDie);
            DEBUG_INFO("        TrippedFlags: {}\n", trigger->TrippedFlags);
            DEBUG_INFO("        IsActive: {}\n", trigger->IsActive);
        }
    }

    DEBUG_INFO("\n\nAbout to dump tag information...\n\n");

    for (int i = 0; i < Tags.Count(); i++)
    {
        TagClass* tag = Tags[i];

        DEBUG_INFO("Tag {}: {}\n", i, tag->Class->GivenName);
        DEBUG_INFO("    AttachCount: {}\n", tag->AttachCount);
        DEBUG_INFO("    CellID: {},{}\n", tag->CellID.X, tag->CellID.Y);
        DEBUG_INFO("    IsToDie: {}\n", tag->IsToDie);
        DEBUG_INFO("    IsSprung: {}\n", tag->IsSprung);
    }

    DEBUG_INFO("\n\nAbout to dump local variable information...\n\n");

    for (int i = 0; i < std::size(ScenExtension->LocalFlags); i++)
    {
        DEBUG_INFO("LocalFlag {}: {}, value: {}\n", i, ScenExtension->LocalFlags[i].VariableName, ScenExtension->LocalFlags[i].Value);
    }

    DEBUG_INFO("\nFinished!\n\n");

    return true;
}


/**
 *  Toggles the instant build cheat for the player.
 * 
 *  @author: CCHyper
 */
const char *InstantBuildCommandClass::Get_Name() const
{
    return "InstantBuild";
}

const char *InstantBuildCommandClass::Get_UI_Name() const
{
    return "Instant Build (Player)";
}

const char *InstantBuildCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *InstantBuildCommandClass::Get_Description() const
{
    return "Toggles the instant build cheat for the player.";
}

bool InstantBuildCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Vinifera_Developer_InstantBuild = !Vinifera_Developer_InstantBuild;

    return true;
}


/**
 *  Toggles the instant build cheat for the AI.
 *  
 *  @note: This will effect ALL the AI houses currently in the game session!
 * 
 *  @author: CCHyper
 */
const char *AIInstantBuildCommandClass::Get_Name() const
{
    return "AIInstantBuild";
}

const char *AIInstantBuildCommandClass::Get_UI_Name() const
{
    return "Instant Build (AI)";
}

const char *AIInstantBuildCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *AIInstantBuildCommandClass::Get_Description() const
{
    return "Toggles the instant build cheat for the AI.";
}

bool AIInstantBuildCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Vinifera_Developer_AIInstantBuild = !Vinifera_Developer_AIInstantBuild;

    return true;
}


/**
 *  Forces the player to win the current game session.
 * 
 *  @author: CCHyper
 */
const char *ForceWinCommandClass::Get_Name() const
{
    return "ForceWin";
}

const char *ForceWinCommandClass::Get_UI_Name() const
{
    return "To Win";
}

const char *ForceWinCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ForceWinCommandClass::Get_Description() const
{
    return "Forces the player to win the current game session.";
}

bool ForceWinCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Player wins.
     */
    return PlayerPtr->Flag_To_Win();
}


/**
 *  Forces the player to lose the current game session.
 * 
 *  @author: CCHyper
 */
const char *ForceLoseCommandClass::Get_Name() const
{
    return "ForceLose";
}

const char *ForceLoseCommandClass::Get_UI_Name() const
{
    return "To Lose";
}

const char *ForceLoseCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ForceLoseCommandClass::Get_Description() const
{
    return "Forces the player to lose the current game session.";
}

bool ForceLoseCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Player loses
     */
    return PlayerPtr->Flag_To_Lose();
}


/**
 *  Forces all of the player's units and structures to explode, losing the current game session.
 * 
 *  @author: CCHyper
 */
const char *ForceDieCommandClass::Get_Name() const
{
    return "ForceDie";
}

const char *ForceDieCommandClass::Get_UI_Name() const
{
    return "To Die";
}

const char *ForceDieCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ForceDieCommandClass::Get_Description() const
{
    return "Forces all of the player's units and structures to explode, losing the current game session.";
}

bool ForceDieCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Player dies.
     */
    return PlayerPtr->Flag_To_Die();
}


/**
 *  Take ownership of any selected objects.
 * 
 *  @author: CCHyper
 */
const char *CaptureObjectCommandClass::Get_Name() const
{
    return "CaptureObject";
}

const char *CaptureObjectCommandClass::Get_UI_Name() const
{
    return "Capture Object";
}

const char *CaptureObjectCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *CaptureObjectCommandClass::Get_Description() const
{
    return "Take ownership of any selected objects.";
}

bool CaptureObjectCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Iterate over all currently selected objects and take ownership of them.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass * object = CurrentObjects[i];
        if (!object || !object->Is_Techno()) {
            continue;
        }

        /**
         *  We own this object already, skip it.
         */
        if (object->Owner_HouseClass() == PlayerPtr) {
            continue;
        }

        TechnoClass *techno = dynamic_cast<TechnoClass *>(object);
        techno->Captured(PlayerPtr);
    }

    Map.Recalc();

    return true;
}


/**
 *  Promote selected units to higher veterancy level
 *
 *  @author: hacklex
 */
const char* VeterancyPromoteCommandClass::Get_Name() const
{
    return "PromoteSelected";
}

const char* VeterancyPromoteCommandClass::Get_UI_Name() const
{
    return "Promote Selected";
}

const char* VeterancyPromoteCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char* VeterancyPromoteCommandClass::Get_Description() const
{
    return "Promote selected units to higher veterancy level";
}

bool VeterancyPromoteCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass* object = CurrentObjects[i];
        if (!object || !object->Is_Techno()) {
            continue;
        }
        TechnoClass* techno = static_cast<TechnoClass*>(object);
        if (techno->House->Is_Player_Control()) {
            if (techno->Crew.IsRookie) {
                techno->Crew.Set_Veteran(true);
            } else if (techno->Crew.IsVeteran) {
                techno->Crew.Set_Elite(true);
            }
        }
    }

    return true;
}

/**
 *  A shorter name for a list of TechnoClass pointers.
 */
using TechnoList = DynamicVectorClass<TechnoClass*>;

std::map<Classify_Function, DynamicVectorClass<TechnoClass*>*> UnitFilterLastFullSelectionByClassifiers;

/**
 *  Checks if two lists are equal, meaning they contain the same TechnoClass pointers.
 *  We expect that the lists are actually sets, so they should not contain duplicates.
 */
static bool Set_Equals(TechnoList& a, TechnoList& b)
{
    if (a.Count() != b.Count()) {
        return false;
    }
    for (int i = 0; i < a.Count(); ++i) {
        if (!a.Is_Present(b[i])) {
            return false;
        }
    }
    return true;
}

/**
 *  Checks if the current set is equal to the union of two other sets.
 */
static bool Equals_Union_Of_Two_Other_Sets(TechnoList& current, TechnoList& a, TechnoList& b)
{
    if (current.Count() != a.Count() + b.Count()) {
        return false;
    }
    for (int i = 0; i < current.Count(); ++i) {
        if (!a.Is_Present(current[i]) && !b.Is_Present(current[i])) {
            return false;
        }
    }
    return true;
}

/**
 *  Classifies a TechnoClass object based on its veterancy level.
 *  Returns:
 *  - 0 for Elite
 *  - 1 for Veteran
 *  - 2 for Rookie
 */
static int Get_Veterancy_Level(TechnoClass* techno)
{
    if (techno->Crew.IsElite) {
        return 0;
    } else if (techno->Crew.IsVeteran) {
        return 1;
    } else {
        return 2;
    }
}

/**
 *  Classifies a TechnoClass object based on its health level.
 *  Returns:
 *  - 0 for Red (low health)
 *  - 1 for Yellow (medium health)
 *  - 2 for Green (high health)
 */
static int Get_Health_Level(TechnoClass* techno) {
    auto ratio = techno->Get_Health_Ratio();
    if (Rule->ConditionRed >= ratio) {
        return 0;
    }
    if (Rule->ConditionYellow >= ratio) {
        return 1;
    }
    return 2;
}

/**
 *  A pointer to a function that classifies a TechnoClass by assigning it an integer tier from 0 to 2.
 */
typedef int (*Classify_Function)(TechnoClass*);

/**
 *  Returns the tier other than the two specified.
 */
int Get_Other_Tier(int a, int b)
{
    // (0, 1) => 2, (0, 2) => 1, (1, 2) => 0
    return ((a + b) * 2) % 3;
}

/**
 *  Reclassifies the TechnoClass objects in the lists_by_tiers array based on the classify_function.
 *  It moves objects from one tier to another based on the classification result.
 *  Might be needed when the objects' properties change, such as health or veterancy.
 */ 
void Reclassify(const Classify_Function& classify_function, TechnoList* lists_by_tiers) {
    for (int from_tier = 0; from_tier < 3; from_tier++) { 
        for (int i = lists_by_tiers[from_tier].Count() - 1; i >= 0; i--) {
            auto current_tier = classify_function(lists_by_tiers[from_tier][i]);
            if (current_tier != from_tier) {
                lists_by_tiers[current_tier].Add(lists_by_tiers[from_tier][i]);
                lists_by_tiers[from_tier].Delete(i);
            }
        }
    }
}

/**
 *  Classifies the TechnoClass objects in the current_selection list into three tiers based on the classify_function.
 *  The results are stored in the lists_by_tiers array, where each index corresponds to a tier.
 *  We expect the array to have three elements, one for each tier.
 */
void Classify(const Classify_Function &classify_function, TechnoList &current_selection, TechnoList *lists_by_tiers)
{
    for (int i = 0; i < current_selection.Count(); ++i) {
        int tier = classify_function(current_selection[i]);
        if (tier >= 0 && tier < 3) {
            lists_by_tiers[tier].Add(current_selection[i]);
        }
    }
}

/**
 *  Performs the filtering of the selection based on the classification function.
 *  If shift is pressed, the next tier will be added to the selection,
 *  otherwise the selection will be replaced with the next tier.
 */
bool Process_Filter(const Classify_Function &classify_function, bool is_shift_pressed)
{    
    /**
     *  Each classify_function has its own last_full_selection and last_selection arrays.
     */
    if (UnitFilterLastFullSelectionByClassifiers.empty()) {
        UnitFilterLastFullSelectionByClassifiers[Get_Veterancy_Level] = new TechnoList();
        UnitFilterLastFullSelectionByClassifiers[Get_Health_Level] = new TechnoList();
    }

    /**
     *  We fetch the last full selection for the given classify_function.
     */
    TechnoList& last_full_selection = *(UnitFilterLastFullSelectionByClassifiers[classify_function]);
    TechnoList last_selection[3]; 

    /**
     *  Then we classify the last full selection into three tiers.
     */
    Classify(classify_function, last_full_selection, last_selection);
    TechnoList current_selection[3];
    TechnoList current_technos;
    int best_selected_tier = 3;
    int worst_selected_tier = -1;

    /**
     *  Collecting info about current selection,
     *  splitting it into three tiers based on the value returned by classify_function.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass* object = CurrentObjects[i];
        if (!object || !object->Is_Techno() || !static_cast<TechnoClass*>(object)->House->Is_Player_Control()) {

            /**
             *  Skip non-techno objects and objects not owned by the player.
             */
            continue;
        }
        TechnoClass* techno = static_cast<TechnoClass*>(object);
        current_technos.Add(techno);
        int tier = classify_function(techno);
        best_selected_tier = std::min(tier, best_selected_tier);
        worst_selected_tier = std::max(tier, worst_selected_tier);
        if (tier >= 0 && tier < 3) {
            current_selection[tier].Add(techno);
        }
    }

    if (!Set_Equals(current_technos, last_full_selection) && // current selection differs from the last full selection
        !Set_Equals(current_technos, last_selection[0]) && // current selection is not exactly any of the tiers 
        !Set_Equals(current_technos, last_selection[1]) && 
        !Set_Equals(current_technos, last_selection[2]) &&
        !Equals_Union_Of_Two_Other_Sets(current_technos, last_selection[0], last_selection[1]) && // and not a union of any two tiers
        !Equals_Union_Of_Two_Other_Sets(current_technos, last_selection[0], last_selection[2]) &&
        !Equals_Union_Of_Two_Other_Sets(current_technos, last_selection[1], last_selection[2])) {

        /**
         *  We have a new selection that is not a subset of the last selection,
         *  so we start a new filtering process.
         */
        if (is_shift_pressed || best_selected_tier == worst_selected_tier) {

            /**
             *  Shift only makes sense if we already have started filtering.
             *  Can't add anything if we haven't filtered yet or the new selection is already of same rank.
             */
            return true;
        }
        last_full_selection.Clear();
        last_selection[0].Clear();
        last_selection[1].Clear();
        last_selection[2].Clear();

        /**
         *  Fill the last_full_selection and last_selection arrays.
         */
        for (int i = 0; i < current_technos.Count(); ++i) {
            last_full_selection.Add(current_technos[i]);
            last_selection[classify_function(current_technos[i])].Add(current_technos[i]);
        }

        /**
         *  Unselect all objects except the ones in the best tier among the current selection.
         */
        for (int i = best_selected_tier + 1; i < 3; ++i) {
            for (int k = 0; k < current_selection[i].Count(); ++k) {
                current_selection[i][k]->Unselect();
            }
        }

        /**
         *  Play the selection sound for the best tier.
         */
        if (best_selected_tier >= 0 && best_selected_tier < 3) {
            for (int k = 0; k < current_selection[best_selected_tier].Count(); ++k) {
                current_selection[best_selected_tier][k]->Response_Select();
            }
        }
    } else {

        /**
         *  We're already filtering.
         */
        int next_tier = worst_selected_tier;
        int loop_breaker = 3;
        if (best_selected_tier != worst_selected_tier) {

            /**
             *  There are at least two tiers in the selection.
             */
            if (Set_Equals(current_technos, last_full_selection)) {

                /**
                 *  If the current selection is the same as the last full selection,
                 *  we restrict the selection to the best tier.
                 */
                next_tier = best_selected_tier;
            } else {

                /**
                 *  If the current selection is not the same as the last full selection,
                 *  we find the tier not in the current selection.
                 */
                next_tier = Get_Other_Tier(best_selected_tier, worst_selected_tier);
            }
        } else {
            do {
                loop_breaker--;
                next_tier = (next_tier + 1) % 3;

                /**
                 *  We loop through the tiers until we find a non-empty one.
                 */
            } while (last_selection[next_tier].Count() == 0 && loop_breaker > 0);
        }
        if (next_tier == -1) {

            /**
             *  Couldn't find the next tier.
             */
            if (is_shift_pressed) {

                /**
                 *  Nothing to do if we can't find a next tier.
                 */
                return true;
            } else {

                /**
                 *  If we're not in add mode, we select the best tier.
                 */
                next_tier = best_selected_tier;
            }
        }
        if (next_tier >= 0 && next_tier < 3 && last_selection[next_tier].Count() > 0) {

            /**
             *  We found the next tier, and it is not empty.
             */
            if (!is_shift_pressed) {
                for (int i = 0; i < current_technos.Count(); ++i) {
                    current_technos[i]->Unselect();
                }
            }

            /**
             *  Select() also plays the selection sound.
             */
            for (int i = 0; i < last_selection[next_tier].Count(); ++i) {
                last_selection[next_tier][i]->Select();
            }
        }
    }

    return true;
}

/**
 *  Cycle through elite/veteran/green units among the last heterogenous selection.
 *
 *  @author: hacklex
 */
const char* VeterancyFilterCommandClass::Get_Name() const
{
    return "VeterancyFilter";
}

const char* VeterancyFilterCommandClass::Get_UI_Name() const
{
    return "Veterancy Filter";
}

const char* VeterancyFilterCommandClass::Get_Category() const
{
    return "Selection";
}

const char* VeterancyFilterCommandClass::Get_Description() const
{
    return "Filter out elite/veteran/rookie units from the last mixed selection.";
}

bool VeterancyFilterCommandClass::Process()
{
    return Process_Filter(Get_Veterancy_Level, false);
}


/**
 *  Cycle through elite/veteran/green units among the last heterogenous selection.
 *
 *  @author: hacklex
 */
const char* VeterancyFilterAddNextCommandClass::Get_Name() const
{
    return "VeterancyFilterAddLower";
}

const char* VeterancyFilterAddNextCommandClass::Get_UI_Name() const
{
    return "Veterancy Filter (Add Lower)";
}

const char* VeterancyFilterAddNextCommandClass::Get_Category() const
{
    return "Selection";
}

const char* VeterancyFilterAddNextCommandClass::Get_Description() const
{
    return "Add units of lower veterancy to the already filtered subset.";
}

bool VeterancyFilterAddNextCommandClass::Process()
{
    return Process_Filter(Get_Veterancy_Level, true);
}


/**
 *  Cycle through elite/veteran/green units among the last heterogenous selection.
 *
 *  @author: hacklex
 */
const char* HealthFilterCommandClass::Get_Name() const
{
    return "HealthFilter";
}

const char* HealthFilterCommandClass::Get_UI_Name() const
{
    return "Health Filter";
}

const char* HealthFilterCommandClass::Get_Category() const
{
    return "Selection";
}

const char* HealthFilterCommandClass::Get_Description() const
{
    return "Filter out red/yellow/green units from the last mixed selection.";
}

bool HealthFilterCommandClass::Process()
{
    return Process_Filter(Get_Health_Level, false);
}


/**
 *  Cycle through elite/veteran/green units among the last heterogenous selection.
 *
 *  @author: hacklex
 */
const char* HealthFilterAddNextCommandClass::Get_Name() const
{
    return "HealthFilterAddLower";
}

const char* HealthFilterAddNextCommandClass::Get_UI_Name() const
{
    return "Health Filter (Add Lower)";
}

const char* HealthFilterAddNextCommandClass::Get_Category() const
{
    return "Selection";
}

const char* HealthFilterAddNextCommandClass::Get_Description() const
{
    return "Add units of higher health group (yellow, green) to the already filtered subset.";
}

bool HealthFilterAddNextCommandClass::Process()
{
    return Process_Filter(Get_Health_Level, true);
}


/**
 *  Enters beacon placement mode.
 *
 *  @author: ZivDero
 */
const char* BeaconPlacementCommandClass::Get_Name() const
{
    return "PlaceBeacon";
}

const char* BeaconPlacementCommandClass::Get_UI_Name() const
{
    return "Place Beacon";
}

const char* BeaconPlacementCommandClass::Get_Category() const
{
    return "Interface";
}

const char* BeaconPlacementCommandClass::Get_Description() const
{
    return "Used to place a communication beacon.";
}

bool BeaconPlacementCommandClass::Process()
{
    if (BeaconManagerClass::Are_Beacons_Enabled()) {
        if (!PlayerPtr->IsDefeated) {
            TacticalMapExtension->Beacon_Mode_Control(-1);
        }
    }

    return true;
}


/**
 *  Grants all available special weapons to the player.
 * 
 *  @author: CCHyper
 */
const char *SpecialWeaponsCommandClass::Get_Name() const
{
    return "SpecialWeapons";
}

const char *SpecialWeaponsCommandClass::Get_UI_Name() const
{
    return "Special Weapons";
}

const char *SpecialWeaponsCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *SpecialWeaponsCommandClass::Get_Description() const
{
    return "Grants all available special weapons to the player.";
}

bool SpecialWeaponsCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Iterate over all the special weapon slots for the player house
     *  and make them all available, fully charged!
     */
    for (SuperWeaponType i = SUPER_FIRST; i < SuperWeaponTypes.Count(); ++i) {

        PlayerPtr->SuperWeapon[i]->Enable(true, true, true);
        PlayerPtr->SuperWeapon[i]->Forced_Charge(true);
        Map.Add(RTTI_SPECIAL, i);
    }

    return true;
}


/**
 *  Hands out free money to the player.
 * 
 *  @author: CCHyper
 */
const char *FreeMoneyCommandClass::Get_Name() const
{
    return "FreeMoney";
}

const char *FreeMoneyCommandClass::Get_UI_Name() const
{
    return "Free Money";
}

const char *FreeMoneyCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *FreeMoneyCommandClass::Get_Description() const
{
    return "Gives free money to the player.";
}

bool FreeMoneyCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Give 10,000 credits to the player.
     */
    PlayerPtr->Refund_Money(10000);

    return true;
}


/**
 *  Fires a lightning bolt at the current mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *LightningBoltCommandClass::Get_Name() const
{
    return "LightningBolt";
}

const char *LightningBoltCommandClass::Get_UI_Name() const
{
    return "Lightning Bolt";
}

const char *LightningBoltCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *LightningBoltCommandClass::Get_Description() const
{
    return "Fires a lightning bolt at the current mouse location.";
}

bool LightningBoltCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Cell mouse_cell = Get_Cell_Under_Mouse();

    IonStorm_Lightning_Strike_At(mouse_cell);

    return true;
}


/**
 *  Fires an ion blast bolt at the current mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *IonBlastCommandClass::Get_Name() const
{
    return "IonBlast";
}

const char *IonBlastCommandClass::Get_UI_Name() const
{
    return "Ion Blast";
}

const char *IonBlastCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *IonBlastCommandClass::Get_Description() const
{
    return "Fires an ion blast bolt at the current mouse location.";
}

bool IonBlastCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    new IonBlastClass(mouse_coord);

    return true;
}


/**
 *  Spawns an explosion at the mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *ExplosionCommandClass::Get_Name() const
{
    return "Explosion";
}

const char *ExplosionCommandClass::Get_UI_Name() const
{
    return "Explosion";
}

const char *ExplosionCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ExplosionCommandClass::Get_Description() const
{
    return "Spawns a explosion at the mouse location.";
}

bool ExplosionCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    /**
     *  The damage to deal at the coord.
     */
    int damage = Rule->MaxDamage;

    /**
     *  Pick a random warhead from the list, using C4Warhead as a backup.
     */
    const WarheadTypeClass* warheadtypeptr = Warheads[WarheadTypeClass::From_Name(Percent_Chance(50) ? "AP" : "HE")];
    if (!warheadtypeptr) {
        warheadtypeptr = Rule->C4Warhead;
    }

    /**
     *  What anim should we use for this criteria.
     */
    const AnimTypeClass *cellanim = Combat_Anim(damage, warheadtypeptr, cellptr->Land_Type(), &mouse_coord);
    if (!cellanim) {
        return false;
    }

    new AnimClass(cellanim, mouse_coord);

    Explosion_Damage(mouse_coord, damage, nullptr, warheadtypeptr);

    return true;
}


/**
 *  Spawns a large explosion at the mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *SuperExplosionCommandClass::Get_Name() const
{
    return "SuperExplosion";
}

const char *SuperExplosionCommandClass::Get_UI_Name() const
{
    return "Super Explosion";
}

const char *SuperExplosionCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *SuperExplosionCommandClass::Get_Description() const
{
    return "Spawns a large explosion at the mouse location.";
}

bool SuperExplosionCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    /**
     *  The damage to deal at the coord.
     */
    int damage = Rule->MaxDamage;

    /**
     *  Pick a random warhead from the list, using C4Warhead as a backup.
     */
    const WarheadTypeClass *warheadtypeptr = Warheads[WarheadTypeClass::From_Name("Super")];
    if (!warheadtypeptr) {
        warheadtypeptr = Rule->C4Warhead;
    }

    /**
     *  What anim should we use for this criteria.
     */
    const AnimTypeClass *cellanim = Combat_Anim(damage, warheadtypeptr, cellptr->Land_Type(), &mouse_coord);
    if (!cellanim) {
        return false;
    }

    new AnimClass(cellanim, mouse_coord);

    Explosion_Damage(mouse_coord, damage, nullptr, warheadtypeptr);

    return true;
}


/**
 *  Exits the game completely.
 * 
 *  @author: CCHyper
 */
const char *BailOutCommandClass::Get_Name() const
{
    return "BailOut";
}

const char *BailOutCommandClass::Get_UI_Name() const
{
    return "Bail Out";
}

const char *BailOutCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *BailOutCommandClass::Get_Description() const
{
    return "Exits the game to the desktop.";
}

bool BailOutCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    DEBUG_WARNING("Bail out!");
    Fatal("Bail out!");

    return true;
}


/**
 *  Toggles the ion storm on/off.
 * 
 *  @author: CCHyper
 */
const char *IonStormCommandClass::Get_Name() const
{
    return "IonStorm";
}

const char *IonStormCommandClass::Get_UI_Name() const
{
    return "Ion Storm";
}

const char *IonStormCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *IonStormCommandClass::Get_Description() const
{
    return "Toggles an ion storm on/off.";
}

bool IonStormCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    if (IonStorm_Is_Active()) {
        IonStorm_Stop();
    } else {
        IonStorm_Start(TICKS_PER_SECOND * Rule->IonStormDuration/*, TICKS_PER_SECOND * Rule->IonStormWarning*/); // No warning (instant).
    }

    return true;
}


/**
 *  Saves a snapshot of the current scenario state.
 * 
 *  @author: CCHyper
 */
const char *MapSnapshotCommandClass::Get_Name() const
{
    return "MapSnapshot";
}

const char *MapSnapshotCommandClass::Get_UI_Name() const
{
    return "Scenario Snapshot";
}

const char *MapSnapshotCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *MapSnapshotCommandClass::Get_Description() const
{
    return "Saves a snapshot of the current scenario state (Saved as 'SCEN_<date-time>.MAP.).";
}

bool MapSnapshotCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    char buffer[128];

    /**
     *  Generate a unique filename with the current timestamp.
     */
    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;
    Get_Full_Time(day, month, year, hour, min, sec);
    std::snprintf(buffer, sizeof(buffer), "SCEN_%02u-%02u-%04u_%02u-%02u-%02u.MAP", day, month, year, hour, min, sec);

    DEBUG_INFO("Saving map snapshot...");

    Write_Scenario_INI(buffer);
    
    DEBUG_INFO(" COMPLETE!\n");

    DEBUG_INFO("Filename: {}\n", buffer);

    return true;
}


/**
 *  Removes the selected object(s) from the game world.
 * 
 *  @author: CCHyper
 */
const char *DeleteObjectCommandClass::Get_Name() const
{
    return "DeleteObject";
}

const char *DeleteObjectCommandClass::Get_UI_Name() const
{
    return "Delete Selected";
}

const char *DeleteObjectCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DeleteObjectCommandClass::Get_Description() const
{
    return "Removes the selected object(s) from the game world.";
}

bool DeleteObjectCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass *object = CurrentObjects[i];
        if (!object) {
            continue;
        }

        /**
         *  Buildings need to be "sold".
         */
        if (object->RTTI == RTTI_BUILDING) {
            object->Sell_Back(1);
        } else {
            object->Unselect();
            object->Limbo();
            delete object;
        }
    }

    Map.Recalc();

    return true;
}


/**
 *  Spawn all buildable units and structures at mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *SpawnAllCommandClass::Get_Name() const
{
    return "SpawnAll";
}

const char *SpawnAllCommandClass::Get_UI_Name() const
{
    return "Spawn All";
}

const char *SpawnAllCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *SpawnAllCommandClass::Get_Description() const
{
    return "Spawn all buildable units and structures at mouse location.";
}

/**
 *  Attempt to unlimbo the object at the cell specified.
 */
bool SpawnAllCommandClass::Try_Unlimbo(TechnoClass *techno, Cell &cell)
{
    if (techno) {

        int map_cell_x = Map.MapRect.X;
        int map_cell_y = Map.MapRect.Y;
        int map_cell_right = map_cell_x + Map.MapRect.Width;
        int map_cell_bottom = map_cell_y + Map.MapRect.Height;

        /**
         *  Generally try to prevent the objects from spawning off the right of the screen.
         */
        map_cell_right = std::min(map_cell_right, cell.X + 26);

        Cell attempt = cell;

        while (attempt.Y < map_cell_bottom) {

            Coord coord = attempt.As_Coord();
            if (techno->Unlimbo(coord)) {

                attempt.X++;
                if (attempt.X > map_cell_right - 2) {
                    attempt.X = cell.X; //map_cell_x + 2;
                    attempt.Y++;
                }

                cell = attempt;
                return true;
            }

            attempt.X++;
            if (attempt.X > map_cell_right - 2) {
                attempt.X = cell.X; //map_cell_x + 2;
                attempt.Y++;
            }
        }

        cell = attempt;
    }

    return false;
}

bool SpawnAllCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Dont spawn anything lower than this row.
     */
    int map_cell_bottom = Map.MapRect.Y + Map.MapRect.Height;

    /**
     *  Default spawn location (top left of map).
     */
    Cell origin(Map.MapRect.X + 2, Map.MapRect.Y + 2);

    /**
     *  If mouse position is valid, convert to world coordinates and update
     *  the spawn origin position to that of the mouse position.
     */
    if (Get_Mouse_Point() != Point2D(0, 0)) {
        origin = Get_Cell_Under_Mouse();
    }

    Cell attempt = origin;

    /**
     *  Attempt to spawn all ownable objects for the player house.
     */

    for (StructType index = STRUCT_FIRST; index < BuildingTypes.Count(); ++index) {
        BuildingTypeClass const & building_type = *BuildingTypes[index];
        if (building_type.Get_Ownable() /*&& building_type.Level != -1*/) {
            BuildingClass * building = (BuildingClass *)building_type.Create_One_Of(PlayerPtr);
            if (building) {
                attempt = origin;
                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(building, attempt)) {
                        DEBUG_INFO("BuildingType {} spawned at {},{}.\n", building_type.Name(),  attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    for (UnitType index = UNIT_FIRST; index < UnitTypes.Count(); ++index) {
        UnitTypeClass const & unit_type = *UnitTypes[index];
        if (unit_type.Get_Ownable() /*&& unit_type.Level != -1*/) {
            UnitClass * unit = (UnitClass *)unit_type.Create_One_Of(PlayerPtr);
            if (unit) {

                attempt = origin;

                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(unit, attempt)) {
                        DEBUG_INFO("UnitType {} spawned at {},{}.\n", unit_type.Name(), attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    for (InfantryType index = INFANTRY_FIRST; index < InfantryTypes.Count(); ++index) {
        InfantryTypeClass const & infantry_type = *InfantryTypes[index];
        if (infantry_type.Get_Ownable() /*&& infantry_type.Level != -1*/) {
            InfantryClass * inf = (InfantryClass *)infantry_type.Create_One_Of(PlayerPtr);
            if (inf) {
                attempt = origin;
                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(inf, attempt)) {
                        DEBUG_INFO("InfantryType {} spawned at {},{}.\n", infantry_type.Name(),  attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    for (AircraftType index = AIRCRAFT_FIRST; index < AircraftTypes.Count(); ++index) {
        AircraftTypeClass const & aircraft_type = *AircraftTypes[index];

        /**
         *  DROPPOD breaks the game!
         */
        //if (index == AIRCRAFT_DROPPOD) continue;
        if (aircraft_type == "DPOD") continue;

        if (aircraft_type.Get_Ownable() /*&& aircraft_type.Level != -1*/) {
            AircraftClass * air = (AircraftClass *)aircraft_type.Create_One_Of(PlayerPtr);
            if (air) {
                attempt = origin;
                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(air, attempt)) {
                        DEBUG_INFO("AircraftType {} spawned at {},{}.\n", aircraft_type.Name(),  attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    return true;
}


/**
 *  Apply damage to all selected objects.
 */
const char *DamageCommandClass::Get_Name() const
{
    return "Damage";
}

const char *DamageCommandClass::Get_UI_Name() const
{
    return "Damage";
}

const char *DamageCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DamageCommandClass::Get_Description() const
{
    return "Apply damage to all selected objects.";
}

bool DamageCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Iterate over all selected objects and deal 50 hit points. Use C4Damage as the backup.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        int damage = std::max(50, Rule->MinDamage);
        const WarheadTypeClass *warhead = Warheads[WarheadTypeClass::From_Name("SA")];
        if (!warhead) {
            warhead = Rule->C4Warhead;
        }
        CurrentObjects[i]->Take_Damage(damage, 0, warhead, nullptr);
    }

    Map.Recalc();

    return true;
}


/**
 *  Toggle the elite status of the selected objects.
 * 
 *  @author: CCHyper
 */
const char *ToggleEliteCommandClass::Get_Name() const
{
    return "ToggleElite";
}

const char *ToggleEliteCommandClass::Get_UI_Name() const
{
    return "Toggle Elite";
}

const char *ToggleEliteCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleEliteCommandClass::Get_Description() const
{
    return "Toggle the elite status of the selected objects.";
}

bool ToggleEliteCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    for (int i = 0; i < CurrentObjects.Count(); ++i) {

        TechnoClass *techno = reinterpret_cast<TechnoClass *>(CurrentObjects[i]);
        if (!techno) {
            continue;
        }
        
        /**
         *  Upgrade to rookie.
         */
        if (techno->Crew.Is_Dumbass()) {
            techno->Crew.Set_Rookie(true);
            continue;
        }

        /**
         *  Upgrade to veteran.
         */
        if (techno->Crew.IsRookie) {
            techno->Crew.Set_Veteran(true);
            continue;
        }
        
        /**
         *  Upgrade to elite.
         */
        if (techno->Crew.IsVeteran) {
            techno->Crew.Set_Elite(true);
            continue;
        }
        
        /**
         *  Degrade elite back to dumbass.
         */
        if (techno->Crew.IsElite) {
            techno->Crew.Set_Dumbass(true);
            continue;
        }
    }

    return true;
}


/**
 *  Unlock all available build options for the player house.
 * 
 *  @author: CCHyper
 */
const char *BuildCheatCommandClass::Get_Name() const
{
    return "BuildCheat";
}

const char *BuildCheatCommandClass::Get_UI_Name() const
{
    return "Build Cheat";
}

const char *BuildCheatCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *BuildCheatCommandClass::Get_Description() const
{
    return "Unlock all available build options for the player house.";
}

bool BuildCheatCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Toggle the build cheat flag.
     */
    Vinifera_Developer_BuildCheat = !Vinifera_Developer_BuildCheat;

    /**
     *  Flag the player house to recalculate buildables.
     */
    PlayerPtr->IsRecalcNeeded = true;

    if (!ScenarioInit) {

        /**
         *  Update all factories.
         */
        for (int index = 0; index < Buildings.Count(); index++) {
            BuildingClass *building = Buildings[index];
            if (building) {
                if (building->Owner_HouseClass() == PlayerPtr) {
                    building->Update_Buildables();
                }
            }
        }
    }

    Map.Recalc();

    return true;
}


/**
 *  Toggles the visibility of the map shroud.
 * 
 *  @author: CCHyper
 */
const char *ToggleShroudCommandClass::Get_Name() const
{
    return "ToggleShroud";
}

const char *ToggleShroudCommandClass::Get_UI_Name() const
{
    return "Toggle Shroud";
}

const char *ToggleShroudCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleShroudCommandClass::Get_Description() const
{
    return "Toggles the visibility of the map shroud.";
}

bool ToggleShroudCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Toggle the unshroud flag.
     */
    Vinifera_Developer_Unshroud = !Vinifera_Developer_Unshroud;

    /**
     *  #NOTE:
     *  This is temporary code until the Unshroud flag is correctly
     *  hooked into DisplayClass and RadarClass!
     */
    if (Vinifera_Developer_Unshroud) {

        //Map.Reveal_The_Map();

        if (!PlayerPtr->IsVisionary) {

            PlayerPtr->IsVisionary = true;

            Map.Reset_Iterator();

            for (CellClass *cell = Map.Iterate(); cell != nullptr; cell = Map.Iterate()) {
                Map.Map_Cell(cell->CellID, PlayerPtr);
            }

            Map.Flag_To_Redraw(true);

        }

    } else {
        Map.Shroud_The_Map();
    }

    /**
     *  Force a redraw of the screen.
     */
    Map.Flag_To_Redraw(true);

    return true;
}


/**
 *  Heal the selected objects by 50 hit points.
 * 
 *  @author: CCHyper
 */
const char *HealCommandClass::Get_Name() const
{
    return "Heal";
}

const char *HealCommandClass::Get_UI_Name() const
{
    return "Heal";
}

const char *HealCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *HealCommandClass::Get_Description() const
{
    return "Heal the selected objects by 50 hit points.";
}

bool HealCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Iterate over all selected objects and heal by 50 hit points.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        int damage = -50;
        CurrentObjects[i]->Take_Damage(damage, 0, Rule->C4Warhead, nullptr);
    }

    return true;
}


/**
 *  Toggles if weapons are inert or not.
 * 
 *  @author: CCHyper
 */
const char *ToggleInertCommandClass::Get_Name() const
{
    return "ToggleInert";
}

const char *ToggleInertCommandClass::Get_UI_Name() const
{
    return "Toggle Inert";
}

const char *ToggleInertCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleInertCommandClass::Get_Description() const
{
    return "Toggles if weapons are inert or not.";
}

bool ToggleInertCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  This flags controls whether weapons are inert. An inert weapon doesn't do
     *  any damage. Effectively, if this is true, then units will never die.
     */
    Scen->Special.IsInert = !Scen->Special.IsInert;

    return true;
}


/**
 *  Dumps all the current AI house base node info to the log output.
 * 
 *  @author: CCHyper
 */
const char *DumpAIBaseNodesCommandClass::Get_Name() const
{
    return "DumpAIBaseNodes";
}

const char *DumpAIBaseNodesCommandClass::Get_UI_Name() const
{
    return "Dump AI Base Nodes";
}

const char *DumpAIBaseNodesCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DumpAIBaseNodesCommandClass::Get_Description() const
{
    return "Dumps all the current AI house base node info to the log output.";
}

bool DumpAIBaseNodesCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    DEBUG_INFO("About to dump AI base nodes...\n\n");

    for (int house_index = 0; house_index < Houses.Count(); ++house_index) {
        HouseClass *house = Houses[house_index];

        /**
         *  Make sure we only process non-player houses.
         */
        if (!house->Is_Player_Control() && !house->Is_Human_Player()) {

            DEBUG_INFO("\n");

            DEBUG_INFO("{:02} \"{}\":\n", house_index, house->Class->Name());

            //DEBUG_INFO("  field_50: {}\n", house->Base.field_50);
            //DEBUG_INFO("  field_64: {}\n", house->Base.field_64);
            //DEBUG_INFO("  field_68: {}\n", house->Base.field_68);
            //DEBUG_INFO("  field_6C: {}\n", house->Base.field_6C);
            //DEBUG_INFO("  field_70: {}\n", house->Base.field_70);
            DEBUG_INFO("  PercentBuilt: {:03}\n", house->Base.PercentBuilt);

            DEBUG_INFO("  Nodes.Count: {}\n", house->Base.Nodes.Count());

            /**
             *  Iterate all nodes for this house.
             */
            for (int node_index = 0; node_index < house->Base.Nodes.Count(); ++node_index) {
                BaseNodeClass &node = house->Base.Nodes[node_index];

                if (node.Type == STRUCT_NONE) {
                    continue;
                }

                const char *name = BuildingTypeClass::Name_From(node.Type);
                DEBUG_INFO("  Node {:03}: \"{}\" at {},{}\n", node_index, name, node.CellID.X, node.CellID.Y);
            }
        }
    }

    DEBUG_INFO("\nFinished!\n\n");

    return true;
}


/**
 *  Toggles the berzerk state of the selected infantry.
 * 
 *  @author: CCHyper
 */
const char *ToggleBerzerkCommandClass::Get_Name() const
{
    return "ToggleBerzerk";
}

const char *ToggleBerzerkCommandClass::Get_UI_Name() const
{
    return "Toggle Berzerk";
}

const char *ToggleBerzerkCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleBerzerkCommandClass::Get_Description() const
{
    return "Toggles the berzerk state of the selected infantry.";
}

bool ToggleBerzerkCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Iterate over all selected infantry and toggle their berzerk state.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass *object = CurrentObjects[i];
        if (object && object->Is_Infantry()) {
            InfantryClass *infantry = reinterpret_cast<InfantryClass *>(object);
            if (infantry) {
                infantry->IsBerzerk = !infantry->IsBerzerk;
            }
        }        
    }

    return true;
}


/**
 *  Increase the shroud darkness by one step (cell).
 * 
 *  @author: CCHyper
 */
const char *EncroachShadowCommandClass::Get_Name() const
{
    return "EncroachShadow";
}

const char *EncroachShadowCommandClass::Get_UI_Name() const
{
    return "Encroach Shadow";
}

const char *EncroachShadowCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *EncroachShadowCommandClass::Get_Description() const
{
    return "Increase the shroud darkness by one step (cell).";
}

bool EncroachShadowCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Map.Encroach_Shadow();

    Map.Flag_To_Redraw(2);

    return true;
}


/**
 *  Increase the fog of war by one step (cell).
 * 
 *  @author: CCHyper
 */
const char *EncroachFogCommandClass::Get_Name() const
{
    return "EncroachFog";
}

const char *EncroachFogCommandClass::Get_UI_Name() const
{
    return "Encroach Fog";
}

const char *EncroachFogCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *EncroachFogCommandClass::Get_Description() const
{
    return "Increase the fog of war by one step (cell).";
}

bool EncroachFogCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Map.Encroach_Fog();

    Map.Flag_To_Redraw(2);

    return true;
}


/**
 *  Toggles alliance with the selected objects house.
 * 
 *  @author: CCHyper
 */
const char *ToggleAllianceCommandClass::Get_Name() const
{
    return "ToggleAlly";
}

const char *ToggleAllianceCommandClass::Get_UI_Name() const
{
    return "Toggle Alliance";
}

const char *ToggleAllianceCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleAllianceCommandClass::Get_Description() const
{
    return "Toggles alliance with the selected objects house.";
}

bool ToggleAllianceCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Fetch the currently selected object and toggle the players alliance with its owner.
     */
    if (CurrentObjects.Count() == 1) {
        ObjectClass *object = CurrentObjects.Fetch_Head();
        if (object && object->Is_Techno()) {
            TechnoClass *techno = reinterpret_cast<TechnoClass *>(object);
            if (techno) {
                if (PlayerPtr != techno->House) {
                    if (PlayerPtr->Is_Ally(techno->House) || techno->House->Is_Ally(PlayerPtr)) {
                        PlayerPtr->Make_Enemy(techno->House);
                        techno->House->Make_Enemy(PlayerPtr);
                    } else {
                        PlayerPtr->Make_Ally(techno->House);
                        techno->House->Make_Ally(PlayerPtr);
                    }
                }
            }
        }        
    }

    return true;
}


/**
 *  Adds 2000 power units to the player.
 * 
 *  @author: CCHyper
 */
const char *AddPowerCommandClass::Get_Name() const
{
    return "AddPower";
}

const char *AddPowerCommandClass::Get_UI_Name() const
{
    return "Add Power";
}

const char *AddPowerCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *AddPowerCommandClass::Get_Description() const
{
    return "Adds 2000 power units to the player.";
}

bool AddPowerCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Adjust the power value of the player house. 
     */
    PlayerPtr->Adjust_Power(2000);

    Map.Recalc();

    return true;
}


/**
 *  Places a random crate at the mouse location.
 * 
 *  @author: CCHyper
 */
const char *PlaceCrateCommandClass::Get_Name() const
{
    return "PlaceCrate";
}

const char *PlaceCrateCommandClass::Get_UI_Name() const
{
    return "Place Crate";
}

const char *PlaceCrateCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceCrateCommandClass::Get_Description() const
{
    return "Places a random crate at the mouse location.";
}

bool PlaceCrateCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Cell mouse_cell = Get_Cell_Under_Mouse();

    CellClass *cell = &Map[mouse_cell];
    if (!cell) {
        return false;
    }

    /**
     *  Some safety checks;
     *   - Don't place in unshrouded cells.
     *   - Bridges are overlay, don't place there.
     *   - Make sure the cell does not already contain overlay.
     */
    if (!cell->IsVisible || cell->Is_Bridge_Here() || cell->Overlay != OVERLAY_NONE) {
        return false;
    }

    if (!Map.Place_Crate(mouse_cell)) {
        return false;
    }

    DEBUG_INFO("Crate placed at {}, {}\n", mouse_cell.X, mouse_cell.Y);

    return true;
}


/**
 *  Displays cell coordinates of the mouse cursor.
 * 
 *  @author: CCHyper
 */
const char *CursorPositionCommandClass::Get_Name() const
{
    return "CursorPosition";
}

const char *CursorPositionCommandClass::Get_UI_Name() const
{
    return "Cursor Position";
}

const char *CursorPositionCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *CursorPositionCommandClass::Get_Description() const
{
    return "Displays cell coordinates of the mouse cursor.";
}

bool CursorPositionCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Toggle the show cursor position flag.
     */
    Vinifera_Developer_ShowCursorPosition = !Vinifera_Developer_ShowCursorPosition;

    return true;
}


/**
 *  Toggle frame step mode to step through the game frame-by-frame (for inspection).
 * 
 *  @author: CCHyper
 */
const char *ToggleFrameStepCommandClass::Get_Name() const
{
    return "ToggleFrameStep";
}

const char *ToggleFrameStepCommandClass::Get_UI_Name() const
{
    return "Toggle Frame Step";
}

const char *ToggleFrameStepCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleFrameStepCommandClass::Get_Description() const
{
    return "Toggle frame step mode to step through the game frame-by-frame (for inspection).";
}

bool ToggleFrameStepCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    /**
     *  Toggle the frame step mode flag.
     */
    Vinifera_Developer_FrameStep = !Vinifera_Developer_FrameStep;
    Vinifera_Developer_FrameStepCount = 0;

    return true;
}


/**
 *  Frame Step Only: Step forward 1 frame.
 * 
 *  @author: CCHyper
 */
const char *Step1FrameCommandClass::Get_Name() const
{
    return "Step1Frame";
}

const char *Step1FrameCommandClass::Get_UI_Name() const
{
    return "Step Forward 1 Frame";
}

const char *Step1FrameCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *Step1FrameCommandClass::Get_Description() const
{
    return "Frame Step Only: Step forward 1 frame.";
}

bool Step1FrameCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    if (!Vinifera_Developer_FrameStep) {
        return false;
    }

    Vinifera_Developer_FrameStepCount = 1;

    return true;
}


/**
 *  Frame Step Only: Step forward 5 frames.
 * 
 *  @author: CCHyper
 */
const char *Step5FramesCommandClass::Get_Name() const
{
    return "Step5Frame";
}

const char *Step5FramesCommandClass::Get_UI_Name() const
{
    return "Step Forward 5 Frames";
}

const char *Step5FramesCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *Step5FramesCommandClass::Get_Description() const
{
    return "Frame Step Only: Step forward 5 frames.";
}

bool Step5FramesCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    if (!Vinifera_Developer_FrameStep) {
        return false;
    }

    Vinifera_Developer_FrameStepCount = 5;

    return true;
}


/**
 *  Frame Step Only: Step forward 10 frames.
 * 
 *  @author: CCHyper
 */
const char *Step10FramesCommandClass::Get_Name() const
{
    return "Step10Frames";
}

const char *Step10FramesCommandClass::Get_UI_Name() const
{
    return "Step Forward 10 Frames";
}

const char *Step10FramesCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *Step10FramesCommandClass::Get_Description() const
{
    return "Frame Step Only: Step forward 10 frames.";
}

bool Step10FramesCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    if (!Vinifera_Developer_FrameStep) {
        return false;
    }

    Vinifera_Developer_FrameStepCount = 10;

    return true;
}


/**
 *  Toggles AI control of the player house.
 * 
 *  @author: CCHyper
 */
const char *ToggleAIControlCommandClass::Get_Name() const
{
    return "ToggleAIControl";
}

const char *ToggleAIControlCommandClass::Get_UI_Name() const
{
    return "Toggle AI Control";
}

const char *ToggleAIControlCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleAIControlCommandClass::Get_Description() const
{
    return "Toggles AI control of the player house.";
}

bool ToggleAIControlCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    HouseClass *player_house = PlayerPtr;

    if (player_house->IsPlayerControl) {

        /**
         *  AI takes control of the player house. We flag both the automated
         *  production and the alerted state, both of these will enter the
         *  automated building production and auto team systems.
         */

        player_house->IsHuman = false;
        player_house->IsPlayerControl = false;

        player_house->IsStarted = true;
        player_house->IsAlerted = true;

        /**
         *  Crank up the AI IQ to the max available.
         */
        player_house->IQ = Rule->MaxIQ;

        player_house->Difficulty = DIFF_HARD;

        DEV_DEBUG_INFO("Developer Mode: AI has taken control of player.\n");

    } else {

        /**
         *  Player retakes control from the AI. Disable any automation flags
         *  to allow the player have complete control of the house again.
         */
    
        player_house->IsHuman = true;
        player_house->IsPlayerControl = true;

        player_house->IsStarted = false;
        player_house->IsAlerted = false;

        /**
         *  Reset the IQ level.
         */
        player_house->IQ = 0;

        player_house->Difficulty = DIFF_NORMAL;

        DEV_DEBUG_INFO("Developer Mode: Player has resumed control.\n");
    }

    /**
     *  Toggle the global state flag.
     */
    Vinifera_Developer_AIControl = !Vinifera_Developer_AIControl;

    return true;
}


/**
 *  Cycle the camera between the starting waypoints on the map.
 * 
 *  @author: CCHyper
 */
const char *StartingWaypointsCommandClass::Get_Name() const
{
    return "StartingWaypoints";
}

const char *StartingWaypointsCommandClass::Get_UI_Name() const
{
    return "Cycle Starting Waypoints";
}

const char *StartingWaypointsCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *StartingWaypointsCommandClass::Get_Description() const
{
    return "Cycle the camera between the starting waypoints on the map.";
}

bool StartingWaypointsCommandClass::Process()
{
    if (Session.Type != GAME_SKIRMISH) {
        return false;
    }

    /**
     *  Fetch the next starting waypoint. We clamp to the first 8 waypoints
     *  as Tiberian Sun only supports these for starting locatons.
     */
    static int _current_index = 0;
    Coord wp_coord = Scen->Waypoint_Coord(_current_index++ % 8);
    if (wp_coord == COORD_NONE) {
        return false;
    }

    wp_coord.Z = Map.Get_Height_GL(wp_coord);

    /**
     *  Center the tactical camera at this waypoint.
     */
    TacticalMap->Set_Tactical_Position(wp_coord);

    /**
     *  Clear any interface actions if they are active.
     */
    if (Map.PendingObject) {
        Map.Set_Cursor_Pos(Cell(0,0));
    }
    Map.Break_Follow_Mode();

    Map.Flag_To_Redraw(true);

    return true;
}


/**
 *  Places a random infantry at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceInfantryCommandClass::Get_Name() const
{
    return "PlaceInfantry";
}

const char *PlaceInfantryCommandClass::Get_UI_Name() const
{
    return "Place Infantry";
}

const char *PlaceInfantryCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceInfantryCommandClass::Get_Description() const
{
    return "Places a random infantry at the mouse cell.";
}

bool PlaceInfantryCommandClass::Process()
{
    if (Session.Type != GAME_SKIRMISH) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }
    
    DynamicVectorClass<InfantryTypeClass *> available_infantry;

    int owner_id = 1 << PlayerPtr->Class->HeapID;

    /**
     *  Build a list of infantry from the available starting units.
     */
    for (int i = 0; i < InfantryTypes.Count(); ++i) {
        InfantryTypeClass *infantrytype = InfantryTypes[i];
        if (infantrytype && infantrytype->IsAllowedToStartInMultiplayer) {
            if (infantrytype->Level <= PlayerPtr->Control.TechLevel && (owner_id & infantrytype->Ownable) != 0) {
                available_infantry.Add(infantrytype);
            }
        }
    }

    if (!available_infantry.Count()) {
        DEBUG_WARNING("Failed to generate list of available InfantryTypes!\n");
        return false;
    }

    InfantryTypeClass *infantrytype = available_infantry[Random_Pick(0, available_infantry.Count()-1)];

    /**
     *  Create an instance of the infantry.
     */
    InfantryClass *inf = reinterpret_cast<InfantryClass *>(infantrytype->Create_One_Of(PlayerPtr));
    if (!inf->Unlimbo(mouse_coord)) {
        delete inf;
        return false;
    }

    DEBUG_INFO("Placed infantry \"{}\" at {},{},{}\n", inf->Name(), inf->Position.X, inf->Position.Y, inf->Position.Z);
    return true;
}


/**
 *  Places a random unit at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceUnitCommandClass::Get_Name() const
{
    return "PlaceUnit";
}

const char *PlaceUnitCommandClass::Get_UI_Name() const
{
    return "Place Unit";
}

const char *PlaceUnitCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceUnitCommandClass::Get_Description() const
{
    return "Places a random unit at the mouse cell.";
}

bool PlaceUnitCommandClass::Process()
{
    if (Session.Type != GAME_SKIRMISH) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }
    
    DynamicVectorClass<UnitTypeClass *> available_units;

    int owner_id = 1 << PlayerPtr->Class->HeapID;

    /**
     *  Build a list of units from the available starting units.
     */
    for (int i = 0; i < UnitTypes.Count(); ++i) {
        UnitTypeClass *unittype = UnitTypes[i];
        if (unittype && unittype->IsAllowedToStartInMultiplayer) {
            if (Rule->BaseUnit->Fetch_ID() != unittype->Fetch_ID()) {
                if (unittype->Level <= PlayerPtr->Control.TechLevel && (owner_id & unittype->Ownable) != 0) {
                    available_units.Add(unittype);
                }
            }
        }
    }

    if (!available_units.Count()) {
        DEBUG_WARNING("Failed to generate list of available UnitTypes!\n");
        return false;
    }

    UnitTypeClass *unittype = available_units[Random_Pick(0, available_units.Count()-1)];

    /**
     *  Create an instance of the unit.
     */
    UnitClass *unit = reinterpret_cast<UnitClass *>(unittype->Create_One_Of(PlayerPtr));
    if (!unit->Unlimbo(mouse_coord)) {
        delete unit;
        return false;
    }

    DEBUG_INFO("Placed unit \"{}\" at {},{},{}\n", unit->Name(), unit->Position.X, unit->Position.Y, unit->Position.Z);
    return true;
}


/**
 *  Places tiberium at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceTiberiumCommandClass::Get_Name() const
{
    return "PlaceTiberium";
}

const char *PlaceTiberiumCommandClass::Get_UI_Name() const
{
    return "Place Tiberium";
}

const char *PlaceTiberiumCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceTiberiumCommandClass::Get_Description() const
{
    return "Places tiberium at the mouse cell.";
}

bool PlaceTiberiumCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    if (cellptr->Place_Tiberium(TIBERIUM_FIRST, 1)) {
        DEBUG_INFO("Placed tiberium \"{}\" at {},{},{}\n", Tiberiums[TIBERIUM_FIRST]->IniName, mouse_coord.X, mouse_coord.Y, mouse_coord.Z);
        return true;
    }

    return false;
}


/**
 *  Reduce tiberium at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *ReduceTiberiumCommandClass::Get_Name() const
{
    return "ReduceTiberium";
}

const char *ReduceTiberiumCommandClass::Get_UI_Name() const
{
    return "Reduce Tiberium";
}

const char *ReduceTiberiumCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ReduceTiberiumCommandClass::Get_Description() const
{
    return "Reduces tiberium at the mouse cell.";
}

bool ReduceTiberiumCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    if (cellptr->Reduce_Tiberium(1)) {
        DEBUG_INFO("Reduced tiberium \"{}\" at {},{},{}\n", Tiberiums[TIBERIUM_FIRST]->IniName, mouse_coord.X, mouse_coord.Y, mouse_coord.Z);
        return true;
    }

    return false;
}


/**
 *  Places fully grown tiberium at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceFullTiberiumCommandClass::Get_Name() const
{
    return "PlaceFullTiberium";
}

const char *PlaceFullTiberiumCommandClass::Get_UI_Name() const
{
    return "Place Fully Grown Tiberium";
}

const char *PlaceFullTiberiumCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceFullTiberiumCommandClass::Get_Description() const
{
    return "Places fully grown tiberium at the mouse cell.";
}

bool PlaceFullTiberiumCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    if (cellptr->Place_Tiberium(TIBERIUM_FIRST, 11)) {
        DEBUG_INFO("Placed fully grown tiberium \"{}\" at {},{},{}\n", Tiberiums[TIBERIUM_FIRST]->IniName, mouse_coord.X, mouse_coord.Y, mouse_coord.Z);
        return true;
    }

    return false;
}


/**
 *  Removes tiberium at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *RemoveTiberiumCommandClass::Get_Name() const
{
    return "RemoveTiberium";
}

const char *RemoveTiberiumCommandClass::Get_UI_Name() const
{
    return "Remove Tiberium";
}

const char *RemoveTiberiumCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *RemoveTiberiumCommandClass::Get_Description() const
{
    return "Removes tiberium at the mouse cell.";
}

bool RemoveTiberiumCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    if (cellptr->Reduce_Tiberium(12)) {
        DEBUG_INFO("Removed tiberium at {},{},{}\n", mouse_coord.X, mouse_coord.Y, mouse_coord.Z);
        return true;
    }

    return false;
}


/**
 *  Toggles the instant recharge cheat for the players super weapons.
 * 
 *  @author: CCHyper
 */
const char *InstantSuperRechargeCommandClass::Get_Name() const
{
    return "InstantSpecialRecharge";
}

const char *InstantSuperRechargeCommandClass::Get_UI_Name() const
{
    return "Instant Special Recharge (Player)";
}

const char *InstantSuperRechargeCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *InstantSuperRechargeCommandClass::Get_Description() const
{
    return "Toggles the instant recharge cheat for the players super weapons.";
}

bool InstantSuperRechargeCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Vinifera_Developer_InstantSuperRecharge = !Vinifera_Developer_InstantSuperRecharge;

    return true;
}


/**
 *  Toggles the instant recharge cheat for the AI player super weapons.
 * 
 *  @author: CCHyper
 */
const char *AIInstantSuperRechargeCommandClass::Get_Name() const
{
    return "AIInstantSpecialRecharge";
}

const char *AIInstantSuperRechargeCommandClass::Get_UI_Name() const
{
    return "Instant Special Recharge (AI)";
}

const char *AIInstantSuperRechargeCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *AIInstantSuperRechargeCommandClass::Get_Description() const
{
    return "Toggles the instant recharge cheat for the AI player super weapons.";
}

bool AIInstantSuperRechargeCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Vinifera_Developer_AIInstantSuperRecharge = !Vinifera_Developer_AIInstantSuperRecharge;

    return true;
}


/**
 *  Toggles the instant recharge cheat for the AI player super weapons.
 *
 *  @author: CCHyper
 */
const char *DumpNetworkCRCCommandClass::Get_Name() const
{
    return "DumpNetworkCRC";
}

const char *DumpNetworkCRCCommandClass::Get_UI_Name() const
{
    return "Dump Network CRC's";
}

const char *DumpNetworkCRCCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DumpNetworkCRCCommandClass::Get_Description() const
{
    return "Dumps all the current game network state to an output log.";
}

bool DumpNetworkCRCCommandClass::Process()
{
    static unsigned _last_frame = -1;

    if (Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Check to make sure we are not within a window that might cause rapid network desync.
     */
    if (_last_frame != -1 && Frame < (_last_frame+30)) {
        return false;
    }

    /**
     *  Store the last execution frame.
     */
    _last_frame = Frame;

    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;

    Get_Full_Time(day, month, year, hour, min, sec);

    /**
     *  Create a unique filename for the sync log based on the current time and the player name.
     */
    char filename_buffer[512];
    std::snprintf(filename_buffer, sizeof(filename_buffer), "%s\\SYNC_%s-%02d_%02u-%02u-%04u_%02u-%02u-%02u.LOG",
        Vinifera_DebugDirectory,
        PlayerPtr->IniName.c_str(),
        PlayerPtr->HeapID,
        day, month, year, hour, min, sec);

    /**
     *  Open the sync log.
     */
    FILE *fp = std::fopen(filename_buffer, "w+");
    if (fp == nullptr) {
        DEBUG_ERROR("Failed to open sync log file for writing!\n");
        return false;
    }

    DEBUG_INFO("Writing sync log to file {}.\n", filename_buffer);

    Extension::Print_CRCs(fp, nullptr);

    std::fclose(fp);

    return true;
}

/**
 *  Dumps all the type heaps to an output log.
 *
 *  @author: ZivDero
 */
const char* DumpHeapsCommandClass::Get_Name() const
{
    return "DumpHeaps";
}

const char* DumpHeapsCommandClass::Get_UI_Name() const
{
    return "Dump Heaps";
}

const char* DumpHeapsCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char* DumpHeapsCommandClass::Get_Description() const
{
    return "Dumps all the type heaps to an output log.";
}

/**
 *  Handy macro for defining the logging the heaps CRCs.
 *
 *  @author: ZivDero
 */
#define LOG_HEAP(class_name, heap_name) \
    { \
        DEBUG_INFO(#class_name ":\n"); \
        if (!heap_name.Count()) { \
            DEBUG_INFO("  EMPTY\n"); \
        } else { \
            for (unsigned i = 0; i < heap_name.Count(); ++i) { \
                class_name *ptr = heap_name[i]; \
                if (ptr != nullptr) { \
                    DEBUG_INFO("  {:04}={}\n", i, ptr->Name()); \
                } \
            } \
        } \
        DEBUG_INFO("\n"); \
    }

bool DumpHeapsCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    DEBUG_INFO("\nAbout to dump heaps...\n\n");

    LOG_HEAP(HouseTypeClass, HouseTypes);

    LOG_HEAP(UnitTypeClass, UnitTypes);
    LOG_HEAP(InfantryTypeClass, InfantryTypes);
    LOG_HEAP(BuildingTypeClass, BuildingTypes);
    LOG_HEAP(AircraftTypeClass, AircraftTypes);

    LOG_HEAP(TerrainTypeClass, TerrainTypes);
    LOG_HEAP(SmudgeTypeClass, SmudgeTypes);
    LOG_HEAP(OverlayTypeClass, OverlayTypes);

    LOG_HEAP(AnimTypeClass, AnimTypes);
    LOG_HEAP(VoxelAnimTypeClass, VoxelAnimTypes);
    LOG_HEAP(ParticleTypeClass, ParticleTypes);
    LOG_HEAP(ParticleSystemTypeClass, ParticleSystemTypes);

    LOG_HEAP(WeaponTypeClass, Weapons);
    LOG_HEAP(WarheadTypeClass, Warheads);
    LOG_HEAP(SuperWeaponTypeClass, SuperWeaponTypes);
    LOG_HEAP(BulletTypeClass, BulletTypes);

    LOG_HEAP(TiberiumClass, Tiberiums);
    LOG_HEAP(ArmorTypeClass, ArmorTypes);
    LOG_HEAP(RocketTypeClass, RocketTypes);

    DEBUG_INFO("\nFinished!\n\n");

    return true;
}


/**
 *  Reloads the Rules and Art INI files.
 * 
 *  @author: CCHyper
 */
const char *ReloadRulesCommandClass::Get_Name() const
{
    return "ReloadRules";
}

const char *ReloadRulesCommandClass::Get_UI_Name() const
{
    return "Reload Rules";
}

const char *ReloadRulesCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ReloadRulesCommandClass::Get_Description() const
{
    return "Reloads the Rules and Art INI files.";
}

bool ReloadRulesCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Vinifera_Developer_IsToReloadRules = true;

    return true;
}


/**
 *  Creates a meteor shower around the current mouse cell.
 * 
 *  @author: CCHyper
 */
const char *MeteorShowerCommandClass::Get_Name() const
{
    return "MeteorShower";
}

const char *MeteorShowerCommandClass::Get_UI_Name() const
{
    return "Meteor Shower";
}

const char *MeteorShowerCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *MeteorShowerCommandClass::Get_Description() const
{
    return "Creates a meteor shower around the current mouse cell.";
}

bool MeteorShowerCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    if (!Map.In_Radar(mouse_coord)) {
        return false;
    }

    static int const _meteor_counts[] = { 4, 8, 10 };

    /**
     *  Random pick how many meteors in the shower.
     */
    int count = Random_Pick<unsigned>(0, std::size(_meteor_counts)-1);

    const AnimTypeClass *large_meteor = AnimTypes[AnimTypeClass::From_Name("METLARGE")];
    const AnimTypeClass *small_meteor = AnimTypes[AnimTypeClass::From_Name("METSMALL")];

    for (int i = 0; i < count; ++i) {

        /**
         *  Add a random adjust to the position of the meteor within the shower.
         */
        int x_adj = Scen->RandomNumber() % (count * (CELL_LEPTON_W/2));
        int y_adj = Scen->RandomNumber() % (count * (CELL_LEPTON_H/2));

        Coord where = mouse_coord;

        where.X += x_adj;
        where.Y += y_adj;
        where.Z = Map.Get_Height_GL(where);

        const AnimTypeClass *anim = Percent_Chance(30) ? large_meteor : small_meteor;

        new AnimClass(anim, where);
    }

    return true;
}


/**
 *  Sends a meteor at the current mouse cell.
 * 
 *  @author: CCHyper
 */
const char *MeteorImpactCommandClass::Get_Name() const
{
    return "MeteorImpact";
}

const char *MeteorImpactCommandClass::Get_UI_Name() const
{
    return "Meteor Impact";
}

const char *MeteorImpactCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *MeteorImpactCommandClass::Get_Description() const
{
    return "Sends a meteor at the current mouse cell.";
}

bool MeteorImpactCommandClass::Process()
{
    if (Session.Players.Count() > 1) {
        return false;
    }

    Coord mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Height_GL(mouse_coord);

    if (!Map.In_Radar(mouse_coord)) {
        return false;
    }

    /**
     *  Pick a random a random meteor object.
     */
    const VoxelAnimTypeClass *voxelanimtypeptr = VoxelAnimTypes[VoxelAnimTypeClass::From_Name(Percent_Chance(50) ? "METEOR01" : "METEOR02")];
    if (!voxelanimtypeptr) {
        return false;
    }

    new VoxelAnimClass(voxelanimtypeptr, mouse_coord);

    return true;
}


/**
 *  Toggle the in-game ImGui debug overlay window.
 *
 *  @author: ZivDero
 */
const char *ToggleDebugOverlayCommandClass::Get_Name() const
{
    return "ToggleDebugOverlay";
}

const char *ToggleDebugOverlayCommandClass::Get_UI_Name() const
{
    return "Toggle Game Info";
}

const char *ToggleDebugOverlayCommandClass::Get_Category() const
{
    return "Interface";
}

const char *ToggleDebugOverlayCommandClass::Get_Description() const
{
    return "Shows or hides the Game Info overlay window.";
}

bool ToggleDebugOverlayCommandClass::Process()
{
    DebugOverlay::IsVisible = !DebugOverlay::IsVisible;
    return true;
}


/**
 *  Toggle the developer-mode scenario debug window.
 *
 *  @author: ZivDero
 */
const char *ToggleScenarioOverlayCommandClass::Get_Name() const
{
    return "ToggleScenarioOverlay";
}

const char *ToggleScenarioOverlayCommandClass::Get_UI_Name() const
{
    return "Toggle Scenario Overlay";
}

const char *ToggleScenarioOverlayCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleScenarioOverlayCommandClass::Get_Description() const
{
    return "Shows or hides the Vinifera scenario debug window (types, instances, variables, waypoints, AI nodes).";
}

bool ToggleScenarioOverlayCommandClass::Process()
{
    ScenarioOverlay::IsVisible = !ScenarioOverlay::IsVisible;
    return true;
}
