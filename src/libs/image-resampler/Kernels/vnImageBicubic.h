
//
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnImageBicubic.h
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

#ifndef __VN_IMAGE_BICUBIC_H__
#define __VN_IMAGE_BICUBIC_H__

#include "../Base/vnBase.h"
#include "../Base/vnMath.h"
#include "../Base/vnImage.h"

//
// Cubic weighing function
//
// Source: Mitchell, Netravali, "Reconstruction Filters in Computer Graphics" 1988
//
//   Several of the popular cubic functions used for bi-directional image filtering 
//   can be generated as a simple weight function with two parameters. Thus, we use a 
//   weight function to generate the majority of our bicubic kernels.
//

inline FLOAT32 vnBicubicWeight( FLOAT32 fB, FLOAT32 fC, FLOAT32 fDistance )
{
    //
    // Our bicubic function is designed to provide feedback over a radius of 2.0 pixels.
    //

    FLOAT32 fRange  = fDistance;
    FLOAT32 fResult = 0.0;

    if ( fRange < 1.0 )
    {
        FLOAT32 fCubicTerm = ( 12.0 - 9.0 * fB - 6.0 * fC ) * ( fRange * fRange * fRange );
        FLOAT32 fQuadTerm  = ( -18.0 + 12.0 * fB + 6.0 * fC ) * ( fRange * fRange );
        FLOAT32 fConstTerm = ( 6.0 - 2.0 * fB );

        fResult = ( 1.0f / 6.0f ) * ( fCubicTerm + fQuadTerm + fConstTerm );                
    }

    else if ( fRange >= 1.0 && fRange < 2.0 )
    {
        FLOAT32 fCubicTerm = ( -fB - 6.0 * fC ) * ( fRange * fRange * fRange );
        FLOAT32 fQuadTerm  = ( 6.0 * fB + 30.0 * fC ) * ( fRange * fRange );
        FLOAT32 fLinTerm   = ( -12.0 * fB - 48.0 * fC ) * ( fRange );
        FLOAT32 fConstTerm = ( 8.0 * fB + 24.0 * fC );

        fResult = ( 1.0f / 6.0f ) * ( fCubicTerm + fQuadTerm + fLinTerm + fConstTerm );    
    }

    if ( fResult < 0 ) fResult = 0.0;

    return fResult;
}
 
//
// Bicubic Kernel
//
//   BicubicKernel performs a simple combined 2D bicubic sampling of a source image.
//   Bicubic filters are separable, but this function performs a non-separated kernel
//   that facilitates non-uniform sampling filters. 
// 
// Parameters:
// 
//   pSrcImage:  The read-only source image to filter.
//
//   fCoeffB:    The alpha mitchell-netravali coefficient
//
//   fCoeffC:    The beta mitchell-netravali coefficient
//
//   fX, fY:     Specifies the coordinates to sample in the source image.
//
//   pRawOutput: A pointer to a pixel buffer. Upon return, this buffer will contain the 
//               sampled result of the operation.
//
// Supported Image Formats: 
//    
//   All image formats are supported. 
//

VN_STATUS vnBicubicKernel( CONST CVImage & pSrcImage, 
                           FLOAT32 fCoeffB,
                           FLOAT32 fCoeffC,
                           FLOAT32 fX, 
                           FLOAT32 fY,  
                           UINT8 * pRawOutput );

//
// Bicubic Kernel
//
//   BicubicKernel performs a simple horizontal or vertical bicubic sampling of a 
//   source image. 
// 
//   (!) Note: bicubic filters are separable. This interface may be used to improve 
//             operator performance by sampling the horizontal and vertical directions
//             separately.
//
// Parameters:
// 
//   pSrcImage:  The read-only source image to filter.
//
//   fCoeffB:    The alpha mitchell-netravali coefficient
//
//   fCoeffC:    The beta mitchell-netravali coefficient
//
//   fX, fY:   Specifies the coordinates to sample in the source image.
//
//   bDirection: Specifies the direction to sample. FALSE indicates horizontal sampling,
//               while TRUE indicates vertical.
//
//   pRawOutput: a pointer to a pixel buffer. Upon return, this buffer will contain the 
//               sampled result of the operation.
//
// Supported Image Formats: 
//    
//   All image formats are supported. 
//

VN_STATUS vnBicubicKernel( CONST CVImage & pSrcImage, 
                           FLOAT32 fCoeffB,
                           FLOAT32 fCoeffC,
                           FLOAT32 fX, 
                           FLOAT32 fY, 
                           BOOL bDirection, 
                           UINT8 * pRawOutput );

#endif // __VN_IMAGE_BICUBIC_H__