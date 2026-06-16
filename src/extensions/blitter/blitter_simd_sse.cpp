/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SSE2 instantiations of the templated blitter kernel.
 *
 *          Compiled with /arch:SSE2 (see CMakeLists.txt). The kernel body lives
 *          in blitter_simd_impl.h; this TU just pins the SimdTier::SSE2 tier.
 *          Floating-point free.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "blitter_simd_impl.h"

#define INSTANTIATE(cfg) \
    template void Blit_Row<SimdTier::SSE2, cfg>(void*, void const*, int, int, void*, void*, int, int, \
                                                unsigned short const*, unsigned char const*, \
                                                unsigned char const* const*, unsigned short, \
                                                unsigned short const*)

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

/* Alpha families. */
INSTANTIATE(CFG_PlainXlatAlpha);
INSTANTIATE(CFG_TransXlatAlpha);
INSTANTIATE(CFG_ZRemapXlatAlpha);
INSTANTIATE(CFG_Lucent75Alpha);
INSTANTIATE(CFG_Lucent50Alpha);
INSTANTIATE(CFG_Lucent25Alpha);
INSTANTIATE(CFG_TransXlatAlphaZRead);
INSTANTIATE(CFG_ZRemapXlatAlphaZRead);
INSTANTIATE(CFG_Lucent75AlphaZRead);
INSTANTIATE(CFG_Lucent50AlphaZRead);
INSTANTIATE(CFG_Lucent25AlphaZRead);
INSTANTIATE(CFG_TransXlatAlphaZReadWrite);
INSTANTIATE(CFG_ZRemapXlatAlphaZReadWrite);
INSTANTIATE(CFG_Lucent75AlphaZReadWrite);
INSTANTIATE(CFG_Lucent50AlphaZReadWrite);
INSTANTIATE(CFG_Lucent25AlphaZReadWrite);
INSTANTIATE(CFG_Lucent75AlphaZReadWarp);
INSTANTIATE(CFG_Lucent50AlphaZReadWarp);
INSTANTIATE(CFG_Lucent25AlphaZReadWarp);

/* Warp (no alpha). */
INSTANTIATE(CFG_Lucent75ZReadWarp);
INSTANTIATE(CFG_Lucent50ZReadWarp);
INSTANTIATE(CFG_Lucent25ZReadWarp);

#undef INSTANTIATE

#define INSTANTIATE_RLE(cfg) \
    template void RLE_Blit_Row<SimdTier::SSE2, cfg>(void*, void const*, int, int, int, void*, void*, int, int, \
                                                    void const*, unsigned short const*, unsigned char const*, \
                                                    unsigned char const* const*, unsigned short, unsigned short const*)

/* RLE non-Z families. */
INSTANTIATE_RLE(CFG_TransXlat);
INSTANTIATE_RLE(CFG_ZRemapXlat);
INSTANTIATE_RLE(CFG_Darken);
INSTANTIATE_RLE(CFG_Lucent75);
INSTANTIATE_RLE(CFG_Lucent50);
INSTANTIATE_RLE(CFG_Lucent25);

#undef INSTANTIATE_RLE
