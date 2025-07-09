
//
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnImageFormat.h
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

#ifndef __VN_IMAGE_FORMAT_H__
#define __VN_IMAGE_FORMAT_H__

#include "vnBase.h"

//
// An image format is a 32 bit value that specifies a unique image identifier with the 
// following pattern:
//
//   Bit(s)       Title           Description
//   ------       --------        ------------------------------------- 
//   31:          Endian,         zero for little, one for big
//   30:          Precision,      zero for fixed, one for float
//   29:          Space,          zero for color, one for depth
//   28:          Signed,         zero for unsigned, one for signed
//   27-24:       Reserved,       reserved for future compression support
//   23-18:       Channel 0,      specifies the bit count of the first channel
//   17-12:       Channel 1,      specifies the bit count of the second channel
//   11-6:        Channel 2:      specifies the bit count of the third channel
//   5-0:         Channel 3:      specifies the bit count of the fourth channel
//

#define VN_IMAGE_MAX_CHANNEL_COUNT                      (4)
#define VN_IMAGE_MAX_CHANNEL_SHIFT                      (6)

#define VN_IMAGE_ENDIAN_MASK                            (0x01)
#define VN_IMAGE_PRECISION_MASK                         (0x01)
#define VN_IMAGE_SPACE_MASK                             (0x01)
#define VN_IMAGE_SIGNED_MASK                            (0x01)
#define VN_IMAGE_RESERVED_MASK                          (0x0F)
#define VN_IMAGE_CHANNEL_MASK                           (0x3F)

#define VN_IMAGE_ENDIAN_SHIFT                           (0x1F)
#define VN_IMAGE_PRECISION_SHIFT                        (0x1E)
#define VN_IMAGE_SPACE_SHIFT                            (0x1D)
#define VN_IMAGE_SIGNED_SHIFT                           (0x1C)
#define VN_IMAGE_RESERVED_SHIFT                         (0x18)
#define VN_IMAGE_CHANNEL_0_SHIFT                        (0x12)
#define VN_IMAGE_CHANNEL_1_SHIFT                        (0x0C)
#define VN_IMAGE_CHANNEL_2_SHIFT                        (0x06)
#define VN_IMAGE_CHANNEL_3_SHIFT                        (0x00)

//
// The following restrictions exist when defining new image formats. If these 
// restrictions are violated, the format may still be specified, but is not 
// considered valid. In these cases, individual filter support should be verified
// and not assumed.
//
// Restrictions:
//
//   1. The pixel rate (i.e. the sum of the bit rates of all valid channels), should
//      be 8-aligned.
//
//   2. Channels may not mix float and fixed data types.
//
//   3. The current maximum supported bit rate per channel is 32, thus the maximum
//      pixel rate is 128. 
//
//   4. Channels must be used contiguously. You may not skip a channel (e.g. use /
//      R and B channels without G). A format with a channel count of N must consume
//      its first N channels. If you require more advanced channel selection, opt for
//      a set of planar images.
//
//   5. All multi-channel formats must be interleaved. Planar multi-channel formats 
//      are supported at a higher level using multiple single channel planar images.
//

enum VN_IMAGE_ENDIAN
{
    VN_IMAGE_ENDIAN_LITTLE          = 0,
    VN_IMAGE_ENDIAN_BIG             = 1,
    VN_IMAGE_ENDIAN_FORCE_BYTE      = 0x7F
};

enum VN_IMAGE_PRECISION
{
    VN_IMAGE_PRECISION_FIXED        = 0,
    VN_IMAGE_PRECISION_FLOAT        = 1,
    VN_IMAGE_PRECISION_FORCE_BYTE   = 0x7F
};

enum VN_IMAGE_SPACE
{
    VN_IMAGE_SPACE_COLOR            = 0,
    VN_IMAGE_SPACE_DEPTH            = 1,
    VN_IMAGE_SPACE_FORCE_BYTE       = 0x7F
};

//
// (!) Note: we support heterogeneous channel rates, with a maximum pixel rate of 
//           256 bits per pixel for both float and fixed formats. The following 
//           examples demonstrate (but are not limited to) the types of formats 
//           that are permitted with this format:
//
//           R16G32B32F ----- float format with 16, 32, and 32 bpc for RGB respectively.
//
//           R3G3B2     ----- fixed format with 3, 3, and 2 bpc for RGB respectively.
//
//           R32F       ----- float format, single channel, 32 bit
//

#define VN_IMAGE_CHANNEL_RATE( c, f )       ( ( (f) >> ( ( ( VN_IMAGE_MAX_CHANNEL_COUNT - 1 ) - (c) ) * \
                                              VN_IMAGE_MAX_CHANNEL_SHIFT ) ) & VN_IMAGE_CHANNEL_MASK )

#define VN_IS_FLOAT_FORMAT( x )             ( ( (x) >> VN_IMAGE_PRECISION_SHIFT ) & VN_IMAGE_PRECISION_MASK )

#define VN_IS_SIGNED_FORMAT( x )            ( ( (x) >> VN_IMAGE_SIGNED_SHIFT ) & VN_IMAGE_SIGNED_MASK )

#define VN_IMAGE_PIXEL_RATE( x )            ( ( ( (x) >> VN_IMAGE_CHANNEL_0_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) + \
                                              ( ( (x) >> VN_IMAGE_CHANNEL_1_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) + \
                                              ( ( (x) >> VN_IMAGE_CHANNEL_2_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) + \
                                              ( ( (x) >> VN_IMAGE_CHANNEL_3_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) )

#define VN_IMAGE_CHANNEL_COUNT( x )         ( !!( ( (x) >> VN_IMAGE_CHANNEL_0_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) + \
                                              !!( ( (x) >> VN_IMAGE_CHANNEL_1_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) + \
                                              !!( ( (x) >> VN_IMAGE_CHANNEL_2_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) + \
                                              !!( ( (x) >> VN_IMAGE_CHANNEL_3_SHIFT ) & VN_IMAGE_CHANNEL_MASK ) )

//
// For a format to be considered valid, it must have:
//
//   1. at least one non-zero sized channel
//
//   2. correct usage of sign and float: float formats are not allowed to be signed (because they have no unsigned)
//

#define VN_IS_FORMAT_VALID( f )             ( VN_IMAGE_CHANNEL_RATE( 0, f ) || \
                                              VN_IMAGE_CHANNEL_RATE( 1, f ) || \
                                              VN_IMAGE_CHANNEL_RATE( 2, f ) || \
                                              VN_IMAGE_CHANNEL_RATE( 3, f ) || \
                                              ( VN_IS_FLOAT_FORMAT(f) && VN_IS_SIGNED_FORMAT(f) ) )


//
// (!) N.B. We distinguish between rgba and depth color spaces in order to better support
//          graphics hardware that manages these separately. This allows the engine to 
//          distinguish between identical data layouts that require different usage.
//

#define VN_MAKE_IMAGE_FORMAT( endian, precision, space, sign, reserved, zero, one, two, three ) (            \
                            ( ( ( endian )    & VN_IMAGE_ENDIAN_MASK )    << VN_IMAGE_ENDIAN_SHIFT )     |   \
                            ( ( ( precision ) & VN_IMAGE_PRECISION_MASK ) << VN_IMAGE_PRECISION_SHIFT )  |   \
                            ( ( ( space )     & VN_IMAGE_SPACE_MASK )     << VN_IMAGE_SPACE_SHIFT )      |   \
                            ( ( ( sign )      & VN_IMAGE_SIGNED_MASK )    << VN_IMAGE_SIGNED_SHIFT )     |   \
                            ( ( ( reserved )  & VN_IMAGE_RESERVED_MASK )  << VN_IMAGE_RESERVED_SHIFT )   |   \
                            ( ( ( zero )      & VN_IMAGE_CHANNEL_MASK )   << VN_IMAGE_CHANNEL_0_SHIFT )  |   \
                            ( ( ( one )       & VN_IMAGE_CHANNEL_MASK )   << VN_IMAGE_CHANNEL_1_SHIFT )  |   \
                            ( ( ( two )       & VN_IMAGE_CHANNEL_MASK )   << VN_IMAGE_CHANNEL_2_SHIFT )  |   \
                            ( ( ( three )     & VN_IMAGE_CHANNEL_MASK )   << VN_IMAGE_CHANNEL_3_SHIFT ) )

//
// The image format list that is enumerated below was designed to support a broad set of 
// useful formats, and to support the easy addition of new formats in the future. This list
// is reserved for interleaved RGB formats only, and should not include planar formats or
// formats in alternate color spaces (e.g. YUV).
// 

enum VN_IMAGE_FORMAT
{
    VN_IMAGE_FORMAT_NONE                = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 0, 0, 0, 0 ),   
    
    //
    // Unsigned integer formats
    //

    VN_IMAGE_FORMAT_R8                  = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 8, 0, 0, 0 ),
    VN_IMAGE_FORMAT_R16                 = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 16, 0, 0, 0 ), 
    VN_IMAGE_FORMAT_D16                 = VN_MAKE_IMAGE_FORMAT( 0, 0, 1, 0, 0, 16, 0, 0, 0 ),
    VN_IMAGE_FORMAT_R24                 = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 24, 0, 0, 0 ),
    VN_IMAGE_FORMAT_D32                 = VN_MAKE_IMAGE_FORMAT( 0, 0, 1, 0, 0, 32, 0, 0, 0 ),
    VN_IMAGE_FORMAT_R32                 = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 32, 0, 0, 0 ),

    VN_IMAGE_FORMAT_R3G3B2              = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 3, 3, 2, 0 ),
    VN_IMAGE_FORMAT_R5G6B5              = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 5, 6, 5, 0 ),
    VN_IMAGE_FORMAT_R5G5B5A1            = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 5, 5, 5, 1 ),
    VN_IMAGE_FORMAT_R4G4B4A4            = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 4, 4, 4, 4 ),

    VN_IMAGE_FORMAT_R8G8B8              = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 8, 8, 8, 0 ),
    VN_IMAGE_FORMAT_R16G16B16           = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 16, 16, 16, 0 ),
    VN_IMAGE_FORMAT_R32G32B32           = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 32, 32, 32, 0 ),

    VN_IMAGE_FORMAT_R8G8B8A8            = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 8, 8, 8, 8 ),
    VN_IMAGE_FORMAT_R10G10B10A2         = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 10, 10, 10, 2 ),
    VN_IMAGE_FORMAT_R16G16B16A16        = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 16, 16, 16, 16 ),
    VN_IMAGE_FORMAT_R32G32B32A32        = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 0, 0, 32, 32, 32, 32 ),

    //
    // Signed integer formats
    //

    VN_IMAGE_FORMAT_R8S                 = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 1, 0, 8, 0, 0, 0 ),    
    VN_IMAGE_FORMAT_R16S                = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 1, 0, 16, 0, 0, 0 ),     
    VN_IMAGE_FORMAT_R32S                = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 1, 0, 32, 0, 0, 0 ),
   
    VN_IMAGE_FORMAT_R8G8B8S             = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 1, 0, 8, 8, 8, 0 ),    
    VN_IMAGE_FORMAT_R16G16B16S          = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 1, 0, 16, 16, 16, 0 ),    
    VN_IMAGE_FORMAT_R32G32B32S          = VN_MAKE_IMAGE_FORMAT( 0, 0, 0, 1, 0, 32, 32, 32, 0 ),
   
    //
    // Float formats
    //
   
    VN_IMAGE_FORMAT_R16F                = VN_MAKE_IMAGE_FORMAT( 0, 1, 0, 0, 0, 16, 0, 0, 0 ),
    VN_IMAGE_FORMAT_R32F                = VN_MAKE_IMAGE_FORMAT( 0, 1, 0, 0, 0, 32, 0, 0, 0 ),

    VN_IMAGE_FORMAT_R32G32F             = VN_MAKE_IMAGE_FORMAT( 0, 1, 0, 0, 0, 32, 32, 0, 0 ),
    
    VN_IMAGE_FORMAT_R16G16B16F          = VN_MAKE_IMAGE_FORMAT( 0, 1, 0, 0, 0, 16, 16, 16, 0 ),
    VN_IMAGE_FORMAT_R32G32B32F          = VN_MAKE_IMAGE_FORMAT( 0, 1, 0, 0, 0, 32, 32, 32, 0 ),
    
    VN_IMAGE_FORMAT_R16G16B16A16F       = VN_MAKE_IMAGE_FORMAT( 0, 1, 0, 0, 0, 16, 16, 16, 16 ),
    VN_IMAGE_FORMAT_R32G32B32A32F       = VN_MAKE_IMAGE_FORMAT( 0, 1, 0, 0, 0, 32, 32, 32, 32 ), 
    
    // ...

    VN_IMAGE_FORMAT_FORCE_DWORD         = 0x7FFFFFFF
};

//
// Format aliases
//

#define VN_IMAGE_FORMAT_L8              VN_IMAGE_FORMAT_R8
#define VN_IMAGE_FORMAT_U8              VN_IMAGE_FORMAT_R8
#define VN_IMAGE_FORMAT_V8              VN_IMAGE_FORMAT_R8
#define VN_IMAGE_FORMAT_L16             VN_IMAGE_FORMAT_R16
#define VN_IMAGE_FORMAT_U16             VN_IMAGE_FORMAT_R16
#define VN_IMAGE_FORMAT_V16             VN_IMAGE_FORMAT_R16

#endif // __VN_IMAGE_FORMAT_H__