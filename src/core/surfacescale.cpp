/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SURFACESCALE.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         
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
#include "surfacescale.h"
#include "vinifera_util.h"
#include "xsurface.h"
#include "vnImagine.h"


/**
 *  The scaling is performed with the use of the Image-Resampler library;
 *  https://github.com/ramenhut/image-resampler
 */


/** 
 *  Scales an input surface to fit the destination surface using the Image-Resampler library.
 * 
 *  @author: CCHyper
 */
static bool Scale_Surface_ImageResampler(XSurface *src, XSurface *dst, VN_IMAGE_KERNEL_TYPE kernel)
{
    if (!src || !dst) {
        return false;
    }

    int src_width = src->Get_Width();
    int src_height = src->Get_Height();
    int src_bpp = src->Get_Bytes_Per_Pixel();

    int dst_width = dst->Get_Width();
    int dst_height = dst->Get_Height();
    int dst_bpp = dst->Get_Bytes_Per_Pixel();

    if (src_bpp != 2 || dst_bpp != 2) {
        return false;
    }

    unsigned char *src_buff = (unsigned char *)src->Lock();
    unsigned char *dst_buff = (unsigned char *)dst->Lock();

    if (!src_buff || !dst_buff) {
        src->Unlock();
        dst->Unlock();
        return false;
    }

    CVImage source_image;
    CVImage resampled_image;

    VN_IMAGE_FORMAT format = VN_IMAGE_FORMAT_R5G6B5; // 16-bit RGB

    /**
     *  Create a new image buffer, this will be where the source image data will be stored.
     */
    if (VN_FAILED(vnCreateImage(format, src_width, src_height, &source_image))) {
        return false;
    }

    /**
     *  Copy the source image data into the new image buffer.
     */
    std::memcpy(source_image.QueryData(), src_buff, src_width*src_height*src_bpp);
    
    /**
     *  Resize the source image to fit the destination surface.
     */
    if (VN_FAILED(vnResizeImage(source_image, kernel, dst_width, dst_height, 0, &resampled_image))) {
        return false;
    }
    
    /**
     *  Now copy the resized image data back into the destination surface.
     */
    std::memcpy(dst_buff, resampled_image.QueryData(), dst_width*dst_height*dst_bpp);

    src->Unlock();
    dst->Unlock();

    return true;
}


/** 
 *  Scales an input surface to fit the destination surface using various algorithms.
 * 
 *  @author: CCHyper
 */
bool Scale_Surface_Nearest(XSurface *src, XSurface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_NEAREST);
}

bool Scale_Surface_Bilinear(XSurface *src, XSurface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_BILINEAR);
}

bool Scale_Surface_Bicubic(XSurface *src, XSurface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_BICUBIC);
}

bool Scale_Surface_Cardinal(XSurface *src, XSurface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_CARDINAL);
}

bool Scale_Surface_Lanczos(XSurface *src, XSurface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_LANCZOS3);
}
