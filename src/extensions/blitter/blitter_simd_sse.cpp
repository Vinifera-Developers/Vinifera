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
 *          blend, z-test and masked store are vectorised 8 pixels at a time. All
 *          arithmetic reproduces the vanilla integer expressions exactly
 *          (non-saturating adds), so output is bit-identical.
 *
 *          Z-buffer pointers index a global ring buffer (DepthBuffer) and wrap at
 *          its end after each pixel; the vectorised path only runs over a span
 *          guaranteed contiguous (zb + 8 <= BufferEnd), and the scalar path
 *          reproduces the per-pixel Wrap_Overflow at the boundary.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "blitter_simd.h"
#include "zbuffer.h"

#include <emmintrin.h> // SSE2


/**
 *  Global ring z-buffer (defined in TSpp via Make_Global). Z blitters wrap their
 *  z pointer against it after each pixel.
 */
extern ZBuffer*& DepthBuffer;


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
 *  Apply the blend for one pixel given the already-translated source colour.
 *  (Darken ignores the source; the caller must already have decided to write.)
 */
template<BlitConfig CFG>
static inline void Blend1(unsigned short* d, unsigned short s, unsigned short mask)
{
    if constexpr (CFG.blend == Blend::Copy) {
        *d = s;
    } else if constexpr (CFG.blend == Blend::Darken) {
        *d = (unsigned short)(((*d) >> 1) & mask);
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

/**
 *  Translate 8 source pixels into a vector (scalar gather).
 */
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

/**
 *  Per-lane opaque (write-enable) mask from 8 source bytes: 0xFFFF where != 0.
 */
static inline __m128i Opaque8(unsigned char const* src, __m128i ones)
{
    __m128i vb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(src));   // 8 source bytes
    __m128i istrans = _mm_cmpeq_epi8(vb, _mm_setzero_si128());             // 0xFF where c == 0
    __m128i istrans16 = _mm_unpacklo_epi8(istrans, istrans);              // expand low 8 -> 8 words
    return _mm_xor_si128(istrans16, ones);                                // invert -> opaque
}

/**
 *  Vectorised blend of 8 pixels.
 */
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

} // namespace


template<SimdTier ISA, BlitConfig CFG>
void Blit_Row(void* dest, void const* source, int length,
              int z_min, void* z_buff, void* /*a_buff*/, int /*alpha_level*/, int /*warp_offset*/,
              unsigned short const* xlat, unsigned char const* remap1,
              unsigned char const* const* remap2, unsigned short mask)
{
    unsigned short* d = static_cast<unsigned short*>(dest);
    unsigned char const* src = static_cast<unsigned char const*>(source);

    const __m128i vmask = _mm_set1_epi16((short)mask);
    const __m128i ones = _mm_set1_epi16((short)0xFFFF);

    if constexpr (CFG.zread) {

        /**
         *  Depth-tested path. The z pointer indexes the global ring z-buffer and
         *  wraps after each pixel; only run the wide path over a guaranteed
         *  contiguous span, and reproduce the per-pixel wrap with the scalar path.
         */
        const __m128i vzbias = _mm_set1_epi16((short)0x8000);
        const bool z_in_range = (z_min >= 0 && z_min <= 0xFFFF);
        const __m128i vzmin_b = _mm_xor_si128(_mm_set1_epi16((short)(unsigned short)z_min), vzbias);
        const unsigned short zwrite_val =
            CFG.zwbyte ? (unsigned short)(unsigned char)z_min : (unsigned short)z_min;
        const __m128i vzwrite = _mm_set1_epi16((short)zwrite_val);

        unsigned char* zb = static_cast<unsigned char*>(z_buff);
        const unsigned int zend = DepthBuffer->Get_Buffer_End();

        int i = 0;
        while (i < length) {

            int zpix = (int)((zend - (unsigned int)zb) / 2);    // z slots until the ring wraps

            if (z_in_range && (length - i) >= 8 && zpix >= 8) {

                __m128i vsrc16 = _mm_setzero_si128();
                if constexpr (CFG.blend != Blend::Darken) {
                    vsrc16 = Translate8<CFG>(src + i, xlat, remap1, remap2);
                }
                __m128i vtrans = CFG.trans ? Opaque8(src + i, ones) : ones;
                __m128i vzb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zb));
                __m128i vzpass = _mm_cmpgt_epi16(_mm_xor_si128(vzb, vzbias), vzmin_b);   // z_min < zb
                __m128i vwrite = _mm_and_si128(vtrans, vzpass);

                __m128i vdst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d + i));
                __m128i vblend = Blend8<CFG>(vdst, vsrc16, vmask);
                __m128i result = _mm_or_si128(_mm_and_si128(vwrite, vblend), _mm_andnot_si128(vwrite, vdst));
                _mm_storeu_si128(reinterpret_cast<__m128i*>(d + i), result);

                if constexpr (CFG.zwrite) {
                    __m128i znew = _mm_or_si128(_mm_and_si128(vwrite, vzwrite), _mm_andnot_si128(vwrite, vzb));
                    _mm_storeu_si128(reinterpret_cast<__m128i*>(zb), znew);
                }

                i += 8;
                zb += 16;

            } else {

                unsigned short zval = *reinterpret_cast<unsigned short*>(zb);
                unsigned char c = src[i];
                bool draw = (z_min < (int)zval) && (!CFG.trans || c != 0);
                if (draw) {
                    unsigned short s = 0;
                    if constexpr (CFG.blend != Blend::Darken) {
                        s = Translate1<CFG>(c, xlat, remap1, remap2);
                    }
                    Blend1<CFG>(d + i, s, mask);
                    if constexpr (CFG.zwrite) {
                        *reinterpret_cast<unsigned short*>(zb) = zwrite_val;
                    }
                }
                i += 1;
                zb += 2;
            }

            zb = reinterpret_cast<unsigned char*>(DepthBuffer->Wrap_Overflow((unsigned int)zb));
        }

    } else {

        /**
         *  Straight-line path (no z-buffer): dest/source never wrap.
         */
        int i = 0;
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


/**
 *  Explicit SSE2 instantiations.
 */
#define INSTANTIATE(cfg) \
    template void Blit_Row<SimdTier::SSE2, cfg>(void*, void const*, int, int, void*, void*, int, int, \
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
