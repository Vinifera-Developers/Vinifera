/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Vinifera global values.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "ccfile.h"
#include "extension_globals.h"
#include "vector.h"

#include <unordered_map>


class PrerequisiteGroupClass;
class KamikazeTrackerClass;
class AircraftTrackerClass;
class SpawnManagerClass;
class EBoltClass;
class TheaterTypeClass;
class ArmorTypeClass;
class RocketTypeClass;
class MouseTypeClass;
class ActionTypeClass;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;


extern bool Vinifera_DeveloperMode;

extern bool Vinifera_AudioDebug;

extern bool Vinifera_PerformingLoad;

extern bool Vinifera_PrintFileErrors;
extern bool Vinifera_FatalFileErrors;
extern bool Vinifera_AssertFileErrors;

extern char Vinifera_ExceptionDatabaseFilename[PATH_MAX];
extern char Vinifera_DebugDirectory[PATH_MAX];
extern char Vinifera_ScreenshotDirectory[PATH_MAX];
extern char Vinifera_SavedGamesDirectory[PATH_MAX];

extern char Vinifera_ProjectName[64];
extern char Vinifera_ProjectVersion[64];
extern char Vinifera_IconName[64];
extern char Vinifera_CursorName[64];


/**
 *  Defines and constants.
 */
#define TEXT_S_S "%s: %s"


/**
 *  Developer mode globals.
 */
extern bool Vinifera_Developer_InstantBuild;
extern bool Vinifera_Developer_AIInstantBuild;
extern bool Vinifera_Developer_InstantSuperRecharge;
extern bool Vinifera_Developer_AIInstantSuperRecharge;
extern bool Vinifera_Developer_BuildCheat;
extern bool Vinifera_Developer_Unshroud;
extern bool Vinifera_Developer_ShowCursorPosition;
extern bool Vinifera_Developer_FrameStep;
extern int Vinifera_Developer_FrameStepCount;
extern bool Vinifera_Developer_AIControl;
extern bool Vinifera_Developer_IsToReloadRules;


/**
 *  SDL globals.
 */
extern SDL_Window* SDLWindow;
extern SDL_Renderer* SDLWindowRenderer;
extern SDL_Texture* SDLWindowTexture;
extern int SDLWindowWidth;
extern int SDLWindowHeight;


/**
 *  Various globals.
 */
extern bool Vinifera_SkipLogoMovies;
extern bool Vinifera_SkipStartupMovies;

extern bool Vinifera_NoTacticalVersionString;

extern bool Vinifera_ShowSuperWeaponTimers;

extern unsigned Vinifera_TotalPlayTime;

extern DynamicVectorClass<MFCD *> ViniferaMapsMixes;
extern DynamicVectorClass<MFCD *> ViniferaMoviesMixes;

extern MFCD *GenericMix;
extern MFCD *IsoGenericMix;

extern KamikazeTrackerClass *KamikazeTracker;
extern AircraftTrackerClass *AircraftTracker;

extern int EnvironmentGlobals[/*std::size(ScenExtension->GlobalFlags)*/500];

extern std::unordered_map<std::string, std::string> Vinifera_TutorialText;


/**
 *  Global vectors and heaps.
 */
extern DynamicVectorClass<EBoltClass *> EBolts;
extern DynamicVectorClass<TheaterTypeClass *> TheaterTypes;
extern DynamicVectorClass<ArmorTypeClass *> ArmorTypes;
extern DynamicVectorClass<SpawnManagerClass *> SpawnManagers;
extern DynamicVectorClass<RocketTypeClass *> RocketTypes;
extern DynamicVectorClass<MouseTypeClass *> MouseTypes;
extern DynamicVectorClass<ActionTypeClass *> ActionTypes;
extern DynamicVectorClass<PrerequisiteGroupClass*> PrerequisiteGroups;


/**
 *  Skip to menus.
 */
extern bool Vinifera_SkipToTSMenu;
extern bool Vinifera_SkipToFSMenu;
extern bool Vinifera_SkipToLAN;
extern bool Vinifera_SkipToSkirmish;
extern bool Vinifera_SkipToCampaign;
extern bool Vinifera_SkipToInternet;
extern bool Vinifera_ExitAfterSkip;


/**
 *  Definition for the exception database struct. If you update this
 *  struct, make sure you check if you need to update the exception handler.
 */
struct ExceptionInfoDatabaseStruct
{
    ExceptionInfoDatabaseStruct() :
        Address(0x00000000),
        CanContinue(false),
        Ignore(false),
        Description()
    {
        std::memset(Description, 0, sizeof(Description));
    }

    bool operator==(const ExceptionInfoDatabaseStruct &src) const { return false; }
    bool operator!=(const ExceptionInfoDatabaseStruct &src) const { return true; }

    uint32_t Address;
    bool CanContinue;
    bool Ignore;
    char Description[1024];
};

extern DynamicVectorClass<ExceptionInfoDatabaseStruct> ExceptionInfoDatabase;
