/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWMENUEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended NewMenuClass.
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
#include "vinifera_globals.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "iomap.h"
#include "newmenu.h"
#include "session.h"
#include "cd.h"
#include "addon.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "syringe.h"


/**
 *  Have we performed the menu skip?
 */
static bool MenuSkipDone = false;


/**
 *  Force draw the title screen background.
 * 
 *  @author: CCHyper
 */
static void Draw_Title_Screen(bool firestorm)
{
    Load_Title_Screen(firestorm ? "FSTBACK.PCX" : "TSTBACK.PCX", HiddenSurface, &OriginalPalette);
    Update_Visible_Surface(false, HiddenSurface);
}


/**
 *  Set the addon mode values. This will also set the Firestorm
 *  disk if firestorm mode is requested.
 * 
 *  @author: CCHyper
 */
static void Set_Addon_Mode(bool firestorm)
{
    Disable_Addon(ADDON_ANY);

    if (firestorm) {
         Enable_Addon(ADDON_FIRESTORM);
    }

    Set_Required_Addon(firestorm ? ADDON_FIRESTORM : ADDON_BASE_GAME);

    if (firestorm) {
        CD::SetRequiredDisk(DISK_FIRESTORM);
        CD().ForceAvailable(DISK_FIRESTORM);
        Session.Read_Scenario_Descriptions();
    }
}


/**
 *  #issue-241
 * 
 *  Patch to allow skipping directly to game mode or dialogs.
 * 
 *  @author: CCHyper
 */
static bool firsttime = true;
DEFINE_HOOK(0x004E8831, _NewMenuClass_Process_SkipToMenus_Patch, 0x7)
{
    GET(NewMenuClass *, newmenu, ECX);

    /**
     *  Stolen instruction.
     */
    // Session.IsWDT
    Session.field_4 = false;

    /**
     *  -1 = Game Select
     *   0 = Tiberian Sun
     *   1 = Firestorm
     */
    int gamemode;
    if (firsttime) {
        gamemode = -1;           // Default to Game Select.
        firsttime = false;
    } else {
        gamemode = newmenu->GameMode;
    }

    /**
     *  0 = Exit
     *  1 = Campaign
     *  2 = Load
     *  3 = Network
     *  4 = Internet
     *  5 = Modem
     *  6 = Skirmish
     *  7 = WDT
     *  8 = Options
     *  9 = Network Load
     *  10 = Intro/Sneak Peak
     *  11 = Version
     *  12 = Credits
     *  13 = Main Menu
     */
    int mode = 0;               // Default to Exit.

    if (Vinifera_SkipToTSMenu) {
        DEBUG_INFO("Skipping to the Tiberian Sun menu.\n");
        Vinifera_SkipToTSMenu = false;
        Set_Addon_Mode(false);
        gamemode = 0;
    }

    if (Vinifera_SkipToFSMenu) {
        DEBUG_INFO("Skipping to the Firestorm menu.\n");
        Vinifera_SkipToFSMenu = false;
        Set_Addon_Mode(true);
        gamemode = 1;
    }

    if (Vinifera_SkipToLAN) {
        DEBUG_INFO("Skipping to the LAN dialog.\n");
        mode = 3;
        goto set_dialog_check_for_exit;
    }

    if (Vinifera_SkipToCampaign) {
        DEBUG_INFO("Skipping to the Campaign dialog.\n");
        mode = 1;
        goto set_dialog_check_for_exit;
    }

    if (Vinifera_SkipToSkirmish) {
        DEBUG_INFO("Skipping to the Skirmish dialog.\n");
        mode = 6;
        goto set_dialog_check_for_exit;
    }

    if (Vinifera_SkipToInternet) {
        DEBUG_INFO("Skipping to the Internet dialog.\n");
        mode = 4;
        goto set_dialog_check_for_exit;
    }

    /**
     *  Should we exit the game after we have returned from a
     *  dialog that we skipped directly to?
     */
    if (MenuSkipDone && Vinifera_ExitAfterSkip) {
        DEBUG_INFO("Forcing game exit.\n");
        Vinifera_ExitAfterSkip = false;
        mode = 0;
        goto set_dialog;
    }
    
    /**
     *  Show the desired game menu.
     */
show_game_menu:
    newmenu->GameMode = gamemode;
    return 0;
    
    /**
     *  Set the desired dialog, making sure Exit was not set.
     */
set_dialog_check_for_exit:

    /**
     *  Clear any globals.
     */
    Vinifera_SkipToLAN = false;
    Vinifera_SkipToSkirmish = false;
    Vinifera_SkipToCampaign = false;
    Vinifera_SkipToInternet = false;

    MenuSkipDone = true;

    if (mode == 0) {
        goto show_game_menu;
    }

    /**
     *  The "new menu" system design is a little awkward, so we need to
     *  force the PCX filename it updates the screen with behind dialogs.
     */
    if (gamemode == 1) {
        std::strcpy((char *)newmenu->BackgroundImage, "FSTBACK.PCX");
    } else {
        std::strcpy((char *)newmenu->BackgroundImage, "TSTBACK.PCX");
    }
    
    /**
     *  Force the addon mode so Firestorm works correctly.
     */
    if (gamemode == 1) {
        Set_Addon_Mode(true);
    } else {
        Set_Addon_Mode(false);
    }

    /**
     *  Set the desired dialog, no checks.
     */
set_dialog:
    R->EAX(mode);
    return 0x004E883D;
}


/**
 *  Main function for patching the hooks.
 */
void NewMenuExtension_Hooks()
{
    
}

