/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Centralises intrinsic includes and implements standard C fallbacks
 *          or inline asm when needed.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "macros.h"

#include <cstdint>
#include <intrin.h>

// CPUID instructions.
// #define __cpuidex(regs, cpuid_type, count) __cpuid_count(cpuid_type, count, regs[0], regs[1], regs[2], regs[3])

// GCC and MSVC use the same name but have different signatures so we use a new common name.
#define __cpuidc __cpuid

// Rotate instructions
#define __rotl8  _rotl8
#define __rotl16 _rotl16
#define __rotr8  _rotr8
#define __rotr16 _rotr16
#define __rotl32 _rotl
#define __rotr32 _rotr
#define __rotl64 _rotr64
#define __rotr64 _rotr64
