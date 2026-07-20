/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for implementing all the extended classes.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension_hooks.h"

#include "abstractext_hooks.h"
#include "aircraftext_hooks.h"
#include "aircrafttracker_hooks.h"
#include "aircrafttypeext_hooks.h"
#include "animext_hooks.h"
#include "animtypeext_hooks.h"
#include "astarext_hooks.h"
#include "audio_hooks.h"
#include "audio_ui_hooks.h"
#include "beacon_hooks.h"
#include "buildingext_hooks.h"
#include "buildingtypeext_hooks.h"
#include "bulletext_hooks.h"
#include "bullettypeext_hooks.h"
#include "campaignext_hooks.h"
#include "cargoext_hooks.h"
#include "ccfileext_hooks.h"
#include "cciniext_hooks.h"
#include "cdext_hooks.h"
#include "cellext_hooks.h"
#include "combatext_hooks.h"
#include "commandext_hooks.h"
#include "conquerext_hooks.h"
#include "creditext_hooks.h"
#include "displayext_hooks.h"
#include "drivelocomotionext_hooks.h"
#include "dropshipext_hooks.h"
#include "empulseext_hooks.h"
#include "environmentext_hooks.h"
#include "eventext_hooks.h"
#include "factoryext_hooks.h"
#include "fetchres_hooks.h"
#include "filepcx_hooks.h"
#include "flylocomotionext_hooks.h"
#include "footext_hooks.h"
#include "gadgetext_hooks.h"
#include "houseext_hooks.h"
#include "housetypeext_hooks.h"
#include "infantryext_hooks.h"
#include "infantrytypeext_hooks.h"
#include "iniext_hooks.h"
#include "initext_hooks.h"
#include "isotiletypeext_hooks.h"
#include "mainloopext_hooks.h"
#include "mapext_hooks.h"
#include "mapseedext_hooks.h"
#include "missionext_hooks.h"
#include "mouseext_hooks.h"
#include "msglistext_hooks.h"
#include "multimissionext_hooks.h"
#include "multiscoreext_hooks.h"
#include "newmenuext_hooks.h"
#include "objectext_hooks.h"
#include "objecttypeext_hooks.h"
#include "optionsext_hooks.h"
#include "overlayext_hooks.h"
#include "overlaytypeext_hooks.h"
#include "particleext_hooks.h"
#include "particlesysext_hooks.h"
#include "particlesystypeext_hooks.h"
#include "particletypeext_hooks.h"
#include "playmovie_hooks.h"
#include "prerequisitegroup_hooks.h"
#include "radioext_hooks.h"
#include "rawfileext_hooks.h"
#include "rulesext_hooks.h"
#include "scenarioext_hooks.h"
#include "scoreclassext_hooks.h"
#include "scrollext_hooks.h"
#include "sdl_hooks.h"
#include "sdlmouse_hooks.h"
#include "sdlsurface_hooks.h"
#include "sessionext_hooks.h"
#include "sidebarext_hooks.h"
#include "sideext_hooks.h"
#include "skirmishdlg_hooks.h"
#include "smudgeext_hooks.h"
#include "smudgetypeext_hooks.h"
#include "spawnmanager_hooks.h"
#include "storageext_hooks.h"
#include "superext_hooks.h"
#include "supertypeext_hooks.h"
#include "tacticalext_hooks.h"
#include "tactionext_hooks.h"
#include "teamext_hooks.h"
#include "teamtypeext_hooks.h"
#include "technoext_hooks.h"
#include "technotypeext_hooks.h"
#include "terrainext_hooks.h"
#include "terraintypeext_hooks.h"
#include "teventext_hooks.h"
#include "textprintext_hooks.h"
#include "theatertype_hooks.h"
#include "themeext_hooks.h"
#include "tiberiumext_hooks.h"
#include "tooltipext_hooks.h"
#include "triggerext_hooks.h"
#include "triggertypeext_hooks.h"
#include "txtlabelext_hooks.h"
#include "unitext_hooks.h"
#include "unittypeext_hooks.h"
#include "voxelanimext_hooks.h"
#include "voxelanimtypeext_hooks.h"
#include "vqaext_hooks.h"
#include "warheadtypeext_hooks.h"
#include "waveext_hooks.h"
#include "weapontypeext_hooks.h"
#include "xsurfaceext_hooks.h"


void Extension_Hooks()
{
    /**
     *  Abstract and stack class extensions here.
     */
    AbstractClassExtension_Hooks();

    /**
     *  All game type class extensions here.
     */
    ObjectTypeClassExtension_Hooks();
    TechnoTypeClassExtension_Hooks();

    /**
     *  All game class extensions here.
     */
    MissionClassExtension_Hooks();
    RadioClassExtension_Hooks();
    TechnoClassExtension_Hooks();
    FootClassExtension_Hooks();
    EventClassExtension_Hooks();
    UnitClassExtension_Hooks();
    AircraftClassExtension_Hooks();
    AircraftTypeClassExtension_Hooks();
    AnimClassExtension_Hooks();
    AnimTypeClassExtension_Hooks();
    BuildingClassExtension_Hooks();
    BuildingTypeClassExtension_Hooks();
    BulletClassExtension_Hooks();
    BulletTypeClassExtension_Hooks();
    CampaignClassExtension_Hooks();
    CargoClassExtension_Hooks();
    CellClassExtension_Hooks();
    CreditClassExtension_Hooks();
    FactoryClassExtension_Hooks();
    HouseClassExtension_Hooks();
    HouseTypeClassExtension_Hooks();
    InfantryClassExtension_Hooks();
    InfantryTypeClassExtension_Hooks();
    //IsometricTileClassExtension_Hooks();                  // Not yet implemented
    IsometricTileTypeClassExtension_Hooks();
    MapClassExtension_Hooks();
    //BuildingLightExtension_Hooks();                       // Not yet implemented
    ObjectClassExtension_Hooks();
    OverlayClassExtension_Hooks();
    OverlayTypeClassExtension_Hooks();
    ParticleClassExtension_Hooks();
    ParticleTypeClassExtension_Hooks();
    ParticleSystemClassExtension_Hooks();
    ParticleSystemTypeClassExtension_Hooks();
    //ScriptClassExtension_Hooks();                         // Not yet implemented
    //ScriptTypeClassExtension_Hooks();                     // Not yet implemented
    SideClassExtension_Hooks();
    SmudgeClassExtension_Hooks();
    SmudgeTypeClassExtension_Hooks();
    SuperWeaponTypeClassExtension_Hooks();
    //TaskForceClassExtension_Hooks();                      // Not yet implemented
    TeamClassExtension_Hooks();
    TeamTypeClassExtension_Hooks();
    TerrainClassExtension_Hooks();
    TerrainTypeClassExtension_Hooks();
    TriggerClassExtension_Hooks();
    TriggerTypeClassExtension_Hooks();
    UnitTypeClassExtension_Hooks();
    VoxelAnimClassExtension_Hooks();
    VoxelAnimTypeClassExtension_Hooks();
    WaveClassExtension_Hooks();
    //TagClassExtension_Hooks();                            // Not yet implemented
    //TagTypeClassExtension_Hooks();                        // Not yet implemented
    TiberiumClassExtension_Hooks();
    TActionClassExtension_Hooks();
    TEventClassExtension_Hooks();
    WeaponTypeClassExtension_Hooks();
    WarheadTypeClassExtension_Hooks();
    //WaypointClassExtension_Hooks();                       // Not yet implemented
    //TubeClassExtension_Hooks();                           // Not yet implemented
    //LightSourceClassExtension_Hooks();                    // Not yet implemented
    EMPulseClassExtension_Hooks();
    TacticalExtension_Hooks();
    SuperClassExtension_Hooks();
    //AITriggerClassExtension_Hooks();                      // Not yet implemented
    //AITriggerTypeClassExtension_Hooks();                  // Not yet implemented
    //NeuronClassExtension_Hooks();                         // Not yet implemented
    //FoggedObjectClassExtension_Hooks();                   // Not yet implemented
    //AlphaShapeClassExtension_Hooks();                     // Not yet implemented
    //VeinholeMonsterClassExtension_Hooks();                // Not yet implemented
    StorageClassExtension_Hooks();

    /**
     *  Locomotors.
     */
    DriveLocomotionClassExtension_Hooks();
    FlyLocomotionClassExtension_Hooks();

    /**
     *  All global class extensions here.
     */
    RulesClassExtension_Hooks();
    ScenarioClassExtension_Hooks();
    SessionClassExtension_Hooks();
    OptionsClassExtension_Hooks();

    ThemeClassExtension_Hooks();

    DisplayClassExtension_Hooks();
    ScrollClassExtension_Hooks();
    SidebarClassExtension_Hooks();
    MouseClassExtension_Hooks();
    GadgetClassExtension_Hooks();

    /**
     *  Various modules and functions.
     */
    GameInit_Hooks();
    MainLoop_Hooks();
    NewMenuExtension_Hooks();
    CommandExtension_Hooks();
    CDExtension_Hooks();
    PlayMovieExtension_Hooks();
    VQAExtension_Hooks();
    INIClassExtension_Hooks();
    CCINIClassExtension_Hooks();
    RawFileClassExtension_Hooks();
    CCFileClassExtension_Hooks();

    MessageListClassExtension_Hooks();
    TextLabelClassExtension_Hooks();
    ToolTipManagerExtension_Hooks();
    TextPrintExtension_Hooks();

    CombatExtension_Hooks();
    DropshipExtension_Hooks();
    EnvironmentExtension_Hooks();
    MapSeedClassExtension_Hooks();
    MultiScoreExtension_Hooks();
    ScoreClassExtension_Hooks();
    MultiMissionExtension_Hooks();
    ConquerExtension_Hooks();

    SDLSurface_Hooks();
    SDLMouse_Hooks();
    SDL_Hooks();

    /**
     *  Dialogs and associated code.
     */
    SkirmishDialog_Hooks();

    /**
     *  Miscellaneous hooks
     */
    AStarClassExtension_Hooks();
    FilePCXExtension_Hooks();
    FetchRes_Hooks();
    XSurfaceExtension_Hooks();

    /**
     *  New classes and interfaces.
     */
    TheaterTypeClassExtension_Hooks();
    SpawnManager_Hooks();
    AircraftTracker_Hooks();
    PrerequisiteGroup_Hooks();
    Beacon_Hooks();

    Audio_Hooks();
    Audio_UI_Hooks();
}
