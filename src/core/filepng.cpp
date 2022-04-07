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
#include "bsurface.h"
#include "dsurface.h"
#include "buff.h"
#include "stristr.h"
#include "debughandler.h"
#include "asserthandler.h"
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


/** 
 *  Read the contents of a PNG file into a graphic surface.
 * 
 *  @author: CCHyper
 */
BSurface *Read_PNG_File(FileClass *name, unsigned char *palette, void *buff, long size)
{
    ASSERT(name != nullptr);

    LodePNGState state;
    BSurface *pic = nullptr;

    unsigned char *png_image = nullptr;     // Output png image.
    unsigned int png_width;
    unsigned int png_height;

    unsigned char *png_buffer = nullptr;    // Raw png loaded from file.
    size_t png_buffersize;

    if (!stristr(name->File_Name(), ".png")) {
        DEBUG_ERROR("Read_PNG_File() - Invalid filename!\n");
        return false;
    }

    if (!name->Is_Available()) return nullptr;

    bool file_opened = false;
    if (!name->Is_Open()) {
        name->Open(FILE_ACCESS_READ);
        file_opened = true;
    }

    png_buffersize = name->Size();

    png_buffer = (unsigned char *)std::malloc(png_buffersize);
    if (!png_buffer) {
        DEBUG_ERROR("Read_PNG_File() - Failed to allocate PNG buffer!\n");
        return nullptr;
    }

    if (!name->Read(png_buffer, png_buffersize)) {
        DEBUG_ERROR("Read_PNG_File() - Failed to read PNG file!\n");

        //delete [] png_buffer;
        std::free(png_buffer);

        return nullptr;
    }

    lodepng_state_init(&state);

    state.info_raw.colortype = LCT_RGB;
    state.info_raw.bitdepth = 8;
    state.decoder.color_convert = false;

    /**
     *  Decode the PNG data.
     */
    unsigned error = lodepng_decode(&png_image, &png_width, &png_height, &state, png_buffer, png_buffersize);
    if (!png_image || error) {
        DEBUG_ERROR("Read_PNG_File() - Failed to decode PNG data!\n");
    
        lodepng_state_cleanup(&state);
    
        //delete [] png_buffer;
        std::free(png_buffer);
        std::free(png_image);
    
        return nullptr;
    }

    /**
     *  We only support standard 8bit PNG RGB, report error otherwise.
     */
    if (state.info_raw.bitdepth == 16
     || state.info_raw.colortype == LCT_GREY
     || state.info_raw.colortype == LCT_PALETTE
     || state.info_raw.colortype == LCT_GREY_ALPHA
     || state.info_raw.colortype == LCT_RGBA) {

        DEBUG_ERROR("Read_PNG_File() - Unsupported PNG format type!\n");

        lodepng_state_cleanup(&state);
    
        //delete [] png_buffer;
        std::free(png_buffer);
        std::free(png_image);
    
        return nullptr;
    }


#ifndef NDEBUG
    DEBUG_INFO("Read_PNG_File() - bitdepth: %d, colortype: %d.\n",
        state.info_raw.bitdepth, state.info_raw.colortype);
#endif

    if (buff) {
        Buffer b(buff, size);
        pic = new BSurface(png_width, png_height, 2, b);
    } else {
        pic = new BSurface(png_width, png_height, 2);
    }
    ASSERT(pic != nullptr);

    //size_t buffersize = lodepng_get_raw_size(png_width, png_height, &state.info_raw);
    //ASSERT(buffersize == (png_width * png_height));

    /**
     *  Copy the decoded PNG data into the image surface.
     */
    for (int y = 0; y < pic->Get_Height(); ++y) {
    
        unsigned short *buffptr = (unsigned short *)pic->Lock(0, y);
        for (int x = 0; x < pic->Get_Width(); ++x) {
    
            int r = *png_image++; // & 0xFF;
            int g = *png_image++; // & 0xFF;
            int b = *png_image++; // & 0xFF;
    
            *buffptr++ = DSurface::RGB_To_Pixel(r, g, b);
        }
    
        pic->Unlock();
    }

    std::free(png_buffer);

    lodepng_state_cleanup(&state);

    if (file_opened) {
        name->Close();
    }

    return pic;
}


/** 
 *  Read the contents of a PNG file into a graphic surface.
 * 
 *  @author: CCHyper
 */
BSurface *Read_PNG_File(FileClass *name, const Buffer &buff, PaletteClass *palette)
{
    return Read_PNG_File(name, (unsigned char *)palette, buff.Get_Buffer(), buff.Get_Size());
}
