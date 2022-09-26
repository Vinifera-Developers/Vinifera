/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_SAVELOAD.H
 *
 *  @authors       CCHyper
 *
 *  @brief         
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

#include "always.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "wstring.h"
#include "swizzle.h"
#include "newswizzle.h"


/**
 *  Wrappers for the new swizzle manager for providing debug information.
 */
#ifdef VINIFERA_USE_NEW_SWIZZLE_MANAGER

#define VINIFERA_SWIZZLE_RESET(func) \
    { \
        ((ViniferaSwizzleManagerClass &)SwizzleManager).Reset(); \
    }

#define VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(pointer, variable) \
    { \
        Wstring funcname = __FUNCTION__; \
        funcname += "()"; \
        ((ViniferaSwizzleManagerClass &)SwizzleManager).Swizzle_Dbg(pointer, __FILE__, __LINE__, funcname.Peek_Buffer(), variable); \
    }

#define VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(vector, variable) \
    { \
        Wstring funcname = __FUNCTION__; \
        funcname += "()"; \
        for (int i = 0; i < vector.Count(); ++i) { \
            ((ViniferaSwizzleManagerClass &)SwizzleManager).Swizzle_Dbg((void **)&vector[i], __FILE__, __LINE__, funcname.Peek_Buffer(), variable); \
        } \
    }

#define VINIFERA_SWIZZLE_FETCH_SWIZZLE_ID(pointer, id, variable) \
    { \
        Wstring funcname = __FUNCTION__; \
        funcname += "()"; \
        ((ViniferaSwizzleManagerClass &)SwizzleManager).Fetch_Swizzle_ID_Dbg(pointer, id, __FILE__, __LINE__, funcname.Peek_Buffer(), variable); \
    }

#define VINIFERA_SWIZZLE_REGISTER_POINTER(id, pointer, variable) \
    { \
        Wstring funcname = __FUNCTION__; \
        funcname += "()"; \
        ((ViniferaSwizzleManagerClass &)SwizzleManager).Here_I_Am_Dbg(id, pointer, __FILE__, __LINE__, funcname.Peek_Buffer(), variable); \
    }

#else
#define VINIFERA_SWIZZLE_RESET(func)                                      SWIZZLE_RESET(func);
#define VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(pointer, variable)         SWIZZLE_REQUEST_POINTER_REMAP(pointer);
#define VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(vector, variable)     SWIZZLE_REQUEST_POINTER_REMAP_LIST(vector, variable)
#define VINIFERA_SWIZZLE_FETCH_SWIZZLE_ID(pointer, id, variable)          SWIZZLE_FETCH_POINTER_ID(pointer, id);
#define VINIFERA_SWIZZLE_REGISTER_POINTER(id, pointer, variable)          SWIZZLE_REGISTER_POINTER(id, pointer);
#endif
