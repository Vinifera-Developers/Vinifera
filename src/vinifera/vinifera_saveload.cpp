/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_SAVELOAD.CPP
 *
 *  @authors       CCHyper
 *
 *  @brief         Utility functions for saving and loading.
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
#include "vinifera_saveload.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "tibsun_util.h"
#include "vinifera_util.h"
#include "vinifera_gitinfo.h"
#include "wstring.h"
#include "saveload.h"
#include "extension.h"
#include "debughandler.h"

#include "addon.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include "aitrigtype.h"
#include "alphashape.h"
#include "anim.h"
#include "animtype.h"
#include "armortype.h"
#include "building.h"
#include "buildinglight.h"
#include "buildingtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "campaign.h"
#include "ccini.h"
#include "empulse.h"
#include "endgame.h"
#include "factory.h"
#include "foggedobject.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "infantry.h"
#include "infantrytype.h"
#include "iomap.h"
#include "isotile.h"
#include "isotiletype.h"
#include "kamikazetracker.h"
#include "lightsource.h"
#include "logic.h"
#include "objecttype.h"
#include "overlay.h"
#include "overlaytype.h"
#include "particle.h"
#include "particlesys.h"
#include "particlesystype.h"
#include "particletype.h"
#include "radarevent.h"
#include "rockettype.h"
#include "rules.h"
#include "scenario.h"
#include "scenarioext.h"
#include "script.h"
#include "scripttype.h"
#include "session.h"
#include "side.h"
#include "smudge.h"
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
#include "trigger.h"
#include "triggertype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "veinholemonster.h"
#include "verses.h"
#include "voxelanim.h"
#include "voxelanimtype.h"
#include "warheadtype.h"
#include "wave.h"
#include "waypointpath.h"
#include "weapontype.h"


#include "scenario.h"
#include "endgame.h"
#include "rules.h"
#include "iomap.h"
#include "logic.h"
#include "tactical.h"
#include "session.h"
#include "addon.h"
#include "ccini.h"
#include "cstream.h"
#include <atlbase.h>
#include <atlcom.h>

#include "aircrafttracker.h"
#include "animtypeext.h"
#include "buildingtypeext.h"
#include "hooker.h"
#include "language.h"
#include "loadoptions.h"
#include "miscutil.h"
#include "savever.h"
#include "vinifera_savever.h"
#include "windialog.h"


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
    DEBUG_INFO("Saving %s...\n", heap_name);

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

    DEBUG_INFO("  Count: %d\n", list.Count());

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
            return false;
        }

        /**
         *  Release the interface.
         */
        hr = lpPS->Release();
        if (FAILED(hr)) {
            DEBUG_ERROR("  Release failed!\n");
            return false;
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
static bool Vinifera_Load_Vector(IStream *pStm, DynamicVectorClass<T> &list, const char *heap_name)
{
    DEBUG_INFO("Loading %s...\n", heap_name);

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

    DEBUG_INFO("  Count: %d\n", count);
    
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


/**
 *  Saves the game state to the file stream.
 */
bool Vinifera_Put_All(IStream *pStm, bool save_net)
{
    /**
     *  Save the scenario global information.
     */
    DEBUG_INFO("Saving Scenario...\n");
    Scen->Save(pStm);

    DEBUG_INFO("Saving EndGame...\n");
    EndGame.Save(pStm);

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
    if (FAILED(Vinifera_Save_Vector(pStm, WarheadTypes, "Warheads"))) { return false; }
    if (FAILED(Vinifera_Save_Vector(pStm, WeaponTypes, "Weapons"))) { return false; }
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
    if (FAILED(Vinifera_Save_Vector(pStm, TerrainTypes, "TerrianTypes"))) { return false; }
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
    Scen->Load(pStm);

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

        if (!ScenExtension->Read_Tutorial_INI(scen_ini, true)) {
            DEBUG_ERROR("Failed to read tutorial strings from scenario file!\n");
            return false;
        }
    }

    Disable_Addon(ADDON_NONE);

    DEBUG_INFO("Setting required addon to '%d'\n", Scen->RequiredAddOn);
    Set_Required_Addon(Scen->RequiredAddOn);

    if (!Is_Addon_Available(Scen->RequiredAddOn)) {
        DEBUG_ERROR("Addon '%d' is not installed!\n", Scen->RequiredAddOn);
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
    Rect composite_rect(0, 0, tactical_rect.Width, ScreenRect.Height);
    Rect tile_rect(0, 0, tactical_rect.Width, ScreenRect.Height);
    Rect sidebar_rect(tactical_rect.X, tactical_rect.Y, SidebarClass::SIDE_WIDTH, ScreenRect.Height);
    DEBUG_INFO("About to call Allocate_Surfaces()...\n");
    Allocate_Surfaces(&ScreenRect, &composite_rect, &tile_rect, &sidebar_rect);

    DEBUG_INFO("About to call Map.Set_View_Dimensions()...\n");
    Map.Set_View_Dimensions(tactical_rect);
    }

    DEBUG_INFO("Loading EndGame...\n");
    EndGame.Load(pStm);

    Init_Theater(Scen->Theater);

    DEBUG_INFO("About to call Load_Art_INI()...\n");
    RulesClass::Load_Art_INI();

    if (Is_Addon_Enabled(ADDON_FIRESTORM)) {
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
    if (FAILED(Vinifera_Load_Vector(pStm, WarheadTypes, "Warheads"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, WeaponTypes, "Weapons"))) { return false; }
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

    Map.Flag_To_Redraw(2);

    //Vinifera_Remap_Extension_Pointers();

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
        Extension::Fetch<TechnoClassExtension>(techno)->Put_Storage_Pointers();
    }

    for (int i = 0; i < Houses.Count(); i++) {
        const HouseClass* house = Houses[i];
        Extension::Fetch<HouseClassExtension>(house)->Put_Storage_Pointers();
    }

    for (int i = 0; i < BuildingTypes.Count(); i++) {
        const BuildingTypeClass* buildingtype = BuildingTypes[i];
        Extension::Fetch<BuildingTypeClassExtension>(buildingtype)->Fetch_Building_Normal_Image(Scen->Theater);
    }
}


/**
 *  Saves the game to a file on the disk.
 *
 *  @author: ZivDero
 */
bool Vinifera_Save_Game(const char* file_name, const char* descr, bool)
{
    WCHAR wide_file_name[PATH_MAX];
    char formatted_file_name[PATH_MAX];

    /**
     *  Format the save game path here just in case to make sure it contains the subdirectory.
     *  In the future, it should be the call sites of Save_Game that are patched so that we can still
     *  save to an arbitrary location, but until the TS-Patches spawner is ported, this needs to happen.
     */
    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(file_name), nullptr);

    DEBUG_INFO("SAVING GAME [%s - %s]\n", formatted_file_name, descr);

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
     *  Convert the file name to a wide string.
     */
    MultiByteToWideChar(CP_ACP, 0, formatted_file_name, -1, wide_file_name, std::size(wide_file_name));

    DEBUG_INFO("Creating DocFile\n");
    CComPtr<IStorage> storage;
    HRESULT hr = StgCreateDocfile(wide_file_name, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &storage);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to create storage.\n");
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
    versioninfo.Set_Campaign_Number(Scen->CampaignID);
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
        DEBUG_FATAL("Failed to write version information.\n");
        return false;
    }

    DEBUG_INFO("Creating content stream.\n");
    CComPtr<IStream> docfile;
    hr = storage->CreateStream(L"CONTENTS", STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &docfile);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to create content stream.\n");
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

    hr = linkstream->Link_Stream(docfile);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to link stream to compressor.\n");
        return false;
    }

    CComPtr<IStream> stream;
    linkstream->QueryInterface(__uuidof(IStream), (void**)&stream);

    DEBUG_INFO("Calling Vinifera_Put_All().\n");
    bool result = Vinifera_Put_All(stream, false);

    DEBUG_INFO("Unlinking content stream from compressor.\n");
    hr = linkstream->Unlink_Stream(nullptr);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to link unstream from compressor.\n");
        return false;
    }

    DEBUG_INFO("Releasing content stream.\n");
    docfile.Release();

    DEBUG_INFO("Closing DocFile.\n");
    hr = storage->Commit(STGC_DEFAULT);
    if (FAILED(hr)) {
        DEBUG_FATAL("Failed to commit storage.\n");
        return false;
    }

    DEBUG_INFO("SAVING GAME [%s] - Complete.\n", formatted_file_name);
    
    return result;
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

    DEBUG_INFO("LOADING GAME [%s]\n", formatted_file_name);

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
        DEBUG_FATAL("Error loading save game \"%s\"!\n", formatted_file_name);
        return false;
    }

    DEBUG_INFO("Unlinking content stream from decompressor.\n");
    linkstream->Unlink_Stream(nullptr);

    SwizzleManager.Reset();
    Post_Load_Game();
    Vinifera_Post_Load_Game();
    AnimTypeClassExtension::All_Set_Biggest_Frame();
    Map.Init_IO();
    Map.Activate(1);
    Map.Set_Dimensions();
    TiberiumClass::Growth_Init_Clear();
    TiberiumClass::Init_Cells();
    Map.Total_Radar_Refresh();
    TacticalViewActive = true;
    ScenarioStarted = true;

    DEBUG_INFO("LOADING GAME [%s] - Complete\n", formatted_file_name);

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

    TacticalViewActive = false;
    ScenarioStarted = false;

    _makepath(formatted_file_name, nullptr, Vinifera_SavedGamesDirectory, Filename_From_Path(filename), nullptr);
    const bool result = Load_Game(formatted_file_name);

    if (handle) {
        WinDialogClass::End_Dialog(handle);
    }

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
                DEBUG_WARNING("Save file \"%s\" is incompatible! Tiberian Sun: File version 0x%X, Expected version 0x%X.\n", formatted_file_name, game_version, GameVersion);
                return false;
            }

            unsigned vinifera_version = saveversion.Get_Vinifera_Version();
            if (vinifera_version != ViniferaGameVersion) {
                DEBUG_WARNING("Save file \"%s\" is incompatible! Vinifera: File version 0x%X, Expected version 0x%X.\n", formatted_file_name, vinifera_version, ViniferaGameVersion);
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
            DEBUG_WARNING("Failed to read save file \"%s\"!\n", formatted_file_name);
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
