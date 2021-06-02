/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FILEPNG.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Function for writing PNG files from a graphic surface.
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
#include "filepng.h"
#include "ccfile.h"
#include "surface.h"
#include "debughandler.h"
#include <lodepng.h>


/** 
 *  Writes the contents of a graphic surface as PNG to a file instance.
 * 
 *  @author: CCHyper
 */
bool Write_PNG_File(FileClass *name, Surface &pic, const PaletteClass *palette, bool greyscale)
{
    int pic_width = pic.Get_Width();
    int pic_height = pic.Get_Height();

    /**
     *  Copy graphic data from the surface to the buffer.
     */
    unsigned short *buffer = (unsigned short *)std::malloc(pic_height * (pic_width * sizeof(unsigned short)));
    if (!buffer) {
        return false;
    }

    std::memcpy(buffer, (unsigned short *)pic.Lock(), pic_height * (pic_width * sizeof(unsigned short)));
    pic.Unlock();

    unsigned short *bufferptr = buffer;

    /**
     *  Convert the pixel data from 16bit to 24bit.
     */
    unsigned char *image = (unsigned char *)std::malloc(pic_height * pic_width * 3);

    static struct rgb {
        unsigned char r, g, b;
    };
    rgb *imageptr = (rgb *)image;

    for (int i = 0; i < (pic_width * pic_height); ++i) {
        unsigned short value = *(bufferptr++);
        unsigned char r = (value & 0xF800) >> 11; // Extract the 5 R bits
        unsigned char g = (value & 0x07E0) >> 5;  // Extract the 6 G bits
        unsigned char b = (value & 0x001F);       // Extract the 5 B bits
        imageptr[i].r = (r * 255) / 31;
        imageptr[i].g = (g * 255) / 63;
        imageptr[i].b = (b * 255) / 31;
    }

    /**
     *  Encode the graphic data to png data to be written to the file.
     */
    unsigned char *png = nullptr;
    size_t pngsize = 0;
    int error = lodepng_encode_memory(&png, &pngsize, (unsigned char *)image, pic_width, pic_height, LCT_RGB, 8);

    /**
     *  Now write data to the file.
     */
    name->Open(FILE_ACCESS_WRITE);
    name->Write(png, pngsize);
    name->Close();

    /**
     *  Cleanup buffers.
     */
    std::free(png);
    std::free(image);
    std::free(buffer);

    /**
     *  Handle any errors.
     */
    if (error) {
        DEBUG_ERROR("lodepng_encode error %u: %s\n", error, lodepng_error_text(error));
        return false;
    }

    return true;
}
