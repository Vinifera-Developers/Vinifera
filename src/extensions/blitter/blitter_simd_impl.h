/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Shared implementation of the templated blitter kernel.
 *
 *          Included by exactly two translation units, each compiled with its own
 *          /arch and explicitly instantiating one tier:
 *              blitter_simd_sse.cpp   -> /arch:SSE2  -> Blit_Row<SSE2,  ...>
 *              blitter_simd_avx2.cpp  -> /arch:AVX2  -> Blit_Row<AVX2,  ...>
 *          The `if constexpr (ISA == SimdTier::AVX2)` paths reference AVX2
 *          intrinsics; in the SSE2 TU those branches are discarded before code
 *          generation, so /arch:SSE2 never has to emit a VEX instruction.
 *
 *          Both TUs MUST remain strictly floating-point free.
 *
 *          The 8->16-bit palette translate is a gather. On SSE2 it is a tight
 *          scalar lookup; on AVX2 it uses vpgatherdd against a cached, zero-
 *          extended u32 shadow of the translate table (so the 16-bit gather is
 *          fully in-bounds and safe). The transparency mask, translucency blend,
 *          z-test and masked store are vectorised (8 wide on SSE2, 16 wide on
 *          AVX2). All arithmetic reproduces the vanilla integer expressions
 *          exactly (non-saturating adds), so output is bit-identical.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once

#include "blitter_simd.h"
#include "zbuffer.h"
#include "abuffer.h"
#include "dsurface.h"   // channel shift statics for the alpha-composite family

#include <immintrin.h>  // SSE2 .. AVX2
#include <cstdint>
#include <cstdlib>      // malloc (AVX2 shadow table)


extern ZBuffer*& DepthBuffer;
extern ABuffer*& AlphaBuffer;


namespace
{

/**
 *  Translate one 8-bit source pixel to its 16-bit destination colour.
 */
template<BlitConfig CFG>
static inline unsigned short Translate1(unsigned char c, unsigned short const* xlat,
                                        unsigned char const* remap1, unsigned char const* const* remap2)
{
    unsigned idx;
    if constexpr (CFG.xlat == XlatMode::Direct)     idx = c;
    else if constexpr (CFG.xlat == XlatMode::Remap) idx = remap1[c];
    else                                            idx = (*remap2)[c];
    return xlat[idx];
}

/**
 *  Apply the blend for one pixel given the already-translated source colour `s`
 *  and the background `bg` (= *d normally, or dest[warp_offset] for warp blits).
 */
template<BlitConfig CFG>
static inline void Blend1_bg(unsigned short* d, unsigned short s, unsigned short bg, unsigned short mask)
{
    if constexpr (CFG.blend == Blend::Copy) {
        *d = s;
    } else if constexpr (CFG.blend == Blend::Darken) {
        *d = (unsigned short)((bg >> 1) & mask);
    } else if constexpr (CFG.blend == Blend::L50) {
        *d = (unsigned short)(((bg >> 1) & mask) + ((s >> 1) & mask));
    } else if constexpr (CFG.blend == Blend::L25) {
        unsigned short qs = (unsigned short)((s >> 2) & mask);
        unsigned short qd = (unsigned short)((bg >> 2) & mask);
        *d = (unsigned short)(qd + qs + qs + qs);
    } else { // L75
        unsigned short qs = (unsigned short)((s >> 2) & mask);
        unsigned short qd = (unsigned short)((bg >> 2) & mask);
        *d = (unsigned short)(qd + qd + qd + qs);
    }
}

template<BlitConfig CFG>
static inline void Blend1(unsigned short* d, unsigned short s, unsigned short mask)
{
    Blend1_bg<CFG>(d, s, *d, mask);
}

/**
 *  Scalar reference compose for one pixel (no z). Used for the row tail.
 */
template<BlitConfig CFG>
static inline void Compose_Scalar(unsigned short* d, unsigned char c,
                                  unsigned short const* xlat, unsigned char const* remap1,
                                  unsigned char const* const* remap2, unsigned short mask)
{
    if constexpr (CFG.trans) {
        if (c == 0) return;
    }
    unsigned short s = 0;
    if constexpr (CFG.blend != Blend::Darken) {
        s = Translate1<CFG>(c, xlat, remap1, remap2);
    }
    Blend1<CFG>(d, s, mask);
}


/* ----------------------------- SSE2 (128-bit, 8 wide) ----------------------------- */

template<BlitConfig CFG>
static inline __m128i Translate8(unsigned char const* src, unsigned short const* xlat,
                                 unsigned char const* remap1, unsigned char const* const* remap2)
{
    unsigned short tmp[8];
    for (int k = 0; k < 8; ++k) {
        tmp[k] = Translate1<CFG>(src[k], xlat, remap1, remap2);
    }
    return _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp));
}

static inline __m128i Opaque8(unsigned char const* src, __m128i ones)
{
    __m128i vb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(src));   // 8 source bytes
    __m128i istrans = _mm_cmpeq_epi8(vb, _mm_setzero_si128());             // 0xFF where c == 0
    __m128i istrans16 = _mm_unpacklo_epi8(istrans, istrans);              // expand low 8 -> 8 words
    return _mm_xor_si128(istrans16, ones);                                // invert -> opaque
}

template<BlitConfig CFG>
static inline __m128i Blend8(__m128i vdst, __m128i vsrc16, __m128i vmask)
{
    if constexpr (CFG.blend == Blend::Copy) {
        return vsrc16;
    } else if constexpr (CFG.blend == Blend::Darken) {
        return _mm_and_si128(_mm_srli_epi16(vdst, 1), vmask);
    } else if constexpr (CFG.blend == Blend::L50) {
        return _mm_add_epi16(_mm_and_si128(_mm_srli_epi16(vdst, 1), vmask),
                             _mm_and_si128(_mm_srli_epi16(vsrc16, 1), vmask));
    } else if constexpr (CFG.blend == Blend::L25) {
        __m128i qs = _mm_and_si128(_mm_srli_epi16(vsrc16, 2), vmask);
        __m128i qd = _mm_and_si128(_mm_srli_epi16(vdst, 2), vmask);
        return _mm_add_epi16(qd, _mm_add_epi16(qs, _mm_add_epi16(qs, qs)));   // qd + 3*qs
    } else { // L75
        __m128i qs = _mm_and_si128(_mm_srli_epi16(vsrc16, 2), vmask);
        __m128i qd = _mm_and_si128(_mm_srli_epi16(vdst, 2), vmask);
        return _mm_add_epi16(_mm_add_epi16(qd, qd), _mm_add_epi16(qd, qs));   // 3*qd + qs
    }
}


/* ----------------------------- AVX2 (256-bit, 16 wide) ---------------------------- */

/**
 *  Cached zero-extended u32 shadow of a 256-entry translate table, so vpgatherdd
 *  (which gathers 32-bit lanes) reads fully in bounds. Tables live for the
 *  drawer's lifetime and there are only a few, so a tiny single-threaded cache
 *  suffices (blitting is on the render thread only).
 */
template<SimdTier ISA>
[[maybe_unused]] static const uint32_t* Get_U32_Shadow(unsigned short const* table)
{
    struct Entry { unsigned short const* key; uint32_t* data; };
    static Entry cache[8] = {};
    for (auto& e : cache) if (e.key == table) return e.data;
    for (auto& e : cache) {
        if (e.key == nullptr) {
            e.key = table;
            e.data = static_cast<uint32_t*>(std::malloc(256 * sizeof(uint32_t)));
            for (int i = 0; i < 256; ++i) e.data[i] = table[i];
            return e.data;
        }
    }
    return cache[0].data; // cache exhausted (not expected); harmless fallback
}

/**
 *  Translate 16 source pixels via vpgatherdd against the u32 shadow.
 */
template<BlitConfig CFG>
[[maybe_unused]] static inline __m256i Translate16_AVX2(unsigned char const* src, uint32_t const* shadow,
                                                        unsigned char const* remap1, unsigned char const* const* remap2)
{
    unsigned char const* rtable = nullptr;
    if constexpr (CFG.xlat == XlatMode::ZRemap) rtable = *remap2;

    uint32_t idx[16];
    for (int k = 0; k < 16; ++k) {
        unsigned char c = src[k];
        if constexpr (CFG.xlat == XlatMode::Direct)     idx[k] = c;
        else if constexpr (CFG.xlat == XlatMode::Remap) idx[k] = remap1[c];
        else                                            idx[k] = rtable[c];
    }
    __m256i vi0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(idx));
    __m256i vi1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(idx + 8));
    __m256i g0 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(shadow), vi0, 4); // 8 u32
    __m256i g1 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(shadow), vi1, 4); // 8 u32
    // Pack 16 u32 (low 16 bits valid) into 16 u16, fixing the 128-bit lane interleave.
    __m256i packed = _mm256_packus_epi32(_mm256_and_si256(g0, _mm256_set1_epi32(0xFFFF)),
                                         _mm256_and_si256(g1, _mm256_set1_epi32(0xFFFF)));
    return _mm256_permute4x64_epi64(packed, 0xD8); // 11 01 10 00
}

/**
 *  Per-lane opaque mask for 16 source bytes: 0xFFFF where != 0.
 */
template<SimdTier ISA>
[[maybe_unused]] static inline __m256i Opaque16(unsigned char const* src, __m256i ones)
{
    __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));  // 16 source bytes
    __m128i istrans = _mm_cmpeq_epi8(vb, _mm_setzero_si128());            // 0xFF where c == 0
    __m256i istrans16 = _mm256_cvtepi8_epi16(istrans);                   // sign-extend -> 16 words
    return _mm256_xor_si256(istrans16, ones);
}

template<BlitConfig CFG>
[[maybe_unused]] static inline __m256i Blend16(__m256i vdst, __m256i vsrc16, __m256i vmask)
{
    if constexpr (CFG.blend == Blend::Copy) {
        return vsrc16;
    } else if constexpr (CFG.blend == Blend::Darken) {
        return _mm256_and_si256(_mm256_srli_epi16(vdst, 1), vmask);
    } else if constexpr (CFG.blend == Blend::L50) {
        return _mm256_add_epi16(_mm256_and_si256(_mm256_srli_epi16(vdst, 1), vmask),
                                _mm256_and_si256(_mm256_srli_epi16(vsrc16, 1), vmask));
    } else if constexpr (CFG.blend == Blend::L25) {
        __m256i qs = _mm256_and_si256(_mm256_srli_epi16(vsrc16, 2), vmask);
        __m256i qd = _mm256_and_si256(_mm256_srli_epi16(vdst, 2), vmask);
        return _mm256_add_epi16(qd, _mm256_add_epi16(qs, _mm256_add_epi16(qs, qs)));
    } else { // L75
        __m256i qs = _mm256_and_si256(_mm256_srli_epi16(vsrc16, 2), vmask);
        __m256i qd = _mm256_and_si256(_mm256_srli_epi16(vdst, 2), vmask);
        return _mm256_add_epi16(_mm256_add_epi16(qd, qd), _mm256_add_epi16(qd, qs));
    }
}

/**
 *  Compose 8 contiguous opaque RLE pixels with a depth test (no alpha / warp / dest-remap).
 *  The caller guarantees zp[0..7] do not cross the ring boundary. The depth test is the
 *  signed `(z_min - zs[k]) < (int)zp[k]`; because zp is an unsigned 16-bit value but the
 *  threshold is a wide signed int, the comparison is done in 32-bit lanes. The z-write
 *  value `(unsigned short)(z_min - zs[k])` is a plain 16-bit subtract (wraps identically).
 */
template<BlitConfig CFG>
static inline void RLE_Z_Chunk8(unsigned short* d, unsigned char const* src, unsigned short* zp,
                                signed char const* zs, int z_min,
                                unsigned short const* xlat, unsigned char const* remap1,
                                unsigned char const* const* remap2, __m128i vmask)
{
    const __m128i zero = _mm_setzero_si128();

    __m128i vzp16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zp));
    __m128i vzp_lo = _mm_unpacklo_epi16(vzp16, zero);          // 4 x u32
    __m128i vzp_hi = _mm_unpackhi_epi16(vzp16, zero);

    __m128i vzs8 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(zs));
    __m128i sgn8 = _mm_cmpgt_epi8(zero, vzs8);                 // sign bits
    __m128i vzs16 = _mm_unpacklo_epi8(vzs8, sgn8);             // 8 x s16
    __m128i sgn16 = _mm_cmpgt_epi16(zero, vzs16);
    __m128i vzs_lo = _mm_unpacklo_epi16(vzs16, sgn16);         // 4 x s32
    __m128i vzs_hi = _mm_unpackhi_epi16(vzs16, sgn16);

    __m128i vzmin = _mm_set1_epi32(z_min);
    __m128i vT_lo = _mm_sub_epi32(vzmin, vzs_lo);              // threshold = z_min - zs
    __m128i vT_hi = _mm_sub_epi32(vzmin, vzs_hi);
    __m128i vp_lo = _mm_cmpgt_epi32(vzp_lo, vT_lo);            // zp > threshold  <=>  threshold < zp
    __m128i vp_hi = _mm_cmpgt_epi32(vzp_hi, vT_hi);
    __m128i vpass = _mm_packs_epi32(vp_lo, vp_hi);             // 8 x i16 mask (0xFFFF / 0)

    __m128i vsrc16 = zero;
    if constexpr (CFG.blend != Blend::Darken) {
        vsrc16 = Translate8<CFG>(src, xlat, remap1, remap2);
    }
    __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d));
    __m128i vblend = Blend8<CFG>(vdst, vsrc16, vmask);
    __m128i vres = _mm_or_si128(_mm_and_si128(vpass, vblend), _mm_andnot_si128(vpass, vdst));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(d), vres);

    if constexpr (CFG.zwrite) {
        __m128i vTwrite = _mm_sub_epi16(_mm_set1_epi16((short)z_min), vzs16);   // (u16)(z_min - zs)
        __m128i vznew = _mm_or_si128(_mm_and_si128(vpass, vTwrite), _mm_andnot_si128(vpass, vzp16));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(zp), vznew);
    }
}

/**
 *  Skip `skipper` pixels into the RLE source stream (for left-edge clipping). Returns the
 *  number of leading transparent pixels that remain in the stream after the skip ended
 *  (the skip may have stopped in the middle of a transparent run). Mirrors the engine's
 *  Skip_Leading_Pixels exactly.
 */
static inline int Skip_Leading_Pixels(unsigned char const*& sptr, int skipper)
{
    while (skipper > 0) {
        if (*sptr++ == 0) {
            skipper -= *sptr++;
        } else {
            skipper--;
        }
    }
    return -skipper;
}

/**
 *  One alpha-composite pixel (BlitTranslucentWriteAlpha): blend source colour `srcc` over
 *  background `bg` by weight `alpha` (0..255), per channel, in the surface pixel format.
 *  Reproduces the engine's exact integer math (channel = (u8)((u8)(x>>right)<<left), the
 *  (alpha==255 -> 256) bump, >>8, clamp 255). Used at the ring boundary and as the reference.
 */
static inline unsigned short Composite1(unsigned short bg, unsigned short srcc, int alpha)
{
    int a1 = alpha;
    int a2 = 255 - alpha;
    if (a1 == 255) a1 = 256;

    const int Rr = (int)DSurface::Get_Red_Right(),   Rl = (int)DSurface::Get_Red_Left();
    const int Gr = (int)DSurface::Get_Green_Right(), Gl = (int)DSurface::Get_Green_Left();
    const int Br = (int)DSurface::Get_Blue_Right(),  Bl = (int)DSurface::Get_Blue_Left();

    auto ch = [](int right, int left, unsigned short x) -> int {
        return (unsigned char)((unsigned char)(x >> right) << left);
    };

    int r = (a1 * ch(Rr, Rl, srcc) + a2 * ch(Rr, Rl, bg)) >> 8; if (r > 255) r = 255;
    int g = (a1 * ch(Gr, Gl, srcc) + a2 * ch(Gr, Gl, bg)) >> 8; if (g > 255) g = 255;
    int b = (a1 * ch(Br, Bl, srcc) + a2 * ch(Br, Bl, bg)) >> 8; if (b > 255) b = 255;

    return (unsigned short)((b >> Bl << Br) | (g >> Gl << Gr) | (r >> Rl << Rr));
}

} // namespace


template<SimdTier ISA, BlitConfig CFG>
void Blit_Row(void* dest, void const* source, int length,
              int z_min, void* z_buff, void* a_buff, int alpha_level, int warp_offset,
              unsigned short const* xlat, unsigned char const* remap1,
              unsigned char const* const* remap2, unsigned short mask,
              unsigned short const* alut)
{
    unsigned short* d = static_cast<unsigned short*>(dest);
    unsigned char const* src = static_cast<unsigned char const*>(source);

    /**
     *  Alpha-buffer writers (no dest/z touch): store min(z_min + k*value, 255) for every
     *  opaque pixel, k = alpha_level (mult) or 1. The alpha ring wraps after each pixel, so the
     *  8-wide block only runs over a contiguous span; the scalar step does the boundary wrap.
     */
    if constexpr (CFG.awrite) {
        unsigned short* ap = static_cast<unsigned short*>(a_buff);
        const unsigned int aend = AlphaBuffer->Get_Buffer_End();
        const __m128i zero = _mm_setzero_si128();
        const __m128i vzmin = _mm_set1_epi32(z_min);
        const __m128i val16 = _mm_set1_epi16((short)alpha_level);
        const __m128i vthr = _mm_set1_epi32(254);   // t >= 255  <=>  t > 254
        const __m128i v255 = _mm_set1_epi32(255);
        const bool clean = (z_min >= 0) && (!CFG.awrite_mult || (unsigned)alpha_level <= 0xFFFFu);

        int i = 0;
        while (i < length) {
            if (clean && (length - i) >= 8 && (int)((aend - (unsigned int)ap) / 2) >= 8) {
                __m128i v8 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(src + i));
                __m128i v16 = _mm_unpacklo_epi8(v8, zero);            // 8x u16 value
                __m128i nz = _mm_cmpeq_epi16(v16, zero);              // 0xFFFF where value==0 (skip)
                __m128i plo, phi;
                if constexpr (CFG.awrite_mult) {
                    __m128i lo = _mm_mullo_epi16(v16, val16);
                    __m128i hi = _mm_mulhi_epu16(v16, val16);
                    plo = _mm_unpacklo_epi16(lo, hi);                 // 4x u32 product
                    phi = _mm_unpackhi_epi16(lo, hi);
                } else {
                    plo = _mm_unpacklo_epi16(v16, zero);              // 4x u32 value
                    phi = _mm_unpackhi_epi16(v16, zero);
                }
                plo = _mm_add_epi32(plo, vzmin);
                phi = _mm_add_epi32(phi, vzmin);
                __m128i ml = _mm_cmpgt_epi32(plo, vthr);
                __m128i mh = _mm_cmpgt_epi32(phi, vthr);
                plo = _mm_or_si128(_mm_and_si128(ml, v255), _mm_andnot_si128(ml, plo));
                phi = _mm_or_si128(_mm_and_si128(mh, v255), _mm_andnot_si128(mh, phi));
                __m128i t16 = _mm_packs_epi32(plo, phi);             // 8x u16 (0..255)
                __m128i old = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ap));
                __m128i res = _mm_or_si128(_mm_and_si128(nz, old), _mm_andnot_si128(nz, t16));
                _mm_storeu_si128(reinterpret_cast<__m128i*>(ap), res);
                i += 8; ap += 8;
                if ((unsigned int)ap >= aend) ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
            } else {
                unsigned char v = src[i];
                if (v != 0) {
                    int t = CFG.awrite_mult ? (z_min + alpha_level * (int)v) : (z_min + (int)v);
                    if (t >= 255) t = 255;
                    *ap = (unsigned short)t;
                }
                ++i; ++ap;
                ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
            }
        }
        return;
    }

    /**
     *  Per-channel RGB565 alpha composite into dest. a_buff[i] is the per-pixel weight: the
     *  source colour gets weight a1 = (a==255 ? 256 : a), the background a2 = 255 - a, each
     *  channel = (a1*src_c + a2*dst_c) >> 8 (clamped), recombined to the surface pixel format.
     */
    if constexpr (CFG.acomposite) {
        unsigned short* ap = static_cast<unsigned short*>(a_buff);
        const unsigned int aend = AlphaBuffer->Get_Buffer_End();
        const __m128i zero = _mm_setzero_si128();
        const __m128i m0xFF = _mm_set1_epi16(0x00FF);
        const __m128i v255 = _mm_set1_epi16(255);
        const __m128i v1 = _mm_set1_epi16(1);
        const __m128i sRr = _mm_cvtsi32_si128((int)DSurface::Get_Red_Right());
        const __m128i sRl = _mm_cvtsi32_si128((int)DSurface::Get_Red_Left());
        const __m128i sGr = _mm_cvtsi32_si128((int)DSurface::Get_Green_Right());
        const __m128i sGl = _mm_cvtsi32_si128((int)DSurface::Get_Green_Left());
        const __m128i sBr = _mm_cvtsi32_si128((int)DSurface::Get_Blue_Right());
        const __m128i sBl = _mm_cvtsi32_si128((int)DSurface::Get_Blue_Left());

        int i = 0;
        while (i < length) {
            if ((length - i) >= 8 && (int)((aend - (unsigned int)ap) / 2) >= 8) {
                unsigned short tmp[8];
                for (int k = 0; k < 8; ++k) tmp[k] = xlat[src[i + k]];
                __m128i vsrcc = _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp));
                __m128i v8 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(src + i));
                __m128i nz = _mm_cmpeq_epi16(_mm_unpacklo_epi8(v8, zero), zero);     // transparent lanes
                __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d + i));
                __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ap));   // 8x u16 weight
                __m128i a2 = _mm_sub_epi16(v255, a);
                __m128i a1 = _mm_add_epi16(a, _mm_and_si128(_mm_cmpeq_epi16(a, v255), v1));

                auto chan = [&](const __m128i cr, const __m128i cl) -> __m128i {
                    __m128i sc = _mm_and_si128(_mm_sll_epi16(_mm_and_si128(_mm_srl_epi16(vsrcc, cr), m0xFF), cl), m0xFF);
                    __m128i dc = _mm_and_si128(_mm_sll_epi16(_mm_and_si128(_mm_srl_epi16(vdst, cr), m0xFF), cl), m0xFF);
                    __m128i p1 = _mm_mullo_epi16(a1, sc);   // a1<=256, sc<=255 -> fits u16
                    __m128i p2 = _mm_mullo_epi16(a2, dc);   // a2<=255, dc<=255 -> fits u16
                    __m128i slo = _mm_add_epi32(_mm_unpacklo_epi16(p1, zero), _mm_unpacklo_epi16(p2, zero));
                    __m128i shi = _mm_add_epi32(_mm_unpackhi_epi16(p1, zero), _mm_unpackhi_epi16(p2, zero));
                    __m128i r16 = _mm_packs_epi32(_mm_srli_epi32(slo, 8), _mm_srli_epi32(shi, 8));
                    __m128i m = _mm_cmpgt_epi16(r16, v255);
                    r16 = _mm_or_si128(_mm_and_si128(m, v255), _mm_andnot_si128(m, r16));
                    return _mm_sll_epi16(_mm_srl_epi16(r16, cl), cr);
                };

                __m128i comp = _mm_or_si128(_mm_or_si128(chan(sRr, sRl), chan(sGr, sGl)), chan(sBr, sBl));
                __m128i res = _mm_or_si128(_mm_and_si128(nz, vdst), _mm_andnot_si128(nz, comp));
                _mm_storeu_si128(reinterpret_cast<__m128i*>(d + i), res);
                i += 8; ap += 8;
                if ((unsigned int)ap >= aend) ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
            } else {
                unsigned char v = src[i];
                if (v != 0) d[i] = Composite1(d[i], xlat[v], (int)*ap);
                ++i; ++ap;
                ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
            }
        }
        return;
    }

    /**
     *  Translucency (L50/L75) gated on the alpha buffer: draw only where the source is opaque AND
     *  a_buff[i] is non-zero (Nonzero) or zero (Zero). a_buff is read-only and advances+wraps per
     *  pixel, so the 8-wide block runs over the contiguous span and the scalar step wraps.
     */
    if constexpr (CFG.agate) {
        unsigned short* ap = static_cast<unsigned short*>(a_buff);
        const unsigned int aend = AlphaBuffer->Get_Buffer_End();
        const __m128i zero = _mm_setzero_si128();
        const __m128i vmaskL = _mm_set1_epi16((short)mask);
        const __m128i onesL = _mm_set1_epi16((short)0xFFFF);

        int i = 0;
        while (i < length) {
            if ((length - i) >= 8 && (int)((aend - (unsigned int)ap) / 2) >= 8) {
                unsigned short tmp[8];
                for (int k = 0; k < 8; ++k) tmp[k] = xlat[src[i + k]];
                __m128i vsrc16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp));
                __m128i trans = Opaque8(src + i, onesL);                         // 0xFFFF where value!=0
                __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ap));
                __m128i azero = _mm_cmpeq_epi16(a, zero);                        // 0xFFFF where a_buff==0
                __m128i gate = CFG.agate_zero ? azero : _mm_andnot_si128(azero, onesL);
                __m128i draw = _mm_and_si128(trans, gate);
                __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d + i));
                __m128i blend = Blend8<CFG>(vdst, vsrc16, vmaskL);
                __m128i res = _mm_or_si128(_mm_and_si128(draw, blend), _mm_andnot_si128(draw, vdst));
                _mm_storeu_si128(reinterpret_cast<__m128i*>(d + i), res);
                i += 8; ap += 8;
                if ((unsigned int)ap >= aend) ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
            } else {
                unsigned char v = src[i];
                bool gate = CFG.agate_zero ? (*ap == 0) : (*ap != 0);
                if (v != 0 && gate) Blend1_bg<CFG>(d + i, xlat[v], d[i], mask);
                ++i; ++ap;
                ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
            }
        }
        return;
    }

    const __m128i vmask = _mm_set1_epi16((short)mask);
    const __m128i ones = _mm_set1_epi16((short)0xFFFF);

    if constexpr (CFG.zread || CFG.alpha) {

        /**
         *  General per-pixel-tracked path (128-bit; valid under both SSE2 and
         *  AVX2). Handles the z-buffer and/or alpha-buffer ring pointers (each
         *  wraps after every pixel) and warp. The wide step only runs over a span
         *  that is contiguous in every active ring; the scalar step reproduces the
         *  per-pixel Wrap_Overflow at the boundaries.
         */
        const __m128i vzbias = _mm_set1_epi16((short)0x8000);
        const bool z_in_range = (!CFG.zread) || (z_min >= 0 && z_min <= 0xFFFF);
        const __m128i vzmin_b = _mm_xor_si128(_mm_set1_epi16((short)(unsigned short)z_min), vzbias);
        const unsigned short zwrite_val =
            CFG.zwbyte ? (unsigned short)(unsigned char)z_min : (unsigned short)z_min;
        const __m128i vzwrite = _mm_set1_epi16((short)zwrite_val);

        unsigned char* zb = static_cast<unsigned char*>(z_buff);
        unsigned char* ap = static_cast<unsigned char*>(a_buff);
        const unsigned int zend = CFG.zread ? DepthBuffer->Get_Buffer_End() : 0u;
        const unsigned int aend = (CFG.alpha && !CFG.alpha_static) ? AlphaBuffer->Get_Buffer_End() : 0u;

        int i = 0;
        while (i < length) {

            /**
             *  Warp reads dest[i + warp_offset], i.e. the *already-written* background a few
             *  pixels back; that is an intra-row read-after-write the wide block (which loads
             *  all 8 backgrounds before storing) cannot honour. Warp is rare, so just run the
             *  bit-exact scalar path for it.
             */
            bool wide = z_in_range && (length - i) >= 8;
            if constexpr (CFG.warp) wide = false;
            if constexpr (CFG.zread) { if ((int)((zend - (unsigned int)zb) / 2) < 8) wide = false; }
            if constexpr (CFG.alpha && !CFG.alpha_static) { if ((int)((aend - (unsigned int)ap) / 2) < 8) wide = false; }

            if (wide) {

                __m128i vsrc16 = _mm_setzero_si128();
                if constexpr (CFG.blend != Blend::Darken) {
                    unsigned short tmp[8];
                    unsigned short const* apw = reinterpret_cast<unsigned short const*>(ap);
                    for (int k = 0; k < 8; ++k) {
                        unsigned char c = src[i + k];
                        unsigned base;
                        if constexpr (CFG.xlat == XlatMode::Direct)     base = c;
                        else if constexpr (CFG.xlat == XlatMode::Remap) base = remap1[c];
                        else                                            base = (*remap2)[c];
                        unsigned bits = 0;
                        if constexpr (CFG.alpha) bits = CFG.alpha_static ? alut[apw[0]] : alut[apw[k]];
                        tmp[k] = xlat[base | bits];
                    }
                    vsrc16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp));
                }

                __m128i vwrite = CFG.trans ? Opaque8(src + i, ones) : ones;
                __m128i vzb = _mm_setzero_si128();
                if constexpr (CFG.zread) {
                    vzb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zb));
                    __m128i vzpass = _mm_cmpgt_epi16(_mm_xor_si128(vzb, vzbias), vzmin_b);
                    vwrite = _mm_and_si128(vwrite, vzpass);
                }

                __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d + i));
                __m128i vbg = vdst;
                if constexpr (CFG.warp) vbg = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d + i + warp_offset));
                __m128i vblend = Blend8<CFG>(vbg, vsrc16, vmask);
                __m128i result = _mm_or_si128(_mm_and_si128(vwrite, vblend), _mm_andnot_si128(vwrite, vdst));
                _mm_storeu_si128(reinterpret_cast<__m128i*>(d + i), result);

                if constexpr (CFG.zwrite) {
                    __m128i znew = _mm_or_si128(_mm_and_si128(vwrite, vzwrite), _mm_andnot_si128(vwrite, vzb));
                    _mm_storeu_si128(reinterpret_cast<__m128i*>(zb), znew);
                }

                i += 8;
                if constexpr (CFG.zread) zb += 16;
                if constexpr (CFG.alpha && !CFG.alpha_static) ap += 16;

            } else {

                unsigned char c = src[i];
                bool draw = (!CFG.trans || c != 0);
                if constexpr (CFG.zread) draw = draw && (z_min < (int)*reinterpret_cast<unsigned short*>(zb));
                if (draw) {
                    unsigned short s = 0;
                    if constexpr (CFG.blend != Blend::Darken) {
                        unsigned base;
                        if constexpr (CFG.xlat == XlatMode::Direct)     base = c;
                        else if constexpr (CFG.xlat == XlatMode::Remap) base = remap1[c];
                        else                                            base = (*remap2)[c];
                        unsigned bits = 0;
                        if constexpr (CFG.alpha) bits = alut[*reinterpret_cast<unsigned short*>(ap)];
                        s = xlat[base | bits];
                    }
                    unsigned short bg = CFG.warp ? d[i + warp_offset] : d[i];
                    Blend1_bg<CFG>(d + i, s, bg, mask);
                    if constexpr (CFG.zwrite) {
                        *reinterpret_cast<unsigned short*>(zb) = zwrite_val;
                    }
                }
                i += 1;
                if constexpr (CFG.zread) zb += 2;
                if constexpr (CFG.alpha && !CFG.alpha_static) ap += 2;
            }

            if constexpr (CFG.zread) zb = reinterpret_cast<unsigned char*>(DepthBuffer->Wrap_Overflow((unsigned int)zb));
            if constexpr (CFG.alpha && !CFG.alpha_static) ap = reinterpret_cast<unsigned char*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
        }

    } else {

        int i = 0;

        if constexpr (ISA == SimdTier::AVX2) {

            /**
             *  16-wide AVX2 path: vpgatherdd translate + 256-bit compose.
             */
            const uint32_t* shadow = (CFG.blend != Blend::Darken) ? Get_U32_Shadow<ISA>(xlat) : nullptr;
            const __m256i vmask256 = _mm256_set1_epi16((short)mask);
            const __m256i ones256 = _mm256_set1_epi16((short)0xFFFF);

            for (; i + 16 <= length; i += 16) {
                __m256i vsrc16 = _mm256_setzero_si256();
                if constexpr (CFG.blend != Blend::Darken) {
                    vsrc16 = Translate16_AVX2<CFG>(src + i, shadow, remap1, remap2);
                }
                __m256i vwrite = CFG.trans ? Opaque16<ISA>(src + i, ones256) : ones256;
                __m256i vdst = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(d + i));
                __m256i vblend = Blend16<CFG>(vdst, vsrc16, vmask256);
                __m256i result = _mm256_or_si256(_mm256_and_si256(vwrite, vblend),
                                                 _mm256_andnot_si256(vwrite, vdst));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(d + i), result);
            }
            _mm256_zeroupper();
        }

        /**
         *  8-wide SSE2 path (also mops up the [8,16) remainder after the AVX2 loop).
         */
        for (; i + 8 <= length; i += 8) {
            __m128i vsrc16 = _mm_setzero_si128();
            if constexpr (CFG.blend != Blend::Darken) {
                vsrc16 = Translate8<CFG>(src + i, xlat, remap1, remap2);
            }
            __m128i vwrite = CFG.trans ? Opaque8(src + i, ones) : ones;
            __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d + i));
            __m128i vblend = Blend8<CFG>(vdst, vsrc16, vmask);
            __m128i result = _mm_or_si128(_mm_and_si128(vwrite, vblend), _mm_andnot_si128(vwrite, vdst));
            _mm_storeu_si128(reinterpret_cast<__m128i*>(d + i), result);
        }
        for (; i < length; ++i) {
            Compose_Scalar<CFG>(d + i, src[i], xlat, remap1, remap2, mask);
        }
    }
}


template<SimdTier ISA, BlitConfig CFG>
void RLE_Blit_Row(void* dest, void const* source, int length, int leadskip, int z_min,
                  void* z_buff, void* a_buff, int /*alpha_level*/, int warp_offset, void const* zshape,
                  unsigned short const* xlat, unsigned char const* remap1,
                  unsigned char const* const* remap2, unsigned short mask,
                  unsigned short const* alut)
{
    unsigned short* dptr = static_cast<unsigned short*>(dest);
    unsigned char const* sptr = static_cast<unsigned char const*>(source);
    unsigned short* zp = static_cast<unsigned short*>(z_buff);
    unsigned short* ap = static_cast<unsigned short*>(a_buff);
    signed char const* zs = static_cast<signed char const*>(zshape);

    /**
     *  A row is "fast" (its opaque runs vectorise with no per-pixel bookkeeping) when there
     *  is no depth test, alpha gather, warp background or dest-remap. "Z_FAST" rows have a
     *  depth test but still none of alpha/warp/dest-remap, so their opaque runs vectorise
     *  8-wide with a per-lane depth test. Everything else uses a scalar opaque compose.
     */
    constexpr bool FAST   = !CFG.zread && !CFG.alpha && !CFG.warp && !CFG.remapdest;
    constexpr bool Z_FAST =  CFG.zread && !CFG.alpha && !CFG.warp && !CFG.remapdest && !CFG.rle_zs2;

    const __m128i vmask = _mm_set1_epi16((short)mask);
    const unsigned int zend = CFG.zread ? DepthBuffer->Get_Buffer_End() : 0u;

    /**
     *  Left-edge clip. zshape is NOT advanced here (matches the engine); the depth and
     *  alpha ring pointers advance by the count of transparent pixels and wrap once.
     */
    if (leadskip > 0) {
        int trans = Skip_Leading_Pixels(sptr, leadskip);
        dptr += trans;
        length -= trans;
        if constexpr (CFG.zread) {
            zp += trans;
            zp = reinterpret_cast<unsigned short*>(DepthBuffer->Wrap_Overflow((unsigned int)zp));
        }
        if constexpr (CFG.alpha) {
            ap += trans;
            ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
        }
    }

    while (length > 0) {
        unsigned char value = *sptr++;

        if (value == 0) {

            /**
             *  Transparent run: advance past `count` pixels. The depth/alpha pointers add
             *  the whole count then wrap once (a single Wrap_Overflow), per the engine.
             */
            int count = *sptr++;
            length -= count;
            dptr += count;
            if constexpr (CFG.zread && !CFG.rle_skip_noz) { zp += count; zs += count; }
            if constexpr (CFG.alpha) { ap += count; }

        } else if constexpr (FAST) {

            /**
             *  Opaque run: the source bytes from here up to the next 0x00 (capped by the
             *  remaining length) are consecutive opaque pixels -> translate + blend + store
             *  wide. `run[0]` is the value we just consumed.
             */
            unsigned char const* run = sptr - 1;
            int run_len = 1;
            while (run_len < length && run[run_len] != 0) ++run_len;

            int k = 0;
            if constexpr (ISA == SimdTier::AVX2) {
                const uint32_t* shadow = (CFG.blend != Blend::Darken) ? Get_U32_Shadow<ISA>(xlat) : nullptr;
                const __m256i vmask256 = _mm256_set1_epi16((short)mask);
                for (; k + 16 <= run_len; k += 16) {
                    __m256i vsrc16 = _mm256_setzero_si256();
                    if constexpr (CFG.blend != Blend::Darken) {
                        vsrc16 = Translate16_AVX2<CFG>(run + k, shadow, remap1, remap2);
                    }
                    __m256i vdst = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(dptr + k));
                    __m256i vres = Blend16<CFG>(vdst, vsrc16, vmask256);
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dptr + k), vres);
                }
                _mm256_zeroupper();
            }
            for (; k + 8 <= run_len; k += 8) {
                __m128i vsrc16 = _mm_setzero_si128();
                if constexpr (CFG.blend != Blend::Darken) {
                    vsrc16 = Translate8<CFG>(run + k, xlat, remap1, remap2);
                }
                __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(dptr + k));
                __m128i vres = Blend8<CFG>(vdst, vsrc16, vmask);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dptr + k), vres);
            }
            for (; k < run_len; ++k) {
                Compose_Scalar<CFG>(dptr + k, run[k], xlat, remap1, remap2, mask);
            }

            dptr += run_len;
            sptr = run + run_len;
            length -= run_len;

        } else if constexpr (Z_FAST) {

            /**
             *  Depth-tested opaque run (no alpha/warp/dest-remap). Process 8-wide while the
             *  depth ring stays contiguous; drop to one scalar pixel at the ring boundary
             *  (which wraps). zshape is linear and never wraps.
             */
            unsigned char const* run = sptr - 1;
            int run_len = 1;
            while (run_len < length && run[run_len] != 0) ++run_len;

            int k = 0;
            while (k < run_len) {
                int until_wrap = (int)((zend - (unsigned int)zp) / 2);
                if (run_len - k >= 8 && until_wrap >= 8) {
                    RLE_Z_Chunk8<CFG>(dptr + k, run + k, zp, zs, z_min, xlat, remap1, remap2, vmask);
                    k += 8; zp += 8; zs += 8;
                    if ((unsigned int)zp >= zend) zp = reinterpret_cast<unsigned short*>(DepthBuffer->Wrap_Overflow((unsigned int)zp));
                } else {
                    int thr = z_min - (int)*zs;
                    if (thr < (int)*zp) {
                        unsigned short s = 0;
                        if constexpr (CFG.blend != Blend::Darken) {
                            unsigned base;
                            if constexpr (CFG.xlat == XlatMode::Direct)     base = run[k];
                            else if constexpr (CFG.xlat == XlatMode::Remap) base = remap1[run[k]];
                            else                                            base = (*remap2)[run[k]];
                            s = xlat[base];
                        }
                        Blend1_bg<CFG>(dptr + k, s, dptr[k], mask);
                        if constexpr (CFG.zwrite) *zp = (unsigned short)thr;
                    }
                    ++k; ++zp; ++zs;
                    zp = reinterpret_cast<unsigned short*>(DepthBuffer->Wrap_Overflow((unsigned int)zp));
                }
            }

            dptr += run_len;
            sptr = run + run_len;
            length -= run_len;

        } else {

            /**
             *  Scalar opaque compose for the depth / alpha / warp / dest-remap families.
             *  The depth test is the signed `(z_min - *zshape) < (int)*zp`; on z-write the
             *  stored value is that same `z_min - *zshape`.
             */
            bool zpass = true;
            int znew = z_min;
            if constexpr (CFG.zread) {
                int thr = z_min - (int)*zs;
                zpass = thr < (int)*zp;
                /**
                 *  ZRemapXlatZReadWrite (rle_zs2) is shipped with a bug: the stored depth is
                 *  sampled from the NEXT zshape entry and zshape advances by two per pixel.
                 */
                znew = CFG.rle_zs2 ? (z_min - (int)zs[1]) : thr;
            }
            if (zpass) {
                if constexpr (CFG.remapdest) {
                    *dptr = xlat[*dptr];
                } else {
                    unsigned base;
                    if constexpr (CFG.xlat == XlatMode::Direct)     base = value;
                    else if constexpr (CFG.xlat == XlatMode::Remap) base = remap1[value];
                    else                                            base = (*remap2)[value];
                    unsigned bits = 0;
                    if constexpr (CFG.alpha) bits = alut[*ap];
                    unsigned short s = xlat[base | bits];
                    unsigned short bg = CFG.warp ? dptr[warp_offset] : *dptr;
                    Blend1_bg<CFG>(dptr, s, bg, mask);
                }
                if constexpr (CFG.zwrite) *zp = (unsigned short)znew;
            }
            ++dptr;
            --length;
            if constexpr (CFG.zread) { ++zp; zs += (CFG.rle_zs2 ? 2 : 1); }
            if constexpr (CFG.alpha) { ++ap; }
        }

        if constexpr (CFG.zread) zp = reinterpret_cast<unsigned short*>(DepthBuffer->Wrap_Overflow((unsigned int)zp));
        if constexpr (CFG.alpha) ap = reinterpret_cast<unsigned short*>(AlphaBuffer->Wrap_Overflow((unsigned int)ap));
    }
}
