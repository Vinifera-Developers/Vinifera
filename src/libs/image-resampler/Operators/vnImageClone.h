
//
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnImageClone.h
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

#ifndef __VN_IMAGE_CLONE_H__
#define __VN_IMAGE_CLONE_H__

#include "../Base/vnBase.h"
#include "../Base/vnImage.h"
#include "../Base/vnImageFormat.h"

//
// CloneImage Operator
//
//   CloneImage is a simple shorthand for creating an identical copy of an image.
//
// Parameters:
// 
//   pSrcImage:  The read-only source image to copy.
//
//   pDestImage: an uninitialized pointer to an image object. Upon successful
//               return, this object will be fully initialized and contain a 
//               clone of pSrcImage.
//
// Supported Image Formats: 
//    
//   All image formats are supported. 
//

inline VN_STATUS vnCloneImage( CONST CVImage & pSrcImage, CVImage * pDestImage )
{
    if ( VN_PARAM_CHECK )
    {
        if ( VN_IMAGE_FORMAT_NONE == pSrcImage.QueryFormat() || !pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }

        if ( &pSrcImage == pDestImage )
        {
            return vnPostError( VN_ERROR_INVALIDARG );
        }
    }

    if ( VN_FAILED( vnCreateImage( pSrcImage.QueryFormat(), pSrcImage.QueryWidth(), pSrcImage.QueryHeight(), pDestImage ) ) )
    {
        return vnPostError( VN_ERROR_EXECUTION_FAILURE );
    }

    memcpy( (pDestImage)->QueryData(), pSrcImage.QueryData(), pSrcImage.SlicePitch() );

    return VN_SUCCESS;
}

#endif // __VN_IMAGE_CLONE_H__