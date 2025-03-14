/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_GLOBALS.CPP
 *
 *  @authors       CCHyper
 *
 *  @brief         Vinifera global values.
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
#pragma once

#include "vinifera_globals.h"

#include "aircrafttracker.h"


bool Vinifera_DeveloperMode = false;

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

bool Vinifera_SkipLogoMovies = false;
bool Vinifera_SkipStartupMovies = false;

bool Vinifera_NoTacticalVersionString = false;

bool Vinifera_ShowSuperWeaponTimers = true;

/**
 *  The total play time from all previous sessions of the current game.
 */
unsigned Vinifera_TotalPlayTime = 0;

DynamicVectorClass<MFCC *> ViniferaMapsMixes;
DynamicVectorClass<MFCC *> ViniferaMoviesMixes;

DynamicVectorClass<EBoltClass *> EBolts;
DynamicVectorClass<TheaterTypeClass *> TheaterTypes;
DynamicVectorClass<ArmorTypeClass *> ArmorTypes;
DynamicVectorClass<CrateTypeClass *> CrateTypes;
DynamicVectorClass<SpawnManagerClass *> SpawnManagers;
DynamicVectorClass<RocketTypeClass*> RocketTypes;
DynamicVectorClass<MouseTypeClass *> MouseTypes;
DynamicVectorClass<ActionTypeClass *> ActionTypes;

KamikazeTrackerClass* KamikazeTracker = nullptr;
AircraftTrackerClass* AircraftTracker = nullptr;

MFCC *GenericMix = nullptr;
MFCC *IsoGenericMix = nullptr;

bool Vinifera_SkipToTSMenu = false;
bool Vinifera_SkipToFSMenu = false;
bool Vinifera_SkipToLAN = false;
bool Vinifera_SkipToSkirmish = false;
bool Vinifera_SkipToCampaign = false;
bool Vinifera_SkipToInternet = false;
bool Vinifera_ExitAfterSkip = false;

bool Vinifera_NewSidebar = false;
bool Vinifera_NoVersionString = false;

DynamicVectorClass<ExceptionInfoDatabaseStruct> ExceptionInfoDatabase;
