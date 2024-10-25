/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains various hooks that do not fit elsewhere.
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
#include "vinifera_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "vinifera_functions.h"
#include "vinifera_saveload.h"
#include "vinifera_gitinfo.h"
#include "dsurface.h"
#include "wwmouse.h"
#include "blowfish.h"
#include "blowstraw.h"
#include "blowpipe.h"
#include "iomap.h"
#include "theme.h"
#include "tracker.h"
#include "loadoptions.h"
#include "language.h"
#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "ebolt.h"
#include "kamikazetracker.h"
#include "spawnmanager.h"


/**
 *  This function is for intercepting the calls to Detach_This_From_All to also
 *  process the object through the extension interface.
 * 
 *  @author: CCHyper
 */
static void _Detach_This_From_All_Intercept(TARGET target, bool all)
{
    Extension::Detach_This_From_All(target, all);

    if (target->What_Am_I() == RTTI_AIRCRAFT)
        KamikazeTracker->Detach(reinterpret_cast<AircraftClass const*>(target));

    Detach_This_From_All(target, all);
}


/**
 *  This function is for intercepting the calls to Free_Heaps to also process
 *  the extension interface.
 * 
 *  @author: CCHyper
 */
static void _Free_Heaps_Intercept()
{
    /**
     *  Cleanup global heaps/vectors.
     */
    ++ScenarioInit;

    EBoltClass::Clear_All();
    ArmorTypes.Clear();
    RocketTypes.Clear();
    SpawnManagerClass::Clear_All();

    Extension::Free_Heaps();

    --ScenarioInit;

    Free_Heaps();
}


/**
 *  This patch calls the Print_CRCs function from extension interface.
 *
 *  @author: CCHyper
 */
static void _Print_CRCs_Intercept(EventClass *ev)
{
    /**
     *  Call the original function to print the object CRCs.
     */
    DEBUG_INFO("About to call Print_CRCs...\n");
    Print_CRCs(ev);

    /**
     *  Calls a reimplementation of Print_CRCs that prints both the original
     *  information and the new class extension CRCs.
     */
    DEBUG_INFO("About to call Extension::Print_CRCs...\n");
    Extension::Print_CRCs(ev);
}


#if 0
/**
 *  This function is for intercepting the call to Clear_Scenarion in Load_All
 *  to flag that we are performing a load operation, which stops the game from
 *  creating extensions while the Windows API calls the class factories to create
 *  the instances.
 *
 *  @author: tomsons26
 */
static void _On_Load_Clear_Scenario_Intercept()
{
    Clear_Scenario();

    /**
     *  Now the scenario data has been cleaned up, we can now tell the extension
     *  hooks that we will be creating the extension classes via the class factories.
     */
    Vinifera_PerformingLoad = true;
}
#endif


/**
 *  Draws the version text on the main menu.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Version_Text_Draw_Patch)
{
    GET_REGISTER_STATIC(XSurface *, surface, ecx);

    Vinifera_Draw_Version_Text(surface);

    _asm { ret }
}


/**
 *  Draws the version text over the loading screen background.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ProgressClass_Load_Screen_Version_Text_Patch)
{
    Vinifera_Draw_Version_Text(HiddenSurface);

    /**
     *  Stolen bytes/code.
     */
original_code:
    _asm { mov eax, [HiddenSurface] }
    _asm { mov edx, [eax] } // Second dereference required due to the global reference in TS++.

    JMP(0x005ADFC4);
}


/**
 *  Draws the version text over the loading screen background.
 * 
 *  @note: This has to be after the New menu initialisation, otherwise the menu
 *         title page will draw over the text.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Game_Loading_Screen_Version_Text_Patch)
{
    /**
     *  Flag as pre-init, as we need to draw this differently.
     */
    Vinifera_Draw_Version_Text(PrimarySurface, true);

    /**
     *  Stolen bytes/code.
     */
original_code:
    Call_Back();

    JMP(0x004E0852);
}


/**
 *  Draws the version text over the menu background.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Load_Title_Page_Version_Text_Patch)
{
    Vinifera_Draw_Version_Text(HiddenSurface, true);

    _asm { ret }
}


/**
 *  Patch in the Vinifera command line parser.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WinMain_Parse_Command_Line)
{
    GET_REGISTER_STATIC(int, argc, edi);
    static char **argv; _asm { lea eax, [ebp-0x178] } _asm { mov argv, eax }

    /**
     *  Parse_Command_Line could return 
     */
    if (!Parse_Command_Line(argc, argv) || !Vinifera_Parse_Command_Line(argc, argv)) {
        JMP(0x00601A3B); // Failure.
    } else {
        JMP(0x00601085);
    }
}


/**
 *  Patch in the main Vinifera startup function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WinMain_Vinifera_Startup)
{
    if (Vinifera_Startup()) {
        JMP(0x005FFC41);
    }

    /**
     *  Something went wrong!
     */

    DEBUG_ERROR("Failed to initialise Vinifera systems!\n");

    _asm { mov esi, EXIT_FAILURE }
    JMP(0x00601A6B);
}


/**
 *  Patch in the Vinifera com object register function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WinMain_Register_Com_Objects)
{
    Vinifera_Register_Com_Objects();

    JMP(0x00600FA3);
}


/**
 *  Patch in the main Vinifera shutdown function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Game_Shutdown_Vinifera_Shutdown)
{
    if (!Vinifera_Shutdown()) {

        /**
         *  Something went wrong!
         */

        DEBUG_ERROR("Failed to shutdown Vinifera systems!\n");
    }

    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret }
}


/**
 *  Patch in the Vinifera init game function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Main_Game_Vinifera_Init_Game)
{
    GET_REGISTER_STATIC(int, argc, ecx);
    GET_REGISTER_STATIC(char **, argv, edx);
    static int retval;

    retval = Vinifera_Pre_Init_Game(argc, argv);
    if (retval) {
        if (retval < 0) {
            goto show_error;
        }
        goto failure;
    }
    DEV_DEBUG_INFO("Vinifera_Pre_Init_Game returned OK.\n");

    retval = Init_Game(argc, argv);
    if (retval) {
        if (retval < 0) {
            goto show_error;
        }
        goto failure;
    }
    DEV_DEBUG_INFO("Init_Game returned OK.\n");

    retval = Vinifera_Post_Init_Game(argc, argv);
    if (retval) {
        if (retval < 0) {
            goto show_error;
        }
        goto failure;
    }
    DEV_DEBUG_INFO("Vinifera_Post_Init_Game returned OK.\n");

success:
    JMP(0x00462990);

failure:
    JMP(0x00462932);

show_error:
    JMP(0x00462938);
}


/**
 *  #issue-96
 * 
 *  Remove the requirement for BLOWFISH.DLL (Blowfish encryption) and now
 *  handle the encryption/decryption internally.
 * 
 *  @author: CCHyper
 */

class FakeBlowfishClass
{
    public:
        BlowfishEngine *Hook_Ctor() { return new (reinterpret_cast<BlowfishEngine *>(this)) BlowfishEngine; }
        void Hook_Dtor() { reinterpret_cast<BlowfishEngine *>(this)->BlowfishEngine::~BlowfishEngine(); }
};

static void _Remove_External_Blowfish_Dependency_Patch()
{
    /**
     *  The following two patches remove dependency on BLOWFISH.DLL being registered at startup.
     */
    Patch_Jump(0x00600F6E, 0x00600FA3); // This forces the game init process to skip BLOWFISH.DLL loading errors.
    Patch_Jump(0x005FFE46, 0x005FFF2B); // This skips code registering BLOWFISH.DLL.

    /**
     *  Hook in the implementations of BlowStraw, BlowPipe and BlowfishEngine.
     */
    Hook_Virtual(0x00424230, BlowStraw::Get);
    Hook_Virtual(0x00424320, BlowStraw::Key);
    Hook_Virtual(0x00424080, BlowPipe::Flush);
    Hook_Virtual(0x004240C0, BlowPipe::Put);
    Hook_Virtual(0x004241F0, BlowPipe::Key);

    Hook_Function(0x00423F70, &FakeBlowfishClass::Hook_Ctor);
    Hook_Function(0x00423FE0, &FakeBlowfishClass::Hook_Dtor);
    Hook_Function(0x00423FF0, &BlowfishEngine::Submit_Key);
    Hook_Function(0x00424020, &BlowfishEngine::Encrypt);
    Hook_Function(0x00424050, &BlowfishEngine::Decrypt);
}


/**
 *  Clear any game session and global variables before next game.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Select_Game_Clear_Globals_Patch)
{
    /**
     *  Clear any developer mode globals.
     */
    Vinifera_Developer_AIInstantBuild = false;
    Vinifera_Developer_InstantBuild = false;
    Vinifera_Developer_InstantSuperRecharge = false;
    Vinifera_Developer_AIInstantSuperRecharge = false;
    Vinifera_Developer_BuildCheat = false;
    Vinifera_Developer_Unshroud = false;
    Vinifera_Developer_FrameStep = false;
    Vinifera_Developer_FrameStepCount = 0;
    Vinifera_Developer_AIControl = false;

    /**
     *  Reset any globals.
     */
    Vinifera_ShowSuperWeaponTimers = true;
    Vinifera_TotalPlayTime = 0;

    /**
     *  Stolen bytes/code.
     */
    Map.Set_Default_Mouse(MOUSE_NORMAL);

    JMP(0x004E1F30);
}


/**
 *  Replaces the division-by-zero crash in SwizzleManagerClass::Process_Tables() with
 *  a readable error, produces a crash dump and then exit.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SwizzleManagerClass_Process_Tables_Remap_Failed_Error)
{
    static int old_ptr;

    _asm { mov eax, [edi+0x4] }
    _asm { mov old_ptr, eax }
    //GET_REGISTER_STATIC(int, old_ptr, edi);

    DEBUG_ERROR("Swizzle Manager - Failed to remap pointer! (old_ptr = 0x%08X)!\n", old_ptr);

    ShowCursor(TRUE);

    MessageBoxA(MainWindow, "Failed to process save game file!", "Vinifera", MB_OK|MB_ICONEXCLAMATION);

#if 0
    if (!IsDebuggerPresent()) {
        Vinifera_Generate_Mini_Dump();
    }
#endif

    Fatal("Swizzle Manager - Failed to remap pointer! (old_ptr = 0x%08X)!\n", old_ptr);

    /**
     *  We won't ever get here, but its here just for clean analysis.
     */
    JMP(0x0060DC15);
}


#ifndef RELEASE
/**
 *  Disables the Load, Save and Delete buttons in the options menu.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SaveLoad_Disable_Buttons)
{
    GET_REGISTER_STATIC(HWND, hDlg, ebp);

    EnableWindow(GetDlgItem(hDlg, 1310), FALSE); // Load button
    EnableWindow(GetDlgItem(hDlg, 1311), FALSE); // Save button
    EnableWindow(GetDlgItem(hDlg, 1312), FALSE); // Delete button

    JMP(0x004B6DF5);
}

/**
 *  Disables the Load button on the Firestorm main menu.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_NewMenuClass_Process_Disable_Load_Button_Firestorm)
{
    JMP(0x0057FFAC);
}

/**
 *  Disables the Load button on the Tiberian Sun main menu.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_NewMenuClass_Process_Disable_Load_Button_TiberianSun)
{
    JMP(0x00580075);
}
#endif


/**
 *  #issue-269
 * 
 *  Adds a "Load Game" button to the dialog shown on mission lose.
 * 
 *  @author: CCHyper
 */
static bool _Save_Games_Available()
{
    return LoadOptionsClass().Read_Save_Files();
}

static bool _Do_Load_Dialog()
{
    return LoadOptionsClass().Load_Dialog();
}


DECLARE_PATCH(_Do_Lose_Create_Lose_WWMessageBox)
{
    static int ret;

    /**
     *  Show the message box.
     */
retry_dialog:
    ret = Vinifera_Do_WWMessageBox(Text_String(TXT_TO_REPLAY), Text_String(TXT_YES), Text_String(TXT_NO), "Load Game");
    switch (ret) {
        default:
        case 0: // User pressed "Yes"
            JMP(0x005DCE1A);

        case 1: // User pressed "No"
            JMP(0x005DCE56);

        case 2: // User pressed "Load Game"
        {
#if 0 //!defined(RELEASE) && defined(NDEBUG)
            /**
             *  We disable loading in non-release.
             */
            Vinifera_Do_WWMessageBox("Saving and Loading is disabled for non-release builds.", Text_String(TXT_OK));
#else
            /**
             *  If no save games are available, notify the user and return back
             *  and reissue the main dialog.
             */
            if (!_Save_Games_Available()) {
                Vinifera_Do_WWMessageBox("No saved games available.", Text_String(TXT_OK));
                goto retry_dialog;
            }

            /**
             *  Show the load game dialog.
             */
            ret = _Do_Load_Dialog();
            if (ret) {
                Theme.Stop();
                JMP(0x005DCE48);
            }
#endif

            /**
             *  Reissue the dialog if the user pressed cancel on the load dialog.
             */
            goto retry_dialog;
        }
    };
}


/**
 *  Produces a random serial number for this client.
 * 
 *  #NOTE:
 *  The result number string will be invalid and will not pass WWOnline/XWIS
 *  checks, this is for local network use only.
 * 
 *  @author: CCHyper
 */
static void Decrypt_Serial(char *buffer)
{
    static bool _done = false;
    static char _buf[] = { "0000000000000000000000" };
    static const char _alphanum[] = { "0123456789" };

    /**
     *  Generate a one-time random number string.
     */
    if (!_done) {
        std::srand(timeGetTime());
        for (int i = 0; i < std::size(_buf); ++i) {
            _buf[i] = _alphanum[std::rand() % (std::size(_alphanum)-1)];
        }
        _done = true;
    }

    std::strncpy(buffer, _buf, sizeof(_buf));
}


/**
 *  Export function that returns the supported save file version.
 *
 *  @author: CCHyper
 */
__declspec(dllexport) uint32_t __cdecl Vinifera_Save_File_Version()
{
    return Extension::Get_Save_Version_Number();
}


void Vinifera_Hooks()
{
    /**
     *  Remove the requirement for BLOWFISH.DLL (Blowfish encryption).
     */
    _Remove_External_Blowfish_Dependency_Patch();

    /**
     *  Draw the build version info on the bottom on the screen.
     */
    Patch_Jump(0x004E53C0, &_Version_Text_Draw_Patch);
    Patch_Jump(0x005ADFBE, &_ProgressClass_Load_Screen_Version_Text_Patch);
    Patch_Jump(0x004E084D, &_Init_Game_Loading_Screen_Version_Text_Patch);
    Patch_Jump(0x004E3B7A, &_Load_Title_Page_Version_Text_Patch);

    /**
     *  Add in Vinifera startup/shutdown hooks.
     */
    Patch_Jump(0x00601070, &_WinMain_Parse_Command_Line);
    Patch_Jump(0x005FF81C, &_WinMain_Vinifera_Startup);
    Patch_Jump(0x00600F6E, &_WinMain_Register_Com_Objects);
    Patch_Jump(0x00602474, &_Game_Shutdown_Vinifera_Shutdown);
    Patch_Jump(0x00462927, &_Main_Game_Vinifera_Init_Game);

    /**
     *  Clear any game session and global variables before next game.
     */
    Patch_Jump(0x004E1F24, &_Select_Game_Clear_Globals_Patch);

#ifndef NDEBUG
    /**
     *  These patches remove the Digest requirement for LANGRULE.INI, allowing
     *  this file to be used by developers to quickly test features.
     */
    Patch_Byte(0x005C656E+1, 0); // CCINIClass::Load argument from "true" to "false".
    Patch_Byte(0x004E1436, 0x53); // CCINIClass::Load argument from "true" to "false".
#endif

#ifndef NDEBUG
    /**
     *  This patch allows 1 player LAN games for testing various network features.
     */
    Patch_Jump(0x00577029, 0x00577071);

    /**
     *  Allow up to 7 AI players in LAN games.
     */
    Patch_Byte(0x0057C97E+3, 0x07);
#endif

    /**
     *  Fire an assert on save/load fail, rather than hard crash.
     */
    Patch_Jump(0x0060DBFF, &_SwizzleManagerClass_Process_Tables_Remap_Failed_Error);

    /**
     *  Patch in the new save and load system functions.
     */
    Patch_Jump(0x005D4FE0, &Vinifera_Save_Game);
    Patch_Jump(0x005D6910, &Vinifera_Load_Game);

    /**
     *  Hooks related to saving/loading games.
     */
    SaveGame_Hooks();

    /**
     *  Set the save game version.
     */
    ViniferaGameVersion = Extension::Get_Save_Version_Number();
    DEBUG_INFO("Save game version number: 0x%X\n", ViniferaGameVersion);

    Patch_Jump(0x005DCDFD, &_Do_Lose_Create_Lose_WWMessageBox);

    /**
     *  This patch randomises the serial number for this client.
     */
    Patch_Jump(0x00576410, &Decrypt_Serial);

    /**
     *  These two patches changes the last character of the Autorun and Game
     *  application mutex GUID's So Vinifera can be run alongside another instance
     *  of Tiberian Sun (and even Red Alert 2 or Yuri's Revenge).
     * 
     *  "b350c6d2-2f36-11d3-a72c-0090272fa661" -> "b350c6d2-2f36-11d3-a72c-0090272fa66n"
     *  "29e3bb2a-2f36-11d3-a72c-0090272fa661" -> "29e3bb2a-2f36-11d3-a72c-0090272fa66n"
     */
    std::srand(timeGetTime());
    unsigned char num = (std::rand() % 10)+48;
    Patch_Byte(0x0070EEAB, num);
    Patch_Byte(0x0070EF0F, num);

#if defined(TS_CLIENT)
    /**
     *  Remove calls to SessionClass::Read_Scenario_Descriptions() in TS Client
     *  compatable builds. This will speed up the initialisation and loading
     *  process, as the reason of PKT and MPR files are not required when using
     *  the Client.
     */
    Patch_Byte_Range(0x004E8901, 0x90, 5); // NewMenu::Process
    Patch_Byte_Range(0x004E8910, 0x90, 5); // ^
    Patch_Byte_Range(0x00564BA9, 0x90, 10); // Select_MPlayer_Game
    Patch_Byte_Range(0x0057FE2A, 0x90, 10); // NewMenuClass::Process_Game_Select
    Patch_Byte_Range(0x00580377, 0x90, 10); // NewMenuClass::Process_Game_Select
#endif

    /**
     *  Various patches to intercept the games object tracking and heap processing.
     */
    Patch_Call(0x0053DF7A, &_Free_Heaps_Intercept); // MapSeedClass::Init_Random
    Patch_Call(0x005DC590, &_Free_Heaps_Intercept); // Clear_Scenario
    Patch_Call(0x00601BA2, &_Free_Heaps_Intercept); // Game_Shutdown

    Patch_Call(0x0040DBB3, &_Detach_This_From_All_Intercept); // AircraftClass::~AircraftClass
    Patch_Call(0x0040F123, &_Detach_This_From_All_Intercept); // AircraftClass_Fall_To_Death
    Patch_Call(0x0040FCD3, &_Detach_This_From_All_Intercept); // AircraftTypeClass::~AircraftTypeClass
    Patch_Call(0x00410223, &_Detach_This_From_All_Intercept); // AircraftTypeClass::~AircraftTypeClass
    Patch_Call(0x004142C6, &_Detach_This_From_All_Intercept); // AnimClass::~AnimClass
    Patch_Call(0x00426662, &_Detach_This_From_All_Intercept); // BuildingClass::~BuildingClass
    Patch_Call(0x0043F94D, &_Detach_This_From_All_Intercept); // BuildingTypeClass::~BuildingTypeClass
    Patch_Call(0x0044407D, &_Detach_This_From_All_Intercept); // BuildingTypeClass::~BuildingTypeClass
    Patch_Call(0x004445F3, &_Detach_This_From_All_Intercept); // BulletClass::~BulletClass
    Patch_Call(0x004474D3, &_Detach_This_From_All_Intercept); // BulletClass::~BulletClass
    Patch_Call(0x00447DC3, &_Detach_This_From_All_Intercept); // BulletTypeClass::~BulletTypeClass
    Patch_Call(0x00448723, &_Detach_This_From_All_Intercept); // BulletTypeClass::~BulletTypeClass
    Patch_Call(0x00448AE3, &_Detach_This_From_All_Intercept); // CampaignClass::~CampaignClass
    Patch_Call(0x00448EF3, &_Detach_This_From_All_Intercept); // CampaignClass::~CampaignClass
    Patch_Call(0x00456A26, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456A58, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456A7F, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456AAB, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456AD2, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x004571F9, &_Detach_This_From_All_Intercept); // CellClass::Reduce_Wall
    Patch_Call(0x004927D3, &_Detach_This_From_All_Intercept); // EMPulseClass::~EMPulseClass
    Patch_Call(0x004931E3, &_Detach_This_From_All_Intercept); // EMPulseClass::~EMPulseClass
    Patch_Call(0x00496DB3, &_Detach_This_From_All_Intercept); // FactoryClass::~FactoryClass
    Patch_Call(0x00497AA3, &_Detach_This_From_All_Intercept); // FactoryClass::~FactoryClass
    Patch_Call(0x004BB6DB, &_Detach_This_From_All_Intercept); // HouseClass::~HouseClass
    Patch_Call(0x004CDE93, &_Detach_This_From_All_Intercept); // HouseTypeClass::~HouseTypeClass
    Patch_Call(0x004CE603, &_Detach_This_From_All_Intercept); // HouseTypeClass::~HouseTypeClass
    Patch_Call(0x004D22DC, &_Detach_This_From_All_Intercept); // InfantryClass::~InfantryClass
    Patch_Call(0x004DA3B4, &_Detach_This_From_All_Intercept); // InfantryTypeClass::~InfantryTypeClass
    Patch_Call(0x004DB133, &_Detach_This_From_All_Intercept); // InfantryTypeClass::~InfantryTypeClass
    Patch_Call(0x004F2173, &_Detach_This_From_All_Intercept); // IsometricTileClass::~IsometricTileClass
    Patch_Call(0x004F23E3, &_Detach_This_From_All_Intercept); // IsometricTileClass::~IsometricTileClass
    Patch_Call(0x004F3344, &_Detach_This_From_All_Intercept); // IsometricTileTypeClass::~IsometricTileTypeClass
    Patch_Call(0x005015E3, &_Detach_This_From_All_Intercept); // LightSourceClass::~LightSourceClass
    Patch_Call(0x00501DA3, &_Detach_This_From_All_Intercept); // LightSourceClass::~LightSourceClass
    Patch_Call(0x00585F9E, &_Detach_This_From_All_Intercept); // ObjectClass::Detach_All
    Patch_Call(0x00586DB5, &_Detach_This_From_All_Intercept); // ObjectClass::entry_E4
    Patch_Call(0x0058B563, &_Detach_This_From_All_Intercept); // OverlayClass::~OverlayClass
    Patch_Call(0x0058CB13, &_Detach_This_From_All_Intercept); // OverlayClass::~OverlayClass
    Patch_Call(0x0058D196, &_Detach_This_From_All_Intercept); // OverlayTypeClass::~OverlayTypeClass
    Patch_Call(0x0058DC86, &_Detach_This_From_All_Intercept); // OverlayTypeClass::~OverlayTypeClass
    Patch_Call(0x005A32FA, &_Detach_This_From_All_Intercept); // ParticleClass::~ParticleClass
    Patch_Call(0x005A503A, &_Detach_This_From_All_Intercept); // ParticleClass::~ParticleClass
    Patch_Call(0x005A56D4, &_Detach_This_From_All_Intercept); // ParticleSystemClass::~ParticleSystemClass
    Patch_Call(0x005AE573, &_Detach_This_From_All_Intercept); // ParticleSystemTypeClass::~ParticleSystemTypeClass
    Patch_Call(0x005AEC63, &_Detach_This_From_All_Intercept); // ParticleSystemTypeClass::~ParticleSystemTypeClass
    Patch_Call(0x005AF153, &_Detach_This_From_All_Intercept); // ParticleTypeClass::~ParticleTypeClass
    Patch_Call(0x005AFC33, &_Detach_This_From_All_Intercept); // ParticleTypeClass::~ParticleTypeClass
    Patch_Call(0x005E78C3, &_Detach_This_From_All_Intercept); // ScriptClass::~ScriptClass
    Patch_Call(0x005E7B83, &_Detach_This_From_All_Intercept); // ScriptTypeClass::~ScriptTypeClass
    Patch_Call(0x005E81E3, &_Detach_This_From_All_Intercept); // ScriptClass::~ScriptClass
    Patch_Call(0x005E8293, &_Detach_This_From_All_Intercept); // ScriptTypeClass::~ScriptTypeClass
    Patch_Call(0x005F1AE3, &_Detach_This_From_All_Intercept); // SideClass::~SideClass
    Patch_Call(0x005F1D93, &_Detach_This_From_All_Intercept); // SideClass::~SideClass
    Patch_Call(0x005FAAD3, &_Detach_This_From_All_Intercept); // SmudgeClass::~SmudgeClass
    Patch_Call(0x005FAF03, &_Detach_This_From_All_Intercept); // SmudgeClass::~SmudgeClass
    Patch_Call(0x005FB313, &_Detach_This_From_All_Intercept); // SmudgeTypeClass::~SmudgeTypeClass
    Patch_Call(0x005FC023, &_Detach_This_From_All_Intercept); // SmudgeTypeClass::~SmudgeTypeClass
    Patch_Call(0x00618D03, &_Detach_This_From_All_Intercept); // TActionClass::~TActionClass
    Patch_Call(0x0061DAD3, &_Detach_This_From_All_Intercept); // TActionClass::~TActionClass
    Patch_Call(0x0061E4B6, &_Detach_This_From_All_Intercept); // TagClass::~TagClass
    Patch_Call(0x0061E73B, &_Detach_This_From_All_Intercept); // TagClass::~TagClass
    Patch_Call(0x0061E9AA, &_Detach_This_From_All_Intercept); // TagClass::Spring
    Patch_Call(0x0061F164, &_Detach_This_From_All_Intercept); // TagTypeClass::~TagTypeClass
    Patch_Call(0x00621503, &_Detach_This_From_All_Intercept); // TaskForceClass::~TaskForceClass
    Patch_Call(0x00621E43, &_Detach_This_From_All_Intercept); // TaskForceClass::~TaskForceClass
    Patch_Call(0x006224E3, &_Detach_This_From_All_Intercept); // TeamClass::~TeamClass
    Patch_Call(0x00627EF3, &_Detach_This_From_All_Intercept); // TeamTypeClass::~TeamTypeClass
    Patch_Call(0x00629293, &_Detach_This_From_All_Intercept); // TeamTypeClass::~TeamTypeClass
    Patch_Call(0x0063F188, &_Detach_This_From_All_Intercept); // TerrainClass::~TerrainClass
    Patch_Call(0x00640C38, &_Detach_This_From_All_Intercept); // TerrainClass::~TerrainClass
    Patch_Call(0x00641653, &_Detach_This_From_All_Intercept); // TerrainTypeClass::~TerrainTypeClass
    Patch_Call(0x00641D83, &_Detach_This_From_All_Intercept); // TerrainTypeClass::~TerrainTypeClass
    Patch_Call(0x00642223, &_Detach_This_From_All_Intercept); // TEventClass::~TEventClass
    Patch_Call(0x00642F23, &_Detach_This_From_All_Intercept); // TEventClass::~TEventClass
    Patch_Call(0x00644A45, &_Detach_This_From_All_Intercept); // TiberiumClass::~TiberiumClass
    Patch_Call(0x006491A3, &_Detach_This_From_All_Intercept); // TriggerClass::~TriggerClass
    Patch_Call(0x00649943, &_Detach_This_From_All_Intercept); // TriggerClass::~TriggerClass
    Patch_Call(0x00649E03, &_Detach_This_From_All_Intercept); // TriggerTypeClass::~TriggerTypeClass
    Patch_Call(0x0064AFD3, &_Detach_This_From_All_Intercept); // TubeClass::~TubeClass
    Patch_Call(0x0064B603, &_Detach_This_From_All_Intercept); // TubeClass::~TubeClass
    Patch_Call(0x0064D8A9, &_Detach_This_From_All_Intercept); // UnitClass::~UnitClass
    Patch_Call(0x0065BAD3, &_Detach_This_From_All_Intercept); // UnitTypeClass::~UnitTypeClass
    Patch_Call(0x0065C793, &_Detach_This_From_All_Intercept); // UnitTypeClass::~UnitTypeClass
    Patch_Call(0x0065DF23, &_Detach_This_From_All_Intercept); // VoxelAnimClass::~VoxelAnimClass
    Patch_Call(0x0065F5A3, &_Detach_This_From_All_Intercept); // VoxelAnimTypeClass::~VoxelAnimTypeClass
    Patch_Call(0x00660093, &_Detach_This_From_All_Intercept); // VoxelAnimTypeClass::~VoxelAnimTypeClass
    Patch_Call(0x00661227, &_Detach_This_From_All_Intercept); // VeinholeMonsterClass::~VeinholeMonsterClass
    Patch_Call(0x00661C00, &_Detach_This_From_All_Intercept); // VeinholeMonsterClass::Take_Damage
    Patch_Call(0x0066EF73, &_Detach_This_From_All_Intercept); // WarheadTypeClass::~WarheadTypeClass
    Patch_Call(0x0066FA93, &_Detach_This_From_All_Intercept); // WarheadTypeClass::~WarheadTypeClass
    Patch_Call(0x006702D4, &_Detach_This_From_All_Intercept); // WaveClass::~WaveClass
    Patch_Call(0x00672E73, &_Detach_This_From_All_Intercept); // WaveClass::~WaveClass
    Patch_Call(0x00673563, &_Detach_This_From_All_Intercept); // WaypointPathClass::~WaypointPathClass
    Patch_Call(0x00673AA3, &_Detach_This_From_All_Intercept); // WaypointPathClass::~WaypointPathClass
    Patch_Call(0x00680C54, &_Detach_This_From_All_Intercept); // WeaponTypeClass::~WeaponTypeClass
    Patch_Call(0x006818F4, &_Detach_This_From_All_Intercept); // WeaponTypeClass::~WeaponTypeClass

    Patch_Call(0x005B1363, &_Print_CRCs_Intercept);
    Patch_Call(0x005B5340, &_Print_CRCs_Intercept);

    //Patch_Call(0x005D6BEC, &_On_Load_Clear_Scenario_Intercept); // Load_All
}
