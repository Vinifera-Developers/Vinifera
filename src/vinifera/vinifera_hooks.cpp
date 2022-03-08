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
     *  Stolen bytes/code.
     */
    Map.Set_Default_Mouse(MOUSE_NORMAL);

    JMP(0x004E1F30);
}


#ifndef NDEBUG
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
#endif


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
#endif

#ifndef NDEBUG
    /**
     *  This patch randomises the serial number for this client.
     */
    Patch_Jump(0x00576410, &Decrypt_Serial);
#endif

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
