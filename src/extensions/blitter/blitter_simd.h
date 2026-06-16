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
    bool     alpha  = false;    // fold AlphaLightingRemap::Get_Table(level)[*a_buff] into the translate index
    bool     zread  = false;    // depth test: only write where z_min < *z_buff
    bool     zwrite = false;    // also store z_min into the z-buffer where written
    bool     warp   = false;    // blend against dest[warp_offset] (the already-written background)
    Blend    blend  = Blend::Copy;
    bool     zwbyte = false;    // z-write truncates to (unsigned char) z_min (BlitPlainXlatZReadWrite only)
    bool     alpha_static = false;  // engine bug (BlitTransZRemapXlatAlphaZReadWrite): a_buff ptr never advances -> every pixel reads a_buff[0]
    bool     remapdest = false;     // RLE only: *dest = RemapTable16[*dest] (the source byte only marks opacity)
    bool     rle_skip_noz = false;  // RLE engine bug (Lucent75/25 ZReadWrite): transparent runs do NOT advance z/zshape -> they lag dptr
    bool     rle_zs2 = false;       // RLE engine bug (ZRemapXlatZReadWrite): opaque pixels advance zshape by 2 and z-write samples zshape[+1]
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
              unsigned short mask,
              unsigned short const* alut);


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

/* Alpha (intensity lighting: index |= AlphaLightingRemap::Get_Table(level)[*a_buff]). */
inline constexpr BlitConfig CFG_PlainXlatAlpha  { .xlat = XlatMode::Direct, .trans = false, .alpha = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_TransXlatAlpha  { .xlat = XlatMode::Direct, .trans = true,  .alpha = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_ZRemapXlatAlpha { .xlat = XlatMode::ZRemap, .trans = true,  .alpha = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_Lucent75Alpha   { .xlat = XlatMode::Direct, .trans = true,  .alpha = true, .blend = Blend::L75 };
inline constexpr BlitConfig CFG_Lucent50Alpha   { .xlat = XlatMode::Direct, .trans = true,  .alpha = true, .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent25Alpha   { .xlat = XlatMode::Direct, .trans = true,  .alpha = true, .blend = Blend::L25 };

/* Alpha + Z-read. */
inline constexpr BlitConfig CFG_TransXlatAlphaZRead  { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_ZRemapXlatAlphaZRead { .xlat = XlatMode::ZRemap, .trans = true, .alpha = true, .zread = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_Lucent75AlphaZRead   { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .blend = Blend::L75 };
inline constexpr BlitConfig CFG_Lucent50AlphaZRead   { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent25AlphaZRead   { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .blend = Blend::L25 };

/* Alpha + Z-read/write. */
inline constexpr BlitConfig CFG_TransXlatAlphaZReadWrite  { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .zwrite = true, .blend = Blend::Copy };
inline constexpr BlitConfig CFG_ZRemapXlatAlphaZReadWrite { .xlat = XlatMode::ZRemap, .trans = true, .alpha = true, .zread = true, .zwrite = true, .blend = Blend::Copy, .alpha_static = true };
inline constexpr BlitConfig CFG_Lucent75AlphaZReadWrite   { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .zwrite = true, .blend = Blend::L75 };
inline constexpr BlitConfig CFG_Lucent50AlphaZReadWrite   { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .zwrite = true, .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent25AlphaZReadWrite   { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .zwrite = true, .blend = Blend::L25 };

/* Alpha + Z-read + Warp (background read from dest[warp_offset]). */
inline constexpr BlitConfig CFG_Lucent75AlphaZReadWarp { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .warp = true, .blend = Blend::L75 };
inline constexpr BlitConfig CFG_Lucent50AlphaZReadWarp { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .warp = true, .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent25AlphaZReadWarp { .xlat = XlatMode::Direct, .trans = true, .alpha = true, .zread = true, .warp = true, .blend = Blend::L25 };

/* Z-read + Warp (no alpha). */
inline constexpr BlitConfig CFG_Lucent75ZReadWarp { .xlat = XlatMode::Direct, .trans = true, .zread = true, .warp = true, .blend = Blend::L75 };
inline constexpr BlitConfig CFG_Lucent50ZReadWarp { .xlat = XlatMode::Direct, .trans = true, .zread = true, .warp = true, .blend = Blend::L50 };
inline constexpr BlitConfig CFG_Lucent25ZReadWarp { .xlat = XlatMode::Direct, .trans = true, .zread = true, .warp = true, .blend = Blend::L25 };


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
        unsigned short const* alut = nullptr;

        if constexpr (CFG.blend != Blend::Darken) { xlat = this->TranslateTable; }
        if constexpr (CFG.xlat == XlatMode::Remap) { remap1 = this->RemapTable; }
        if constexpr (CFG.xlat == XlatMode::ZRemap) { remap2 = this->RemapTable; }
        if constexpr (CFG.blend != Blend::Copy) { mask = this->Mask; }
        if constexpr (CFG.alpha) { alut = this->AlphaLightingRemap->Get_Table(alpha_level); }

        Blit_Row<ISA, CFG>(dest, source, length, z_min, z_buff, a_buff, alpha_level, warp_offset,
                           xlat, remap1, remap2, mask, alut);
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

template<SimdTier I> using SimdBlitPlainXlatAlpha       = SimdBlit<I, BlitPlainXlatAlpha<unsigned short>,       CFG_PlainXlatAlpha>;
template<SimdTier I> using SimdBlitTransXlatAlpha       = SimdBlit<I, BlitTransXlatAlpha<unsigned short>,       CFG_TransXlatAlpha>;
template<SimdTier I> using SimdBlitTransZRemapXlatAlpha = SimdBlit<I, BlitTransZRemapXlatAlpha<unsigned short>, CFG_ZRemapXlatAlpha>;
template<SimdTier I> using SimdBlitTransLucent75Alpha   = SimdBlit<I, BlitTransLucent75Alpha<unsigned short>,   CFG_Lucent75Alpha>;
template<SimdTier I> using SimdBlitTransLucent50Alpha   = SimdBlit<I, BlitTransLucent50Alpha<unsigned short>,   CFG_Lucent50Alpha>;
template<SimdTier I> using SimdBlitTransLucent25Alpha   = SimdBlit<I, BlitTransLucent25Alpha<unsigned short>,   CFG_Lucent25Alpha>;

template<SimdTier I> using SimdBlitTransXlatAlphaZRead       = SimdBlit<I, BlitTransXlatAlphaZRead<unsigned short>,       CFG_TransXlatAlphaZRead>;
template<SimdTier I> using SimdBlitTransZRemapXlatAlphaZRead = SimdBlit<I, BlitTransZRemapXlatAlphaZRead<unsigned short>, CFG_ZRemapXlatAlphaZRead>;
template<SimdTier I> using SimdBlitTransLucent75AlphaZRead   = SimdBlit<I, BlitTransLucent75AlphaZRead<unsigned short>,   CFG_Lucent75AlphaZRead>;
template<SimdTier I> using SimdBlitTransLucent50AlphaZRead   = SimdBlit<I, BlitTransLucent50AlphaZRead<unsigned short>,   CFG_Lucent50AlphaZRead>;
template<SimdTier I> using SimdBlitTransLucent25AlphaZRead   = SimdBlit<I, BlitTransLucent25AlphaZRead<unsigned short>,   CFG_Lucent25AlphaZRead>;

template<SimdTier I> using SimdBlitTransXlatAlphaZReadWrite       = SimdBlit<I, BlitTransXlatAlphaZReadWrite<unsigned short>,       CFG_TransXlatAlphaZReadWrite>;
template<SimdTier I> using SimdBlitTransZRemapXlatAlphaZReadWrite = SimdBlit<I, BlitTransZRemapXlatAlphaZReadWrite<unsigned short>, CFG_ZRemapXlatAlphaZReadWrite>;
template<SimdTier I> using SimdBlitTransLucent75AlphaZReadWrite   = SimdBlit<I, BlitTransLucent75AlphaZReadWrite<unsigned short>,   CFG_Lucent75AlphaZReadWrite>;
template<SimdTier I> using SimdBlitTransLucent50AlphaZReadWrite   = SimdBlit<I, BlitTransLucent50AlphaZReadWrite<unsigned short>,   CFG_Lucent50AlphaZReadWrite>;
template<SimdTier I> using SimdBlitTransLucent25AlphaZReadWrite   = SimdBlit<I, BlitTransLucent25AlphaZReadWrite<unsigned short>,   CFG_Lucent25AlphaZReadWrite>;

template<SimdTier I> using SimdBlitTransLucent75AlphaZReadWarp = SimdBlit<I, BlitTransLucent75AlphaZReadWarp<unsigned short>, CFG_Lucent75AlphaZReadWarp>;
template<SimdTier I> using SimdBlitTransLucent50AlphaZReadWarp = SimdBlit<I, BlitTransLucent50AlphaZReadWarp<unsigned short>, CFG_Lucent50AlphaZReadWarp>;
template<SimdTier I> using SimdBlitTransLucent25AlphaZReadWarp = SimdBlit<I, BlitTransLucent25AlphaZReadWarp<unsigned short>, CFG_Lucent25AlphaZReadWarp>;

template<SimdTier I> using SimdBlitTransLucent75ZReadWarp = SimdBlit<I, BlitTransLucent75ZReadWarp<unsigned short>, CFG_Lucent75ZReadWarp>;
template<SimdTier I> using SimdBlitTransLucent50ZReadWarp = SimdBlit<I, BlitTransLucent50ZReadWarp<unsigned short>, CFG_Lucent50ZReadWarp>;
template<SimdTier I> using SimdBlitTransLucent25ZReadWarp = SimdBlit<I, BlitTransLucent25ZReadWarp<unsigned short>, CFG_Lucent25ZReadWarp>;
template<SimdTier I> using SimdBlitTransLucent50ZReadWrite   = SimdBlit<I, BlitTransLucent50ZReadWrite<unsigned short>,   CFG_Lucent50ZReadWrite>;
template<SimdTier I> using SimdBlitTransLucent75ZReadWrite   = SimdBlit<I, BlitTransLucent75ZReadWrite<unsigned short>,   CFG_Lucent75ZReadWrite>;


/* ====================================================================================
 *  RLE blitters
 *
 *  RLE shapes (units, buildings, infantry, trees) are a byte stream: a 0x00 byte
 *  followed by a count = skip that many transparent pixels; any non-zero byte = one
 *  opaque pixel whose value is the palette index. The decode is inherently scalar, but
 *  a contiguous run of opaque (non-zero) bytes is translated + blended + stored wide.
 *
 *  These reuse the very same BlitConfig values as the standard families above (the
 *  per-pixel compose is identical) and a separate row kernel (RLE_Blit_Row) that owns
 *  the decode loop, leadskip clipping, the zshape depth offset and the per-token ring
 *  wrap.
 * ==================================================================================== */

/**
 *  The RLE per-scanline kernel. Defined/instantiated alongside Blit_Row in the SSE2 and
 *  AVX2 TUs. `xlat` doubles as the 16-bit dest-remap table for the RemapDest families;
 *  `zshape` is the per-pixel signed depth bias (z = z_min - *zshape).
 */
template<SimdTier ISA, BlitConfig CFG>
void RLE_Blit_Row(void* dest, void const* source, int length, int leadskip, int z_min,
                  void* z_buff, void* a_buff, int alpha_level, int warp_offset, void const* zshape,
                  unsigned short const* xlat,
                  unsigned char const* remap1,
                  unsigned char const* const* remap2,
                  unsigned short mask,
                  unsigned short const* alut);


/**
 *  A SIMD RLE blitter. Derives from the modelled vanilla RLE class (inheriting its real
 *  table-storing constructor) and overrides only Blit. RLEBlitter has no BlitBackward.
 */
template<SimdTier ISA, class Vanilla, BlitConfig CFG>
class SimdRLEBlit : public Vanilla
{
public:
    using Vanilla::Vanilla;

    virtual void Blit(void* dest, void const* source, int length, int leadskip, int z_min,
                      int z_buff, int a_buff, int alpha_level, int warp_offset, int zshape) const override
    {
        unsigned short const* xlat = nullptr;
        unsigned char const* remap1 = nullptr;
        unsigned char const* const* remap2 = nullptr;
        unsigned short mask = 0;
        unsigned short const* alut = nullptr;

        if constexpr (CFG.remapdest) {
            xlat = this->RemapTable;                 // T const* : 16-bit dest remap
        } else {
            if constexpr (CFG.blend != Blend::Darken) { xlat = this->TranslateTable; }
            if constexpr (CFG.xlat == XlatMode::Remap)  { remap1 = this->RemapTable; }
            if constexpr (CFG.xlat == XlatMode::ZRemap) { remap2 = this->RemapTable; }
        }
        if constexpr (CFG.blend != Blend::Copy) { mask = this->Mask; }
        if constexpr (CFG.alpha) { alut = this->AlphaLightingRemap->Get_Table(alpha_level); }

        RLE_Blit_Row<ISA, CFG>(dest, source, length, leadskip, z_min,
                               reinterpret_cast<void*>(z_buff), reinterpret_cast<void*>(a_buff),
                               alpha_level, warp_offset, reinterpret_cast<void const*>(zshape),
                               xlat, remap1, remap2, mask, alut);
    }
};


/* The RLE taxonomy (non-Z wave). Reuses the standard CFG values verbatim. */
template<SimdTier I> using SimdRLEBlitTransXlat       = SimdRLEBlit<I, RLEBlitTransXlat<unsigned short>,       CFG_TransXlat>;
template<SimdTier I> using SimdRLEBlitTransZRemapXlat = SimdRLEBlit<I, RLEBlitTransZRemapXlat<unsigned short>, CFG_ZRemapXlat>;
template<SimdTier I> using SimdRLEBlitTransDarken     = SimdRLEBlit<I, RLEBlitTransDarken<unsigned short>,     CFG_Darken>;
template<SimdTier I> using SimdRLEBlitTransLucent75   = SimdRLEBlit<I, RLEBlitTransLucent75<unsigned short>,   CFG_Lucent75>;
template<SimdTier I> using SimdRLEBlitTransLucent50   = SimdRLEBlit<I, RLEBlitTransLucent50<unsigned short>,   CFG_Lucent50>;
template<SimdTier I> using SimdRLEBlitTransLucent25   = SimdRLEBlit<I, RLEBlitTransLucent25<unsigned short>,   CFG_Lucent25>;

/* RLE Z-read. */
template<SimdTier I> using SimdRLEBlitTransXlatZRead       = SimdRLEBlit<I, RLEBlitTransXlatZRead<unsigned short>,       CFG_TransXlatZRead>;
template<SimdTier I> using SimdRLEBlitTransZRemapXlatZRead = SimdRLEBlit<I, RLEBlitTransZRemapXlatZRead<unsigned short>, CFG_ZRemapXlatZRead>;
template<SimdTier I> using SimdRLEBlitTransDarkenZRead     = SimdRLEBlit<I, RLEBlitTransDarkenZRead<unsigned short>,     CFG_DarkenZRead>;
template<SimdTier I> using SimdRLEBlitTransLucent75ZRead   = SimdRLEBlit<I, RLEBlitTransLucent75ZRead<unsigned short>,   CFG_Lucent75ZRead>;
template<SimdTier I> using SimdRLEBlitTransLucent50ZRead   = SimdRLEBlit<I, RLEBlitTransLucent50ZRead<unsigned short>,   CFG_Lucent50ZRead>;
template<SimdTier I> using SimdRLEBlitTransLucent25ZRead   = SimdRLEBlit<I, RLEBlitTransLucent25ZRead<unsigned short>,   CFG_Lucent25ZRead>;

/* RLE Z-read/write (stored depth = z_min - *zshape). The three families below carry distinct
 * shipped engine bugs in their depth bookkeeping; the SIMD must reproduce them bit-for-bit. */
inline constexpr BlitConfig CFG_RLE_ZRemapXlatZReadWrite { .xlat = XlatMode::ZRemap, .trans = true, .zread = true, .zwrite = true, .blend = Blend::Copy, .rle_zs2 = true };
inline constexpr BlitConfig CFG_RLE_Lucent75ZReadWrite   { .xlat = XlatMode::Direct, .trans = true, .zread = true, .zwrite = true, .blend = Blend::L75, .rle_skip_noz = true };
inline constexpr BlitConfig CFG_RLE_Lucent25ZReadWrite   { .xlat = XlatMode::Direct, .trans = true, .zread = true, .zwrite = true, .blend = Blend::L25, .rle_skip_noz = true };

template<SimdTier I> using SimdRLEBlitTransXlatZReadWrite       = SimdRLEBlit<I, RLEBlitTransXlatZReadWrite<unsigned short>,       CFG_TransXlatZReadWrite>;
template<SimdTier I> using SimdRLEBlitTransZRemapXlatZReadWrite = SimdRLEBlit<I, RLEBlitTransZRemapXlatZReadWrite<unsigned short>, CFG_RLE_ZRemapXlatZReadWrite>;
template<SimdTier I> using SimdRLEBlitTransDarkenZReadWrite     = SimdRLEBlit<I, RLEBlitTransDarkenZReadWrite<unsigned short>,     CFG_DarkenZReadWrite>;
template<SimdTier I> using SimdRLEBlitTransLucent75ZReadWrite   = SimdRLEBlit<I, RLEBlitTransLucent75ZReadWrite<unsigned short>,   CFG_RLE_Lucent75ZReadWrite>;
template<SimdTier I> using SimdRLEBlitTransLucent50ZReadWrite   = SimdRLEBlit<I, RLEBlitTransLucent50ZReadWrite<unsigned short>,   CFG_Lucent50ZReadWrite>;
template<SimdTier I> using SimdRLEBlitTransLucent25ZReadWrite   = SimdRLEBlit<I, RLEBlitTransLucent25ZReadWrite<unsigned short>,   CFG_RLE_Lucent25ZReadWrite>;
