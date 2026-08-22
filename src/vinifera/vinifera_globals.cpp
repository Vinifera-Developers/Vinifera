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

#include <chrono>
#include <optional>

bool Vinifera_DeveloperMode = false;

bool Vinifera_AudioDebug = false;

bool Vinifera_PerformingLoad = false;

bool Vinifera_PrintFileErrors = true;
bool Vinifera_FatalFileErrors = false;
bool Vinifera_AssertFileErrors = false;

std::string Vinifera_ExceptionDatabaseFilename { "GAME.EDB" };
std::string Vinifera_DebugDirectory { "Debug" };
std::string Vinifera_ScreenshotDirectory { "Screenshots" };
std::string Vinifera_SavedGamesDirectory { "Saved Games" };

std::string Vinifera_ProjectName;
std::string Vinifera_ProjectVersion;
std::string Vinifera_IconName;
std::string Vinifera_CursorName;

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

DynamicVectorClass<MFCD *> ViniferaMapsMixes;
DynamicVectorClass<MFCD*> ViniferaMoviesMixes;

unsigned Vinifera_PlaythroughID = 0;

int PendingMultiplayerSaveLoadSlot = -1;
std::optional<std::chrono::steady_clock::time_point> PendingMultiplayerSaveLoadTime;

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

int EnvironmentGlobals[/*std::size(ScenExtension->GlobalFlags)*/MAX_ENVIRONMENT_GLOBALS];

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

std::unordered_map<Cell, int, CellHasher> BridgeHealths;