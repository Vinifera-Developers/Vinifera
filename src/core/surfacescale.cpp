/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  *
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "surfacescale.h"

#include "Surface.h"
#include "vinifera_util.h"
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
static bool Scale_Surface_ImageResampler(Surface *src, Surface *dst, VN_IMAGE_KERNEL_TYPE kernel)
{
    if (!src || !dst) {
        return false;
    }

    int src_width = src->Get_Width();
    int src_height = src->Get_Height();
    int src_bpp = src->Bytes_Per_Pixel();

    int dst_width = dst->Get_Width();
    int dst_height = dst->Get_Height();
    int dst_bpp = dst->Bytes_Per_Pixel();

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
bool Scale_Surface_Nearest(Surface *src, Surface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_NEAREST);
}

bool Scale_Surface_Bilinear(Surface *src, Surface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_BILINEAR);
}

bool Scale_Surface_Bicubic(Surface *src, Surface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_BICUBIC);
}

bool Scale_Surface_Cardinal(Surface *src, Surface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_CARDINAL);
}

bool Scale_Surface_Lanczos(Surface *src, Surface *dst)
{
    return Scale_Surface_ImageResampler(src, dst, VN_IMAGE_KERNEL_LANCZOS3);
}
