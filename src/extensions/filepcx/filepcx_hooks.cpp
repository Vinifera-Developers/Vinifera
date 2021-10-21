/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FILEPCX_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for Read_PCX_File.
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
#include "filepcx_hooks.h"
#include "filepcx.h"
#include "filepng.h"
#include "ccfile.h"
#include "stristr.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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
