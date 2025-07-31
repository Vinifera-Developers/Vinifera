/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MACROS.H
 *
 *  @authors       CCHyper, OmniBlade
 *
 *  @brief         Common utility macros.
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

// For counting variadic macro arguments.
#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define VA_NARGS2(...) ((int)(sizeof((int[]){ __VA_ARGS__ })/sizeof(int)))

// The ubiquitous stringify macros for formatting strings.
#ifndef STRINGIZE
#define STRINGIZE_HELPER(str) #str
#define STRINGIZE(str) STRINGIZE_HELPER(str)
#define STRINGIZE_JOIN(str1, str2) STRINGIZE_HELPER(str1 ## str2)
#endif // STRINGIZE

// Define some C++ keywords when standard is less than C++11, mainly for watcom support
#if __cplusplus <= 199711L && (!defined _MSC_VER || _MSC_VER < 1600)
#define nullptr NULL
#define override
#define final
#define static_assert(x, ...)
#define constexpr
#define noexcept
#endif

// These allow evaluation of compiler specific attributes and intrinics on GCC like compilers.
// If they don't exist we want them to evaluate to false.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

// This section defines some keywords controlling inlining and unused variables
// where the keywords needed differ between compilers.
#define __noinline __declspec(noinline)
#define __unused __pragma(warning(suppress : 4100 4101))
#define __mayalias
#define __noreturn __declspec(noreturn)
#define __nothrow __declspec(nothrow)
#define __selectany __declspec(selectany)
#define __novtable __declspec(novtable)

/**
 *  Defines operator overloads to enable bit operations on enum values, useful for
 *  using an enum to define flags for a bitfield.
 *  
 *  Example usage:
 *   enum MyEnum {
 *       ENUM_A = 0,
 *       ENUM_B = 1,
 *       ENUM_C = 2,
 *   };
 *  
 *   DEFINE_ENUMERATION_BITWISE_OPERATORS(MyEnum);
 */
#ifdef __cplusplus
#if !defined(DEFINE_ENUMERATION_BITWISE_OPERATORS)
#define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) \
    extern "C++" { \
    __forceinline constexpr ENUMTYPE operator|(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return ENUMTYPE(((int)a) | ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator&(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return ENUMTYPE(((int)a) & ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator~(ENUMTYPE const a) { return ENUMTYPE(~((int)a)); } \
    __forceinline constexpr ENUMTYPE operator^(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return ENUMTYPE(((int)a) ^ ((int)b)); \
    } \
    __forceinline ENUMTYPE &operator^=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) ^= ((int &)b)); } \
    __forceinline ENUMTYPE &operator&=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) &= ((int &)b)); } \
    __forceinline ENUMTYPE &operator|=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) |= ((int &)b)); } \
    __forceinline constexpr ENUMTYPE operator<<(ENUMTYPE a, int const b) { return (ENUMTYPE)(((int)a) << ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator>>(ENUMTYPE a, int const b) { return (ENUMTYPE)(((int)a) >> ((int)b)); } \
    __forceinline ENUMTYPE &operator<<=(ENUMTYPE &a, int const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) << ((int)b)); \
    } \
    __forceinline ENUMTYPE &operator>>=(ENUMTYPE &a, int const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) >> ((int)b)); \
    } \
    }
#endif // !DEFINE_ENUMERATION_BITWISE_OPERATORS
#else
#define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus

/**
 *  Defines operator overloads to enable stadard math operations on an enum.
 *  Useful when an enum represents a range that can be iterated over.
 *  
 *  Example usage:
 *   enum MyEnum {
 *       ENUM_A = 0,
 *       ENUM_B = 1,
 *       ENUM_C = 2,
 *   };
 *  
 *   DEFINE_ENUMERATION_OPERATORS(MyEnum);
 */
#ifdef __cplusplus
#if !defined(DEFINE_ENUMERATION_OPERATORS)
#define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) \
    extern "C++" { \
    __forceinline ENUMTYPE operator++(ENUMTYPE const &a) { return (ENUMTYPE)(++((int &)a)); } \
    __forceinline ENUMTYPE operator--(ENUMTYPE const &a) { return (ENUMTYPE)(--((int &)a)); } \
    __forceinline constexpr ENUMTYPE operator+(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) + ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator-(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) - ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator*(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) * ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator/(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) / ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator%(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) % ((int)b)); \
    } \
    __forceinline ENUMTYPE &operator+=(ENUMTYPE &a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) + ((ENUMTYPE)b)); \
    } \
    __forceinline ENUMTYPE &operator-=(ENUMTYPE &a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) - ((ENUMTYPE)b)); \
    } \
    __forceinline ENUMTYPE operator++(ENUMTYPE const &a, int const b) { return (ENUMTYPE)(++((int &)a)); } \
    __forceinline ENUMTYPE operator--(ENUMTYPE const &a, int const b) { return (ENUMTYPE)(--((int &)a)); } \
    __forceinline constexpr ENUMTYPE operator+(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) + ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator-(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) - ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator*(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) * ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator/(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) / ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator%(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) % ((int)b)); } \
    __forceinline ENUMTYPE &operator+=(ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) + ((int)b)); } \
    __forceinline ENUMTYPE &operator-=(ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) - ((int)b)); } \
    }
#endif // !DEFINE_ENUMERATION_OPERATORS
#else
#define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus


// Statements like:
// #pragma message(Reminder "Fix this problem!")
// Which will cause messages like:
// C:\Source\Project\main.cpp(47): Reminder: Fix this problem!
// to show up during compiles. Note that you can NOT use the
// words "error" or "warning" in your reminders, since it will
// make the IDE think it should abort execution. You can double
// click on these messages and jump to the line in question.
#define MakeString(M, L) M(L)
#define $Line MakeString(STRINGIZE, __LINE__)
#define Reminder __FILE__ "(" $Line ") : Reminder: "
