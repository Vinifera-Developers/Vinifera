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


/**
 *  Constant of the current build version number. This number should be
 *  a sum of all the extended class sizes plus the build date.
 */
unsigned ViniferaSaveGameVersion = 0x0;


/**
 *  Save file header.
 */
typedef struct ViniferaSaveFileHeaderStruct
{
    // Header marker.
    char Marker[20];

    // Git commit hash.
    char CommitHash[40];

    // Constant header marker to check for.
    static const char * Marker_String() { return "VINIFERA_SAVE_FILE"; }

private:
    char _padding[1024
                  - sizeof(Marker)
                  - sizeof(CommitHash)];
};
static_assert(sizeof(ViniferaSaveFileHeaderStruct), "ViniferaSaveFileHeaderStruct must be 1024 bytes in size!");

static ViniferaSaveFileHeaderStruct ViniferaSaveFileHeader; 


/**
 *  Saves the header marker for validating data on load.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Save_Header(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    /**
     *  Save the new header.
     */
    std::memset(&ViniferaSaveFileHeader, 0, sizeof(ViniferaSaveFileHeader));

    strncpy(ViniferaSaveFileHeader.Marker, ViniferaSaveFileHeaderStruct::Marker_String(), sizeof(ViniferaSaveFileHeader.Marker));
    strncpy(ViniferaSaveFileHeader.CommitHash, Vinifera_Git_Hash(), sizeof(ViniferaSaveFileHeader.CommitHash));

    HRESULT hr = pStm->Write(&ViniferaSaveFileHeader, sizeof(ViniferaSaveFileHeader), nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


/**
 *  Loads the save data header marker.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Load_Header(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    HRESULT hr;

    /**
     *  Load the new header.
     */
    std::memset(&ViniferaSaveFileHeader, 0, sizeof(ViniferaSaveFileHeader));

    hr = pStm->Read(&ViniferaSaveFileHeader, sizeof(ViniferaSaveFileHeader), nullptr);
    if (FAILED(hr)) {
        return false;
    }

    if (std::strncmp(ViniferaSaveFileHeader.Marker, ViniferaSaveFileHeaderStruct::Marker_String(), sizeof(ViniferaSaveFileHeader.Marker)) != 0) {
        DEBUG_WARNING("Invalid header in save file!\n");
        return false;
    }

    //if (std::strncmp(ViniferaSaveFileHeader.CommitHash, Vinifera_Git_Hash(), sizeof(ViniferaSaveFileHeader.CommitHash)) != 0) {
    //    DEV_DEBUG_INFO("Git hash mismatch in save file.\n");
    //    DEV_DEBUG_INFO("  Expected: %s\n", Vinifera_Git_Hash());
    //    DEV_DEBUG_INFO("  Save file: %s\n", ViniferaSaveFileHeader.CommitHash);
    //    //return false;
    //}
    DEV_DEBUG_INFO("Save file commit hash: %s\n", ViniferaSaveFileHeader.CommitHash);

    return true;
}


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
     *  Save the Vinifera data marker which can be used to verify
     *  the state of the data to follow on load.
     */
    DEBUG_INFO("Saving Vinifera header\n");
    if (!Vinifera_Save_Header(pStm)) {
        DEBUG_ERROR("\t***** FAILED!\n");
        return false;
    }

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
     *  Save new Verses.
     */
    if (FAILED(Verses::Save(pStm))) { return false; }

    /**
     *  Save new global class instances.
     */
    KamikazeTracker->Save(pStm, false);

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
     *  Load the Vinifera data marker which can be used to verify
     *  the state of the data to follow.
     */
    DEBUG_INFO("Loading Vinifera header\n");
    if (!Vinifera_Load_Header(pStm)) {
        DEBUG_ERROR("\t***** FAILED!\n");
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to load Vinifera save-file header!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to load Vinifera save-file header!\n");
        return false;
    }

    /**
     *  Clear the existing scenario data, ready for loading.
     */
    DEBUG_INFO("About to call Clear_Scenario()...\n");
    Clear_Scenario();

    /**
     *  Clear the ArmorTypes heap.
     */
    ArmorTypes.Clear();

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
     *  Save new Vinifera objects stored in vectors.
     */
    if (FAILED(Vinifera_Load_Vector(pStm, ArmorTypes, "ArmorTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, RocketTypes, "RocketTypes"))) { return false; }
    if (FAILED(Vinifera_Load_Vector(pStm, SpawnManagers, "SpawnManagers"))) { return false; }

    /**
     *  Save new Verses.
     */
    if (FAILED(Verses::Load(pStm))) { return false; }

    /**
     *  Load new global class instances.
     */
    KamikazeTracker->Clear();
    KamikazeTracker->Load(pStm);

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

void Vinifera_Remap_Storage_Pointers()
{
    for (int i = 0; i < Technos.Count(); i++)
    {
        const TechnoClass* techno = Technos[i];
        Extension::Fetch<TechnoClassExtension>(techno)->Put_Storage_Pointers();
    }

    for (int i = 0; i < Houses.Count(); i++)
    {
        const HouseClass* house = Houses[i];
        Extension::Fetch<HouseClassExtension>(house)->Put_Storage_Pointers();
    }
}
