/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SSE2 (and SSE4.1) implementation of the templated blitter kernel.
 *
 *          THIS TRANSLATION UNIT IS COMPILED WITH /arch:SSE2 (see CMakeLists.txt)
 *          and MUST remain strictly floating-point free, so the /arch-driven
 *          change to scalar FP code generation can never be observed. Use only
 *          integer SSE2/SSE4.1 intrinsics here.
 *
 *          The 8->16-bit palette translate is a gather, which SSE2 cannot do, so
 *          it stays a tight scalar lookup; the transparency mask, translucency
 *          blend and masked store are vectorised 8 pixels at a time. The blend
 *          arithmetic reproduces the vanilla integer expressions exactly
 *          (non-saturating adds), so output is bit-identical.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "blitter_simd.h"

#include <emmintrin.h> // SSE2


namespace
{

/**
 *  Scalar reference compose for a single pixel. Used for the row tail (length
 *  not a multiple of 8) and as the bit-exact oracle this kernel must match.
 */
template<BlitConfig CFG>
static inline void Compose_Scalar(unsigned short* d, unsigned char c,
                                  unsigned short const* xlat, unsigned char const* remap1,
                                  unsigned char const* const* remap2, unsigned short mask)
{
    if constexpr (CFG.trans) {
        if (c == 0) return;
    }

    if constexpr (CFG.blend == Blend::Darken) {
        *d = (unsigned short)(((*d) >> 1) & mask);
        return;
    } else {
        unsigned idx;
        if constexpr (CFG.xlat == XlatMode::Direct)     idx = c;
        else if constexpr (CFG.xlat == XlatMode::Remap) idx = remap1[c];
        else                                            idx = (*remap2)[c];

        unsigned short s = xlat[idx];

        if constexpr (CFG.blend == Blend::Copy) {
            *d = s;
        } else if constexpr (CFG.blend == Blend::L50) {
            *d = (unsigned short)((((*d) >> 1) & mask) + ((s >> 1) & mask));
        } else if constexpr (CFG.blend == Blend::L25) {
            unsigned short qs = (unsigned short)((s >> 2) & mask);
            unsigned short qd = (unsigned short)(((*d) >> 2) & mask);
            *d = (unsigned short)(qd + qs + qs + qs);
        } else { // L75
            unsigned short qs = (unsigned short)((s >> 2) & mask);
            unsigned short qd = (unsigned short)(((*d) >> 2) & mask);
            *d = (unsigned short)(qd + qd + qd + qs);
        }
    }
}

} // namespace


template<SimdTier ISA, BlitConfig CFG>
void Blit_Row(void* dest, void const* source, int length,
              int /*z_min*/, void* /*z_buff*/, void* /*a_buff*/, int /*alpha_level*/, int /*warp_offset*/,
              unsigned short const* xlat, unsigned char const* remap1,
              unsigned char const* const* remap2, unsigned short mask)
{
    unsigned short* d = static_cast<unsigned short*>(dest);
    unsigned char const* src = static_cast<unsigned char const*>(source);
    int i = 0;

    const __m128i vmask = _mm_set1_epi16((short)mask);
    const __m128i ones = _mm_set1_epi16((short)0xFFFF);

    for (; i + 8 <= length; i += 8) {

        /**
         *  Translate 8 source pixels (scalar gather). Darken ignores the source
         *  colour entirely, so skip it there.
         */
        __m128i vsrc16 = _mm_setzero_si128();
        if constexpr (CFG.blend != Blend::Darken) {
            unsigned short tmp[8];
            for (int k = 0; k < 8; ++k) {
                unsigned c = src[i + k];
                unsigned idx;
                if constexpr (CFG.xlat == XlatMode::Direct)     idx = c;
                else if constexpr (CFG.xlat == XlatMode::Remap) idx = remap1[c];
                else                                            idx = (*remap2)[c];
                tmp[k] = xlat[idx];
            }
            vsrc16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(tmp));
        }

        /**
         *  Per-lane write mask. For transparent blitters a lane is written only
         *  where the source byte is non-zero.
         */
        __m128i vwrite;
        if constexpr (CFG.trans) {
            __m128i vb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(src + i)); // 8 source bytes
            __m128i istrans = _mm_cmpeq_epi8(vb, _mm_setzero_si128());               // 0xFF where c == 0
            __m128i istrans16 = _mm_unpacklo_epi8(istrans, istrans);                 // expand low 8 bytes -> 8 words
            vwrite = _mm_xor_si128(istrans16, ones);                                 // invert -> opaque
        } else {
            vwrite = ones;
        }

        __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d + i));

        __m128i vblend;
        if constexpr (CFG.blend == Blend::Copy) {
            vblend = vsrc16;
        } else if constexpr (CFG.blend == Blend::Darken) {
            vblend = _mm_and_si128(_mm_srli_epi16(vdst, 1), vmask);
        } else if constexpr (CFG.blend == Blend::L50) {
            vblend = _mm_add_epi16(_mm_and_si128(_mm_srli_epi16(vdst, 1), vmask),
                                   _mm_and_si128(_mm_srli_epi16(vsrc16, 1), vmask));
        } else if constexpr (CFG.blend == Blend::L25) {
            __m128i qs = _mm_and_si128(_mm_srli_epi16(vsrc16, 2), vmask);
            __m128i qd = _mm_and_si128(_mm_srli_epi16(vdst, 2), vmask);
            vblend = _mm_add_epi16(qd, _mm_add_epi16(qs, _mm_add_epi16(qs, qs)));    // qd + 3*qs
        } else { // L75
            __m128i qs = _mm_and_si128(_mm_srli_epi16(vsrc16, 2), vmask);
            __m128i qd = _mm_and_si128(_mm_srli_epi16(vdst, 2), vmask);
            vblend = _mm_add_epi16(_mm_add_epi16(qd, qd), _mm_add_epi16(qd, qs));    // 3*qd + qs
        }

        // result = write ? vblend : vdst   (SSE2 has no blendv; use and/andnot/or).
        __m128i result = _mm_or_si128(_mm_and_si128(vwrite, vblend),
                                      _mm_andnot_si128(vwrite, vdst));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(d + i), result);
    }

    for (; i < length; ++i) {
        Compose_Scalar<CFG>(d + i, src[i], xlat, remap1, remap2, mask);
    }
}


/**
 *  Explicit SSE2 instantiations (one per first-wave config).
 */
#define INSTANTIATE(cfg) \
    template void Blit_Row<SimdTier::SSE2, cfg>(void*, void const*, int, int, void*, void*, int, int, \
                                                unsigned short const*, unsigned char const*, \
                                                unsigned char const* const*, unsigned short)

INSTANTIATE(CFG_PlainXlat);
INSTANTIATE(CFG_TransXlat);
INSTANTIATE(CFG_RemapXlat);
INSTANTIATE(CFG_ZRemapXlat);
INSTANTIATE(CFG_Darken);
INSTANTIATE(CFG_Lucent25);
INSTANTIATE(CFG_Lucent50);
INSTANTIATE(CFG_Lucent75);

#undef INSTANTIATE
