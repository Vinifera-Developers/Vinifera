/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  AVX2 instantiations of the templated blitter kernel.
 *
 *          Compiled with /arch:AVX2 (see CMakeLists.txt). MSVC may VEX-encode
 *          everything in this TU, so it must only ever run on AVX2 CPUs -- which
 *          is guaranteed by the creation-time CPU-tier guard (these
 *          instantiations are only constructed when Has_AVX2 is true). The
 *          kernel body lives in blitter_simd_impl.h. Floating-point free.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "blitter_simd_impl.h"

#define INSTANTIATE(cfg) \
    template void Blit_Row<SimdTier::AVX2, cfg>(void*, void const*, int, int, void*, void*, int, int, \
                                                unsigned short const*, unsigned char const*, \
                                                unsigned char const* const*, unsigned short)

/* Non-Z families. */
INSTANTIATE(CFG_PlainXlat);
INSTANTIATE(CFG_TransXlat);
INSTANTIATE(CFG_RemapXlat);
INSTANTIATE(CFG_ZRemapXlat);
INSTANTIATE(CFG_Darken);
INSTANTIATE(CFG_Lucent25);
INSTANTIATE(CFG_Lucent50);
INSTANTIATE(CFG_Lucent75);

/* Z-read families. */
INSTANTIATE(CFG_PlainXlatZRead);
INSTANTIATE(CFG_TransXlatZRead);
INSTANTIATE(CFG_ZRemapXlatZRead);
INSTANTIATE(CFG_DarkenZRead);
INSTANTIATE(CFG_Lucent25ZRead);
INSTANTIATE(CFG_Lucent50ZRead);
INSTANTIATE(CFG_Lucent75ZRead);

/* Z-read/write families. */
INSTANTIATE(CFG_PlainXlatZReadWrite);
INSTANTIATE(CFG_TransXlatZReadWrite);
INSTANTIATE(CFG_ZRemapXlatZReadWrite);
INSTANTIATE(CFG_DarkenZReadWrite);
INSTANTIATE(CFG_Lucent25ZReadWrite);
INSTANTIATE(CFG_Lucent50ZReadWrite);
INSTANTIATE(CFG_Lucent75ZReadWrite);

#undef INSTANTIATE
