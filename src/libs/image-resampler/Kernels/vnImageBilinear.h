
//
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnImageBilinear.h
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

#ifndef __VN_BILINEAR_H__
#define __VN_BILINEAR_H__

#include "../Base/vnBase.h"
#include "../Base/vnMath.h"
#include "../Base/vnImage.h"

//
// Bilinear Kernel
//
//   BilinearKernel performs a simple combined 2D bilinear sampling of a source image.
//   Bilinear filters are separable, but this function performs a non-separated kernel
//   that facilitates non-uniform sampling filters. 
// 
// Parameters:
// 
//   pSrcImage:  The read-only source image to filter.
//
//   fX, fY:     Specifies the coordinates to sample in the source image.
//
//   pRawOutput: a pointer to a pixel buffer. Upon return, this buffer will contain the 
//               sampled result of the operation.
//
// Supported Image Formats: 
//    
//   All image formats are supported. 
//

VN_STATUS vnBilinearKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, UINT8 * pRawOutput );

//
// Bilinear Kernel
//
//   BilinearKernel performs a simple horizontal or vertical bilinear sampling of a 
//   source image. 
// 
//   (!) Note: bilinear filters are separable. This interface may be used to improve 
//             operator performance by sampling the horizontal and vertical directions
//             separately.
//
// Parameters:
// 
//   pSrcImage:  The read-only source image to filter.
//
//   fX, fY:     Specifies the coordinates to sample in the source image.
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

VN_STATUS vnBilinearKernel( CONST CVImage & pSrcImage, FLOAT32 fX, FLOAT32 fY, BOOL bDirection, UINT8 * pRawOutput );

#endif // __VN_BILINEAR_H__