/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SAVELOAD_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for implementing save/load support for
 *                 the extended classes.
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
#include "ext_hooks.h"
#include "iomap.h"
#include "saveload.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "debughandler.h"
#include "fatal.h"

#include "hooker.h"
#include "hooker_macros.h"


struct IStream;


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
    Vinifera_Put_All(pStm);

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
    Vinifera_Load_All(pStm);

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
}


/**
 *  When writing save game info, write our build version.
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
 *  Do not allow save games below our current build version.
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

    DEBUG_ERROR("Swizzle Manager - Failed to re-map pointer! (old_ptr = 0x%08X)!\n", old_ptr);

    ShowCursor(TRUE);

    static char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "SwizzleManagerClass::Process_Tables()\n\nFailed to re-map pointer! (old_ptr = 0x%08X)!", old_ptr);
    MessageBoxA(MainWindow, buffer, "Vinifera", MB_OK|MB_ICONEXCLAMATION);

    Vinifera_Generate_Mini_Dump();
    Fatal("Swizzle Manager - Failed to re-map pointer! (old_ptr = 0x%08X)!\n", old_ptr);

    /**
     *  We won't ever get here, but its here just for clean analysis.
     */
    JMP(0x0060DC15);
}


#ifndef NDEBUG
#include "swizzle.h"
class FakeSwizzleManagerClass final : public SwizzleManagerClass
{
    public:
        COM_DECLSPEC_NOTHROW LONG STDMETHODVCALLTYPE _Swizzle(void **pointer)
        {
            DEBUG_INFO("Swizzle - pointer 0x%08X, 0x%08X\n", pointer, *pointer);
            return SwizzleManagerClass::Swizzle(pointer);
        }

        COM_DECLSPEC_NOTHROW LONG STDMETHODVCALLTYPE _Fetch_Swizzle_ID(void *pointer, LONG *id)
        {
            DEBUG_INFO("Fetch_Swizzle_ID - pointer 0x%08X id 0x%08X\n", pointer, id);
            return SwizzleManagerClass::Fetch_Swizzle_ID(pointer, id);
        }

        COM_DECLSPEC_NOTHROW LONG STDMETHODVCALLTYPE _Here_I_Am(LONG id, void *pointer)
        {
            DEBUG_INFO("Here_I_Am - id 0x%08X pointer 0x%08X\n", id, pointer);
            return SwizzleManagerClass::Here_I_Am(id, pointer);
        }
};
#endif


void SaveLoad_Hooks()
{
    /**
     *  Hook the new save and load system in.
     */
    Patch_Jump(0x005D68F7, &_Put_All_Vinifera_Data);
    Patch_Jump(0x005D78ED, &_Load_All_Vinifera_Data);
    Patch_Jump(0x005D505E, &_Save_Game_Put_Game_Version);

    /**
     *  Change SUN.EXE to our DLL name.
     */
    Patch_Jump(0x005D50DD, &_Save_Game_Change_Module_Filename);

    /**
     *  Handle save files in the dialogs.
     */
    Patch_Jump(0x00505A9E, &_LoadOptionsClass_Read_File_Check_Game_Version);
    Patch_Jump(0x00505ABB, &_LoadOptionsClass_Read_File_Remove_Older_Prefixing);

    Patch_Jump(0x0060DBFF, &_SwizzleManagerClass_Process_Tables_Remap_Failed_Error);

#ifndef RELEASE
    if (!Vinifera_DeveloperMode) {
        /**
         *  Disable loading and saving in non-release builds.
         */
        Patch_Jump(0x004B6D96, &_SaveLoad_Disable_Buttons);
        Patch_Jump(0x0057FF8B, &_NewMenuClass_Process_Disable_Load_Button_Firestorm);
        Patch_Jump(0x0058004D, &_NewMenuClass_Process_Disable_Load_Button_TiberianSun);
    }
#endif

#ifndef NDEBUG
    //Patch_Call(0x, &FakeSwizzleManagerClass::_Swizzle);
    //Patch_Call(0x004CE3C3, &FakeSwizzleManagerClass::_Fetch_Swizzle_ID);
    //Change_Virtual_Address(0x006D7500, Get_Func_Address(&FakeSwizzleManagerClass::_Fetch_Swizzle_ID));
    //Patch_Call(0x00405D47, &FakeSwizzleManagerClass::_Here_I_Am);
    //Patch_Call(0x004CE387, &FakeSwizzleManagerClass::_Here_I_Am);
    //Patch_Call(0x00506627, &FakeSwizzleManagerClass::_Here_I_Am);
    //Patch_Call(0x0061F42E, &FakeSwizzleManagerClass::_Here_I_Am);
    //Patch_Call(0x0062190D, &FakeSwizzleManagerClass::_Here_I_Am);
    //Patch_Call(0x0066300B, &FakeSwizzleManagerClass::_Here_I_Am);
    //Change_Virtual_Address(0x006D7504, Get_Func_Address(&FakeSwizzleManagerClass::_Here_I_Am));
#endif
}
