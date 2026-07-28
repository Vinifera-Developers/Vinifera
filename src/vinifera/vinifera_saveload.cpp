/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Utility functions for saving and loading.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "vinifera_saveload.h"

#include "addon.h"
#include "aircraft.h"
#include "aircrafttracker.h"
#include "aircrafttype.h"
#include "aitrigtype.h"
#include "alphashape.h"
#include "anim.h"
#include "animtype.h"
#include "armortype.h"
#include "audio_static_sound.h"
#include "battleui.h"
#include "beacon.h"
#include "building.h"
#include "buildinglight.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "bullet.h"
#include "bullettype.h"
#include "ccini.h"
#include "cstream.h"
#include "debughandler.h"
#include "empulse.h"
#include "environment.h"
#include "extension.h"
#include "factory.h"
#include "foggedobject.h"
#include "hooker.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "infantry.h"
#include "infantrytype.h"
#include "iomap.h"
#include "isotiletypeext.h"
#include "kamikazetracker.h"
#include "language.h"
#include "lightsource.h"
#include "loadoptions.h"
#include "logic.h"
#include "miscutil.h"
#include "overlaytype.h"
#include "particle.h"
#include "particlesys.h"
#include "particlesystype.h"
#include "particletype.h"
#include "prerequisitegroup.h"
#include "radarevent.h"
#include "rockettype.h"
#include "rules.h"
#include "saveload.h"
#include "savever.h"
#include "scenario.h"
#include "scenarioext.h"
#include "script.h"
#include "scripttype.h"
#include "session.h"
#include "side.h"
#include "smudgetype.h"
#include "spawnmanager.h"
#include "super.h"
#include "supertype.h"
#include "tactical.h"
#include "taction.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforce.h"
#include "team.h"
#include "teamtype.h"
#include "technoext.h"
#include "terrain.h"
#include "terraintype.h"
#include "tevent.h"
#include "tiberium.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "triggertype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "veinholemonster.h"
#include "verses.h"
#include "vinifera_gitinfo.h"
#include "vinifera_savever.h"
#include "vox.h"
#include "voxelanim.h"
#include "voxelanimtype.h"
#include "warheadtype.h"
#include "wave.h"
#include "waypointpath.h"
#include "weapontype.h"
#include "windialog.h"

#include <atlbase.h>


/**
 *  Constant of the current build version number. This number should be
 *  a sum of all the extended class sizes plus the build date.
 */
unsigned ViniferaGameVersion = 0x0;


/**
 *  Saves all active objects to the data stream.
 *
 *  @author: CCHyper
 */
template<class T>
static HRESULT Vinifera_Save_Vector(LPSTREAM &pStm, DynamicVectorClass<T> &list, const char *heap_name)
{
    DEBUG_INFO("Saving {}...\n", heap_name);

    /**
     *  Save the number of instances of this class.
     */
    int count = list.Count();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        DEBUG_ERROR("  Failed to read count!\n");
        return hr;
    }

    if (count <= 0) {
        DEV_DEBUG_INFO("  Count was zero, skipping save.\n");
        return hr;
    }

    DEBUG_INFO("  Count: {}\n", list.Count());

    /**
     *  Save each instance of this class.
     */
    for (int index = 0; index < count; ++index) {

        /**
         *  Tell the extension class to persist itself into the data stream.
         */
        IPersistStream *lpPS = nullptr;
        hr = list[index]->QueryInterface(__uuidof(IPersistStream), (LPVOID *)&lpPS);
        if (FAILED(hr)) {
            DEBUG_ERROR("  QueryInterface failed!\n");
            return hr;
        }

        /**
         *  Save the object itself.
         */
        hr = OleSaveToStream(lpPS, pStm);
        if (FAILED(hr)) {
            DEBUG_ERROR("  OleSaveToStream failed!\n");
            return hr;
        }

        /**
         *  Release the interface.
         */
        hr = lpPS->Release();
        if (FAILED(hr)) {
            DEBUG_ERROR("  Release failed!\n");
            return hr;
        }

    }

    return hr;
}


/**
 *  Loads all active objects form the data stream.
 * 
 *  @author: CCHyper
 */
template<class T>
static HRESULT Vinifera_Load_Vector(IStream *pStm, DynamicVectorClass<T> &list, const char *heap_name)
{
    DEBUG_INFO("Loading {}...\n", heap_name);

    /**
     *  Read the number of instances of this class.
     */
    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    if (count <= 0) {
        DEV_DEBUG_INFO("  Count was zero, skipping load.\n");
        return hr;
    }

    DEBUG_INFO("  Count: {}\n", count);
    
    /**
     *  Read each class instance.
     */
    for (int index = 0; index < count; ++index) {
        
        /**
         *  Load the object.
         */
        IUnknown *spUnk = nullptr;
        hr = OleLoadFromStream(pStm, __uuidof(IUnknown), (LPVOID *)&spUnk);
        if (FAILED(hr)) {
            DEBUG_ERROR("  OleLoadFromStream failed!\n");
            return hr;
        }

    }

    return hr;
}

template<typename TKey, typename TValue, typename THash, typename TEqual>
static HRESULT Save_Unordered_Map(LPSTREAM& pStm, std::unordered_map<TKey, TValue, THash, TEqual>& map, const char* heap_name)
{
    DEBUG_INFO("Saving unordered map {}...\n", heap_name);

    /**
     *  Save the number of entries in the map.
     */
    int count = static_cast<int>(map.size());
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        DEBUG_ERROR("  Failed to write map count!\n");
        return hr;
    }

    if (count <= 0) {
        DEV_DEBUG_INFO("  Map count was zero, skipping save.\n");
        return hr;
    }

    DEBUG_INFO("  Count: {}\n", count);

    /**
     *  Iterate through the map.     
     */
    for (auto const& [key, val] : map) {
        
        hr = pStm->Write(&key, sizeof(TKey), nullptr);
        if (FAILED(hr)) {
            DEBUG_ERROR("  Failed to write Key data!\n");
            return hr;
        }
        
        hr = pStm->Write(&val, sizeof(TValue), nullptr);
        if (FAILED(hr)) {
            DEBUG_ERROR("  Failed to write Value data!\n");
            return hr;
        }
    }

    return S_OK;
}

/**
 * Loads a map of objects from the data stream.
 */
template<typename TKey, typename TValue, typename THash, typename TEqual>
static HRESULT Load_Unordered_Map(IStream* pStm, std::unordered_map<TKey, TValue, THash, TEqual>& map, const char* heap_name)
{
    DEBUG_INFO("Loading unordered map {}...\n", heap_name);
    
    map.clear();

    /**
     * Read the number of entries.
     */
    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        DEBUG_ERROR("  Failed to read map count!\n");
        return hr;
    }

    if (count <= 0) {
        DEV_DEBUG_INFO("  Count was zero, skipping load.\n");
        return hr;
    }

    DEBUG_INFO("  Count: {}\n", count);

    /**
     * Read each Key-Value pair.
     */
    for (int i = 0; i < count; ++i) {
        TKey key;
        TValue value;

        // Read the Key (The Cell data)
        hr = pStm->Read(&key, sizeof(TKey), nullptr);
        if (FAILED(hr)) {
            DEBUG_ERROR("  Failed to read Key at index {}!\n", i);
            return hr;
        }

        // Read the Value (The int)
        hr = pStm->Read(&value, sizeof(TValue), nullptr);
        if (FAILED(hr)) {
            DEBUG_ERROR("  Failed to read Value at index {}!\n", i);
            return hr;
        }

        /**
         * Insert into the map.
         * This automatically invokes your custom Hash and Equality logic.
         */
        map[key] = value;
    }

    return S_OK;
}


/**
 *  Saves the game state to the file stream.
 */
bool Vinifera_Put_All(IStream *pStm, bool save_net)
{
    /**
     *  Multiplayer move flashes are local-only cosmetic anims that live outside
     *  the Anims heap, so they are never written to the save stream, but they
     *  still sit in the map layers, whose pointers cannot be unswizzled on load.
     *  Delete any that are alive before any state is saved.
     */
    while (MoveFlashes.Count() > 0) {
        delete MoveFlashes[MoveFlashes.Count() - 1];
    }

    /**
     *  Save the scenario global information.
     */
    DEBUG_INFO("Saving Scenario...\n");
    Scen->Save_Self(pStm);

    DEBUG_INFO("Saving Environment...\n");
    Environment.Save(pStm);

    DEBUG_INFO("Saving Rule...\n");
    Rule->Save(pStm);

    if (FAILED(Vinifera_Save_Vector(pStm, AnimTypes, "AnimTypes"))) { return false; }

    /**
     *  Save the map. The map must be saved first, since it saves the Theater.
     */
    DEBUG_INFO("Saving Map...\n");
    if (FAILED(Map.Save(pStm))) { return false; }

    if (FAILED(Vinifera_Save_Vector(pStm, Tubes, "Tunnels"))) { return false; }

    /**
     *  Save miscellaneous variables.
     */
    DEBUG_INFO("Saving Misc. Values...\n");
    if (FAILED(Save_Misc_Values(pStm))) { return false; }

    /**
     *  Save the Logic & Map layers.
     */
    DEBUG_INFO("Saving Logic...\n");
    if (FAILED(Logic.Save(pStm))) { return false; }

    DEBUG_INFO("Saving TacticalMap...\n");
    {
        if (FAILED(OleSaveToStream(TacticalMap, pStm))) { return false; }
    }

    /**
     *  Save all game objects. This code saves every object that's stored in a DynamicVector class.
     */
    if (FAILED(Vinifera_Save_Vector(pStm, HouseTypes, "HouseTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Houses, "Houses"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Units, "Units"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, UnitTypes, "UnitTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, InfantryTypes, "InfantryTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Infantry, "Infantry"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, BuildingTypes, "BuildingTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Buildings, "Buildings"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, AircraftTypes, "AircraftTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Aircrafts, "Aircraft"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Anims, "Anims"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, TaskForces, "TaskForces"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, TeamTypes, "TeamTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Teams, "Teams"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, ScriptTypes, "ScriptTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Scripts, "Scripts"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, TagTypes, "TagTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Tags, "Tags"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, TriggerTypes, "TriggerTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Triggers, "Triggers"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, AITriggerTypes, "AITriggerTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, TActions, "Actions"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, TEvents, "Events"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Factories, "Factories"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, VoxelAnimTypes, "VoxelAnimTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, VoxelAnims, "VoxelAnims"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Warheads, "Warheads"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Weapons, "Weapons"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, ParticleTypes, "ParticleTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Particles, "Particles"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, ParticleSystemTypes, "ParticleSystemTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, ParticleSystems, "ParticleSystems"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, BulletTypes, "BulletTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Bullets, "Bullets"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, WaypointPaths, "WaypointPaths"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, SmudgeTypes, "SmudgeTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, OverlayTypes, "OverlayTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, LightSources, "LightSources"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, BuildingLights, "BuildingLights"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Sides, "Sides"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Tiberiums, "Tiberiums"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Empulses, "Empulses"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, SuperWeaponTypes, "SuperWeaponTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Supers, "SuperWeapons"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, TerrainTypes, "TerrainTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Terrains, "Terrains"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, FoggedObjects, "FoggedObjects"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, AlphaShapes, "AlphaShapes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, Waves, "Waves"))) { return false; }
    { DEBUG_INFO("Saving VeinholeMonsters...\n"); if (FAILED(VeinholeMonsterClass::Save_All(pStm))) { DEBUG_ERROR("\t***** FAILED!\n"); return false; } }
    { DEBUG_INFO("Saving RadarEvents...\n"); if (!RadarEventClass::Save_All(pStm)) { DEBUG_ERROR("\t***** FAILED!\n"); return false; } }

    /**
     *  Save new Vinifera objects stored in vectors.
     */
    if (FAILED(Vinifera_Save_Vector(pStm, ArmorTypes, "ArmorTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, RocketTypes, "RocketTypes"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, SpawnManagers, "SpawnManagers"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, PrerequisiteGroups, "PrerequisiteGroups"))) { return false; }

    /**
     *  Save houses' unit tracker data.
     */
    for (int i = 0; i < Houses.Count(); i++) {
        HouseClassExtension::Save_Unit_Trackers(Houses[i], pStm);
    }

    /**
     *  Save new Verses.
     */
    if (FAILED(Verses::Save(pStm))) { return false; }

    /**
     *  Save new global class instances.
     */
    KamikazeTracker->Save(pStm, false);
    AircraftTracker->Save(pStm);

    Save_Tracked_Static_Sounds(pStm);

    /**
     *  Save skirmish values.
     */
    if (Session.Type == GAME_SKIRMISH) {
        DEBUG_INFO("Saving Skirmish Session.Options...\n");
        if (!Session.Options.Save(pStm)) { return false; }
    }

    /**
     *  Save class extensions here.
     */
    DEBUG_INFO("Saving class extensions\n");
    if (!Extension::Save(pStm)) {
        DEBUG_ERROR("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Save the battle UI state.
     */
    DEBUG_INFO("Saving BattleUI...\n");
    if (FAILED(BattleUI.Save(pStm))) { return false; }

    /**
     *  Save the bridge health tracker map
     */
    DEBUG_INFO("Saving bridge health trackers\n");
    if (FAILED(Save_Unordered_Map(pStm, BridgeHealths, "BridgeHealths"))) {
        return false;
    }

    return true;
}


/**
 *  Loads the game state to the file stream in the same way it was saved out.
 * 
 *  @warning: If this routine returns false, the entire game will be in an
 *            unknown state, so the scenario will have to be re-initialized!
 */
bool Vinifera_Get_All(IStream *pStm, bool load_net)
{
    /**
     *  Clear the existing scenario data, ready for loading.
     */
    DEBUG_INFO("About to call Clear_Scenario()...\n");
    Clear_Scenario();

    /**
     *  Now the scenario data has been cleaned up, we can now tell the extension
     *  hooks that we will be creating the extension classes via the class factories.
     * 
     *  Fixes #issue-951, this line was not copied over when the loading process
     *  was reimplemented.
     */
    Vinifera_PerformingLoad = true;

    /**
     *  Load the scenario global information.
     */
    DEBUG_INFO("Loading Scenario...\n");
    Scen->Load_Self(pStm);

    /**
     *  #issue-123
     *
     *  Save files do not store the tutorial messages, so we reload them from
     *  the scenario file.
     */
    {
        DEBUG_INFO("Loading Tutorial section from scenario (if present)...\n");

        CCFileClass scen_file(Scen->ScenarioName);
        CCINIClass scen_ini;

        if (!scen_file.Is_Available()) {
            DEBUG_ERROR("Failed to read scenario file!\n");
            return false;
        }

        scen_ini.Load(scen_file, false);

        if (!ScenExtension->Read_Tutorial_INI(scen_ini)) {
            DEBUG_ERROR("Failed to read tutorial strings from scenario file!\n");
            return false;
        }
    }

    Disable_Addon(ADDON_BASE_GAME);

    DEBUG_INFO("Setting required addon to '{}'\n", (int)Scen->RequiredAddOn);
    Set_Required_Addon(Scen->RequiredAddOn);

    if (!Addon_Installed(Scen->RequiredAddOn)) {
        DEBUG_ERROR("Addon '{}' is not installed!\n", (int)Scen->RequiredAddOn);
        return false;
    }

    Enable_Addon(Scen->RequiredAddOn);

    SideType side = Scen->IsGDI ? SIDE_GDI : SIDE_NOD;
#if defined(TS_CLIENT)
    side = static_cast<SideType>(Scen->IsGDI);
#endif

    DEBUG_INFO("About to call Prep_For_Side()...\n");
    if (!Prep_For_Side(side)) {
        DEBUG_ERROR("Prep_For_Side() failed!\n");
        return false;
    }

    {
    Rect tactical_rect = Get_Tactical_Rect();
    Rect composite_rect(0, 0, tactical_rect.Width, VisibleRect.Height);
    Rect tile_rect(0, 0, tactical_rect.Width, VisibleRect.Height);
    Rect sidebar_rect(tactical_rect.X, tactical_rect.Y, SidebarClass::SIDE_WIDTH, VisibleRect.Height);
    DEBUG_INFO("About to call Allocate_Surfaces()...\n");
    Allocate_Surfaces(VisibleRect, composite_rect, tile_rect, sidebar_rect);

    DEBUG_INFO("About to call Map.Set_View_Dimensions()...\n");
    Map.Set_View_Dimensions(tactical_rect);
    }

    DEBUG_INFO("Loading Environment...\n");
    Environment.Load(pStm);

    Init_Theater(Scen->Theater);

    DEBUG_INFO("About to call Load_Art_INI()...\n");
    RulesClass::Load_Art_INI();

    if (Addon_Enabled(ADDON_FIRESTORM)) {
        DEBUG_INFO("About to call Load_ArtFS_INI()...\n");
        RulesClass::Load_ArtFS_INI();
    }

    DEBUG_INFO("Loading Rule...\n");
    Rule->Load(pStm);

    DEBUG_INFO("About to call Prep_Speech_For_Side()...\n");
    if (!Prep_Speech_For_Side(side)) {
        DEBUG_ERROR("Prep_Speech_For_Side() failed!\n");
        return false;
    }

    if (FAILED(Vinifera_Load_Vector(pStm, AnimTypes, "AnimTypes"))) { return false; }

    /**
     *  Load the map. The map must be loaded first, since it initialises the Theater.
     */
    DEBUG_INFO("Loading Map...\n");
    if (FAILED(Map.Load(pStm))) { return false; }

    if (FAILED(Vinifera_Load_Vector(pStm, Tubes, "Tunnels"))) { return false; }

    /**
     *  Load miscellaneous variables.
     */
    DEBUG_INFO("Loading Misc. Values...\n");
    if (FAILED(Load_Misc_Values(pStm))) { return false; }

    DEBUG_INFO("About to call Map.Clear_SubZones()...\n");
    Map.Clear_SubZones();

    /**
     *  Load the Logic & Map layers.
     */
    DEBUG_INFO("Loading Logic...\n");
    if (FAILED(Logic.Load(pStm))) { return false; }

    DEBUG_INFO("Loading TacticalMap...\n");
    {
        delete TacticalMap;
        IUnknown *spUnk = nullptr;
        if (FAILED(OleLoadFromStream(pStm, __uuidof(IUnknown), (LPVOID *)&spUnk))) { return false; }
    }

    /**
     *  Load all game objects. This code loads every object that's stored in a DynamicVector class.
     */
    if (FAILED(Vinifera_Load_Vector(pStm, HouseTypes, "HouseTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Houses, "Houses"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Units, "Units"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, UnitTypes, "UnitTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, InfantryTypes, "InfantryTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Infantry, "Infantry"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, BuildingTypes, "BuildingTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Buildings, "Buildings"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, AircraftTypes, "AircraftTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Aircrafts, "Aircraft"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Anims, "Anims"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, TaskForces, "TaskForces"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, TeamTypes, "TeamTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Teams, "Teams"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, ScriptTypes, "ScriptTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Scripts, "Scripts"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, TagTypes, "TagTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Tags, "Tags"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, TriggerTypes, "TriggerTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Triggers, "Triggers"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, AITriggerTypes, "AITriggerTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, TActions, "Actions"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, TEvents, "Events"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Factories, "Factories"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, VoxelAnimTypes, "VoxelAnimTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, VoxelAnims, "VoxelAnims"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Warheads, "Warheads"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Weapons, "Weapons"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, ParticleTypes, "ParticleTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Particles, "Particles"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, ParticleSystemTypes, "ParticleSystemTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, ParticleSystems, "ParticleSystems"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, BulletTypes, "BulletTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Bullets, "Bullets"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, WaypointPaths, "WaypointPaths"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, SmudgeTypes, "SmudgeTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, OverlayTypes, "OverlayTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, LightSources, "LightSources"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, BuildingLights, "BuildingLights"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Sides, "Sides"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Tiberiums, "Tiberiums"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Empulses, "Empulses"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, SuperWeaponTypes, "SuperWeaponTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Supers, "SuperWeapons"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, TerrainTypes, "TerrianTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Terrains, "Terrains"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, FoggedObjects, "FoggedObjects"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, AlphaShapes, "AlphaShapes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, Waves, "Waves"))) { return false; }
    { DEBUG_INFO("Loading VeinholeMonsters...\n"); if (FAILED(VeinholeMonsterClass::Load_All(pStm))) { DEBUG_ERROR("\t***** FAILED!\n"); return false; } }
    { DEBUG_INFO("Loading RadarEvents...\n"); if (!RadarEventClass::Load_All(pStm)) { DEBUG_ERROR("\t***** FAILED!\n");  return false; } }

    /**
     *  Load new Vinifera objects stored in vectors.
     */
    if (FAILED(Vinifera_Load_Vector(pStm, ArmorTypes, "ArmorTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, RocketTypes, "RocketTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, SpawnManagers, "SpawnManagers"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, PrerequisiteGroups, "PrerequisiteGroups"))) { return false; }

    /**
     *  Load houses' unit tracker data.
     */
    for (int i = 0; i < Houses.Count(); i++) {
        HouseClassExtension::Load_Unit_Trackers(Houses[i], pStm);
    }

    /**
     *  Load new Verses.
     */
    if (FAILED(Verses::Load(pStm))) { return false; }

    /**
     *  Load new global class instances.
     */
    KamikazeTracker->Clear();
    KamikazeTracker->Load(pStm);

    AircraftTracker->Clear();
    AircraftTracker->Load(pStm);

    Load_Tracked_Static_Sounds(pStm);

    /**
     *  Load skirmish values.
     */
    if (Session.Type == GAME_SKIRMISH) {
        DEBUG_INFO("Loading Skirmish Session.Options...\n");
        if (!Session.Options.Load(pStm)) { return false; }
    }

    /**
     *  Load class extensions here.
     */
    DEBUG_INFO("Loading class extensions\n");
    if (!Extension::Load(pStm)) {
        DEBUG_ERROR("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Load the battle UI state.
     */
    DEBUG_INFO("Loading BattleUI...\n");
    SpeechEnabled = false;
    if (FAILED(BattleUI.Load(pStm))) { return false; }
    SpeechEnabled = true;

    Map.Flag_To_Redraw(GS_REDRAW_ALL);

    //Vinifera_Remap_Extension_Pointers();

    /**
     *  Load the bridge health tracker map
     */
    DEBUG_INFO("Loading bridge health trackers\n");
    if (FAILED(Load_Unordered_Map(pStm, BridgeHealths, "BridgeHealths"))) {
        return false;
    }

    /**
     *  We have finished loading the game data, reset the load flag.
     */
    Vinifera_PerformingLoad = false;

    return true;
}


/**
 *  Request remapping of all the extension pointers so the swizzle manager
 *  can fix up any reference to extension classes.
 *
 *  @author: CCHyper
 */
bool Vinifera_Remap_Extension_Pointers()
{
    DEBUG_INFO("Remapping extension pointers\n");
    if (!Extension::Request_Pointer_Remap()) {
        DEBUG_ERROR("\t***** FAILED!\n");
        return false;
    }

    return true;
}


/**
 *  Restores pointers to storage vectors in vanilla classes.
 *
 *  @author: ZivDero
 */
void Vinifera_Post_Load_Game()
{
    for (int i = 0; i < Technos.Count(); i++) {
        const TechnoClass* techno = Technos[i];
        Extension::Fetch(techno)->Put_Storage_Pointers();
    }

    for (int i = 0; i < Houses.Count(); i++) {
        const HouseClass* house = Houses[i];
        Extension::Fetch(house)->Put_Storage_Pointers();
    }

    for (int i = 0; i < BuildingTypes.Count(); i++) {
        const BuildingTypeClass* buildingtype = BuildingTypes[i];
        Extension::Fetch(buildingtype)->Fetch_Building_Normal_Image(Scen->Theater);
    }

    /**
     *  IsometricTileTypes are created before objects are loaded, so they are read from INI incorrectly.
     *  Let's re-read them now.
     */
    char theater_filename[32];
    std::snprintf(theater_filename, sizeof(theater_filename), "%s.INI", Theaters[Scen->Theater].Root);
    CCFileClass theater_file(theater_filename);
    CCINIClass theater_ini;
    theater_ini.Load(theater_file, false);

    for (auto isotype_extension : IsometricTileTypeExtensions) {
        isotype_extension->Read_INI(theater_ini);
    }

    BeaconManager.Load_Art();
}


/**
 *  Writes the game state to a save file on the disk.
 *
 *  @author: ZivDero
 */
static bool Vinifera_Write_Save_File(const WCHAR* wide_file_name, const char* descr)
{
    DEBUG_INFO("Creating DocFile\n");
    CComPtr<IStorage> storage;
    HRESULT hr = StgCreateDocfile(wide_file_name, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &storage);
    if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create storage.\n");
        return false;
    }

    /**
     *  Write the save file header.
     */
    ViniferaSaveVersionInfo versioninfo;
    versioninfo.Set_Internal_Version(GameVersion);
    versioninfo.Set_Scenario_Description(descr);
    versioninfo.Set_Version(1);
    versioninfo.Set_Player_House(PlayerPtr->Class->Full_Name());
    versioninfo.Set_Campaign_Number(Scen->Campaign);
    versioninfo.Set_Scenario_Number(Scen->Scenario);
    versioninfo.Set_Executable_Name(VINIFERA_DLL);
    versioninfo.Set_Game_Type(Session.Type);

    FILETIME filetime;
    CoFileTimeNow(&filetime);
    versioninfo.Set_Last_Time(filetime);
    versioninfo.Set_Start_Time(filetime);
    versioninfo.Set_Play_Time(filetime);

    versioninfo.Set_Vinifera_Version(ViniferaGameVersion);
    versioninfo.Set_Vinifera_Commit_Hash(Vinifera_Git_Hash());
    versioninfo.Set_Session_ID(Session.UniqueID);
    versioninfo.Set_Difficulty(Scen->Difficulty);
    versioninfo.Set_Total_Play_Time(Vinifera_TotalPlayTime + Scen->ElapsedTimer.Value());

    DEBUG_INFO("Saving version information\n");
    if (FAILED(versioninfo.Save(storage))) {
        DEBUG_ERROR("Failed to write version information.\n");
        return false;
    }

    DEBUG_INFO("Creating content stream.\n");
    CComPtr<IStream> docfile;
    hr = storage->CreateStream(L"CONTENTS", STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &docfile);
    if (FAILED(hr)) {
        DEBUG_ERROR("Failed to create content stream.\n");
        return false;
    }

    DEBUG_INFO("Linking content stream to compressor.\n");
    IUnknown* pUnknown = nullptr;
    CComPtr<ILinkStream> linkstream;
    hr = CoCreateInstance(__uuidof(CStreamClass), nullptr, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pUnknown);
    if (SUCCEEDED(hr)) {
        hr = OleRun(pUnknown);
        if (SUCCEEDED(hr)) {
            pUnknown->QueryInterface(__uuidof(ILinkStream), (void**)&linkstream);
        }
        pUnknown->Release();
    }

    if (linkstream == nullptr) {
        DEBUG_ERROR("Failed to create the stream compressor.\n");
        return false;
    }

    hr = linkstream->Link_Stream(docfile);
    if (FAILED(hr)) {
        DEBUG_ERROR("Failed to link stream to compressor.\n");
        return false;
    }

    CComPtr<IStream> stream;
    linkstream->QueryInterface(__uuidof(IStream), (void**)&stream);

    DEBUG_INFO("Calling Vinifera_Put_All().\n");
    bool result = Vinifera_Put_All(stream, false);

    DEBUG_INFO("Unlinking content stream from compressor.\n");
    hr = linkstream->Unlink_Stream(nullptr);
    if (FAILED(hr)) {
        DEBUG_ERROR("Failed to unlink stream from compressor.\n");
        return false;
    }

    /**
     *  If writing the game state failed, don't commit the storage - the file
     *  is going to be discarded.
     */
    if (!result) {
        DEBUG_ERROR("Failed to write the game state.\n");
        return false;
    }

    DEBUG_INFO("Releasing content stream.\n");
    docfile.Release();

    DEBUG_INFO("Closing DocFile.\n");
    hr = storage->Commit(STGC_DEFAULT);
    if (FAILED(hr)) {
        DEBUG_ERROR("Failed to commit storage.\n");
        return false;
    }

    return true;
}


/**
 *  Saves the game to a file on the disk.
 *
 *  The game state is first written to a temporary file, which replaces the
 *  target file only if the save was successful, so that a failed save can
 *  never destroy an existing save file.
 *
 *  @author: ZivDero
 */
bool Vinifera_Save_Game(const char* file_name, const char* descr, bool)
{
    WCHAR wide_temp_file_name[PATH_MAX];
    char formatted_file_name[PATH_MAX];
    char temp_file_name[PATH_MAX];

    /**
     *  Format the save game path here just in case to make sure it contains the subdirectory.
     *  In the future, it should be the call sites of Save_Game that are patched so that we can still
     *  save to an arbitrary location, but until the TS-Patches spawner is ported, this needs to happen.
     */
    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(file_name), nullptr);

    DEBUG_INFO("SAVING GAME [{} - {}]\n", formatted_file_name, descr);

    /**
     *  This is required for compatibility with TS Client's sidebar hack.
     */
#if defined(TS_CLIENT)
    Scen->IsGDI = Session.IsGDI;
#endif

    /**
     *  Make sure our saved games folder exists.
     */
    if (!Directory_Exists(Vinifera_SavedGamesDirectory)) {
        Create_Directory(Vinifera_SavedGamesDirectory);
    }

    /**
     *  Write the save to a temporary file next to the target file.
     */
    std::snprintf(temp_file_name, sizeof(temp_file_name), "%s.tmp", formatted_file_name);
    MultiByteToWideChar(CP_ACP, 0, temp_file_name, -1, wide_temp_file_name, std::size(wide_temp_file_name));

    if (!Vinifera_Write_Save_File(wide_temp_file_name, descr)) {
        Delete_File(temp_file_name);
        DEBUG_ERROR("SAVING GAME [{}] - FAILED!\n", formatted_file_name);
        return false;
    }

    /**
     *  The save was written out successfully, let it replace the target file.
     */
    if (!Replace_File(temp_file_name, formatted_file_name)) {
        Delete_File(temp_file_name);
        DEBUG_ERROR("SAVING GAME [{}] - FAILED! Could not replace the save file.\n", formatted_file_name);
        return false;
    }

    DEBUG_INFO("SAVING GAME [{}] - Complete.\n", formatted_file_name);

    return true;
}


/**
 *  Load the game from a file on the disk.
 *
 *  @author: ZivDero
 */
bool Vinifera_Load_Game(const char* file_name)
{
    WCHAR wide_file_name[PATH_MAX];
    char formatted_file_name[PATH_MAX];

    /**
     *  Format the save game path here just in case to make sure it contains the subdirectory.
     *  In the future, it should be the call sites of Load_Game that are patched so that we can still
     *  save to an arbitrary location, but until the TS-Patches spawner is ported, this needs to happen.
     */
    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(file_name), nullptr);

    DEBUG_INFO("LOADING GAME [{}]\n", formatted_file_name);

    /**
     *  Convert the file name to a wide string.
     */
    MultiByteToWideChar(CP_ACP, 0, formatted_file_name, -1, wide_file_name, std::size(wide_file_name));

    DEBUG_INFO("Opening DocFile\n");
    CComPtr<IStorage> storage;
    HRESULT hr = StgOpenStorage(wide_file_name, nullptr, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, nullptr, 0, &storage);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to open storage.\n");
        return false;
    }

    /**
     *  Read the save file header.
     */
    ViniferaSaveVersionInfo saveversion;
    hr = saveversion.Load(storage);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to read version information.\n");
        return false;
    }

    storage.Release();
    Session.Type = static_cast<GameEnum>(saveversion.Get_Game_Type());
    Vinifera_TotalPlayTime = saveversion.Get_Total_Play_Time();
    SwizzleManager.Reset();

    DEBUG_INFO("Opening DocFile\n");
    hr = StgOpenStorage(wide_file_name, nullptr, STGM_SHARE_DENY_WRITE, nullptr, 0, &storage);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to open storage.\n");
        return false;
    }

    DEBUG_INFO("Opening content stream.\n");
    CComPtr<IStream> docfile;
    hr = storage->OpenStream(L"CONTENTS", nullptr, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &docfile);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to open content stream.\n");
        return false;
    }

    DEBUG_INFO("Linking content stream to decompressor.\n");
    IUnknown* pUnknown = nullptr;
    CComPtr<ILinkStream> linkstream;
    hr = CoCreateInstance(__uuidof(CStreamClass), nullptr, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pUnknown);
    if (SUCCEEDED(hr)) {
        hr = OleRun(pUnknown);
        if (SUCCEEDED(hr)) {
            pUnknown->QueryInterface(__uuidof(ILinkStream), (void**)&linkstream);
        }
        pUnknown->Release();
    }

    hr = linkstream->Link_Stream(docfile);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to link stream to decompressor.\n");
        return false;
    }

    CComPtr<IStream> stream;
    linkstream->QueryInterface(__uuidof(IStream), (void**)&stream);

    DEBUG_INFO("Calling Vinifera_Get_All().\n");
    if (!Vinifera_Get_All(stream)) {
        DEBUG_FATAL("Error loading save game \"{}\"!\n", formatted_file_name);
        return false;
    }

    DEBUG_INFO("Unlinking content stream from decompressor.\n");
    linkstream->Unlink_Stream(nullptr);

    SwizzleManager.Reset();
    Post_Load_Game();
    Vinifera_Post_Load_Game();
    Map.Init_IO();
    Map.Activate(1);
    Map.Shift_Sidebar();

    /**
     *  Relink factories to the sidebar model items.
     */
    BattleUI.Get_Sidebar().Relink_Factories();

    TiberiumClass::Initialize_Tiberium_Growth_System();
    TiberiumClass::Initialize_Tiberium_Spread_System();
    Map.Total_Radar_Refresh();
    ScenarioActive = true;
    TacticalActive = true;

    DEBUG_INFO("LOADING GAME [{}] - Complete\n", formatted_file_name);

    return true;
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class LoadOptionsClassExt : public LoadOptionsClass
{
public:
    bool _Load_File(const char* filename);
    bool _Save_File(const char* filename, const char* description);
    bool _Delete_File(const char* filename);
    bool _Read_File(FileEntryClass* file, WIN32_FIND_DATA* filename);
};


/**
 *  Opens the "Loading..." window and loads a saved game from the selected file.
 *
 *  @author: ZivDero
 */
bool LoadOptionsClassExt::_Load_File(const char* filename)
{
    char formatted_file_name[PATH_MAX];

    HWND handle = WinDialogClass::Do_Message_Box(Fetch_String(TXT_LOADING), nullptr, nullptr);
    if (handle) {
        WinDialogClass::Display_Dialog(handle);
    }

    ScenarioActive = false;
    TacticalActive = false;

    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(filename), nullptr);
    const bool result = Load_Game(formatted_file_name);

    if (handle) {
        WinDialogClass::End_Dialog(handle);
    }

    // TODO should exit game on failure in TS Client builds

    return result;
}


/**
 *  Opens the "Saving..." window and saves the game to the selected file.
 *
 *  @author: ZivDero
 */
bool LoadOptionsClassExt::_Save_File(const char* filename, const char* description)
{
    char formatted_file_name[PATH_MAX];

    HWND handle = WinDialogClass::Do_Message_Box(Fetch_String(TXT_SAVING_GAME), nullptr, nullptr);
    if (handle) {
        WinDialogClass::Display_Dialog(handle);
    }

    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(filename), nullptr);
    const bool result = Save_Game(formatted_file_name, description, false);

    if (handle) {
        WinDialogClass::End_Dialog(handle);
    }

    return result;
}


/**
 *  Deletes the selected saved game.
 *
 *  @author: ZivDero
 */
bool LoadOptionsClassExt::_Delete_File(const char* filename)
{
    char formatted_file_name[PATH_MAX];

    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(filename), nullptr);
    return DeleteFileA(formatted_file_name);
}


/**
 *  Reads the header from the selected save game file.
 *
 *  @author: ZivDero
 */
bool LoadOptionsClassExt::_Read_File(FileEntryClass* file, WIN32_FIND_DATA* filename)
{
    char formatted_file_name[PATH_MAX];

    if (!file && !filename)
        return false;

    if (std::strcmp(filename->cFileName, NET_SAVE_FILE_NAME) != 0) {

        _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(filename->cFileName), nullptr);

        ViniferaSaveVersionInfo saveversion;
        if (Vinifera_Get_Savefile_Info(formatted_file_name, saveversion)) {

            unsigned game_version = saveversion.Get_Internal_Version();
            if (game_version != GameVersion) {
                DEBUG_WARNING("Save file \"{}\" is incompatible! Tiberian Sun: File version 0x{:X}, Expected version 0x{:X}.\n", formatted_file_name, game_version, GameVersion);
                return false;
            }

            unsigned vinifera_version = saveversion.Get_Vinifera_Version();
            if (vinifera_version != ViniferaGameVersion) {
                DEBUG_WARNING("Save file \"{}\" is incompatible! Vinifera: File version 0x{:X}, Expected version 0x{:X}.\n", formatted_file_name, vinifera_version, ViniferaGameVersion);
                return false;
            }

            wsprintfA(file->Descr, "%s", saveversion.Get_Scenario_Description());
            file->Old = false;
            file->Valid = true;
            file->Scenario = saveversion.Get_Scenario_Number();
            file->Campaign = saveversion.Get_Campaign_Number();
            file->Session = static_cast<GameEnum>(saveversion.Get_Game_Type());
            std::strncpy(file->Filename, formatted_file_name, std::size(file->Filename));
            std::strncpy(file->Handle, saveversion.Get_Player_House(), std::size(file->Handle));
            if (std::strlen(file->Filename) == 0) {
                std::strncpy(file->Filename, filename->cAlternateFileName, std::size(file->Filename));
            }
            file->DateTime = filename->ftLastWriteTime;

            return true;
        }
        else {
            DEBUG_WARNING("Failed to read save file \"{}\"!\n", formatted_file_name);
        }
    }

    return false;
}


/**
 *  Make sure the file name contains the subdirectory in various LoadOptionsClass functions
 *  by patching print calls.
 *
 *  @author: ZivDero
 */
int __cdecl sprintf_LoadOptionsClass_Wrapper1(char* buffer, const char*, int number, char* str)
{
    char formatted_file_name[PATH_MAX];

    // First create the format string itself, using our custom folder, e. g. "Saved Games\SAVE%04lX.%3s"
    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, "SAVE%04lX.%3s", nullptr);

    // Now actually format the path
    return std::sprintf(buffer, formatted_file_name, number, str);
}


/**
 *  Make sure the file name contains the subdirectory in various LoadOptionsClass functions
 *  by patching print calls.
 *
 *  @author: ZivDero
 */
int __cdecl sprintf_LoadOptionsClass_Wrapper2(char* buffer, const char*, char* str)
{
    char formatted_file_name[PATH_MAX];

    // First create the format string itself, using our custom folder, e. g. "Saved Games\*.%3s"
    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, "*.%3s", nullptr);

    // Now actually format the path
    return std::sprintf(buffer, formatted_file_name, str);
}


/**
 *  Main function for patching the hooks.
 */
void SaveGame_Hooks()
{
    Patch_Call(0x00505001, &sprintf_LoadOptionsClass_Wrapper1);
    Patch_Call(0x00505294, &sprintf_LoadOptionsClass_Wrapper1);
    Patch_Call(0x00505509, &sprintf_LoadOptionsClass_Wrapper2);
    Patch_Call(0x00505863, &sprintf_LoadOptionsClass_Wrapper2);
    Patch_Jump(0x00505980, &LoadOptionsClassExt::_Load_File);
    Patch_Jump(0x005059D0, &LoadOptionsClassExt::_Save_File);
    Patch_Jump(0x00505A20, &LoadOptionsClassExt::_Delete_File);
    Patch_Jump(0x00505A40, &LoadOptionsClassExt::_Read_File);
}
