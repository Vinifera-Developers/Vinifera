
//
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnImageBlock.h
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  Additional Information:
//
//   For more information, visit http://www.bertolami.com.
//
#ifndef __VN_IMAGE_BLOCK_H__
#define __VN_IMAGE_BLOCK_H__

#include "../Base/vnBase.h"
#include "../Base/vnMath.h"
#include "../Base/vnImage.h"

//
// Image Blocks
//
// This image library supports a broad set of image formats that vary in terms 
// of their channel counts, pixel bit rates, precision, endian-ness, signed-ness,
// and color space.
//
// As a result, we rely upon the pixel block structure to perform simple conversions
// between our native image formats and a canonical 64 bit pixel format (which we 
// universally store in pixel blocks).
//
// This system allows the rest of the image library to support most image formats
// in a very simple manner.
//

typedef struct VN_PIXEL_BLOCK
{
    //
    // This structure deliberately sheds the notion of the image format.
    // Pixel blocks represent a common normalized pixel format that is 
    // restricted to 32 bit channel formats.
    //

    union
    {
        FLOAT64 fChannelData[4];            
        INT64   iChannelData[4];       
        UINT8   uiChannelBytes[32];
    };

    VN_IMAGE_PRECISION  uiPrecision;
    UINT8               uiChannelCount;

} VN_PIXEL_BLOCK;

//
// (!) Notes on precision
//
//     Although we internally store our pixel data as 256 bit signed precision,
//     we only use the low 32 bits of each pixel block channel when packing and 
//     unpacking the data. 
//
//     Thus, it is the responsibility of operator designers to ensure that pixel
//     block data is within this valid (signed or unsigned) range prior to packing
//     it into an image structure. 
//
//     We internally manage pixels at 256 bit precision to enable large summations 
//     within kernels without overflowing our integer storage, and we store pixels 
//     internally in a signed format so that kernels may use them to directly perform 
//     signed arithmetic. 
//
//        * A slight loss of precision will occur when packing/unpacking 32 bpp
//		    unsigned images. This trade-off was made in consideration of performance
//			issues.
//
//     When converting between precision formats, a couple of conversion rules may 
//     apply:
//
//     o: When converting fixed->fixed of different precisions, the values are scaled
//        to ensure that the conversion maintains the integrity of the values as a 
//        percentage of the total viable range.
//
//     o: When converting float->fixed, the source values are saturated to [-1,1].
//
//     o: When converting fixed->float, the entire fixed integer range is packed within 
//        the float range of [-1, 1]. 
//
//     All integer values are represented internally as signed values. 
//
// Note also that an implicit precision conversion may be performed when packing a
// block block into a raw buffer if the destination buffer precision does not match the
// source block precision.
//

VN_STATUS vnConvertToBlock( UINT8 * pRawPixel, VN_IMAGE_FORMAT internalFormat, VN_PIXEL_BLOCK * pOutBlock );

VN_STATUS vnConvertFromBlock( CONST VN_PIXEL_BLOCK & pSourceBlock, VN_IMAGE_FORMAT format, UINT8 * pRawPixel );

//
// vnConvertBlock
//
// Performs a block conversion based on a destination format.
//

VN_STATUS vnConvertBlock( CONST VN_PIXEL_BLOCK & pSrc, VN_IMAGE_FORMAT destFormat, VN_PIXEL_BLOCK * pDest );

#endif // __VN_IMAGE_BLOCK_H__
