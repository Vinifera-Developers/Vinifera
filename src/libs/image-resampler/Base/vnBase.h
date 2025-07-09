
// Copyright (c) 1998-2009 Joe Bertolami. All Right Reserved.
//
// vnBase.h
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

#ifndef __VN_BASE_H__
#define __VN_BASE_H__

/**********************************************************************************
//
// Platform definitions
//
**********************************************************************************/

#if defined ( _WIN32 ) || defined ( _WIN64 )
    #include "windows.h"                                    
    #pragma warning (disable : 4244)                      // conversion, possible loss of data   
    #pragma warning (disable : 4018)                      // signed / unsigned mismatch
    #pragma warning (disable : 4996)                      // deprecated interfaces
    #pragma warning (disable : 4221)                      // empty translation unit
    #pragma warning (disable : 4273)                      // inconsistent linkage
    
    #define VN_PLATFORM_WINDOWS                           // building a Windows application

#elif defined ( __APPLE__ )
    #include "TargetConditionals.h"
    #include "unistd.h"
    #include "sys/types.h"
    #include "ctype.h"

    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define VN_PLATFORM_IOS                           // building an application for iOS
    #elif TARGET_OS_MAC
        #define VN_PLATFORM_MACOSX                        // building a Mac OSX application
    #endif
#else
    #error "Unsupported target platform detected."
#endif

/**********************************************************************************
//
// Debug definitions
//
**********************************************************************************/

#if defined ( VN_PLATFORM_WINDOWS )
    #ifdef _DEBUG
        #define VN_DEBUG _DEBUG
    #elif defined ( DEBUG )
        #define VN_DEBUG DEBUG
    #endif
    #if defined( VN_DEBUG ) && !defined( debug_break )
        #define debug_break __debugbreak
    #endif
    #define __VN_FUNCTION__  __FUNCTION__
#elif defined ( VN_PLATFORM_IOS ) || defined ( VN_PLATFORM_MACOSX )
   #ifdef DEBUG
       #define VN_DEBUG DEBUG
       #if !defined( debug_break )
           #define debug_break() __builtin_trap()
       #endif
    #endif
    #define __VN_FUNCTION__ __func__
#endif

/**********************************************************************************
//
// Standard headers
//
**********************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "math.h"

/**********************************************************************************
//
// Standard types
//
**********************************************************************************/

#if defined ( VN_PLATFORM_WINDOWS )
    // Types are already defined by the platform.
#elif defined ( VN_PLATFORM_IOS ) || defined ( VN_PLATFORM_MACOSX )
    typedef int64_t INT64;		
    typedef int32_t INT32;		
    typedef int16_t INT16;		
    typedef int8_t  INT8; 

    typedef u_int64_t UINT64;	    
    typedef u_int32_t UINT32;	    
    typedef u_int16_t UINT16;	    
    typedef u_int8_t  UINT8;	 
#endif

typedef float FLOAT32;         
typedef double FLOAT64;           
typedef wchar_t WCHAR;

#define VN_KB       ( (UINT32) 1024 )
#define VN_MB       ( VN_KB * VN_KB )
#define VN_GB       ( VN_MB * VN_KB )

#define CONST       const
#ifndef CHAR
#define CHAR        char
#endif

/**********************************************************************************
//
// Status codes
//
**********************************************************************************/

typedef UINT8 VN_STATUS;

#define VN_SUCCESS                                 (0)
#define VN_ERROR_INVALIDARG                        (1)
#define VN_ERROR_NOTIMPL                           (2)
#define VN_ERROR_OUTOFMEMORY                       (3)
#define VN_ERROR_UNDEFINED                         (4)
#define VN_ERROR_HARDWAREFAIL                      (5)
#define VN_ERROR_INVALID_INDEX                     (6)
#define VN_ERROR_CAPACITY_LIMIT                    (7)
#define VN_ERROR_INVALID_RESOURCE                  (8)
#define VN_ERROR_OPERATION_TIMEDOUT                (9)
#define VN_ERROR_EXECUTION_FAILURE                 (10)
#define VN_ERROR_PERMISSION_DENIED                 (11)
#define VN_ERROR_IO_FAILURE                        (12)
#define VN_ERROR_RESOURCE_UNREACHABLE              (13)
#define VN_ERROR_SYSTEM_FAILURE                    (14)
#define VN_ERROR_NOT_READY                         (15)
#define VN_ERROR_OPERATION_COMPLETED               (16)
#define VN_ERROR_RESOURCE_UNUSED                   (17)

#define VN_SUCCEEDED( code )                       ( (code) == VN_SUCCESS )
#define VN_FAILED( code )                          ( !VN_SUCCEEDED( code ) )

/**********************************************************************************
//
// Debug support
//
**********************************************************************************/

#ifdef VN_DEBUG
    #define VN_PARAM_CHECK (1)
    #define VN_ERR(fmt, ...) do { printf("[VN-ERR] "); \
                                    printf(fmt, ##__VA_ARGS__); \
                                    printf("\n"); debug_break(); \
                             } while(0)

    #define VN_MSG(fmt, ...) do { printf("[VN-MSG] "); \
                                    printf(fmt, ##__VA_ARGS__); \
                                    printf("\n"); \
                             } while(0)
#else
    #define VN_PARAM_CHECK (0)
    #define VN_ERR(fmt, ...)                              
    #define VN_MSG(fmt, ...) do { printf("[VN-MSG] "); \
                                    printf(fmt, ##__VA_ARGS__); \
                                    printf("\n"); \
                             } while(0)
#endif 

#define VN_ERROR_CREATE_STRING(x) ((char *) #x)
#define vnPostError(x) vnPostErrorInternal(x, VN_ERROR_CREATE_STRING(x), __VN_FUNCTION__, (char *) __FILE__, __LINE__)
inline UINT32 vnPostErrorInternal(UINT8 error, CONST CHAR *error_string, CONST CHAR *function, CONST CHAR *filename, UINT32 line) 
{
#ifdef VN_DEBUG      
    CONST CHAR *path = filename;

    for (INT32 i = (INT32) strlen(filename); i >= 0; --i) 
    {
        if (filename[ i ] == '/') 
            break;

        path = &filename[i];    
    }

    VN_ERR("*** RIP *** %s @ %s in %s:%i", error_string, function, path, line);
#endif
    return error;
}

/**********************************************************************************
//
// Standard helpers
//
**********************************************************************************/

#define VN_DISABLE_COPY_AND_ASSIGN(type) \
    type(const type &rvalue); \
    type &operator = (const type &rvalue);

#define VN_TEMPLATE_T                         template <class T>
#define VN_TEMPLATE_SPEC                      template <>

#define VN_VARG(fmt)                          va_list argptr;                             \
                                              CHAR text[1*VN_KB] = {0};                   \
                                              va_start(argptr, fmt);                      \
                                              vsprintf(text, fmt, argptr);                \
                                              va_end(argptr);    

#endif // __VN_BASE_H__