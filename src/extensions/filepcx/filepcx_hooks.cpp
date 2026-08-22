/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for Read_PCX_File.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "filepcx_hooks.h"

#include "asserthandler.h"
#include "ccfile.h"
#include "filepcx.h"
#include "filepng.h"
#include "hooker.h"


/**
 *  #issue-695
 *
 *  Add support for PNG files as an alternative to PCX images.
 *
 *  This intercept allows us to check the filename of the input file and see
 *  if a PNG for alternative exists for it, if so, load that instead of the
 *  PCX image file.
 *
 *  @author: CCHyper
 */
static BSurface *Read_PCX_File_Intercept(FileClass *file, unsigned char *palette, void *buff, long size)
{
    char fnamebuffer[32];
    std::strncpy(fnamebuffer, file->File_Name(), sizeof(fnamebuffer));

    /**
     *  Find the location of the file extension separator.
     */
    char *file_name = std::strchr((char *)fnamebuffer, '.');

    /**
     *  Insert a null-char where the "." was. This will give us the actual
     *  file name without the extension, allowing us to rebuild it.
     */
    *file_name = '\0';

    const char *upper_filename = strupr((char *)fnamebuffer);

    char png_buffer[32-4];
    std::snprintf(png_buffer, sizeof(png_buffer), "%s.PNG", upper_filename);

    /**
     *  Search for the PNG file, and load it if found.
     */
    CCFileClass pngfile(png_buffer);
    if (pngfile.Is_Available()) {

        BSurface *image = Read_PNG_File(&pngfile, palette, buff, size);
        if (image) {
            return image;
        }
    }

    /**
     *  Fallback to the PCX file.
     */
    return (BSurface *)Read_PCX_File(file, palette, buff, size);
}


/**
 *  Main function for patching the hooks.
 */
void FilePCXExtension_Hooks()
{
    Patch_Call(0x00553E02, &Read_PCX_File_Intercept);
    Patch_Call(0x0056C0D2, &Read_PCX_File_Intercept);
    Patch_Call(0x0056D987, &Read_PCX_File_Intercept);
    Patch_Call(0x0056DB53, &Read_PCX_File_Intercept);
    Patch_Call(0x005ACD14, &Read_PCX_File_Intercept);
    Patch_Call(0x005FDD14, &Read_PCX_File_Intercept);
    Patch_Call(0x0067CC27, &Read_PCX_File_Intercept);
    Patch_Call(0x0067CC52, &Read_PCX_File_Intercept);
    Patch_Call(0x00686363, &Read_PCX_File_Intercept);
}
