/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VQAEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended VQAClass.
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
#include "vqaext_hooks.h"
#include "vqa.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-87
 * 
 *  Patch to use CCFileClass instead of MixFileClass when loading VQA files. This
 *  allows VQA files to be loaded from the games root directory.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_VQA_Mix_File_Handler_Use_CCFileClass_Patch)
{
    GET_REGISTER_STATIC(VQAClass *, this_ptr, esi);
    GET_STACK_STATIC(char *, filename, esp, 0xC);

    static int error;

    /**
     *  Original code used MixFileClass::Offset to find the file, this limited
     *  the VQA file streamer to only be able to load files from mix files.
     */
#if 0
    static MFCC *mixfile;
    static long offset;
    if (!MFCC::Offset(this_ptr->Filename, nullptr, &mixfile, &offset)) {
        error = 1;
    } else {
        this_ptr->field_64 = this_ptr->File.Open(mixfile->Filename, FILE_ACCESS_READ);
        error = (this_ptr->File.Seek(offset, FILE_SEEK_CURRENT) == 0);
    }
#endif

    /**
     *  ...Now we use CCFileClass, which does use MixFileClass to search for
     *  the file, but also scans for the file locally first.
     */
    this_ptr->File.Set_Name(filename);

    // #REMOVED: This fails as CDFileClass does not implement Is_Available to search the paths.
    //if (this_ptr->File.Is_Available()) {
    //    error = 1;
    //    goto exit_label;
    //}

    this_ptr->field_64 = this_ptr->File.Open(FILE_ACCESS_READ);

    error = !this_ptr->field_64;

exit_label:
    _asm { xor eax, eax }
    _asm { cmp error, 0 }
    _asm { setnz al }
    _asm { pop esi }
    _asm { ret 0x0C }
}


/**
 *  Main function for patching the hooks.
 */
void VQAExtension_Hooks()
{
    Patch_Jump(0x0066C0FD, _VQA_Mix_File_Handler_Use_CCFileClass_Patch);
}
