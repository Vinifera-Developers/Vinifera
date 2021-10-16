
//
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnImageSampler.h
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
// Description:
//
//     Image Sampler is a function that can be used to sample a single pixel in an image. It supports
//     both single and double pass sampling of 1D and 2D data sets. 
//
//  Additional Information:
//
//   For more information, visit http://www.bertolami.com.
//

#ifndef __VN_IMAGE_SAMPLE_H__
#define __VN_IMAGE_SAMPLE_H__

#include "../Base/vnBase.h"
#include "../Base/vnMath.h"
#include "../Base/vnImage.h"
#include "../Base/vnImageFormat.h"

//
// Supported filter set
//

#include "../Kernels/vnImageGaussian.h"
#include "../Kernels/vnImageAverage.h"
#include "../Kernels/vnImageBilinear.h"
#include "../Kernels/vnImageNearest.h"
#include "../Kernels/vnImageBicubic.h"
#include "../Kernels/vnImageSpline.h"
#include "../Kernels/vnImageLanczos.h"
#include "../Kernels/vnImageCoverage.h"

//
// Image Sampler
//
//   Within the image filter layer, the image sampler is the nexus between the image 
//   operators, and the kernels, as shown below. The image sampler, in many cases, allows the 
//   operators to be filter agnostic and support a wide variety of kernels.
//
//      Image Operators                 Caveats:
//                              
//             |                         1. Care must be taken when using the sampler within an
//     +-------|-------+                    operator. Certain kernels are non-separable and will
//      \      |      /                     *not* fail if asked to filter a 1D data set, but will
//       \     |     /                       not produce accurate results if applied to an image in
//        \    |    /                        a separable manner.
//         \   |   /
//          ---|---                       2. Sampling with these kernels is very simple, and does not  
//             V                             incorporate a number of important features like sub-pixel
//                                           estimation. These features are handled by the higher level 
//          Sampler                          operators.
//
//             ^                         3. Format support is decided on a per-kernel basis. Thus, 
//          ---|---                         kernels are not required to support all formats. Pay special 
//         /   |   \                        attention to return codes, as they may indicate an invalid 
//        /    |    \                       sample request.
//       /     |     \
//      /      |      \
//     +-------|-------+
//             |
//
//       Image Kernels
//

#define VN_IMAGE_MAKE_KERNEL( sep, index )          ( ( (sep) << 0x1F ) | ( ( index ) & 0x7FFFFFFF ) )

//
// (!) Note: the individual header files for each of the following filters contain useful 
//           information for selecting a particular filter for an operation.
//

enum VN_IMAGE_KERNEL_TYPE
{
    //
    // Linear family
    //

    VN_IMAGE_KERNEL_NEAREST          = VN_IMAGE_MAKE_KERNEL( 1, 0 ),    // nearest neighbor
    VN_IMAGE_KERNEL_AVERAGE          = VN_IMAGE_MAKE_KERNEL( 1, 1 ),    // simple averaging over the kernel space
    VN_IMAGE_KERNEL_BILINEAR         = VN_IMAGE_MAKE_KERNEL( 1, 2 ),    // bilinear interpolation

    //
    // Mitchell-Netravali Cubic family
    //

    VN_IMAGE_KERNEL_BICUBIC          = VN_IMAGE_MAKE_KERNEL( 1, 4 ),    // generic bicubic kernel
    VN_IMAGE_KERNEL_CATMULL          = VN_IMAGE_MAKE_KERNEL( 1, 5 ),    // Catmull-Rom spline, popularized by GIMP
    VN_IMAGE_KERNEL_MITCHELL         = VN_IMAGE_MAKE_KERNEL( 1, 6 ),    // cubic mitchell-netravali kernel
    VN_IMAGE_KERNEL_CARDINAL         = VN_IMAGE_MAKE_KERNEL( 1, 7 ),    // cubic, popularized by Adobe Photoshop
    VN_IMAGE_KERNEL_SPLINE           = VN_IMAGE_MAKE_KERNEL( 0, 8 ),    // cubic spline
    VN_IMAGE_KERNEL_BSPLINE          = VN_IMAGE_MAKE_KERNEL( 1, 9 ),    // cubic b-spline, popularized by Paint.NET

    //
    // Non-separable cubic family
    //

    VN_IMAGE_KERNEL_LANCZOS          = VN_IMAGE_MAKE_KERNEL( 0, 10 ),   // lanczos-1
    VN_IMAGE_KERNEL_LANCZOS2         = VN_IMAGE_MAKE_KERNEL( 0, 11 ),   // lanczos-2
    VN_IMAGE_KERNEL_LANCZOS3         = VN_IMAGE_MAKE_KERNEL( 0, 12 ),   // lanczos-3
    VN_IMAGE_KERNEL_LANCZOS4         = VN_IMAGE_MAKE_KERNEL( 0, 13 ),   // lanczos-4
    VN_IMAGE_KERNEL_LANCZOS5         = VN_IMAGE_MAKE_KERNEL( 0, 14 ),   // lanczos-5
    
    //
    // Distribution family
    //

    VN_IMAGE_KERNEL_BOKEH            = VN_IMAGE_MAKE_KERNEL( 0, 15 ),   // bokeh
    VN_IMAGE_KERNEL_GAUSSIAN         = VN_IMAGE_MAKE_KERNEL( 1, 16 ),   // gaussian interpolation
    VN_IMAGE_KERNEL_COVERAGE         = VN_IMAGE_MAKE_KERNEL( 1, 17 ),   // coverage kernel

    // ...

    VN_IMAGE_KERNEL_TYPE_DWORD        = 0x7FFFFFFF
};

//
// Kernel Directions
//
//   Some kernels are separabe, and others are not. For those that are separable,
//   it can be faster to perform two 1D passes rather than a single 2D pass. Each
//   separable kernel will identify itself as such in the comments above the kernel
//   interface.
//

enum VN_IMAGE_KERNEL_DIRECTION
{
    VN_IMAGE_KERNEL_1D_HORIZONTAL       = 0x0,
    VN_IMAGE_KERNEL_1D_VERTICAL,
    VN_IMAGE_KERNEL_2D_COMBINED,

    // ...

    VN_IMAGE_KERNEL_DIRECTION_WORD      = 0x7FFF
};

//
// Kernel Edgemode
//
// When sampling the edge of an image, the kernel must decide how best to handle cases
// where coordinates seek beyond the addressable pixel range. Edgemode allows the 
// caller to specify whether the kernel should clamp the coordinates or wrap them 
// around to the other side of the image.
//

enum VN_IMAGE_KERNEL_EDGEMODE
{
    VN_IMAGE_KERNEL_EDGE_WRAP           = 0x0,
    VN_IMAGE_KERNEL_EDGE_CLAMP,
    VN_IMAGE_KERNEL_EDGE_REFLECT,

    // ...

    VN_IMAGE_KERNEL_EDGEMODE_WORD       = 0x7FFF
};

//
// Simple helper utility to check if a kernel is separable.
//

inline BOOL vnIsSeparableKernel( VN_IMAGE_KERNEL_TYPE uiKernel )
{
    return !!( 0x80000000 & uiKernel );
}

//
// SampleImage 
//
//   This function samples a source image at a particular location, using the prescribed filter
//   kernel, and returns the result in the same format as the source image.
//
// Parameters:
// 
//   pSrcImage:   The read-only source image to sample.
//
//   uiKernel:    Indicates the filter kernel to use when sampling.
//
//   uiDirection: Indicates whether the kernel should be run horizontally or vertically
//                over a 1D space, or whether the kernel should be run over
//                the 2D surface of the image.
//
//   uiX:         The x coordinate of the pixel to sample from the source image.
//
//   uiY:         The y coordinate of the pixel to sample from the source image.
//
//   uiRadius:    The radius to supply for the filter kernel.
//
//   pRawOutput:  A pointer to a pixel in a destination buffer that should receive 
//                sampled output. In most cases, this will refer to a pixel in a 
//                destination image (of matching format to the source image), that 
//                should store the sampled result.
//
// Supported Image Formats: 
//    
//   All image formats are supported. 
//

VN_STATUS vnSampleImage( CONST CVImage & pSrcImage, 
                         VN_IMAGE_KERNEL_TYPE uiKernel, 
                         VN_IMAGE_KERNEL_DIRECTION uiDirection, 
                         FLOAT32 fX, 
                         FLOAT32 fY, 
                         FLOAT32 fRadius,
                         UINT8 * pRawOutput );

#endif // __VN_IMAGE_SAMPLE_H__
