
//
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnImageResize.h
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

#ifndef __VN_IMAGE_RESIZE_H__
#define __VN_IMAGE_RESIZE_H__

#include "../Base/vnBase.h"
#include "../Base/vnMath.h"
#include "../Base/vnImage.h"
#include "../Utilities/vnImageSampler.h"

#define VN_IMAGE_RESIZE_PARAMETERS                  UINT16
#define VN_IMAGE_RESIZE_DEFAULT                     0x0000
#define VN_IMAGE_RESIZE_POW2                        0x0001              // align dimensions to pow-2
#define VN_IMAGE_RESIZE_SYMMETRIC                   0x0002              // constrain proportions
#define VN_IMAGE_RESIZE_PAD_EXTEND                  0x0004              // resize canvas but not the content

//
// ResizeImage Operator
//
//   ResizeImage resamples the image to the dimensions specified.
//
// Parameters:
// 
//   pSrcImage:  The read-only source image to resize.
//
//   uiKernel:   The sampling kernel to use when performing the resizing.
//
//   uiWidth:    The destination width to target.
//
//   uiHeight:   The destination height to target.
//
//   pDestImage: a pointer to an image object. Upon successful return, this object will
//               contain a resized view of the source image.
//
// Supported Image Formats: 
//    
//   All image formats are supported. 
//

VN_STATUS vnResizeImage( CONST CVImage & pSrcImage, 
                         VN_IMAGE_KERNEL_TYPE uiKernel, 
                         UINT32 uiWidth, 
                         UINT32 uiHeight, 
                         VN_IMAGE_RESIZE_PARAMETERS uiFlags, 
                         CVImage * pDestImage );

#endif // __VN_IMAGE_RESIZE_H__
