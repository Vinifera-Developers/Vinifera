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
    _asm { mov edx, ViniferaSaveGameVersion };

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

    /**
     *  If the version in the save file does not match our build
     *  version exactly, then don't add this file to the listing.
     * 
     *  For debug builds, we want to allow all save files for
     *  debugging purposes.
     */
#ifndef NDEBUG
    if (version != ViniferaSaveGameVersion) {
        JMP(0x00505AAD);
    }
#endif

    JMP(0x00505ABB);
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


void SaveLoad_Hooks()
{
    /**
     *  Uncomment this code when the extension classes are implemented!
     */
#if 0

    static const char *DLL_NAME = VINIFERA_DLL;

    /**
     *  Hook the new save and load system in.
     */
    Patch_Jump(0x005D68F7, &_Put_All_Vinifera_Data);
    Patch_Jump(0x005D78ED, &_Load_All_Vinifera_Data);
    Patch_Jump(0x005D505E, &_Save_Game_Put_Game_Version);

    /**
     *  Change SUN.EXE to our DLL name.
     */
    Change_Address(0x005D50DD+1, *DLL_NAME); // +1 to skip the "push" opcode.

    /**
     *  Handle save files in the dialogs.
     */
    Patch_Jump(0x00505A9E, &_LoadOptionsClass_Read_File_Check_Game_Version);
    Patch_Jump(0x00505ABB, &_LoadOptionsClass_Read_File_Remove_Older_Prefixing);

#ifndef RELEASE
    /**
     *  Disable loading and saving in non-release builds.
     */
    Patch_Jump(0x004B6D96, &_SaveLoad_Disable_Buttons);
    Patch_Jump(0x0057FF8B, &_NewMenuClass_Process_Disable_Load_Button_Firestorm);
    Patch_Jump(0x0058004D, &_NewMenuClass_Process_Disable_Load_Button_TiberianSun);
#endif

#endif

}
