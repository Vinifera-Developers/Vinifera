
// Copyright (c) 2002-2009 Joe Bertolami. All Right Reserved.
//
// vnMath.h
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
// Additional Information:
//
//   For more information, visit http://www.bertolami.com.

#ifndef __VN_MATH_H__
#define __VN_MATH_H__

#include "vnBase.h"
#include "vnHalf.h"

#define VN_MAX_INT64           (0x7FFFFFFFFFFFFFFF)
#define VN_MAX_INT32           (0x7FFFFFFF)
#define VN_MAX_INT16           (0x7FFF)
#define VN_MAX_INT8            (0x7F)

#define VN_MAX_UINT64          (0xFFFFFFFFFFFFFFFF)
#define VN_MAX_UINT32          (0xFFFFFFFF)
#define VN_MAX_UINT16          (0xFFFF)
#define VN_MAX_UINT8           (0xFF)

#define VN_MIN_INT64           (-VN_MAX_INT64 - 1)
#define VN_MIN_INT32           (-VN_MAX_INT32 - 1)
#define VN_MIN_INT16           (-VN_MAX_INT16 - 1)
#define VN_MIN_INT8            (-VN_MAX_INT8 - 1)

#define VN_PI                  (3.14159262f)

#define VN_MIN2( a, b )        ((a) < (b) ? (a) : (b))
#define VN_MAX2( a, b )        ((a) > (b) ? (a) : (b))
#define VN_MIN3( a, b, c )     ((c) < (a) ? ((c) < (b) ? (c) : (b)) : (a) < (b) ? (a) : (b))
#define VN_MAX3( a, b, c )     ((c) > (a) ? ((c) > (b) ? (c) : (b)) : (a) > (b) ? (a) : (b))

inline FLOAT32 vnClipRange( FLOAT32 fInput, FLOAT32 fLow, FLOAT32 fHigh )
{
    if ( fInput < fLow ) fInput = fLow;
    else if ( fInput > fHigh ) fInput = fHigh;
    return fInput;
}

inline INT32 vnClipRange( INT32 iInput, INT32 iLow, INT32 iHigh )
{
    if ( iInput < iLow ) iInput = iLow;
    else if ( iInput > iHigh ) iInput = iHigh;
    return iInput;
}

inline FLOAT64 vnClipRange64( FLOAT64 fInput, FLOAT64 fLow, FLOAT64 fHigh )
{
    if ( fInput < fLow ) fInput = fLow;
    else if ( fInput > fHigh ) fInput = fHigh;
    return fInput;
}

inline INT64 vnClipRange64( INT64 iInput, INT64 iLow, INT64 iHigh )
{
    if ( iInput < iLow ) iInput = iLow;
    else if ( iInput > iHigh ) iInput = iHigh;
    return iInput;
}

inline FLOAT32 vnSaturate( FLOAT32 fInput )
{
    return vnClipRange( fInput, 0.0f, 1.0f );
}

inline INT32 vnSaturate( INT32 iInput )
{
    return vnClipRange( iInput, 0, 255 );
}

inline BOOL vnIsPow2( UINT32 uiValue )
{
    return ( 0 == ( uiValue & ( uiValue - 1 ) ) );
}

inline FLOAT32 vnGreaterMultiple( FLOAT32 fValue, FLOAT32 fMultiple )
{
    FLOAT32 mod = fmod( fValue, fMultiple );
    
    if ( mod != 0.0f )
    {
        fValue += ( fMultiple - mod );
    }
    
    return fValue;
}

inline UINT32 vnGreaterMultiple( UINT32 uiValue, UINT32 uiMultiple )
{
    UINT32 mod = uiValue % uiMultiple;
    
    if ( 0 != mod )
    {
        uiValue += ( uiMultiple - mod );
    }
    
    return uiValue;
}

inline UINT32 vnAlign( UINT32 uiValue, UINT32 uiAlignment )
{
    return vnGreaterMultiple( uiValue, uiAlignment );
}

inline UINT32 vnAlign16( UINT32 uiValue )
{
    return ( uiValue & 0xF ? uiValue + ~( uiValue & 0xF ) + 1 : uiValue );
}

inline UINT32 vnAlign8( UINT32 uiValue )
{
    return ( uiValue & 0x7 ? uiValue + ~( uiValue & 0x7 ) + 1 : uiValue );
}

inline UINT32 vnAlign2( UINT32 uiValue )
{
    if ( vnIsPow2( uiValue ) )
    {
        return uiValue;
    }
    
    INT32 iPower = 0;
    
    while ( uiValue ) 
    {
        uiValue >>= 1;
        
        iPower++;
    }
    
    return 1 << iPower;
}

#endif // __VN_MATH_H__