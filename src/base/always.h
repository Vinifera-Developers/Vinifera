/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Basic header files and defines that are always needed.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "intrinsics.h"
#include "macros.h"

#include <inttypes.h>
#include <windows.h>
#define NAME_MAX FILENAME_MAX

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif


/**
 *  Enable inline recursion.
 */
#pragma inline_recursion(on)
#pragma inline_depth(255) // Allow lots of inlining.


/**
 *  Alias the ICU unicode functions when not building against it.
 */
#define u_strlen                   wcslen
#define u_strcpy                   wcscpy
#define u_strcat                   wcscat
#define u_vsnprintf_u              vswprintf
#define u_strcmp                   wcscmp
#define u_strcasecmp(x, y, z)      _wcsicmp(x, y)
#define u_isspace                  iswspace
#define u_tolower                  towlower
#define U_COMPARE_CODE_POINT_ORDER 0x8000


/**
 *  Define some stuff here for cross platform consistency.
 */
#define strcasecmp  _stricmp
#define strncasecmp _strnicmp


#if __cplusplus < 201703L
#include <functional>

namespace std
{
    template<class T, class Compare>
    constexpr const T &clamp(const T &v, const T &lo, const T &hi, Compare comp)
    {
        return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
    }

    template<class T>
    constexpr const T &clamp(const T &v, const T &lo, const T &hi)
    {
        return clamp(v, lo, hi, std::less<T>());
    }
}
#endif

#if __cplusplus < 202002L
namespace std
{
    // C++14 compatible replacements for std::bit_width and std::has_single_bit
    template <typename T>
    static constexpr int bit_width_impl(T x, int count = 0)
    {
        return x > 0 ? bit_width_impl(x >> 1, count + 1) : count;
    }

    template <typename T>
    constexpr int bit_width(T x)
    {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return bit_width_impl(x);
    }

    template <typename T>
    constexpr bool has_single_bit(T x)
    {
        static_assert(std::is_integral<T>::value, "T must be an integral type.");
        return x > 0 && (x & (x - 1)) == 0;
    }
}
#endif
