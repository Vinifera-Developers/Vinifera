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
#include "dsurface.h"
#include "wwmouse.h"
#include "blowfish.h"
#include "blowstraw.h"
#include "blowpipe.h"
#include "iomap.h"
#include "theme.h"
#include "extension_saveload.h"
#include "loadoptions.h"
#include "language.h"
#include "vinifera_gitinfo.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "debughandler.h"
#include "asserthandler.h"


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

    /**
     *  Stolen bytes/code.
     */
    Map.Set_Default_Mouse(MOUSE_NORMAL);

    JMP(0x004E1F30);
}


/**
 *  When writing save game info, write the base level Vinifera version. This patch
 *  will block vanilla Tiberian Sun from loading any Vinifera save files.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Save_Game_Put_Game_Version)
{
    static int version;
    version = ViniferaSaveGameVersion;

    /**
     *  If we are in developer mode, offset the build number as these save
     *  files should not appear in normal game modes.
     * 
     *  For debug builds, we force an offset so they don't appear in any
     *  other builds or releases.
     */
#ifndef NDEBUG
    version *= 3;
#else
    if (Vinifera_DeveloperMode) {
        version *= 2;
    }
#endif

    _asm { mov edx, version };

    JMP(0x005D5064);
}


/**
 *  x
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Load_Game_Check_Return_Value)
{
    GET_REGISTER_STATIC(const char *, filename, esi);

    _asm { mov ecx, [esp+0x20] }
    _asm { xor dl, dl }
    _asm { mov eax, 0x005D6BE0 }
    _asm { call eax } // Load_All

    _asm { test al, al }
    _asm { jz failure }

    DEBUG_INFO("Loading of save game \"%s\" complete.\n", filename);
    JMP(0x005D6B1C);

failure:
    DEBUG_ERROR("Error loading save game \"%s\"!\n", filename);
    JMP(0x005D6A65);
}


/**
 *  Do not allow save games below our the base level Vinifera version. This patch
 *  will remove any support for save games made with vanilla Tiberian Sun 2.03!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_LoadOptionsClass_Read_File_Check_Game_Version)
{
    GET_REGISTER_STATIC(int, version, eax);
    static int ver;

    /**
     *  If the version in the save file does not match our build
     *  version exactly, then don't add this file to the listing.
     */
    ver = ViniferaSaveGameVersion;
#ifndef NDEBUG
    ver *= 3;
#else
    if (Vinifera_DeveloperMode) {
        ver *= 2;
    }
#endif
    if (version != ver) {
        JMP(0x00505AAD);
    }

    JMP(0x00505ABB);
}


/**
 *  Change the saved module filename to the DLL name. 
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Save_Game_Change_Module_Filename)
{
    static const char *DLL_NAME = VINIFERA_DLL;
    _asm { push DLL_NAME }

    JMP(0x005D50E2);
}

       
/**
 *  Removes the code which prefixed older save files with "*".
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_LoadOptionsClass_Read_File_Remove_Older_Prefixing)
{
    JMP(0x00505AE9);
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
 *  Patch in the Vinifera data to be saved in the stream.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Put_All_Vinifera_Data)
{
    GET_REGISTER_STATIC(IStream *, pStm, esi);

    /**
     *  Call to the Vinifera data stream saver.
     */
    if (!Vinifera_Put_All(pStm)) {
        goto failed;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    _asm { mov al, 1 }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x8 }
    _asm { ret }

failed:
    _asm { xor al, al }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x8 }
    _asm { ret }
}


/**
 *  Patch in the Vinifera data to be loaded in the stream.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Load_All_Vinifera_Data)
{
    GET_REGISTER_STATIC(IStream *, pStm, esi);

    /**
     *  Call to the Vinifera data stream loader.
     */
    if (!Vinifera_Load_All(pStm)) {
        goto failed;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:

    Map.Flag_To_Redraw(2);

    _asm { mov al, 1 }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0xB0 }
    _asm { ret }

failed:
    _asm { xor al, al }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0xB0 }
    _asm { ret }
}


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
#if !defined(RELEASE) && defined(NDEBUG)
            /**
             *  We disable loading in non-release builds or if extensions are disabled.
             */
            if (!Vinifera_ClassExtensionsDisabled) {
                Vinifera_Do_WWMessageBox("Saving and Loading is disabled for non-release builds.", Text_String(TXT_OK));

            } else {
#endif

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

#if !defined(RELEASE) && defined(NDEBUG)
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
        for (int i = 0; i < ARRAY_SIZE(_buf); ++i) {
            _buf[i] = _alphanum[std::rand() % (ARRAY_SIZE(_alphanum)-1)];
        }
        _done = true;
    }

    std::strncpy(buffer, _buf, sizeof(_buf));
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
     *  Write Vinifera save files with the new base version number.
     */
    Patch_Jump(0x005D505E, &_Save_Game_Put_Game_Version);

    /**
     *  Check the return value of Load_Game to ensure no false game starts.
     */
    Patch_Jump(0x005D6B11, &_Load_Game_Check_Return_Value);

    /**
     *  Change SUN.EXE to our DLL name.
     */
    Patch_Jump(0x005D50DD, &_Save_Game_Change_Module_Filename);

    /**
     *  Handle save files in the dialogs.
     */
    Patch_Jump(0x00505A9E, &_LoadOptionsClass_Read_File_Check_Game_Version);
    Patch_Jump(0x00505ABB, &_LoadOptionsClass_Read_File_Remove_Older_Prefixing);

    /**
     *  Fire an assert on save/load fail, rather than hard crash.
     */
    Patch_Jump(0x0060DBFF, &_SwizzleManagerClass_Process_Tables_Remap_Failed_Error);

    /**
     *  Enable and hook the new save and load system only if extensions are disabled.
     */
    if (Vinifera_ClassExtensionsDisabled) {
        Patch_Jump(0x005D68F7, &_Put_All_Vinifera_Data);
        Patch_Jump(0x005D78ED, &_Load_All_Vinifera_Data);

        Patch_Jump(0x004B6D96, &_SaveLoad_Disable_Buttons);
        Patch_Jump(0x0057FF8B, &_NewMenuClass_Process_Disable_Load_Button_Firestorm);
        Patch_Jump(0x0058004D, &_NewMenuClass_Process_Disable_Load_Button_TiberianSun);
    }

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
}
