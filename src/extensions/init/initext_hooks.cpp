/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains any hooks for the game init process.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "initext_hooks.h"

#include "addon.h"
#include "animtype.h"
#include "audio_manager.h"
#include "audio_util.h"
#include "audio_voc.h"
#include "audio_vox.h"
#include "audio_theme.h"
#include "asserthandler.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "ccini.h"
#include "cd.h"
#include "debughandler.h"
#include "dsaudio.h"
#include "extension_globals.h"
#include "hooker.h"
#include "iomap.h"
#include "mixfile.h"
#include "newmenu.h"
#include "overlaytype.h"
#include "optionsext.h"
#include "playmovie.h"
#include "scenarioext.h"
#include "sdl_functions.h"
#include "session.h"
#include "sessionext.h"
#include "special.h"
#include "shapeset.h"
#include "supertype.h"
#include "syringe.h"
#include "theme.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "uicontrol.h"
#include "unittype.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"

#include <bcrypt.h>
#include <tlhelp32.h> // must be after windows.h
#include <windows.h>


extern HMODULE DLLInstance;


/**
 *  Tiberian Sun resource constants.
 */
#define TS_MAINICON         93
#define TS_MAINCURSOR       104


/**
 *  #issue-218
 *
 *  We abuse SessionClass::IsGDI in this patch to store the current player's
 *  HouseType so it can be used to fetch the SideType from it for loading
 *  the assets. This also means this bugfix works without extending any of
 *  the games classes.
 *
 *  Also sets up the playthrough ID for the session.
 *
 *  @warning: This does mean we are limited to 255 unique houses (oh no!).
 *
 *  @author: CCHyper, Rampastring
 */
DEFINE_HOOK(0x004E2CE4, _Select_Game_PreStart_Patch, 0)
{
    /**
     *  This patch removes the code that sets the "IsGDI" member of SessionClass
     *  bool based on if the house name matched "GDI" or not and stores
     *  the player HouseType directly.
     */
    SessionExtension->House = Session.Players.Fetch_Head()->Player.House;

    /**
     *  Generate a new playthrough ID because we're about to start a new scenario or campaign run.
     */
    Vinifera_Generate_PlaythroughID();

    return 0x004E2D13;
}


/**
 *  #issue-513
 * 
 *  Patch to add check for CD::IsOverrideSwap() to make sure -CD really
 *  was set by the user.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E0469, _Init_CDROM_Access_Local_Files_Patch, 0)
{
    /**
     *  If there are search drives specified then all files are to be
     *  considered local.
     */
    if (CCFileClass::Is_There_Search_Drives()) {
        
        /**
         *  Double check that the game was launched with -CD.
         */
        if (CD::IsOverrideSwap()) {

            /**
             *  This is a workaround to ensure the mix loading code passes.
             */
            //CD::Set_Required_CD(DISK_GDI);

            goto files_local;
        }
    }

    /**
     *  Continue to initialise the CD-ROM code.
     */
init_cdrom:
    return 0x004E0471;

    /**
     *  Flag files as being local, no CD-ROM init.
     */
files_local:
    return 0x004E06F5;
}


static bool CCFile_Is_Available(const char *filename)
{
    return CCFileClass(filename).Is_Available();
}

static bool CCFile_Validate_Is_Available(const char *filename, int size)
{
    return CCFileClass(filename).Is_Available() && CCFileClass(filename).Size() == size;
}


/**
 *  #issue-478
 * 
 *  Adds command line options to skip startup movies.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Play_Startup_Movies()
{
    if (Special.IsFromInstall) {
        DEBUG_INFO("Playing first time intro sequence.\n");
        Play_Movie("EVA.VQA");
    }

    if (!Vinifera_SkipLogoMovies) {
        DEBUG_INFO("Playing startup movies.\n");
        Play_Movie("VINIFERA.VQA");
        Play_Movie("WWLOGO.VQA");
    } else {
        DEBUG_INFO("Skipping logo movies.\n");
    }

    if (!NewMenuClass::Get()) {
        DEBUG_INFO("Playing title movie.\n");
        if (CCFile_Is_Available("FS_TITLE.VQA")) {
            Play_Movie("FS_TITLE.VQA", THEME_NONE, true, false, true);
        } else {
            Play_Movie("STARTUP.VQA", THEME_NONE, true, false, true);
        }
    }

    return true;
}

DEFINE_HOOK(0x004E0786, _Init_Game_Skip_Startup_Movies_Patch, 0)
{
    if (Vinifera_SkipStartupMovies) {
        DEBUG_INFO("Skipping startup movies.\n");
        goto skip_loading_screen;
    }

    if (!Vinifera_Play_Startup_Movies()) {
        goto failed;
    }

loading_screen:
    R->EBX(-1);
    return 0x004E0848;

skip_loading_screen:
    return 0x004E084D;

failed:
    R->EBX(1);
    return 0x004E08B3;
}


/**
 *  Forces Firestorm addon as Present (installed).
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Detect_Addons()
{
    /**
     *  Tiberian Sun is installed and enabled.
     */
    InstalledMode = 1;
    EnabledMode = 1;

    DEBUG_INFO("Forcing Firestorm addon as installed.\n");

    /**
     *  Firestorm is installed.
     */
    InstalledMode |= 2;

    return true;
}

extern bool ImGui_Create_Main_Window(HINSTANCE hInstance);

/**
 *  Creates the main window for Tiberian Sun at 480p resolution.
 * 
 *  @author: ZivDero
 */
void Vinifera_Create_Main_Window_480p(HINSTANCE hInstance, int command_show, int width, int height)
{
    //DEV_DEBUG_INFO("Create_Main_Window(enter)\n");

    SDL_Create_Main_Window(hInstance, width, height);
    ShowCommand = command_show;

    //DEV_DEBUG_INFO("Create_Main_Window(exit)\n");
}


/**
 *  Creates the main window for Tiberian Sun at a custom resolution.
 *
 *  @author: ZivDero
 */
void Vinifera_Create_Main_Window_Custom(HINSTANCE hInstance, int command_show, int width, int height)
{
    // DEV_DEBUG_INFO("Create_Main_Window(enter)\n");

    width = Options.ScreenWidth;
    height = Options.ScreenHeight;

    if (OptionsExtension->WindowWidth > 0 && OptionsExtension->WindowHeight > 0) {
        width = OptionsExtension->WindowWidth;
        height = OptionsExtension->WindowHeight;
    }

    SDL_Create_Main_Window(hInstance, width, height);
    ShowCommand = command_show;

    // DEV_DEBUG_INFO("Create_Main_Window(exit)\n");
}


/**
 *  Own one complete set of side archives until it is committed to the engine
 *  globals or destroyed after a failed load.
 */
struct SideMixFiles
{
    std::unique_ptr<MFCD> Cached;
    std::unique_ptr<MFCD> Uncached;
    std::unique_ptr<MFCD> Disk;
    std::vector<std::unique_ptr<MFCD>> Expansions;

    bool Contains_Cached_Pointer(const void* data) const
    {
        if (Cached && Cached->Contains_Cached_Pointer(data)) return true;
        for (const auto& mix : Expansions) {
            if (mix->Contains_Cached_Pointer(data)) return true;
        }
        return false;
    }
};


static std::unique_ptr<MFCD> Open_Side_Mixfile(const char* name, bool cache)
{
    auto mix = std::make_unique<MFCD>(name, &FastKey);
    if (!mix->Is_Valid()) {
        DEBUG_ERROR("Failed to load side MIX {}.\n", name);
        return nullptr;
    }
    if (cache && !mix->Cache()) {
        DEBUG_ERROR("Failed to cache side MIX {}.\n", name);
        return nullptr;
    }
    return mix;
}


/**
 *  Build and validate a replacement set without changing its owner globals.
 */
static bool Load_Side_Mixfiles(SideType side, SideMixFiles& mixes)
{
    char name[64];

    DEBUG_INFO("Preparing Mixfiles for Side {:02}.\n", (int)side);

    int id = static_cast<int>(side) + 1; // Mix id

    /**
     *  Cached expansion side-specific mixes.
     */
    if (Addon_Enabled(ADDON_ANY) == true) {
        for (int index = 99; index >= 0; index--) {
            std::snprintf(name, sizeof(name), "E%02dSC%02d.MIX", index, id);
            if (CCFileClass(name).Is_Available()) {
                DEBUG_INFO("     Initializing {}\n", name);
                auto mix = Open_Side_Mixfile(name, true);
                if (!mix) return false;
                mixes.Expansions.emplace_back(std::move(mix));
            }
        }
    }

    /**
     *  Cached side-specific mix.
     */
    std::snprintf(name, sizeof(name), "SIDEC%02d.MIX", id);
    if (CCFileClass(name).Is_Available()) {
        DEBUG_INFO("     Initializing {}\n", name);
        mixes.Cached = Open_Side_Mixfile(name, true);
        if (!mixes.Cached) return false;
    }

    /**
     *  Not cached expansion side-specific mixes.
     */
    if (Addon_Enabled(ADDON_ANY) == true) {
        for (int index = 99; index >= 0; index--) {
            std::snprintf(name, sizeof(name), "E%02dSNC%02d.MIX", index, id);

            if (CCFileClass(name).Is_Available()) {
                DEBUG_INFO("     Initializing {}\n", name);
                auto mix = Open_Side_Mixfile(name, false);
                if (!mix) return false;
                mixes.Expansions.emplace_back(std::move(mix));
            }
        }
    }

    /**
     *  Not cached side-specific mix.
     */
    std::snprintf(name, sizeof(name), "SIDENC%02d.MIX", id);
    if (CCFileClass(name).Is_Available()) {
        DEBUG_INFO("     Initializing {}\n", name);
        mixes.Uncached = Open_Side_Mixfile(name, false);
        if (!mixes.Uncached) return false;
    }

    /**
     *  Disk side-specific mix.
     */
    if (Session.Type == GAME_NORMAL) {
        if (Addon_Enabled(ADDON_ANY) == false) {
            std::snprintf(name, sizeof(name), "SIDECD%02d.MIX", id);
        } else {
            std::snprintf(name, sizeof(name), "E%02dSCD%02d.MIX", Get_Required_Addon(), id);
        }
        if (CCFileClass(name).Is_Available()) {
            DEBUG_INFO("     Initializing {}\n", name);
            mixes.Disk = Open_Side_Mixfile(name, false);
            if (!mixes.Disk) return false;
        }
    }

    return true;
}


static ShapeSet* Retrieve_Shape(const char* filename)
{
    return const_cast<ShapeSet*>(MFCD::RetrieveT<ShapeSet>(filename));
}


/**
 *  Reload borrowed type graphics after replacing the archives that supplied
 *  them. Demand-loaded images are left to their existing on-demand loaders.
 */
static void Reload_Side_Graphics(const SideMixFiles& retiring)
{
    const auto theater = Scen->Theater;

    for (auto type : ObjectTypes) {
        if (!type->IsVoxel) {
            switch (type->Fetch_RTTI()) {
                case RTTI_ANIMTYPE: {
                    auto anim = static_cast<AnimTypeClass*>(type);
                    if (!anim->IsDemandLoad) {
                        anim->Image = nullptr;
                        anim->Load_Image(theater);
                    } else if (retiring.Contains_Cached_Pointer(anim->Image)) {
                        anim->Image = nullptr;
                    }
                    break;
                }
                case RTTI_BUILDINGTYPE:
                    // Building graphics are reloaded together below.
                    break;
                case RTTI_OVERLAYTYPE: {
                    auto overlay = static_cast<OverlayTypeClass*>(type);
                    if (!overlay->IsDemandLoad) {
                        overlay->Image = nullptr;
                        overlay->Fetch_Normal_Image();
                    } else if (retiring.Contains_Cached_Pointer(overlay->Image)) {
                        overlay->Image = nullptr;
                    }
                    break;
                }
                default:
                    type->Image = nullptr;
                    type->Fetch_Normal_Image();
                    break;
            }
        }

        type->AlphaImage = nullptr;
        if (!type->AlphaGraphicName.empty()) {
            char filename[_MAX_PATH];
            _makepath(filename, nullptr, nullptr, type->AlphaGraphicName.c_str(), ".SHP");
            type->AlphaImage = Retrieve_Shape(filename);
        }
    }

    for (auto type : BuildingTypes) {
        if (!type->IsDemandLoad || retiring.Contains_Cached_Pointer(type->Image)) type->Image = nullptr;
        if (!type->IsDemandLoadBuildup || retiring.Contains_Cached_Pointer(type->BuildupData)) type->BuildupData = nullptr;
        type->DeployingAnim = nullptr;
        type->UnderDoorAnim = nullptr;
        type->DoorAnim = nullptr;
        type->SpecialZOverlay = nullptr;
        type->BibShape = nullptr;
        type->Fetch_Building_Normal_Image(theater);
    }

    for (auto ext : BuildingTypeExtensions) {
        ext->RoofDeployingAnim = nullptr;
        ext->RoofDoorAnim = nullptr;
        ext->UnderRoofDoorAnim = nullptr;
        if (!Vinifera_PerformingLoad) ext->Fetch_Building_Normal_Image(theater);
    }

    // Cameos use Vinifera's existing lazy loader.
    for (auto type : TechnoTypes) type->CameoData = nullptr;

    for (auto type : UnitTypes) {
        type->AltImage = nullptr;
        if (!type->AltImageFile.empty()) {
            char filename[_MAX_PATH];
            _makepath(filename, nullptr, nullptr, type->AltImageFile.c_str(), ".SHP");
            type->AltImage = Retrieve_Shape(filename);
        }
    }

    for (auto type : SuperWeaponTypes) {
        char filename[_MAX_PATH];
        _makepath(filename, nullptr, nullptr, type->SidebarImage.c_str(), ".SHP");
        type->SidebarIcon = Retrieve_Shape(filename);
        if (!type->SidebarIcon) {
            type->SidebarIcon = Retrieve_Shape("XXICON.SHP");
        }
    }

    DEBUG_INFO("Reloaded side-dependent type graphics after replacing side MIXes.\n");
}


/**
 *  Replace side archives even when reloading the same side. Keep the old cache
 *  allocations alive until type graphics and house UI have been refreshed.
 *  Remove old archives from file searches only after loading succeeds so the
 *  normal type loaders resolve graphics against the replacement search list.
 */
bool Vinifera_Load_Side_Mixfiles(SideType side, bool prepare_house)
{
    std::vector<MFCD*> previous_expansions;
    previous_expansions.reserve(ExpandSideMix.Count());
    for (auto mix : ExpandSideMix) previous_expansions.push_back(mix);

    SideMixFiles replacement;
    if (!Load_Side_Mixfiles(side, replacement)) {
        DEBUG_ERROR("Side MIX replacement failed; previous archives retained.\n");
        return false;
    }

    // Complete every fallible operation before changing the owning globals.
    const auto expansion_count = static_cast<int>(replacement.Expansions.size());
    if (expansion_count > ExpandSideMix.Length() && !ExpandSideMix.Resize(expansion_count)) {
        DEBUG_ERROR("Failed to reserve the side MIX expansion list; previous archives retained.\n");
        return false;
    }
    SideMixFiles retiring;
    retiring.Expansions.reserve(previous_expansions.size());

    // Capacity is fixed now, so publishing the replacement cannot fail.
    for (int i = 0; i < expansion_count; ++i) {
        ExpandSideMix[i] = replacement.Expansions[i].get();
    }
    ExpandSideMix.Set_Active(expansion_count);

    retiring.Cached.reset(SideCMix);
    retiring.Uncached.reset(SideNCMix);
    retiring.Disk.reset(SideCDMix);
    for (auto mix : previous_expansions) retiring.Expansions.emplace_back(mix);
    SideCMix = replacement.Cached.release();
    SideNCMix = replacement.Uncached.release();
    SideCDMix = replacement.Disk.release();
    for (auto& mix : replacement.Expansions) mix.release();

    const auto unlink = [](const auto& mix) { if (mix) mix->Unlink(); };
    unlink(retiring.Cached);
    unlink(retiring.Uncached);
    unlink(retiring.Disk);
    for (const auto& mix : retiring.Expansions) unlink(mix);
    Reload_Side_Graphics(retiring);
    if (prepare_house) {
        Map.Init_For_House();
        AudioTheme.Scan();
    }

    const auto log_release = [](const auto& mix) {
        if (mix) DEBUG_INFO("     Releasing {} after graphics refresh\n", mix->Filename);
    };
    log_release(retiring.Cached);
    log_release(retiring.Uncached);
    log_release(retiring.Disk);
    for (const auto& mix : retiring.Expansions) log_release(mix);
    return true;
}


/**
 *  Reimplementation of Prep_For_Side().
 */
bool Vinifera_Prep_For_Side(SideType side)
{
    return Vinifera_Load_Side_Mixfiles(side, true);
}


/**
 *  Reimplementation of Prep_Speech_For_Side()
 *
 *  Prepare the mixfiles for the player side.
 *
 *  @author: tomsons26, ZivDero
 */
bool Vinifera_Prep_Speech_For_Side(SideType side)
{
    char name[64];

    if (side == SIDE_NONE) {
        return false;
    }

    /**
     *  Free previously loaded speech MIXes.
     */
    if (SpeechMix != nullptr) {
        DEBUG_INFO("     Releasing {}\n", SpeechMix->Filename);
        delete SpeechMix;
        SpeechMix = nullptr;
    }

    while (ExpandSpeechMix.Count() > 0) {
        delete ExpandSpeechMix[0];
        ExpandSpeechMix.Delete(0);
    }

    /**
     *  Load the new generic speech MIX.
     */
    DEBUG_INFO("     Initializing SPEECH.MIX\n");
    if (CCFileClass("SPEECH.MIX").Is_Available()) {
        MFCD* mix = new MFCD("SPEECH.MIX", &FastKey);
        if (mix != nullptr) {
            ExpandSpeechMix.Add(mix);
            DEBUG_INFO(" SPEECH.MIX");
        }
    }

    int id = static_cast<int>(side) + 1;

    /**
     *  Load the per-side mixes.
     */
    for (AddonType addon = ADDON_COUNT; addon > 0; --addon) {
        if (Addon_Enabled(addon) == true) {
            std::snprintf(name, std::size(name), "E%02dVOX%02d.MIX", addon, id);

            if (CCFileClass(name).Is_Available()) {
                MFCD* mix = new MFCD(name, &FastKey);
                if (mix != nullptr) {
                    ExpandSpeechMix.Add(mix);
                    DEBUG_INFO(" {}", name);
                }
            }
        }
    }

    std::snprintf(name, std::size(name), "SPEECH%02d.MIX", id);
    DEBUG_INFO("     Initializing {}\n", name);
    if (CCFileClass(name).Is_Available()) {
        SpeechMix = new MFCD(name, &FastKey);
    }

    //if (SpeechMix == nullptr) {
    //    DEBUG_INFO("     FAILED!\n");
    //    return false;
    //}

    AudioVoxClass::ScanAsync();

    /**
     *  Reload UI.INI after the side mixes are mounted, then layer any
     *  side-specific UI overrides on top.
     */
    UIControls->Read_INI_File("UI.INI", true);
    UIControls->Read_INI_File("UIOVERRIDES.INI");

    return true;
}


/**
 *  Reimplemention of Init_Secondary_Mixfiles()
 *  
 *  Register and cache secondary mixfiles.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Init_Secondary_Mixfiles()
{
    MFCD *mix;
    char buffer[16];

    DEBUG_INFO("\n"); // Fixes missing new-line after "Init Secondary Mixfiles....." print.
    //DEBUG_INFO("Init secondary mixfiles...\n");

    /**
     *  #issue-653
     * 
     *  Adds support for loading GENERIC.MIX and ISOGEN.MIX mix files.
     * 
     *  @author: CCHyper
     */
    if (CCFileClass("GENERIC.MIX").Is_Available()) {
        GenericMix = new MFCD("GENERIC.MIX", &FastKey);
        ASSERT(GenericMix);
    }
    if (!GenericMix) {
        DEV_DEBUG_WARNING("Failed to load GENERIC.MIX!\n");
    } else {
        GenericMix->Cache();
        DEBUG_INFO(" GENERIC.MIX\n");
    }
    if (CCFileClass("ISOGEN.MIX").Is_Available()) {
        IsoGenericMix = new MFCD("ISOGEN.MIX", &FastKey);
        ASSERT(IsoGenericMix);
    }
    if (!IsoGenericMix) {
        DEV_DEBUG_WARNING("Failed to load ISOGEN.MIX!\n");
    } else {
        IsoGenericMix->Cache();
        DEBUG_INFO(" ISOGEN.MIX\n");
    }

    if (CCFileClass("CONQUER.MIX").Is_Available()) {
        ConquerMix = new MFCD("CONQUER.MIX", &FastKey);
        ASSERT(ConquerMix);
    }
    if (!ConquerMix) {
        DEBUG_WARNING("Failed to load CONQUER.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" CONQUER.MIX\n");
    }

    int cd = CD::GetCurrentDisk();

    /**
     *  Make sure we have a grounded volume index (invalid volumes will cause error).
     */
    if (CD::GetCurrentDisk() < 0) {
        cd = 0;
    }

    /**
     *  Mix file indices are 1 based.
     */
    cd += 1;

    /**
     *  #issue-513
     * 
     *  If the CD system has been flagged that the files are local, we
     *  just glob all the map mix files in the game directory.
     * 
     *  @author: CCHyper
     */
    if (CD::IsOverrideSwap()) {

        std::snprintf(buffer, sizeof(buffer), "MAPS*.MIX");
        if (CCFileClass::Find_First_File(buffer)) {
            DEBUG_INFO(" {}\n", buffer);
            MapsMix = new MFCD(buffer, &FastKey);
            ASSERT(MapsMix);
            while (CCFileClass::Find_Next_File(buffer)) {
                DEBUG_INFO(" {}\n", buffer);
                mix = new MFCD(buffer, &FastKey);
                ASSERT(mix);
                if (mix) {
                    ViniferaMapsMixes.Add(mix);
                }
            }
        }
        CCFileClass::Find_Close();

    } else {
        std::snprintf(buffer, sizeof(buffer), "MAPS%02d.MIX", cd);
        if (CCFileClass(buffer).Is_Available()) {
            MapsMix = new MFCD(buffer, &FastKey);
            ASSERT(MapsMix);
        }
    }
    if (!MapsMix) {
        DEBUG_WARNING("Failed to load {}!\n", buffer);
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CD::IsOverrideSwap()) DEBUG_INFO(" {}\n", buffer);
    }

    if (CCFileClass("MULTI.MIX").Is_Available()) {
        MultiMix = new MFCD("MULTI.MIX", &FastKey);
        ASSERT(MultiMix);
    }
    if (!MultiMix) {
        DEBUG_WARNING("Failed to load MULTI.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" MULTI.MIX\n", buffer);
    }

    if (Addon_Installed(ADDON_FIRESTORM)) {
        if (CCFileClass("SOUNDS01.MIX").Is_Available()) {
            FSSoundsMix = new MFCD("SOUNDS01.MIX", &FastKey);
            ASSERT(FSSoundsMix);
        }
        if (!FSSoundsMix) {
            DEBUG_WARNING("Failed to load SOUNDS01.MIX!\n");
            //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
        } else {
            DEBUG_INFO(" SOUNDS01.MIX\n", buffer);
        }
    }

    if (CCFileClass("SOUNDS.MIX").Is_Available()) {
        SoundsMix = new MFCD("SOUNDS.MIX", &FastKey);
        ASSERT(SoundsMix);
    }
    if (!SoundsMix) {
        DEBUG_WARNING("Failed to load SOUNDS.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SOUNDS.MIX\n", buffer);
    }

    if (CCFileClass("SCORES01.MIX").Is_Available()) {
        FSScoresMix = new MFCD("SCORES01.MIX", &FastKey);
        ASSERT(FSScoresMix);
    }
    if (!FSScoresMix) {
        DEBUG_WARNING("Failed to load SCORES01.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SCORES01.MIX\n", buffer);
    }

	/*
	**  Register the score mixfile.
	*/
    if (CCFileClass("SCORES.MIX").Is_Available()) {
        ScoreMix = new MFCD("SCORES.MIX", &FastKey);
        ASSERT(ScoreMix);
    }
    if (!ScoreMix) {
        DEBUG_WARNING("Failed to load SCORES.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SCORES.MIX\n", buffer);
    }
	ScoresPresent = true;
    AudioTheme.Scan();

    /**
     *  #issue-513
     * 
     *  If the CD system has been flagged that the files are local, we
     *  just glob all the movies mix files in the game directory.
     * 
     *  @author: CCHyper
     */
    if (CD::IsOverrideSwap()) {

        std::snprintf(buffer, sizeof(buffer), "MOVIES*.MIX");
        if (CCFileClass::Find_First_File(buffer)) {
            DEBUG_INFO(" {}\n", buffer);
            MoviesMix = new MFCD(buffer, &FastKey);
            ASSERT(MoviesMix);
            while (CCFileClass::Find_Next_File(buffer)) {
                DEBUG_INFO(" {}\n", buffer);
                mix = new MFCD(buffer, &FastKey);
                ASSERT(mix);
                if (mix) {
                    ViniferaMoviesMixes.Add(mix);
                }
            }
        }
        CCFileClass::Find_Close();

    } else {
        std::snprintf(buffer, sizeof(buffer), "MOVIES%02d.MIX", cd);
        if (CCFileClass(buffer).Is_Available()) {
            MoviesMix = new MFCD(buffer, &FastKey);
            ASSERT(MoviesMix);
        }
    }
    if (!MoviesMix) {
        DEBUG_WARNING("Failed to load {}!\n", buffer);
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CD::IsOverrideSwap()) DEBUG_INFO(" {}\n", buffer);
    }

    return true;
}


/**
 *  Register and cache expansion mixfiles.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Init_Expansion_Mixfiles()
{
    MFCD *mix;
    char buffer[16];

    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "EXPAND%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCD(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load {}!\n", buffer);
            } else {
                ExpandMix.Add(mix);
                DEBUG_INFO(" {}\n", buffer);
            }
        }
    }

    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "ECACHE%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCD(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load {}!\n", buffer);
            } else {
                mix->Cache();
                ExpandMix.Add(mix);
                DEBUG_INFO(" {}\n", buffer);
            }
        }
    }

    /**
     *  #issue-648
     * 
     *  Load ELOCAL*.MIX expansion mixfiles.
     * 
     *  #NOTE:
     *  Red Alert 2 uses the wild-card system to load these files, but to retain
     *  the file naming format Tiberian Sun uses, we now use 00-99.
     * 
     *  @author: CCHyper
     */
#if 0
    std::snprintf(buffer, sizeof(buffer), "ELOCAL*.MIX");
    if (CCFileClass::Find_First_File(buffer)) {
        DEBUG_INFO(" {}\n", buffer);
        mix = new MFCD(buffer, &FastKey);
        ASSERT(mix);
        while (CCFileClass::Find_Next_File(buffer)) {
            DEBUG_INFO(" {}\n", buffer);
            mix = new MFCD(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load {}!\n", buffer);
            } else {
                ExpandMix.Add(mix);
                DEBUG_INFO(" {}\n", buffer);
            }
        }
    }
    CCFileClass::Find_Close();
#else
    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "ELOCAL%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCD(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load {}!\n", buffer);
            } else {
                ExpandMix.Add(mix);
                DEBUG_INFO(" {}\n", buffer);
            }
        }
    }
#endif

    return true;
}


/**
 *  Reimplemention of Init_Bootstrap_Mixfiles()
 *  
 *  Registers and caches any mixfiles needed for bootstrapping.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Init_Bootstrap_Mixfiles()
{
    bool ok;
    MFCD *mix;

    DiskID temp = CD::RequiredCD;
    CD::SetRequiredDisk(DISK_LOCAL);

    DEBUG_INFO("\n"); // Fixes missing new-line after "Bootstrap..." print.
    //DEBUG_INFO("Init bootstrap mixfiles...\n");

    if (CCFileClass("PATCH.MIX").Is_Available()) {
        mix = new MFCD("PATCH.MIX", &FastKey);
        ASSERT(mix);
        if (mix) {
            DEBUG_INFO(" PATCH.MIX\n");
        }
    }

    if (CCFileClass("PCACHE.MIX").Is_Available()) {
        mix = new MFCD("PCACHE.MIX", &FastKey);
        ASSERT(mix);
        if (mix) {
            mix->Cache();
            DEBUG_INFO(" PCACHE.MIX\n");
        }
    }

    Vinifera_Init_Expansion_Mixfiles();

    Detect_Addons();

    TibSunMix = new MFCD("TIBSUN.MIX", &FastKey);
    ASSERT(TibSunMix);
    if (!TibSunMix) {
        DEBUG_WARNING("Failed to load TIBSUN.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" TIBSUN.MIX\n");
    }

    /*
    **  Bootstrap enough of the system so that the error dialog
    *   box can successfully be displayed.
    */
    CacheMix = new MFCD("CACHE.MIX", &FastKey);
    ASSERT(CacheMix);
    if (!CacheMix) {
        DEBUG_WARNING("Failed to load CACHE.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CacheMix->Cache()) {
            DEBUG_WARNING("Failed to cache CACHE.MIX!\n");
            return false;
        }
        DEBUG_INFO(" CACHE.MIX\n");
    }

    LocalMix = new MFCD("LOCAL.MIX", &FastKey);
    ASSERT(LocalMix);
    if (!LocalMix) {
        DEBUG_WARNING("Failed to load LOCAL.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" LOCAL.MIX\n");
    }

    CD::SetRequiredDisk(temp);

    return true;
}


/**
 *  Detaches the debugger from the current process.
 *
 *  @author: secsome
 */
bool Detach_Debugger()
{
    auto GetDebuggerProcessId = [](DWORD dwSelfProcessId) -> DWORD {
        DWORD dwParentProcessId = -1;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(2, 0);
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        Process32First(hSnapshot, &pe32);
        do {
            if (pe32.th32ProcessID == dwSelfProcessId) {
                dwParentProcessId = pe32.th32ParentProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
        CloseHandle(hSnapshot);
        return dwParentProcessId;
    };

    HMODULE hModule = LoadLibrary("ntdll.dll");
    if (hModule != NULL) {
        auto const NtRemoveProcessDebug = reinterpret_cast<NTSTATUS(__stdcall*)(HANDLE, HANDLE)>(GetProcAddress(hModule, "NtRemoveProcessDebug"));
        auto const NtSetInformationDebugObject = reinterpret_cast<NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG)>(GetProcAddress(hModule, "NtSetInformationDebugObject"));
        auto const NtQueryInformationProcess = reinterpret_cast<NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG)>(GetProcAddress(hModule, "NtQueryInformationProcess"));
        auto const NtClose = reinterpret_cast<NTSTATUS(__stdcall*)(HANDLE)>(GetProcAddress(hModule, "NtClose"));

        HANDLE hDebug;
        HANDLE hCurrentProcess = GetCurrentProcess();
        NTSTATUS status = NtQueryInformationProcess(hCurrentProcess, 30, &hDebug, sizeof(HANDLE), 0);
        if (status >= 0) {
            ULONG killProcessOnExit = FALSE;
            status = NtSetInformationDebugObject(hDebug, 1, &killProcessOnExit, sizeof(ULONG), NULL);
            if (status >= 0) {
                const auto pid = GetDebuggerProcessId(GetProcessId(hCurrentProcess));
                status = NtRemoveProcessDebug(hCurrentProcess, hDebug);
                if (status >= 0) {
                    return true;
                }
            }
            NtClose(hDebug);
        }
        FreeLibrary(hModule);
    }

    return false;
}

/**
 *  Give the user time to attach the debugger if one is not already present.
 *
 *  @author: ZivDero, CCHyper
 */
DEFINE_HOOK(0x006B7E22, WinMainCRTStartup_Syringe_Patch, 9)
{
    DEBUG_INFO("Syringe is active.\n");

    if (Detach_Debugger()) {
        if (!IsDebuggerPresent()) {
#ifndef NDEBUG
            bool wait_for_debugger = true;
#else
            const char* cmdline = GetCommandLineA();
            bool wait_for_debugger = (std::strstr(cmdline, "-DEBUGGER_ATTACH") != nullptr);
#endif
            if (wait_for_debugger) {
                MessageBox(nullptr, "Attach the debugger now or continue.", "Vinifera", MB_OK | MB_SERVICE_NOTIFICATION);
            }
        }
    }

    return 0;
}


/**
 *  Load tutorial.ini into our new map in Init_Bulk_Data.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004E46BB, _Init_Bulk_Data_Tutorial_Text_Patch, 0)
{
    REF_STACK(CCINIClass, tutorial_ini, 0x18);
    ScenarioClassExtension::Read_Tutorial_INI(tutorial_ini);
    return 0x004E482E;
}


/**
 *  Clear our new tutorial text map in Prog_End.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00601B15, _Prog_End_Tutorial_Text_Patch, 5)
{
    Vinifera_TutorialText.clear();
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void GameInit_Hooks()
{
    Patch_Jump(0x004E3D20, &Vinifera_Init_Bootstrap_Mixfiles);
    Patch_Jump(0x004E4120, &Vinifera_Init_Secondary_Mixfiles);
    Patch_Jump(0x004E7EB0, &Vinifera_Prep_For_Side);
    Patch_Jump(0x004E8460, &Vinifera_Prep_Speech_For_Side);
    Patch_Call(0x006013AB, &Vinifera_Create_Main_Window_480p);
    Patch_Call(0x00601696, &Vinifera_Create_Main_Window_Custom);

    /**
     *  #issue-110
     * 
     *  Unable to load startup mix files is no longer a fatal error. These
     *  patches change the checks in Init_Bulk_Data to skip the cache process
     *  and continue initialisation.
     */
    Patch_Word(0x004E4601, 0x5C74); // jz 0x004E49B7 -> jz 0x004E465F
    Patch_Byte_Range(0x004E4601+2, 0x90, 4);
    Patch_Word(0x004E460F, 0x4E74); // jz 0x004E49B7 -> jz 0x004E465F
    Patch_Byte_Range(0x004E460F+2, 0x90, 4);
    Patch_Word(0x004E4641, 0x1C74); // jz 0x004E49B7 -> jz 0x004E465F
    Patch_Byte_Range(0x004E4641+2, 0x90, 4);
    Patch_Byte_Range(0x004E4657, 0x90, 8);

    /**
     *  #issue-494
     * 
     *  Fixes a bug where FSMENU would play instead of INTRO in Tiberian Sun
     *  mode after returning to the main menu from a game.
     * 
     *  This was a because the game was checking if the Firestorm addon was
     *  installed rather than if it was the currently active game mode.
     */
    Patch_Call(0x004E1F70, &Addon_Enabled);
    Patch_Call(0x004E25A6, &Addon_Enabled);
    Patch_Call(0x004E2890, &Addon_Enabled);
    Patch_Call(0x004E2991, &Addon_Enabled);
    Patch_Call(0x004E86F5, &Addon_Enabled);
    Patch_Call(0x004E8735, &Addon_Enabled);

    /**
     *  Fixes a bug where CompositeSurface is used instead of HiddenSurface in Allocate_Surfaces.
     */
    Patch_Dword(0x004E743D+1, (uint32_t)0x0074C5DC);

    /**
     *  TS Client file structure assumes Firestorm is always installed and enabled.
     */
    Patch_Jump(0x00407050, &Vinifera_Detect_Addons);
}
