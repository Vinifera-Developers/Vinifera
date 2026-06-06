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
#include "cell.h"
#include "vector.h"

#include <chrono>
#include <unordered_map>
#include <windows.h>

#define MAX_ENVIRONMENT_GLOBALS 500

class PrerequisiteGroupClass;
class HouseClass;
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
class SpawnerConfig;


extern bool Vinifera_DeveloperMode;

extern bool Vinifera_AudioDebug;

extern bool Vinifera_PerformingLoad;

extern bool Vinifera_PrintFileErrors;
extern bool Vinifera_FatalFileErrors;
extern bool Vinifera_AssertFileErrors;

extern std::string Vinifera_ExceptionDatabaseFilename;
extern std::string Vinifera_DebugDirectory;
extern std::string Vinifera_ScreenshotDirectory;
extern std::string Vinifera_SavedGamesDirectory;

extern std::string Vinifera_ProjectName;
extern std::string Vinifera_ProjectVersion;
extern std::string Vinifera_IconName;
extern std::string Vinifera_CursorName;

/**
 *  Captured in DllMain DLL_PROCESS_ATTACH. Used by the exception handler to
 *  decide whether it can safely show a modal dialog or touch UI/DirectDraw state.
 */
extern DWORD Vinifera_MainThreadId;


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
extern bool Vinifera_ModernMoviePlaying;


/**
 *  Various globals.
 */
extern bool Vinifera_SkipLogoMovies;
extern bool Vinifera_SkipStartupMovies;

extern bool Vinifera_NoTacticalVersionString;

extern bool Vinifera_ShowSuperWeaponTimers;

extern DynamicVectorClass<MFCD *> ViniferaMapsMixes;
extern DynamicVectorClass<MFCD *> ViniferaMoviesMixes;

extern MFCD *GenericMix;
extern MFCD *IsoGenericMix;

extern KamikazeTrackerClass *KamikazeTracker;
extern AircraftTrackerClass *AircraftTracker;

extern int EnvironmentGlobals[/*std::size(ScenExtension->GlobalFlags)*/500];

extern std::unordered_map<std::string, std::string> Vinifera_TutorialText;

extern bool Vinifera_PlayerOptionsSent;

extern unsigned Vinifera_PlaythroughID;

extern int PendingMultiplayerSaveLoadSlot;
extern std::optional<std::chrono::steady_clock::time_point> PendingMultiplayerSaveLoadTime;

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

struct CellHasher {
    std::size_t operator()(const Cell cell) const
    {
        int X = cell.X;
        int Y = cell.Y;

        return X << 16 | Y;
    }
};

extern std::unordered_map<Cell, int, CellHasher> BridgeHealths;
