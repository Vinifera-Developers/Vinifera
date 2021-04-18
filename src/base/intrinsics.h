/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INTRINSICS.H
 *
 *  @authors       OmniBlade
 *
 *  @brief         Centralises intrinsic includes and implements standard C
 *                 fallbacks or inline asm when needed.
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

#include "macros.h"
#include <cstdint>
#include <intrin.h>

// CPUID instructions.
//#define __cpuidex(regs, cpuid_type, count) __cpuid_count(cpuid_type, count, regs[0], regs[1], regs[2], regs[3])

// GCC and MSVC use the same name but have different signatures so we use a new common name.
#define __cpuidc __cpuid

// Rotate instructions
#define __rotl8 _rotl8
#define __rotl16 _rotl16
#define __rotr8 _rotr8
#define __rotr16 _rotr16
#define __rotl32 _rotl
#define __rotr32 _rotr
#define __rotl64 _rotr64
#define __rotr64 _rotr64
