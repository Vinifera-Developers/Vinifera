/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Vinifera global values.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "vinifera_globals.h"

#include "aircrafttracker.h"
#include "prerequisitegroup.h"


bool Vinifera_DeveloperMode = false;

bool Vinifera_AudioDebug = false;

bool Vinifera_PerformingLoad = false;

bool Vinifera_PrintFileErrors = true;
bool Vinifera_FatalFileErrors = false;
bool Vinifera_AssertFileErrors = false;

char Vinifera_ExceptionDatabaseFilename[PATH_MAX] = { "GAME.EDB" };
char Vinifera_DebugDirectory[PATH_MAX] = { "Debug" };
char Vinifera_ScreenshotDirectory[PATH_MAX] = { "Screenshots" };
char Vinifera_SavedGamesDirectory[PATH_MAX] = { "Saved Games" };

char Vinifera_ProjectName[64] = { '\0' };
char Vinifera_ProjectVersion[64] = { '\0' };
char Vinifera_IconName[64] = { '\0' };
char Vinifera_CursorName[64] = { '\0' };

DWORD Vinifera_MainThreadId = 0;

bool Vinifera_Developer_InstantBuild = false;
bool Vinifera_Developer_AIInstantBuild = false;
bool Vinifera_Developer_InstantSuperRecharge = false;
bool Vinifera_Developer_AIInstantSuperRecharge = false;
bool Vinifera_Developer_BuildCheat = false;
bool Vinifera_Developer_Unshroud = false;
bool Vinifera_Developer_ShowCursorPosition = false;
bool Vinifera_Developer_FrameStep = false;
int Vinifera_Developer_FrameStepCount = 0;
bool Vinifera_Developer_AIControl = false;
bool Vinifera_Developer_IsToReloadRules = false;

SDL_Window* SDLWindow = nullptr;
SDL_Renderer* SDLWindowRenderer = nullptr;
SDL_Texture* SDLWindowTexture = nullptr;
int SDLWindowWidth = 0;
int SDLWindowHeight = 0;
bool Vinifera_ModernMoviePlaying = false;

bool Vinifera_SkipLogoMovies = false;
bool Vinifera_SkipStartupMovies = false;

bool Vinifera_NoTacticalVersionString = false;

bool Vinifera_ShowSuperWeaponTimers = true;


/**
 *  The total play time from all previous sessions of the current game.
 */
unsigned Vinifera_TotalPlayTime = 0;

DynamicVectorClass<MFCD *> ViniferaMapsMixes;
DynamicVectorClass<MFCD *> ViniferaMoviesMixes;

DynamicVectorClass<EBoltClass *> EBolts;
DynamicVectorClass<TheaterTypeClass *> TheaterTypes;
DynamicVectorClass<ArmorTypeClass *> ArmorTypes;
DynamicVectorClass<SpawnManagerClass *> SpawnManagers;
DynamicVectorClass<RocketTypeClass*> RocketTypes;
DynamicVectorClass<MouseTypeClass *> MouseTypes;
DynamicVectorClass<ActionTypeClass *> ActionTypes;
DynamicVectorClass<PrerequisiteGroupClass *> PrerequisiteGroups;

KamikazeTrackerClass* KamikazeTracker = nullptr;
AircraftTrackerClass* AircraftTracker = nullptr;

int EnvironmentGlobals[/*std::size(ScenExtension->GlobalFlags)*/500];

bool Vinifera_PlayerOptionsSent = false;

std::unordered_map<std::string, std::string> Vinifera_TutorialText;

MFCD *GenericMix = nullptr;
MFCD *IsoGenericMix = nullptr;

bool Vinifera_SkipToTSMenu = false;
bool Vinifera_SkipToFSMenu = false;
bool Vinifera_SkipToLAN = false;
bool Vinifera_SkipToSkirmish = false;
bool Vinifera_SkipToCampaign = false;
bool Vinifera_SkipToInternet = false;
bool Vinifera_ExitAfterSkip = false;

DynamicVectorClass<ExceptionInfoDatabaseStruct> ExceptionInfoDatabase;
