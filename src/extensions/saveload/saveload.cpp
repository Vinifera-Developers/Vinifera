/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXT_SAVELOAD.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Handles the saving and loading of extended class data.
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
#include "saveload.h"
#include "wwcrc.h"
#include "vinifera_gitinfo.h"
#include "vinifera_util.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "fatal.h"
#include <unknwn.h> // for IStream

#include "rulesext.h"
#include "tacticalext.h"

#include "objecttypeext.h"
#include "technotypeext.h"
#include "buildingtypeext.h"
#include "unittypeext.h"
#include "infantrytypeext.h"
#include "aircrafttypeext.h"
#include "warheadtypeext.h"
#include "weapontypeext.h"
#include "bullettypeext.h"
#include "supertypeext.h"
#include "voxelanimtypeext.h"
#include "animtypeext.h"
#include "particletypeext.h"
#include "particlesystypeext.h"
#include "isotiletypeext.h"
#include "overlaytypeext.h"
#include "smudgetypeext.h"
#include "terraintypeext.h"
#include "housetypeext.h"
#include "sideext.h"
#include "campaignext.h"
#include "tiberiumext.h"
//#include "taskforceext.h"
//#include "aitrigtypeext.h"
//#include "scripttypeext.h"
//#include "tagtypeext.h"
//#include "triggertypeext.h"

#include "technoext.h"
#include "aircraftext.h"
#include "buildingext.h"
#include "infantryext.h"
#include "unitext.h"
#include "terrainext.h"

#include "waveext.h"


/**
 *  Constant of the current build version number. This number should be
 *  a sum of all the extended class sizes plus the build date.
 */
unsigned ViniferaSaveGameVersion =

            10000

            /**
            *  Global classes.
            */
            + sizeof(RulesClassExtension)
            + sizeof(TacticalExtension)

            /**
             *  Extended type classes.
             */
            + sizeof(ObjectTypeClassExtension)
            + sizeof(TechnoTypeClassExtension)
            + sizeof(BuildingTypeClassExtension)
            + sizeof(UnitTypeClassExtension)
            + sizeof(InfantryTypeClassExtension)
            + sizeof(AircraftTypeClassExtension)
            + sizeof(WarheadTypeClassExtension)
            + sizeof(WeaponTypeClassExtension)
            + sizeof(BulletTypeClassExtension)
            + sizeof(SuperWeaponTypeClassExtension)
            + sizeof(VoxelAnimTypeClassExtension)
            + sizeof(AnimTypeClassExtension)
            + sizeof(ParticleTypeClassExtension)
            + sizeof(ParticleSystemTypeClassExtension)
            + sizeof(IsometricTileTypeClassExtension)
            + sizeof(OverlayTypeClassExtension)
            + sizeof(SmudgeTypeClassExtension)
            + sizeof(TerrainTypeClassExtension)
            + sizeof(HouseTypeClassExtension)
            + sizeof(SideClassExtension)
            + sizeof(CampaignClassExtension)
            + sizeof(TiberiumClassExtension)
            //+ sizeof(TaskForceClassExtension)
            //+ sizeof(AITriggerTypeClassExtension)
            //+ sizeof(ScriptTypeClassExtension)
            //+ sizeof(TagTypeClassExtension)
            //+ sizeof(TriggerTypeClassExtension)
            + sizeof(TechnoClassExtension)
            + sizeof(AircraftClassExtension)
            + sizeof(BuildingClassExtension)
            + sizeof(InfantryClassExtension)
            + sizeof(UnitClassExtension)
            + sizeof(TerrainClassExtension)
            + sizeof(WaveClassExtension)
;


/**
 *  Handy macro for defining the save function for an extended class.
 * 
 *  @author: CCHyper
 */
#define DEFINE_EXTENSION_SAVE(class_name, heap_name) \
    static bool Vinifera_Save_##class_name(IStream *pStm) \
    { \
        if (!pStm) { return false; } \
        for (const auto &i : heap_name.Map) { i.second->Save(pStm, true); } \
        return true; \
    }

/**
 *  Handy macro for defining the load function for an extended class.
 * 
 *  @author: CCHyper
 */
#define DEFINE_EXTENSION_LOAD(class_name, heap_name) \
    static bool Vinifera_Load_##class_name(IStream *pStm) \
    { \
        if (!pStm) { return false; } \
        for (const auto &i : heap_name.Map) { i.second->Load(pStm); } \
        return true; \
    }


static char LoadedCommitHash[40];
static char DataHeaderString[20];
#define VINIFERA_SAVE_HEADER_NAME "VINIFERA_SAVE_DATA  "


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

    HRESULT hr;

    /**
     *  Save the header string.
     */
    hr = pStm->Write(VINIFERA_SAVE_HEADER_NAME, sizeof(DataHeaderString), nullptr);
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
     *  Load the header string.
     */
    hr = pStm->Read(DataHeaderString, sizeof(DataHeaderString), nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


/**
 *  Saves the commit hash for checking on load.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Save_Version_Info(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    HRESULT hr;

    /**
     *  Save the commit hash.
     */
    hr = pStm->Write(Vinifera_Git_Hash(), 40, nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


/**
 *  Load the commit hash for version checks.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Load_Version_Info(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    HRESULT hr;

    /**
     *  Load the commit hash.
     */
    hr = pStm->Read(LoadedCommitHash, 40, nullptr);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}


static bool Vinifera_Save_RulesExtension(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    if (!RulesExtension) {
        return false;
    }

    RulesExtension->Save(pStm, true);

    return true;
}


static bool Vinifera_Load_RulesExtension(IStream *pStm)
{
    if (!pStm) {
        return false;
    }
    
    if (!RulesExtension) {
        return false;
    }

    RulesExtension->Load(pStm);

    return true;
}


static bool Vinifera_Save_TacticalExtension(IStream *pStm)
{
    if (!pStm) {
        return false;
    }

    if (!TacticalExtension) {
        return false;
    }

    TacticalExtension->Save(pStm, true);

    return true;
}


static bool Vinifera_Load_TacticalExtension(IStream *pStm)
{
    if (!pStm) {
        return false;
    }
    
    if (!TacticalExtension) {
        return false;
    }

    TacticalExtension->Load(pStm);

    return true;
}


DEFINE_EXTENSION_SAVE(ObjectTypeClassExtension, ObjectTypeClassExtensions);
DEFINE_EXTENSION_SAVE(TechnoTypeClassExtension, TechnoTypeClassExtensions);
DEFINE_EXTENSION_SAVE(BuildingTypeClassExtension, BuildingTypeClassExtensions);
DEFINE_EXTENSION_SAVE(UnitTypeClassExtension, UnitTypeClassExtensions);
DEFINE_EXTENSION_SAVE(InfantryTypeClassExtension, InfantryTypeClassExtensions);
DEFINE_EXTENSION_SAVE(AircraftTypeClassExtension, AircraftTypeClassExtensions);
DEFINE_EXTENSION_SAVE(WarheadTypeClassExtension, WarheadTypeClassExtensions);
DEFINE_EXTENSION_SAVE(WeaponTypeClassExtension, WeaponTypeClassExtensions);
DEFINE_EXTENSION_SAVE(BulletTypeClassExtension, BulletTypeClassExtensions);
DEFINE_EXTENSION_SAVE(SuperWeaponTypeClassExtension, SuperWeaponTypeClassExtensions);
DEFINE_EXTENSION_SAVE(VoxelAnimTypeClassExtension, VoxelAnimTypeClassExtensions);
DEFINE_EXTENSION_SAVE(AnimTypeClassExtension, AnimTypeClassExtensions);
DEFINE_EXTENSION_SAVE(ParticleTypeClassExtension, ParticleTypeClassExtensions);
DEFINE_EXTENSION_SAVE(ParticleSystemTypeClassExtension, ParticleSystemTypeClassExtensions);
DEFINE_EXTENSION_SAVE(IsometricTileTypeClassExtension, IsometricTileTypeClassExtensions);
DEFINE_EXTENSION_SAVE(OverlayTypeClassExtension, OverlayTypeClassExtensions);
DEFINE_EXTENSION_SAVE(SmudgeTypeClassExtension, SmudgeTypeClassExtensions);
DEFINE_EXTENSION_SAVE(TerrainTypeClassExtension, TerrainTypeClassExtensions);
DEFINE_EXTENSION_SAVE(HouseTypeClassExtension, HouseTypeClassExtensions);
DEFINE_EXTENSION_SAVE(SideClassExtension, SideClassExtensions);
DEFINE_EXTENSION_SAVE(CampaignClassExtension, CampaignClassExtensions);
DEFINE_EXTENSION_SAVE(TiberiumClassExtension, TiberiumClassExtensions);
//DEFINE_EXTENSION_SAVE(TaskForceClassExtension, TaskForceClassExtensions);
//DEFINE_EXTENSION_SAVE(AITriggerTypeClassExtension, AITriggerTypeClassExtensions);
//DEFINE_EXTENSION_SAVE(ScriptTypeClassExtension, ScriptTypeClassExtensions);
//DEFINE_EXTENSION_SAVE(TagTypeClassExtension, TagTypeClassExtensions);
//DEFINE_EXTENSION_SAVE(TriggerTypeClassExtension, TriggerTypeClassExtensions);

DEFINE_EXTENSION_SAVE(TechnoClassExtension, TechnoClassExtensions);
DEFINE_EXTENSION_SAVE(AircraftClassExtension, AircraftClassExtensions);
DEFINE_EXTENSION_SAVE(BuildingClassExtension, BuildingClassExtensions);
DEFINE_EXTENSION_SAVE(InfantryClassExtension, InfantryClassExtensions);
DEFINE_EXTENSION_SAVE(UnitClassExtension, UnitClassExtensions);
DEFINE_EXTENSION_SAVE(TerrainClassExtension, TerrainClassExtensions);

DEFINE_EXTENSION_SAVE(WaveClassExtension, WaveClassExtensions);

DEFINE_EXTENSION_LOAD(ObjectTypeClassExtension, ObjectTypeClassExtensions);
DEFINE_EXTENSION_LOAD(TechnoTypeClassExtension, TechnoTypeClassExtensions);
DEFINE_EXTENSION_LOAD(BuildingTypeClassExtension, BuildingTypeClassExtensions);
DEFINE_EXTENSION_LOAD(UnitTypeClassExtension, UnitTypeClassExtensions);
DEFINE_EXTENSION_LOAD(InfantryTypeClassExtension, InfantryTypeClassExtensions);
DEFINE_EXTENSION_LOAD(AircraftTypeClassExtension, AircraftTypeClassExtensions);
DEFINE_EXTENSION_LOAD(WarheadTypeClassExtension, WarheadTypeClassExtensions);
DEFINE_EXTENSION_LOAD(WeaponTypeClassExtension, WeaponTypeClassExtensions);
DEFINE_EXTENSION_LOAD(BulletTypeClassExtension, BulletTypeClassExtensions);
DEFINE_EXTENSION_LOAD(SuperWeaponTypeClassExtension, SuperWeaponTypeClassExtensions);
DEFINE_EXTENSION_LOAD(VoxelAnimTypeClassExtension, VoxelAnimTypeClassExtensions);
DEFINE_EXTENSION_LOAD(AnimTypeClassExtension, AnimTypeClassExtensions);
DEFINE_EXTENSION_LOAD(ParticleTypeClassExtension, ParticleTypeClassExtensions);
DEFINE_EXTENSION_LOAD(ParticleSystemTypeClassExtension, ParticleSystemTypeClassExtensions);
DEFINE_EXTENSION_LOAD(IsometricTileTypeClassExtension, IsometricTileTypeClassExtensions);
DEFINE_EXTENSION_LOAD(OverlayTypeClassExtension, OverlayTypeClassExtensions);
DEFINE_EXTENSION_LOAD(SmudgeTypeClassExtension, SmudgeTypeClassExtensions);
DEFINE_EXTENSION_LOAD(TerrainTypeClassExtension, TerrainTypeClassExtensions);
DEFINE_EXTENSION_LOAD(HouseTypeClassExtension, HouseTypeClassExtensions);
DEFINE_EXTENSION_LOAD(SideClassExtension, SideClassExtensions);
DEFINE_EXTENSION_LOAD(CampaignClassExtension, CampaignClassExtensions);
DEFINE_EXTENSION_LOAD(TiberiumClassExtension, TiberiumClassExtensions);
//DEFINE_EXTENSION_LOAD(TaskForceClassExtension, TaskForceClassExtensions);
//DEFINE_EXTENSION_LOAD(AITriggerTypeClassExtension, AITriggerTypeClassExtensions);
//DEFINE_EXTENSION_LOAD(ScriptTypeClassExtension, ScriptTypeClassExtensions);
//DEFINE_EXTENSION_LOAD(TagTypeClassExtension, TagTypeClassExtensions);
//DEFINE_EXTENSION_LOAD(TriggerTypeClassExtension, TriggerTypeClassExtensions);

DEFINE_EXTENSION_LOAD(TechnoClassExtension, TechnoClassExtensions);
DEFINE_EXTENSION_LOAD(AircraftClassExtension, AircraftClassExtensions);
DEFINE_EXTENSION_LOAD(BuildingClassExtension, BuildingClassExtensions);
DEFINE_EXTENSION_LOAD(InfantryClassExtension, InfantryClassExtensions);
DEFINE_EXTENSION_LOAD(UnitClassExtension, UnitClassExtensions);
DEFINE_EXTENSION_LOAD(TerrainClassExtension, TerrainClassExtensions);

DEFINE_EXTENSION_LOAD(WaveClassExtension, WaveClassExtensions);


/**
 *  Save all Vinifera data to the file stream.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Put_All(IStream *pStm)
{
    /**
     *  Save the Vinifera data marker which can be used to verify
     *  the state of the data to follow on load.
     */
    DEBUG_INFO("Saving Vinifera header marker\n");
    if (!Vinifera_Save_Header(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Save the build version information for load checks.
     */
    DEBUG_INFO("Saving Vinifera version information\n");
    if (!Vinifera_Save_Version_Info(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Save class extensions here.
     */
    DEBUG_INFO("Saving extended class data...\n");

    DEBUG_INFO("Saving RulesExtension\n");
    if (!Vinifera_Save_RulesExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving TacticalExtension\n");
    if (!Vinifera_Save_TacticalExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving ObjectTypeClassExtension\n");
    if (!Vinifera_Save_ObjectTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving TechnoTypeClassExtension\n");
    if (!Vinifera_Save_TechnoTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving BuildingTypeClassExtension\n");
    if (!Vinifera_Save_BuildingTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving UnitTypeClassExtension\n");
    if (!Vinifera_Save_UnitTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving InfantryTypeClassExtension\n");
    if (!Vinifera_Save_InfantryTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving AircraftTypeClassExtension\n");
    if (!Vinifera_Save_AircraftTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving WarheadTypeClassExtension\n");
    if (!Vinifera_Save_WarheadTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving WeaponTypeClassExtension\n");
    if (!Vinifera_Save_WeaponTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving BulletTypeClassExtension\n");
    if (!Vinifera_Save_BulletTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving SuperWeaponTypeClassExtension\n");
    if (!Vinifera_Save_SuperWeaponTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving VoxelAnimTypeClassExtension\n");
    if (!Vinifera_Save_VoxelAnimTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving AnimTypeClassExtension\n");
    if (!Vinifera_Save_AnimTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving ParticleTypeClassExtension\n");
    if (!Vinifera_Save_ParticleTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving ParticleSystemTypeClassExtension\n");
    if (!Vinifera_Save_ParticleSystemTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving IsometricTileTypeClassExtension\n");
    if (!Vinifera_Save_IsometricTileTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving OverlayTypeClassExtension\n");
    if (!Vinifera_Save_OverlayTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving SmudgeTypeClassExtension\n");
    if (!Vinifera_Save_SmudgeTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving TerrainTypeClassExtension\n");
    if (!Vinifera_Save_TerrainTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving HouseTypeClassExtension\n");
    if (!Vinifera_Save_HouseTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving SideClassExtension\n");
    if (!Vinifera_Save_SideClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving CampaignClassExtension\n");
    if (!Vinifera_Save_CampaignClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving TiberiumClassExtension\n");
    if (!Vinifera_Save_TiberiumClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

//    DEBUG_INFO("Saving TaskForceClassExtension\n");
//    if (!Vinifera_Save_TaskForceClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Saving AITriggerTypeClassExtension\n");
//    if (!Vinifera_Save_AITriggerTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Saving ScriptTypeClassExtension\n");
//    if (!Vinifera_Save_ScriptTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Saving TagTypeClassExtension\n");
//    if (!Vinifera_Save_TagTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Saving TriggerTypeClassExtension\n");
//    if (!Vinifera_Save_TriggerTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }

    DEBUG_INFO("Saving TechnoClassExtension\n");
    if (!Vinifera_Save_TechnoClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving AircraftClassExtension\n");
    if (!Vinifera_Save_AircraftClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving BuildingClassExtension\n");
    if (!Vinifera_Save_BuildingClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving InfantryClassExtension\n");
    if (!Vinifera_Save_InfantryClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving UnitClassExtension\n");
    if (!Vinifera_Save_UnitClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving TerrainClassExtension\n");
    if (!Vinifera_Save_TerrainClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Saving WaveClassExtension\n");
    if (!Vinifera_Save_WaveClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Save global data and values here.
     */
    DEBUG_INFO("Saving global data...\n");

    return true;
}


/**
 *  Load all Vinifera data from the file stream.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Load_All(IStream *pStm)
{
    /**
     *  Load the Vinifera data marker which can be used to verify
     *  the state of the data to follow.
     */
    DEBUG_INFO("Loading Vinifera header marker\n");
    if (!Vinifera_Load_Header(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");

        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to load Vinifera save-file header!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to load Vinifera save-file header!\n");

        return false;
    }

    if (std::strncmp(VINIFERA_SAVE_HEADER_NAME, DataHeaderString, sizeof(DataHeaderString)) != 0) {
        DEBUG_WARNING("Invalid header in save file!");

        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Invalid header in save file!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Invalid header in save file!\n");

        return false;
    }

    /**
     *  Load the build version information.
     */
    DEBUG_INFO("Loading Vinifera version information\n");
    if (!Vinifera_Load_Version_Info(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    if (std::strncmp(Vinifera_Git_Hash(), LoadedCommitHash, 40) != 0) {
        DEBUG_WARNING("Git has mismatch in save file!");
        //return false;
    }

    /**
     *  Load class extensions here.
     */
    DEBUG_INFO("Loading extended class data...\n");

    DEBUG_INFO("Loading RulesExtension\n");
    if (!Vinifera_Load_RulesExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading TacticalExtension\n");
    if (!Vinifera_Load_TacticalExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading ObjectTypeClassExtension\n");
    if (!Vinifera_Load_ObjectTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading TechnoTypeClassExtension\n");
    if (!Vinifera_Load_TechnoTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading BuildingTypeClassExtension\n");
    if (!Vinifera_Load_BuildingTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading UnitTypeClassExtension\n");
    if (!Vinifera_Load_UnitTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading InfantryTypeClassExtension\n");
    if (!Vinifera_Load_InfantryTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading AircraftTypeClassExtension\n");
    if (!Vinifera_Load_AircraftTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading WarheadTypeClassExtension\n");
    if (!Vinifera_Load_WarheadTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading WeaponTypeClassExtension\n");
    if (!Vinifera_Load_WeaponTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading BulletTypeClassExtension\n");
    if (!Vinifera_Load_BulletTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading SuperWeaponTypeClassExtension\n");
    if (!Vinifera_Load_SuperWeaponTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading VoxelAnimTypeClassExtension\n");
    if (!Vinifera_Load_VoxelAnimTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading AnimTypeClassExtension\n");
    if (!Vinifera_Load_AnimTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading ParticleTypeClassExtension\n");
    if (!Vinifera_Load_ParticleTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading ParticleSystemTypeClassExtension\n");
    if (!Vinifera_Load_ParticleSystemTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading IsometricTileTypeClassExtension\n");
    if (!Vinifera_Load_IsometricTileTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading OverlayTypeClassExtension\n");
    if (!Vinifera_Load_OverlayTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading SmudgeTypeClassExtension\n");
    if (!Vinifera_Load_SmudgeTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading TerrainTypeClassExtension\n");
    if (!Vinifera_Load_TerrainTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading HouseTypeClassExtension\n");
    if (!Vinifera_Load_HouseTypeClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading SideClassExtension\n");
    if (!Vinifera_Load_SideClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading CampaignClassExtension\n");
    if (!Vinifera_Load_CampaignClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading TiberiumClassExtension\n");
    if (!Vinifera_Load_TiberiumClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

//    DEBUG_INFO("Loading TaskForceClassExtension\n");
//    if (!Vinifera_Load_TaskForceClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Loading AITriggerTypeClassExtension\n");
//    if (!Vinifera_Load_AITriggerTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Loading ScriptTypeClassExtension\n");
//    if (!Vinifera_Load_ScriptTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Loading TagTypeClassExtension\n");
//    if (!Vinifera_Load_TagTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }
//
//    DEBUG_INFO("Loading TriggerTypeClassExtension\n");
//    if (!Vinifera_Load_TriggerTypeClassExtension(pStm)) {
//        DEBUG_INFO("\t***** FAILED!\n");
//        return false;
//    }

    DEBUG_INFO("Loading TechnoClassExtension\n");
    if (!Vinifera_Load_TechnoClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading AircraftClassExtension\n");
    if (!Vinifera_Load_AircraftClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading BuildingClassExtension\n");
    if (!Vinifera_Load_BuildingClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading InfantryClassExtension\n");
    if (!Vinifera_Load_InfantryClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading UnitClassExtension\n");
    if (!Vinifera_Load_UnitClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading TerrainClassExtension\n");
    if (!Vinifera_Load_TerrainClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    DEBUG_INFO("Loading WaveClassExtension\n");
    if (!Vinifera_Load_WaveClassExtension(pStm)) {
        DEBUG_INFO("\t***** FAILED!\n");
        return false;
    }

    /**
     *  Load global data and values here.
     */
    DEBUG_INFO("Loading global data...\n");

    return true;
}
