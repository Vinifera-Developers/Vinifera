/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended VQAClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "vqaext_hooks.h"

#include "hooker.h"
#include "syringe.h"
#include "vqa.h"


/**
 *  #issue-87
 *
 *  Patch to use CCFileClass instead of MixFileClass when loading VQA files. This
 *  allows VQA files to be loaded from the games root directory.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0066C0FD, _VQA_Mix_File_Handler_Use_CCFileClass_Patch, 0)
{
    GET(VQAClass *, this_ptr, ESI);
    GET_STACK(char *, filename, 0xC);

    /**
     *  Original code used MixFileClass::Offset to find the file, this limited
     *  the VQA file streamer to only be able to load files from mix files.
     */
#if 0
    MFCC *mixfile;
    long offset;
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

    this_ptr->field_64 = this_ptr->File.Open(FILE_ACCESS_READ);

    int error = !this_ptr->field_64;

exit_label:
    R->EAX(error != 0);
    return 0x0066C175;
}


/**
 *  Main function for patching the hooks.
 */
void VQAExtension_Hooks()
{

}
