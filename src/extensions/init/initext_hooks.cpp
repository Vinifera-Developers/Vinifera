/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INITEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains any hooks for the game init process.
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
#include "initext_hooks.h"
#include "initext_functions.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "special.h"
#include "playmovie.h"
#include "ccfile.h"
#include "cd.h"
#include "newmenu.h"
#include "addon.h"
#include "theme.h"
#include "command.h"
#include "side.h"
#include "session.h"
#include "iomap.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Reimplemention of Prep_For_Side()
 *  
 *  Prepare the mixfiles for the player side.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Prep_For_Side(SideType side)
{
    DEBUG_INFO("Preparing Mixfiles for Side %02d.\n", side);

    MFCC *mix;
    char buffer[16];

    int sidenum = (side+1); // Logical side number.

    if (SideCachedMix) {
        DEBUG_INFO("  Releasing %s\n", SideCachedMix->Filename);
        delete SideCachedMix;
        SideCachedMix = nullptr;
    }
    if (SideNotCachedMix) {
        DEBUG_INFO("  Releasing %s\n", SideNotCachedMix->Filename);
        delete SideNotCachedMix;
        SideNotCachedMix = nullptr;
    }
    if (SideCDMix) {
        DEBUG_INFO("  Releasing %s\n", SideCDMix->Filename);
        delete SideCDMix;
        SideCDMix = nullptr;
    }

    for (int i = 0; i < SideMixFiles.Count(); ++i) {
        DEBUG_INFO("  Releasing %s\n", SideMixFiles[i]->Filename);
        delete SideMixFiles[i];
        SideMixFiles.Delete(i);
    }

    if (Addon_Enabled(-1) == 1) {

        for (int i = 99; i >= 0; --i) {
            std::snprintf(buffer, sizeof(buffer), "E%02dSC%02d.MIX", i, sidenum);
            if (CCFileClass(buffer).Is_Available()) {
                mix = new MFCC(buffer, &FastKey);
                ASSERT(mix);
                if (!mix) {
                    DEBUG_WARNING("  Failed to load %s!\n", buffer);
                    return false;
                }
                if (!mix->Cache()) {
                    DEBUG_WARNING("  Failed to cache %s!\n", buffer);
                    return false;
                }
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }

    }

    std::snprintf(buffer, sizeof(buffer), "SIDEC%02d.MIX", sidenum);
    if (CCFileClass(buffer).Is_Available()) {
        SideCachedMix = new MFCC(buffer, &FastKey);
        ASSERT(SideCachedMix);
        if (!SideCachedMix) {
            DEBUG_WARNING("  Failed to load %s!\n", buffer);
            return false;
        }
        if (!SideCachedMix->Cache()) {
            DEBUG_WARNING("  Failed to cache %s!\n", buffer);
            return false;
        }
        DEBUG_INFO(" %s\n", buffer);
    }

    std::snprintf(buffer, sizeof(buffer), "SIDENC%02d.MIX", sidenum);
    if (CCFileClass(buffer).Is_Available()) {
        SideNotCachedMix = new MFCC(buffer, &FastKey);
        ASSERT(SideNotCachedMix);
        if (!SideNotCachedMix) {
            DEBUG_WARNING("  Failed to load %s!\n", buffer);
            return false;
        }
        DEBUG_INFO(" %s\n", buffer);
    }

    if (Session.Type == GAME_NORMAL) {
        if (Addon_Enabled(-1) == 1) {
            std::snprintf(buffer, sizeof(buffer), "E%02dSCD%02d.MIX", Get_Required_Addon(), sidenum);
        } else {
            std::snprintf(buffer, sizeof(buffer), "SIDECD%02d.MIX", sidenum);
        }
        if (CCFileClass(buffer).Is_Available()) {
            SideCDMix = new MFCC(buffer, &FastKey);
            ASSERT(SideCDMix);
            if (!SideCDMix) {
                DEBUG_WARNING("  Failed to load %s!\n", buffer);
                return false;
            }
            DEBUG_INFO(" %s\n", buffer);
        }
    }

    Map.Init_For_House();

    return true;
}


/**
 *  #issue-305
 * 
 *  Fixes bug where the sidebar mouse wheel scrolling "error" sound
 *  can be heard at the main menu.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Main_Window_Procedure_Scroll_Sidebar_Check_Patch)
{
    GET_STACK_STATIC(UINT, wParam, esp, 0x14);
    static bool _mouse_wheel_scolling;

    /**
     *  The code before this patch checks for WM_MOUSEWHEEL.
     */

    /**
     *  We are not currently playing a scenario, no need to execute this command.
     */
    if (!bool_007E4040 && !bool_007E48FC) {
        goto message_handler;
    }

    /**
     *  Are we currently executing a scroll command? This is required because
     *  the Main_Window_Procedure function runs at a Windows level.
     */
    if (_mouse_wheel_scolling) {
        goto message_handler;
    }

    _mouse_wheel_scolling = true;

    /**
     *  Execute the command based on the direction of the mouse wheel.
     */
    if ((wParam & 0x80000000) == 0) {
        CommandClass::Activate_From_Name("SidebarUp");
    } else {
        CommandClass::Activate_From_Name("SidebarDown");
    }

    _mouse_wheel_scolling = false;

executed:
    JMP_REG(eax, 0x00685F9C);

message_handler:
    JMP_REG(ecx, 0x00685FA0);
}


/**
 *  Reimplemention of Init_Secondary_Mixfiles()
 *  
 *  Register and cache secondary mixfiles.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Init_Secondary_Mixfiles()
{
    MFCC *mix;
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
        GenericMix = new MFCC("GENERIC.MIX", &FastKey);
        ASSERT(GenericMix);
    }
    if (!GenericMix) {
        DEV_DEBUG_WARNING("Failed to load GENERIC.MIX!\n");
    } else {
        GenericMix->Cache();
        DEBUG_INFO(" GENERIC.MIX\n");
    }
    if (CCFileClass("ISOGEN.MIX").Is_Available()) {
        IsoGenericMix = new MFCC("ISOGEN.MIX", &FastKey);
        ASSERT(IsoGenericMix);
    }
    if (!IsoGenericMix) {
        DEV_DEBUG_WARNING("Failed to load ISOGEN.MIX!\n");
    } else {
        IsoGenericMix->Cache();
        DEBUG_INFO(" ISOGEN.MIX\n");
    }

    if (CCFileClass("CONQUER.MIX").Is_Available()) {
        ConquerMix = new MFCC("CONQUER.MIX", &FastKey);
        ASSERT(ConquerMix);
    }
    if (!ConquerMix) {
        DEBUG_WARNING("Failed to load CONQUER.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" CONQUER.MIX\n");
    }

    int cd = CD::Get_Volume_Index();

    /**
     *  Make sure we have a grounded volume index (invalid volumes will cause error).
     */
    if (CD::Get_Volume_Index() < 0) {
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
    if (CD::IsFilesLocal) {

        std::snprintf(buffer, sizeof(buffer), "MAPS*.MIX");
        if (CCFileClass::Find_First_File(buffer)) {
            DEBUG_INFO(" %s\n", buffer);
            MapsMix = new MFCC(buffer, &FastKey);
            ASSERT(MapsMix);
            while (CCFileClass::Find_Next_File(buffer)) {
                DEBUG_INFO(" %s\n", buffer);
                mix = new MFCC(buffer, &FastKey);
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
            MapsMix = new MFCC(buffer, &FastKey);
            ASSERT(MapsMix);
        }
    }
    if (!MapsMix) {
        DEBUG_WARNING("Failed to load %s!\n", buffer);
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CD::IsFilesLocal) DEBUG_INFO(" %s\n", buffer);
    }

    if (CCFileClass("MULTI.MIX").Is_Available()) {
        MultiMix = new MFCC("MULTI.MIX", &FastKey);
        ASSERT(MultiMix);
    }
    if (!MultiMix) {
        DEBUG_WARNING("Failed to load MULTI.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" MULTI.MIX\n", buffer);
    }

    if (Addon_407120(ADDON_FIRESTORM)) {
        if (CCFileClass("SOUNDS01.MIX").Is_Available()) {
            FSSoundsMix = new MFCC("SOUNDS01.MIX", &FastKey);
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
        SoundsMix = new MFCC("SOUNDS.MIX", &FastKey);
        ASSERT(SoundsMix);
    }
    if (!SoundsMix) {
        DEBUG_WARNING("Failed to load SOUNDS.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SOUNDS.MIX\n", buffer);
    }

    if (CCFileClass("SCORES01.MIX").Is_Available()) {
        FSScoresMix = new MFCC("SCORES01.MIX", &FastKey);
        ASSERT(FSScoresMix);
    }
    if (!FSScoresMix) {
        DEBUG_WARNING("Failed to load SCORES01.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SCORES01.MIX\n", buffer);
    }

	/*
	**	Register the score mixfile.
	*/
    if (CCFileClass("SCORES.MIX").Is_Available()) {
        ScoreMix = new MFCC("SCORES.MIX", &FastKey);
        ASSERT(ScoreMix);
    }
    if (!ScoreMix) {
        DEBUG_WARNING("Failed to load SCORES.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SCORES.MIX\n", buffer);
    }
	ScoresPresent = true;
	Theme.Scan();

    /**
     *  #issue-513
     * 
     *  If the CD system has been flagged that the files are local, we
     *  just glob all the movies mix files in the game directory.
     * 
     *  @author: CCHyper
     */
    if (CD::IsFilesLocal) {

        std::snprintf(buffer, sizeof(buffer), "MOVIES*.MIX");
        if (CCFileClass::Find_First_File(buffer)) {
            DEBUG_INFO(" %s\n", buffer);
            MoviesMix = new MFCC(buffer, &FastKey);
            ASSERT(MoviesMix);
            while (CCFileClass::Find_Next_File(buffer)) {
                DEBUG_INFO(" %s\n", buffer);
                mix = new MFCC(buffer, &FastKey);
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
            MoviesMix = new MFCC(buffer, &FastKey);
            ASSERT(MoviesMix);
        }
    }
    if (!MoviesMix) {
        DEBUG_WARNING("Failed to load %s!\n", buffer);
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CD::IsFilesLocal) DEBUG_INFO(" %s\n", buffer);
    }

    return true;
}


/**
 *  Register and cache expansion mixfiles.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Init_Expansion_Mixfiles()
{
    MFCC *mix;
    char buffer[16];

    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "EXPAND%02d.MIX", i);
        if (RawFileClass(buffer).Is_Available()) {
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }
    }

    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "ECACHE%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                mix->Cache();
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
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
        DEBUG_INFO(" %s\n", buffer);
        mix = new MFCC(buffer, &FastKey);
        ASSERT(mix);
        while (CCFileClass::Find_Next_File(buffer)) {
            DEBUG_INFO(" %s\n", buffer);
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }
    }
    CCFileClass::Find_Close();
#else
    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "ELOCAL%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
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
static bool Vinifera_Init_Bootstrap_Mixfiles()
{
    bool ok;
    MFCC *mix;

    int temp = CD::RequiredCD;
    CD::Set_Required_CD(-2);

    DEBUG_INFO("\n"); // Fixes missing new-line after "Bootstrap..." print.
    //DEBUG_INFO("Init bootstrap mixfiles...\n");

    if (RawFileClass("PATCH.MIX").Is_Available()) {
        mix = new MFCC("PATCH.MIX", &FastKey);
        ASSERT(mix);
        if (mix) {
            DEBUG_INFO(" PATCH.MIX\n");
        }
    }

    if (CCFileClass("PCACHE.MIX").Is_Available()) {
        mix = new MFCC("PCACHE.MIX", &FastKey);
        ASSERT(mix);
        if (mix) {
            mix->Cache();
            DEBUG_INFO(" PCACHE.MIX\n");
        }
    }

    Vinifera_Init_Expansion_Mixfiles();

    Addon_Present();

    TibSunMix = new MFCC("TIBSUN.MIX", &FastKey);
    ASSERT(TibSunMix);
    if (!TibSunMix) {
        DEBUG_WARNING("Failed to load TIBSUN.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" TIBSUN.MIX\n");
    }

    /*
    **	Bootstrap enough of the system so that the error dialog
    *   box can successfully be displayed.
    */
    CacheMix = new MFCC("CACHE.MIX", &FastKey);
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

    LocalMix = new MFCC("LOCAL.MIX", &FastKey);
    ASSERT(LocalMix);
    if (!LocalMix) {
        DEBUG_WARNING("Failed to load LOCAL.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" LOCAL.MIX\n");
    }

    CD::Set_Required_CD(temp);

    return true;
}


/**
 *  #issue-513
 * 
 *  Patch to add check for CD::IsFilesLocal to make sure -CD really
 *  was set by the user.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_CDROM_Access_Local_Files_Patch)
{
    _asm { add esp, 4 }

    /**
     *  If there are search drives specified then all files are to be
     *  considered local.
     */
    if (CCFileClass::Is_There_Search_Drives()) {
        
        /**
         *  Double check that the game was launched with -CD.
         */
        if (CD::IsFilesLocal) {

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
    JMP(0x004E0471);

    /**
     *  Flag files as being local, no CD-ROM init.
     */
files_local:
    JMP(0x004E06F5);
}


static bool CCFile_Is_Available(const char *filename)
{
    return CCFileClass(filename).Is_Available();
}


/**
 *  #issue-478
 * 
 *  
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Game_Skip_Startup_Movies_Patch)
{
    if (Vinifera_SkipStartupMovies) {
        DEBUG_INFO("Skipping startup movies.\n");
        goto skip_loading_screen;
    }

    if (Special.IsFromInstall) {
        DEBUG_GAME("Playing first time intro sequence.\n");
        Play_Movie("EVA.VQA", THEME_NONE, true, true, true);
    }

    if (!Vinifera_SkipWWLogoMovie) {
        DEBUG_GAME("Playing startup movies.\n");
        Play_Movie("WWLOGO.VQA", THEME_NONE, true, true, true);
    } else {
        DEBUG_INFO("Skipping startup movie.\n");
    }

    if (!NewMenuClass::Get()) {
        if (CCFile_Is_Available("FS_TITLE.VQA")) {
            Play_Movie("FS_TITLE.VQA", THEME_NONE, true, false, true);
        } else {
            Play_Movie("STARTUP.VQA", THEME_NONE, true, false, true);
        }
    }

loading_screen:
    _asm { or ebx, 0xFFFFFFFF }
    JMP(0x004E0848);

skip_loading_screen:
    JMP(0x004E084D);
}


/**
 *  Main function for patching the hooks.
 */
void GameInit_Hooks()
{
    Patch_Jump(0x004E0786, &_Init_Game_Skip_Startup_Movies_Patch);
    Patch_Jump(0x004E0461, &_Init_CDROM_Access_Local_Files_Patch);
    Patch_Jump(0x004E3D20, &Vinifera_Init_Bootstrap_Mixfiles);
    Patch_Jump(0x004E4120, &Vinifera_Init_Secondary_Mixfiles);
    Patch_Jump(0x004E7EB0, &Vinifera_Prep_For_Side);
    Patch_Jump(0x00686190, &Vinifera_Create_Main_Window);

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

    Patch_Jump(0x00685F69, &_Main_Window_Procedure_Scroll_Sidebar_Check_Patch);
}
