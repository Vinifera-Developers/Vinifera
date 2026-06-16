/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Hand-written SIMD replacements for the Tiberian Sun pixel blitters.
 *
 *          The ~110 vanilla blitter classes are a cross-product of orthogonal
 *          per-pixel operations (translate, transparency, remap, translucency,
 *          z-test, alpha, warp). Rather than re-implement each one, a single
 *          `if constexpr` row kernel (`Blit_Row`) is parameterised by a
 *          `BlitConfig` (a C++20 structural NTTP) and an ISA tier, and each
 *          named blitter becomes a thin `SimdBlit` alias that derives from the
 *          fully-modelled vanilla class (inheriting its real, table-storing
 *          constructor and its `BlitBackward`) and overrides only `BlitForward`.
 *
 *          Output is bit-exact with the vanilla routines; the SSE2/SSE4.1
 *          kernels live in blitter_simd_sse.cpp and the AVX2 kernel in
 *          blitter_simd_avx2.cpp, each compiled with the matching /arch and
 *          kept strictly floating-point free.
 *
 *          16-bit destination only (T = unsigned short, RGB565/RGB555); this is
 *          the in-game pixel format.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once

#include "blitblit.h"


class ConvertClass;

/**
 *  Bit-exactness self-test (blitter_simd_selftest.cpp): compares every SIMD
 *  family against its bound vanilla blitter using the live drawer's tables.
 */
void Blitter_SIMD_SelfTest(ConvertClass* drawer);


/**
 *  Instruction-set tier chosen at blitter creation time from CPU capability.
 */
enum class SimdTier { Scalar, SSE2, SSE41, AVX2 };

/**
 *  The orthogonal axes that the vanilla blitter taxonomy is a cross-product of.
 */
enum class XlatMode { Direct, Remap, ZRemap };          // TranslateTable[c] | [RemapTable[c]] | [(*RemapTable)[c]]
enum class Blend    { Copy, Darken, L25, L50, L75 };    // see Blit_Row / the vanilla arithmetic

struct BlitConfig {
    XlatMode xlat   = XlatMode::Direct;
    bool     trans  = false;    // skip source pixels equal to 0
    bool     alpha  = false;    // fold AlphaLightingRemap table into the translate index (not yet implemented)
    bool     zread  = false;    // depth test: only write where z_min < *z_buff
    bool     zwrite = false;    // also store z_min into the z-buffer where written
    bool     warp   = false;    // blend against dest[warp_offset] (not yet implemented)
    Blend    blend  = Blend::Copy;
    bool     zwbyte = false;    // z-write truncates to (unsigned char) z_min (BlitPlainXlatZReadWrite only)
};


/**
 *  The single per-scanline kernel. Defined and explicitly instantiated per
 *  (ISA, CFG) in blitter_simd_sse.cpp / blitter_simd_avx2.cpp. The flat
 *  argument list matches the engine's BlitForward ABI; the resolved table
 *  pointers are passed in by the SimdBlit wrapper below.
 */
template<SimdTier ISA, BlitConfig CFG>
void Blit_Row(void* dest, void const* source, int length,
              int z_min, void* z_buff, void* a_buff, int alpha_level, int warp_offset,
              unsigned short const* xlat,
              unsigned char const* remap1,
              unsigned char const* const* remap2,
              unsigned short mask);


/**
 *  Named configs, shared between the alias table and the explicit instantiations
 *  so both reference the same NTTP value.
 */
inline constexpr BlitConfig CFG_PlainXlat  { .xlat = XlatMode::Direct, .trans = false, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_TransXlat  { .xlat = XlatMode::Direct, .trans = true,  .blend = Blend::Copy };
inline constexpr BlitConfig CFG_RemapXlat  { .xlat = XlatMode::Remap,  .trans = true,  .blend = Blend::Copy };
inline constexpr BlitConfig CFG_ZRemapXlat { .xlat = XlatMode::ZRemap, .trans = true,  .blend = Blend::Copy };
inline constexpr BlitConfig CFG_Darken     { .xlat = XlatMode::Direct, .trans = true,  .blend = Blend::Darken };
inline constexpr BlitConfig CFG_Lucent25   { .xlat = XlatMode::Direct, .trans = true,  .blend = Blend::L25 };
inline constexpr BlitConfig CFG_Lucent50   { .xlat = XlatMode::Direct, .trans = true,  .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent75   { .xlat = XlatMode::Direct, .trans = true,  .blend = Blend::L75 };

/* Z-read (depth test, no write). */
inline constexpr BlitConfig CFG_PlainXlatZRead  { .xlat = XlatMode::Direct, .trans = false, .zread = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_TransXlatZRead  { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_ZRemapXlatZRead { .xlat = XlatMode::ZRemap, .trans = true,  .zread = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_DarkenZRead     { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .blend = Blend::Darken };
inline constexpr BlitConfig CFG_Lucent25ZRead   { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .blend = Blend::L25 };
inline constexpr BlitConfig CFG_Lucent50ZRead   { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent75ZRead   { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .blend = Blend::L75 };

/* Z-read/write (depth test + store z where written). PlainXlat truncates z to a byte. */
inline constexpr BlitConfig CFG_PlainXlatZReadWrite  { .xlat = XlatMode::Direct, .trans = false, .zread = true, .zwrite = true, .blend = Blend::Copy,   .zwbyte = true };
inline constexpr BlitConfig CFG_TransXlatZReadWrite  { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .zwrite = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_ZRemapXlatZReadWrite { .xlat = XlatMode::ZRemap, .trans = true,  .zread = true, .zwrite = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_DarkenZReadWrite     { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .zwrite = true, .blend = Blend::Darken };
inline constexpr BlitConfig CFG_Lucent25ZReadWrite   { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .zwrite = true, .blend = Blend::L25 };
inline constexpr BlitConfig CFG_Lucent50ZReadWrite   { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .zwrite = true, .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent75ZReadWrite   { .xlat = XlatMode::Direct, .trans = true,  .zread = true, .zwrite = true, .blend = Blend::L75 };


/**
 *  A SIMD blitter. Derives from the fully-modelled vanilla class `Vanilla`
 *  (e.g. BlitTransZRemapXlat<unsigned short>) so it inherits the real
 *  table-storing constructor and the vanilla BlitBackward (reused verbatim for
 *  the rare reverse-copy path); only BlitForward is overridden with the kernel.
 *  Member access is guarded by `if constexpr` so only the members that exist for
 *  this CFG's class are ever named.
 */
template<SimdTier ISA, class Vanilla, BlitConfig CFG>
class SimdBlit : public Vanilla
{
public:
    using Vanilla::Vanilla;

    virtual void BlitForward(void* dest, void const* source, int length, int z_min,
                             void* z_buff, void* a_buff, int alpha_level, int warp_offset) const override
    {
        unsigned short const* xlat = nullptr;
        unsigned char const* remap1 = nullptr;
        unsigned char const* const* remap2 = nullptr;
        unsigned short mask = 0;

        if constexpr (CFG.blend != Blend::Darken) { xlat = this->TranslateTable; }
        if constexpr (CFG.xlat == XlatMode::Remap) { remap1 = this->RemapTable; }
        if constexpr (CFG.xlat == XlatMode::ZRemap) { remap2 = this->RemapTable; }
        if constexpr (CFG.blend != Blend::Copy) { mask = this->Mask; }

        Blit_Row<ISA, CFG>(dest, source, length, z_min, z_buff, a_buff, alpha_level, warp_offset,
                           xlat, remap1, remap2, mask);
    }
};


/**
 *  The taxonomy: each named blitter maps to a (vanilla base, config) pair.
 *  (First wave: the non-Z/Alpha/Warp families. Z/Alpha/Warp + RLE follow.)
 */
template<SimdTier I> using SimdBlitPlainXlat      = SimdBlit<I, BlitPlainXlat<unsigned short>,      CFG_PlainXlat>;
template<SimdTier I> using SimdBlitTransXlat      = SimdBlit<I, BlitTransXlat<unsigned short>,      CFG_TransXlat>;
template<SimdTier I> using SimdBlitTransRemapXlat = SimdBlit<I, BlitTransRemapXlat<unsigned short>, CFG_RemapXlat>;
template<SimdTier I> using SimdBlitTransZRemapXlat= SimdBlit<I, BlitTransZRemapXlat<unsigned short>,CFG_ZRemapXlat>;
template<SimdTier I> using SimdBlitTransDarken    = SimdBlit<I, BlitTransDarken<unsigned short>,    CFG_Darken>;
template<SimdTier I> using SimdBlitTransLucent25  = SimdBlit<I, BlitTransLucent25<unsigned short>,  CFG_Lucent25>;
template<SimdTier I> using SimdBlitTransLucent50  = SimdBlit<I, BlitTransLucent50<unsigned short>,  CFG_Lucent50>;
template<SimdTier I> using SimdBlitTransLucent75  = SimdBlit<I, BlitTransLucent75<unsigned short>,  CFG_Lucent75>;

template<SimdTier I> using SimdBlitPlainXlatZRead       = SimdBlit<I, BlitPlainXlatZRead<unsigned short>,       CFG_PlainXlatZRead>;
template<SimdTier I> using SimdBlitTransXlatZRead       = SimdBlit<I, BlitTransXlatZRead<unsigned short>,       CFG_TransXlatZRead>;
template<SimdTier I> using SimdBlitTransZRemapXlatZRead = SimdBlit<I, BlitTransZRemapXlatZRead<unsigned short>, CFG_ZRemapXlatZRead>;
template<SimdTier I> using SimdBlitTransDarkenZRead     = SimdBlit<I, BlitTransDarkenZRead<unsigned short>,     CFG_DarkenZRead>;
template<SimdTier I> using SimdBlitTransLucent25ZRead   = SimdBlit<I, BlitTransLucent25ZRead<unsigned short>,   CFG_Lucent25ZRead>;
template<SimdTier I> using SimdBlitTransLucent50ZRead   = SimdBlit<I, BlitTransLucent50ZRead<unsigned short>,   CFG_Lucent50ZRead>;
template<SimdTier I> using SimdBlitTransLucent75ZRead   = SimdBlit<I, BlitTransLucent75ZRead<unsigned short>,   CFG_Lucent75ZRead>;

template<SimdTier I> using SimdBlitPlainXlatZReadWrite       = SimdBlit<I, BlitPlainXlatZReadWrite<unsigned short>,       CFG_PlainXlatZReadWrite>;
template<SimdTier I> using SimdBlitTransXlatZReadWrite       = SimdBlit<I, BlitTransXlatZReadWrite<unsigned short>,       CFG_TransXlatZReadWrite>;
template<SimdTier I> using SimdBlitTransZRemapXlatZReadWrite = SimdBlit<I, BlitTransZRemapXlatZReadWrite<unsigned short>, CFG_ZRemapXlatZReadWrite>;
template<SimdTier I> using SimdBlitTransDarkenZReadWrite     = SimdBlit<I, BlitTransDarkenZReadWrite<unsigned short>,     CFG_DarkenZReadWrite>;
template<SimdTier I> using SimdBlitTransLucent25ZReadWrite   = SimdBlit<I, BlitTransLucent25ZReadWrite<unsigned short>,   CFG_Lucent25ZReadWrite>;
template<SimdTier I> using SimdBlitTransLucent50ZReadWrite   = SimdBlit<I, BlitTransLucent50ZReadWrite<unsigned short>,   CFG_Lucent50ZReadWrite>;
template<SimdTier I> using SimdBlitTransLucent75ZReadWrite   = SimdBlit<I, BlitTransLucent75ZReadWrite<unsigned short>,   CFG_Lucent75ZReadWrite>;
