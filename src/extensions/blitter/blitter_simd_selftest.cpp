/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Bit-exactness self-test for the SIMD blitters.
 *
 *          For each implemented family this constructs a vanilla blitter (whose
 *          BlitForward is bound to the original address) and the SimdBlit
 *          equivalent, sharing the live ConvertClass translate/remap/mask
 *          tables, then runs both over randomised source/destination buffers
 *          across awkward lengths (0,1,7,8,9,...) and compares the results. Any
 *          mismatch is logged loudly. Intended to run once at startup behind a
 *          dev flag; the oracle is the real engine routine, so it must run
 *          in-process under the injected game.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "blitter_simd.h"

#if defined(BLITTER_TESTS) || defined(BLITTER_BENCH)

#include "convert.h"
#include "cpudetect.h"
#include "zbuffer.h"
#include "abuffer.h"
#include "debughandler.h"
#include "tibsun_globals.h"   // ZBuffer*& DepthBuffer / ABuffer*& AlphaBuffer (bound in TSpp)
#include <intrin.h>           // __rdtsc (benchmark)


namespace
{

/**
 *  Deterministic xorshift PRNG (reproducible runs; no dependency on rand()).
 */
static unsigned g_seed = 0x1234ABCDu;
static inline unsigned Rng()
{
    g_seed ^= g_seed << 13;
    g_seed ^= g_seed >> 17;
    g_seed ^= g_seed << 5;
    return g_seed;
}

/**
 *  Compare one family: run the vanilla and SIMD BlitForward over random inputs
 *  across several lengths and report the number of mismatching pixels.
 */
static int Compare_Family(const char* name, const Blitter& vanilla, const Blitter& simd)
{
    static const int lengths[] = { 0, 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 33, 64, 127, 255 };

    unsigned char  src[256];
    unsigned short da[256];
    unsigned short db[256];

    int mismatches = 0;
    int first_len = -1;
    int first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];

        for (int rep = 0; rep < 64; ++rep) {

            /**
             *  Source: 8-bit indices with a healthy fraction of transparent (0) pixels.
             */
            for (int i = 0; i < len; ++i) {
                src[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(Rng() & 0xFF);
            }

            /**
             *  Destination: identical random 16-bit content for both runs.
             */
            for (int i = 0; i < len; ++i) {
                unsigned short v = (unsigned short)(Rng() & 0xFFFF);
                da[i] = v;
                db[i] = v;
            }

            vanilla.BlitForward(da, src, len, 0, nullptr, nullptr, 1000, 0);
            simd.BlitForward(db, src, len, 0, nullptr, nullptr, 1000, 0);

            for (int i = 0; i < len; ++i) {
                if (da[i] != db[i]) {
                    ++mismatches;
                    if (first_len < 0) { first_len = len; first_idx = i; }
                }
            }
        }
    }

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n",
                      name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }

    return mismatches;
}

/**
 *  Compare one depth-tested family. Points the (init-time null) DepthBuffer at a fake ring
 *  z-buffer so the vanilla oracle and the SIMD path wrap identically; the ring is sized so the
 *  longer rows cross the boundary and exercise the scalar wrap path. ZReadWrite also compares
 *  the z-buffer writes.
 */
static int Compare_Family_Z(const char* name, const Blitter& vanilla, const Blitter& simd, bool zwrite)
{
    static const int lengths[] = { 0, 1, 7, 8, 9, 17, 33, 64, 127, 200, 255 };
    const int z_min = 0x4000;
    const int ZN = 250;     // ring size in words
    const int ZOFF = 50;    // z_buff start offset (rows >200 px wrap the ring)

    unsigned char  src[256];
    unsigned short da[256], db[256];
    static unsigned short zva[512], zvb[512];

    alignas(8) static unsigned char fakez_store[sizeof(ZBuffer)];
    ZBuffer* fz = reinterpret_cast<ZBuffer*>(fakez_store);
    ZBuffer* saved = DepthBuffer;

    int mismatches = 0;
    int first_len = -1, first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];
        for (int rep = 0; rep < 32; ++rep) {

            for (int i = 0; i < len; ++i) {
                src[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(Rng() & 0xFF);
            }
            for (int i = 0; i < len; ++i) {
                unsigned short v = (unsigned short)(Rng() & 0xFFFF);
                da[i] = v; db[i] = v;
            }
            for (int i = 0; i < ZN; ++i) {
                unsigned short zv = (unsigned short)(Rng() & 0x7FFF);    // z_min=0x4000 -> ~half pass
                zva[i] = zv; zvb[i] = zv;
            }

            fz->BufferStart = (unsigned int)zva;
            fz->BufferEnd   = (unsigned int)(zva + ZN);
            fz->BufferSize  = ZN * 2;
            DepthBuffer = fz;
            vanilla.BlitForward(da, src, len, z_min, &zva[ZOFF], nullptr, 1000, 0);

            fz->BufferStart = (unsigned int)zvb;
            fz->BufferEnd   = (unsigned int)(zvb + ZN);
            fz->BufferSize  = ZN * 2;
            DepthBuffer = fz;
            simd.BlitForward(db, src, len, z_min, &zvb[ZOFF], nullptr, 1000, 0);

            for (int i = 0; i < len; ++i) {
                if (da[i] != db[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = i; } }
            }
            if (zwrite) {
                for (int i = 0; i < ZN; ++i) {
                    if (zva[i] != zvb[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = -i - 1; } }
                }
            }
        }
    }

    DepthBuffer = saved;

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n", name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }
    return mismatches;
}

/**
 *  Compare an Alpha and/or Warp family. Points the (init-time null) DepthBuffer/AlphaBuffer at
 *  fake rings so both blitters wrap identically; warp blits read the background from
 *  dest[warp_off], so dest has slack on both sides. The alpha index uses the blitter's own
 *  AlphaLightingRemap (Init'd in its ctor), identical for vanilla and SIMD.
 */
static int Compare_Family_AW(const char* name, const Blitter& vanilla, const Blitter& simd,
                             bool zread, bool zwrite, bool alpha, int warp_off)
{
    static const int lengths[] = { 0, 1, 7, 8, 9, 17, 33, 64, 127, 200, 255 };
    const int z_min = 0x4000;
    const int ZN = 250, ZOFF = 50;
    const int SLACK = 16;

    static unsigned short da[SLACK * 2 + 256], db[SLACK * 2 + 256];
    unsigned char src[256];
    static unsigned short zva[512], zvb[512];
    static unsigned short apa[512], apb[512];

    alignas(8) static unsigned char fakez[sizeof(ZBuffer)];
    alignas(8) static unsigned char fakea[sizeof(ABuffer)];
    ZBuffer* fz = reinterpret_cast<ZBuffer*>(fakez);
    ABuffer* fa = reinterpret_cast<ABuffer*>(fakea);
    ZBuffer* savedz = DepthBuffer;
    ABuffer* saveda = AlphaBuffer;

    int mismatches = 0, first_len = -1, first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];
        for (int rep = 0; rep < 32; ++rep) {

            for (int i = 0; i < len; ++i) {
                src[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(Rng() & 0xFF);
            }
            for (int i = 0; i < SLACK * 2 + len; ++i) {     // fill incl. slack (warp reads it)
                unsigned short v = (unsigned short)(Rng() & 0xFFFF);
                da[i] = v; db[i] = v;
            }
            for (int i = 0; i < ZN; ++i) {
                unsigned short z = (unsigned short)(Rng() & 0x7FFF);
                zva[i] = z; zvb[i] = z;
                unsigned short a = (unsigned short)(Rng() & 0xFF);  // alpha index 0..255
                apa[i] = a; apb[i] = a;
            }

            unsigned short* pda = da + SLACK;
            unsigned short* pdb = db + SLACK;

            if (zread) { fz->BufferStart = (unsigned int)zva; fz->BufferEnd = (unsigned int)(zva + ZN); fz->BufferSize = ZN * 2; DepthBuffer = fz; }
            if (alpha) { fa->BufferStart = (unsigned int)apa; fa->BufferEnd = (unsigned int)(apa + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa; }
            vanilla.BlitForward(pda, src, len, z_min, zread ? &zva[ZOFF] : nullptr, alpha ? &apa[ZOFF] : nullptr, 1000, warp_off);

            if (zread) { fz->BufferStart = (unsigned int)zvb; fz->BufferEnd = (unsigned int)(zvb + ZN); fz->BufferSize = ZN * 2; DepthBuffer = fz; }
            if (alpha) { fa->BufferStart = (unsigned int)apb; fa->BufferEnd = (unsigned int)(apb + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa; }
            simd.BlitForward(pdb, src, len, z_min, zread ? &zvb[ZOFF] : nullptr, alpha ? &apb[ZOFF] : nullptr, 1000, warp_off);

            for (int i = 0; i < len; ++i) {
                if (pda[i] != pdb[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = i; } }
            }
            if (zwrite) {
                for (int i = 0; i < ZN; ++i) {
                    if (zva[i] != zvb[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = -i - 1; } }
                }
            }
        }
    }

    DepthBuffer = savedz;
    AlphaBuffer = saveda;

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n", name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }
    return mismatches;
}

/**
 *  Compare a WriteAlpha family. Writers (composite=false) only write the alpha-buffer ring, so
 *  the comparison is the alpha ring (z_min is the alpha base, alpha_level the multiplier). The
 *  compositor (composite=true) reads the ring as a per-pixel weight and writes dest, so it
 *  compares dest; the weights are 0..255 (with 255 cropping up to hit the a==255 -> 256 path).
 */
static int Compare_Family_WriteAlpha(const char* name, const Blitter& vanilla, const Blitter& simd, bool composite)
{
    static const int lengths[] = { 0, 1, 7, 8, 9, 17, 33, 64, 127, 200, 255 };
    const int ZN = 250, ZOFF = 50;

    static unsigned short da[512], db[512];
    unsigned char src[256];
    static unsigned short apa[512], apb[512];

    alignas(8) static unsigned char fakea[sizeof(ABuffer)];
    ABuffer* fa = reinterpret_cast<ABuffer*>(fakea);
    ABuffer* saveda = AlphaBuffer;

    int mismatches = 0, first_len = -1, first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];
        for (int rep = 0; rep < 32; ++rep) {

            for (int i = 0; i < len; ++i) {
                src[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(1 + (Rng() % 255));
            }
            for (int i = 0; i < len; ++i) {
                unsigned short v = (unsigned short)(Rng() & 0xFFFF);
                da[i] = v; db[i] = v;
            }
            for (int i = 0; i < ZN; ++i) {
                unsigned short a = composite ? (unsigned short)(Rng() & 0xFF) : (unsigned short)(Rng() & 0xFFFF);
                apa[i] = a; apb[i] = a;
            }

            int z_min = composite ? 0x4000 : (int)(Rng() % 200);
            int alpha_level = composite ? 1000 : (int)(1 + (Rng() % 8));

            fa->BufferStart = (unsigned int)apa; fa->BufferEnd = (unsigned int)(apa + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa;
            vanilla.BlitForward(da, src, len, z_min, nullptr, &apa[ZOFF], alpha_level, 0);

            fa->BufferStart = (unsigned int)apb; fa->BufferEnd = (unsigned int)(apb + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa;
            simd.BlitForward(db, src, len, z_min, nullptr, &apb[ZOFF], alpha_level, 0);

            if (composite) {
                for (int i = 0; i < len; ++i) {
                    if (da[i] != db[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = i; } }
                }
            } else {
                for (int i = 0; i < ZN; ++i) {
                    if (apa[i] != apb[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = -i - 1; } }
                }
            }
        }
    }

    AlphaBuffer = saveda;

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n", name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }
    return mismatches;
}

/**
 *  Compare an alpha-gated translucency family (Translucent50/75 Nonzero/ZeroAlpha): the alpha
 *  buffer is read-only and gates the L50/L75 blend. The ring is filled ~50% zero so both the
 *  draw and skip sides of the gate get heavy coverage for both polarities. Compares dest.
 */
static int Compare_Family_Gate(const char* name, const Blitter& vanilla, const Blitter& simd)
{
    static const int lengths[] = { 0, 1, 7, 8, 9, 17, 33, 64, 127, 200, 255 };
    const int ZN = 250, ZOFF = 50;

    static unsigned short da[512], db[512];
    unsigned char src[256];
    static unsigned short apa[512], apb[512];

    alignas(8) static unsigned char fakea[sizeof(ABuffer)];
    ABuffer* fa = reinterpret_cast<ABuffer*>(fakea);
    ABuffer* saveda = AlphaBuffer;

    int mismatches = 0, first_len = -1, first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];
        for (int rep = 0; rep < 32; ++rep) {

            for (int i = 0; i < len; ++i) src[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(1 + (Rng() % 255));
            for (int i = 0; i < len; ++i) { unsigned short v = (unsigned short)(Rng() & 0xFFFF); da[i] = v; db[i] = v; }
            for (int i = 0; i < ZN; ++i) {
                unsigned short a = (Rng() & 1) ? (unsigned short)(1 + (Rng() % 255)) : (unsigned short)0;
                apa[i] = a; apb[i] = a;
            }

            fa->BufferStart = (unsigned int)apa; fa->BufferEnd = (unsigned int)(apa + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa;
            vanilla.BlitForward(da, src, len, 0, nullptr, &apa[ZOFF], 1000, 0);

            fa->BufferStart = (unsigned int)apb; fa->BufferEnd = (unsigned int)(apb + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa;
            simd.BlitForward(db, src, len, 0, nullptr, &apb[ZOFF], 1000, 0);

            for (int i = 0; i < len; ++i) {
                if (da[i] != db[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = i; } }
            }
        }
    }

    AlphaBuffer = saveda;

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n", name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }
    return mismatches;
}

/**
 *  Encode a logical pixel row (0 = transparent, else = opaque palette index) into the
 *  RLE-Zero byte stream the blitters decode: a 0x00 byte + count = a transparent run
 *  (split at 255), any non-zero byte = one opaque pixel. Returns the stream length.
 */
static int Encode_RLE(const unsigned char* logical, int n, unsigned char* stream)
{
    int e = 0, i = 0;
    while (i < n) {
        if (logical[i] == 0) {
            int run = 0;
            while (i < n && logical[i] == 0 && run < 255) { ++run; ++i; }
            stream[e++] = 0;
            stream[e++] = (unsigned char)run;
        } else {
            stream[e++] = logical[i++];
        }
    }
    return e;
}

/**
 *  Compare one non-Z RLE family. Builds random RLE shapes (with transparent runs) across
 *  awkward lengths, sometimes with a left-edge leadskip clip, and checks that the SIMD
 *  decode matches the bound vanilla routine pixel-for-pixel.
 */
static int Compare_RLE_Family(const char* name, const RLEBlitter& vanilla, const RLEBlitter& simd)
{
    static const int lengths[] = { 0, 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 33, 64, 127, 255 };

    unsigned char  logical[600];
    unsigned char  stream[1200];
    unsigned short da[512], db[512];

    int mismatches = 0, first_len = -1, first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];

        for (int rep = 0; rep < 64; ++rep) {

            int leadskip = (len > 0 && (rep & 3) == 0) ? (int)(Rng() % (unsigned)(len + 1)) : 0;
            int total = len + leadskip;

            for (int i = 0; i < total; ++i) {
                logical[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(1 + (Rng() % 255));
            }
            Encode_RLE(logical, total, stream);

            for (int i = 0; i < len; ++i) {
                unsigned short v = (unsigned short)(Rng() & 0xFFFF);
                da[i] = v; db[i] = v;
            }

            vanilla.Blit(da, stream, len, leadskip, 0, 0, 0, 0, 0, 0);
            simd.Blit(db, stream, len, leadskip, 0, 0, 0, 0, 0, 0);

            for (int i = 0; i < len; ++i) {
                if (da[i] != db[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = i; } }
            }
        }
    }

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n", name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }
    return mismatches;
}

/**
 *  Compare a depth-tested RLE family. Fakes a DepthBuffer ring (so both blitters wrap
 *  identically) and supplies a per-pixel signed zshape so the z-test `(z_min - *zshape) <
 *  *zp` is exercised with varied thresholds. ZReadWrite also compares the z-buffer.
 */
static int Compare_RLE_Family_Z(const char* name, const RLEBlitter& vanilla, const RLEBlitter& simd, bool zwrite)
{
    static const int lengths[] = { 0, 1, 7, 8, 9, 17, 33, 64, 127, 200, 255 };
    const int z_min = 0x4000;
    const int ZN = 250, ZOFF = 50;

    unsigned char  logical[600], stream[1200];
    unsigned short da[512], db[512];
    static unsigned short zva[512], zvb[512];
    static signed char    zshape[512];

    alignas(8) static unsigned char fakez[sizeof(ZBuffer)];
    ZBuffer* fz = reinterpret_cast<ZBuffer*>(fakez);
    ZBuffer* saved = DepthBuffer;

    int mismatches = 0, first_len = -1, first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];
        for (int rep = 0; rep < 32; ++rep) {

            int leadskip = (len > 0 && (rep & 3) == 0) ? (int)(Rng() % (unsigned)(len + 1)) : 0;
            int total = len + leadskip;
            for (int i = 0; i < total; ++i) {
                logical[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(1 + (Rng() % 255));
            }
            Encode_RLE(logical, total, stream);

            for (int i = 0; i < len; ++i) {
                unsigned short v = (unsigned short)(Rng() & 0xFFFF);
                da[i] = v; db[i] = v;
            }
            for (int i = 0; i < ZN; ++i) {
                unsigned short zv = (unsigned short)(Rng() & 0x7FFF);
                zva[i] = zv; zvb[i] = zv;
            }
            for (int i = 0; i < 512; ++i) zshape[i] = (signed char)(Rng() & 0xFF);

            void const* zsh = zshape;

            fz->BufferStart = (unsigned int)zva; fz->BufferEnd = (unsigned int)(zva + ZN); fz->BufferSize = ZN * 2; DepthBuffer = fz;
            vanilla.Blit(da, stream, len, leadskip, z_min, &zva[ZOFF], nullptr, 0, 0, zsh);

            fz->BufferStart = (unsigned int)zvb; fz->BufferEnd = (unsigned int)(zvb + ZN); fz->BufferSize = ZN * 2; DepthBuffer = fz;
            simd.Blit(db, stream, len, leadskip, z_min, &zvb[ZOFF], nullptr, 0, 0, zsh);

            for (int i = 0; i < len; ++i) {
                if (da[i] != db[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = i; } }
            }
            if (zwrite) {
                for (int i = 0; i < ZN; ++i) {
                    if (zva[i] != zvb[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = -i - 1; } }
                }
            }
        }
    }

    DepthBuffer = saved;

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n", name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }
    return mismatches;
}

/**
 *  Compare an Alpha and/or Warp RLE family. Fakes a DepthBuffer ring (zread), an AlphaBuffer
 *  ring (alpha), a per-pixel signed zshape (zread), and gives the destination slack on both
 *  sides for the warp background read (dptr[warp_off]). ZReadWrite compares the z-buffer.
 */
static int Compare_RLE_Family_AW(const char* name, const RLEBlitter& vanilla, const RLEBlitter& simd,
                                 bool zread, bool zwrite, bool alpha, int warp_off)
{
    static const int lengths[] = { 0, 1, 7, 8, 9, 17, 33, 64, 127, 200, 255 };
    const int z_min = 0x4000;
    const int ZN = 250, ZOFF = 50, SLACK = 16;

    unsigned char  logical[600], stream[1200];
    static unsigned short da[SLACK * 2 + 256], db[SLACK * 2 + 256];
    static unsigned short zva[512], zvb[512];
    static unsigned short apa[512], apb[512];
    static signed char    zshape[512];

    alignas(8) static unsigned char fakez[sizeof(ZBuffer)];
    alignas(8) static unsigned char fakea[sizeof(ABuffer)];
    ZBuffer* fz = reinterpret_cast<ZBuffer*>(fakez);
    ABuffer* fa = reinterpret_cast<ABuffer*>(fakea);
    ZBuffer* savedz = DepthBuffer;
    ABuffer* saveda = AlphaBuffer;

    int mismatches = 0, first_len = -1, first_idx = -1;

    for (int li = 0; li < (int)(sizeof(lengths) / sizeof(lengths[0])); ++li) {
        int len = lengths[li];
        for (int rep = 0; rep < 32; ++rep) {

            int leadskip = (len > 0 && (rep & 3) == 0) ? (int)(Rng() % (unsigned)(len + 1)) : 0;
            int total = len + leadskip;
            for (int i = 0; i < total; ++i) {
                logical[i] = (Rng() & 3) == 0 ? 0 : (unsigned char)(1 + (Rng() % 255));
            }
            Encode_RLE(logical, total, stream);

            for (int i = 0; i < SLACK * 2 + len; ++i) {     // fill incl. slack (warp reads it)
                unsigned short v = (unsigned short)(Rng() & 0xFFFF);
                da[i] = v; db[i] = v;
            }
            for (int i = 0; i < ZN; ++i) {
                unsigned short z = (unsigned short)(Rng() & 0x7FFF);
                zva[i] = z; zvb[i] = z;
                unsigned short a = (unsigned short)(Rng() & 0xFF);
                apa[i] = a; apb[i] = a;
            }
            for (int i = 0; i < 512; ++i) zshape[i] = (signed char)(Rng() & 0xFF);

            unsigned short* pda = da + SLACK;
            unsigned short* pdb = db + SLACK;
            void const* zsh = zshape;

            if (zread) { fz->BufferStart = (unsigned int)zva; fz->BufferEnd = (unsigned int)(zva + ZN); fz->BufferSize = ZN * 2; DepthBuffer = fz; }
            if (alpha) { fa->BufferStart = (unsigned int)apa; fa->BufferEnd = (unsigned int)(apa + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa; }
            vanilla.Blit(pda, stream, len, leadskip, z_min, zread ? (void*)&zva[ZOFF] : nullptr,
                         alpha ? (void*)&apa[ZOFF] : nullptr, 1000, warp_off, zread ? zsh : nullptr);

            if (zread) { fz->BufferStart = (unsigned int)zvb; fz->BufferEnd = (unsigned int)(zvb + ZN); fz->BufferSize = ZN * 2; DepthBuffer = fz; }
            if (alpha) { fa->BufferStart = (unsigned int)apb; fa->BufferEnd = (unsigned int)(apb + ZN); fa->BufferSize = ZN * 2; AlphaBuffer = fa; }
            simd.Blit(pdb, stream, len, leadskip, z_min, zread ? (void*)&zvb[ZOFF] : nullptr,
                      alpha ? (void*)&apb[ZOFF] : nullptr, 1000, warp_off, zread ? zsh : nullptr);

            for (int i = 0; i < len; ++i) {
                if (pda[i] != pdb[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = i; } }
            }
            if (zwrite) {
                for (int i = 0; i < ZN; ++i) {
                    if (zva[i] != zvb[i]) { ++mismatches; if (first_len < 0) { first_len = len; first_idx = -i - 1; } }
                }
            }
        }
    }

    DepthBuffer = savedz;
    AlphaBuffer = saveda;

    if (mismatches != 0) {
        DEBUG_WARNING("[SIMD blit] {}: {} MISMATCHES (first at len={} idx={})\n", name, mismatches, first_len, first_idx);
    } else {
        DEBUG_INFO("[SIMD blit] {}: ok\n", name);
    }
    return mismatches;
}

/**
 *  Run the full family matrix for one SIMD tier. Returns the total mismatch count.
 */
template<SimdTier ISA>
static int Run_Tier(const char* tier_name,
                    unsigned short const* xl, unsigned short const* il, int lv,
                    unsigned short hb, unsigned short qb,
                    unsigned char const* const* rm)
{
    DEBUG_INFO("[SIMD blit] --- {} tier ---\n", tier_name);
    int total = 0;

    { BlitPlainXlat<unsigned short> v(xl);          SimdBlitPlainXlat<ISA> s(xl);          total += Compare_Family("PlainXlat", v, s); }
    { BlitTransXlat<unsigned short> v(xl);          SimdBlitTransXlat<ISA> s(xl);          total += Compare_Family("TransXlat", v, s); }
    { BlitTransZRemapXlat<unsigned short> v(rm, xl);SimdBlitTransZRemapXlat<ISA> s(rm, xl); total += Compare_Family("TransZRemapXlat", v, s); }
    { BlitTransDarken<unsigned short> v(hb);        SimdBlitTransDarken<ISA> s(hb);        total += Compare_Family("TransDarken", v, s); }
    { BlitTransLucent75<unsigned short> v(xl, qb);  SimdBlitTransLucent75<ISA> s(xl, qb);   total += Compare_Family("TransLucent75", v, s); }
    { BlitTransLucent50<unsigned short> v(xl, hb);  SimdBlitTransLucent50<ISA> s(xl, hb);   total += Compare_Family("TransLucent50", v, s); }
    { BlitTransLucent25<unsigned short> v(xl, qb);  SimdBlitTransLucent25<ISA> s(xl, qb);   total += Compare_Family("TransLucent25", v, s); }

    { BlitPlainXlatZRead<unsigned short> v(xl);          SimdBlitPlainXlatZRead<ISA> s(xl);          total += Compare_Family_Z("PlainXlatZRead", v, s, false); }
    { BlitTransXlatZRead<unsigned short> v(xl);          SimdBlitTransXlatZRead<ISA> s(xl);          total += Compare_Family_Z("TransXlatZRead", v, s, false); }
    { BlitTransZRemapXlatZRead<unsigned short> v(rm, xl);SimdBlitTransZRemapXlatZRead<ISA> s(rm, xl); total += Compare_Family_Z("TransZRemapXlatZRead", v, s, false); }
    { BlitTransDarkenZRead<unsigned short> v(hb);        SimdBlitTransDarkenZRead<ISA> s(hb);        total += Compare_Family_Z("TransDarkenZRead", v, s, false); }
    { BlitTransLucent75ZRead<unsigned short> v(xl, qb);  SimdBlitTransLucent75ZRead<ISA> s(xl, qb);   total += Compare_Family_Z("TransLucent75ZRead", v, s, false); }
    { BlitTransLucent50ZRead<unsigned short> v(xl, hb);  SimdBlitTransLucent50ZRead<ISA> s(xl, hb);   total += Compare_Family_Z("TransLucent50ZRead", v, s, false); }
    { BlitTransLucent25ZRead<unsigned short> v(xl, qb);  SimdBlitTransLucent25ZRead<ISA> s(xl, qb);   total += Compare_Family_Z("TransLucent25ZRead", v, s, false); }

    { BlitPlainXlatZReadWrite<unsigned short> v(xl);          SimdBlitPlainXlatZReadWrite<ISA> s(xl);          total += Compare_Family_Z("PlainXlatZReadWrite", v, s, true); }
    { BlitTransXlatZReadWrite<unsigned short> v(xl);          SimdBlitTransXlatZReadWrite<ISA> s(xl);          total += Compare_Family_Z("TransXlatZReadWrite", v, s, true); }
    { BlitTransZRemapXlatZReadWrite<unsigned short> v(rm, xl);SimdBlitTransZRemapXlatZReadWrite<ISA> s(rm, xl); total += Compare_Family_Z("TransZRemapXlatZReadWrite", v, s, true); }
    { BlitTransDarkenZReadWrite<unsigned short> v(hb);        SimdBlitTransDarkenZReadWrite<ISA> s(hb);        total += Compare_Family_Z("TransDarkenZReadWrite", v, s, true); }
    { BlitTransLucent75ZReadWrite<unsigned short> v(xl, qb);  SimdBlitTransLucent75ZReadWrite<ISA> s(xl, qb);   total += Compare_Family_Z("TransLucent75ZReadWrite", v, s, true); }
    { BlitTransLucent50ZReadWrite<unsigned short> v(xl, hb);  SimdBlitTransLucent50ZReadWrite<ISA> s(xl, hb);   total += Compare_Family_Z("TransLucent50ZReadWrite", v, s, true); }
    { BlitTransLucent25ZReadWrite<unsigned short> v(xl, qb);  SimdBlitTransLucent25ZReadWrite<ISA> s(xl, qb);   total += Compare_Family_Z("TransLucent25ZReadWrite", v, s, true); }

    /* Alpha (no z). */
    { BlitPlainXlatAlpha<unsigned short> v(il, lv);       SimdBlitPlainXlatAlpha<ISA> s(il, lv);       total += Compare_Family_AW("PlainXlatAlpha", v, s, false, false, true, 0); }
    { BlitTransXlatAlpha<unsigned short> v(il, lv);       SimdBlitTransXlatAlpha<ISA> s(il, lv);       total += Compare_Family_AW("TransXlatAlpha", v, s, false, false, true, 0); }
    { BlitTransZRemapXlatAlpha<unsigned short> v(rm,il,lv);SimdBlitTransZRemapXlatAlpha<ISA> s(rm,il,lv);total += Compare_Family_AW("TransZRemapXlatAlpha", v, s, false, false, true, 0); }
    { BlitTransLucent75Alpha<unsigned short> v(il,lv,qb); SimdBlitTransLucent75Alpha<ISA> s(il,lv,qb); total += Compare_Family_AW("TransLucent75Alpha", v, s, false, false, true, 0); }
    { BlitTransLucent50Alpha<unsigned short> v(il,lv,hb); SimdBlitTransLucent50Alpha<ISA> s(il,lv,hb); total += Compare_Family_AW("TransLucent50Alpha", v, s, false, false, true, 0); }
    { BlitTransLucent25Alpha<unsigned short> v(il,lv,qb); SimdBlitTransLucent25Alpha<ISA> s(il,lv,qb); total += Compare_Family_AW("TransLucent25Alpha", v, s, false, false, true, 0); }

    /* Alpha + Z-read. */
    { BlitTransXlatAlphaZRead<unsigned short> v(il,lv);        SimdBlitTransXlatAlphaZRead<ISA> s(il,lv);        total += Compare_Family_AW("TransXlatAlphaZRead", v, s, true, false, true, 0); }
    { BlitTransZRemapXlatAlphaZRead<unsigned short> v(rm,il,lv);SimdBlitTransZRemapXlatAlphaZRead<ISA> s(rm,il,lv);total += Compare_Family_AW("TransZRemapXlatAlphaZRead", v, s, true, false, true, 0); }
    { BlitTransLucent75AlphaZRead<unsigned short> v(il,lv,qb);  SimdBlitTransLucent75AlphaZRead<ISA> s(il,lv,qb);  total += Compare_Family_AW("TransLucent75AlphaZRead", v, s, true, false, true, 0); }
    { BlitTransLucent50AlphaZRead<unsigned short> v(il,lv,hb);  SimdBlitTransLucent50AlphaZRead<ISA> s(il,lv,hb);  total += Compare_Family_AW("TransLucent50AlphaZRead", v, s, true, false, true, 0); }
    { BlitTransLucent25AlphaZRead<unsigned short> v(il,lv,qb);  SimdBlitTransLucent25AlphaZRead<ISA> s(il,lv,qb);  total += Compare_Family_AW("TransLucent25AlphaZRead", v, s, true, false, true, 0); }

    /* Alpha + Z-read/write. */
    { BlitTransXlatAlphaZReadWrite<unsigned short> v(il,lv);        SimdBlitTransXlatAlphaZReadWrite<ISA> s(il,lv);        total += Compare_Family_AW("TransXlatAlphaZReadWrite", v, s, true, true, true, 0); }
    { BlitTransZRemapXlatAlphaZReadWrite<unsigned short> v(rm,il,lv);SimdBlitTransZRemapXlatAlphaZReadWrite<ISA> s(rm,il,lv);total += Compare_Family_AW("TransZRemapXlatAlphaZReadWrite", v, s, true, true, true, 0); }
    { BlitTransLucent75AlphaZReadWrite<unsigned short> v(il,lv,qb);  SimdBlitTransLucent75AlphaZReadWrite<ISA> s(il,lv,qb);  total += Compare_Family_AW("TransLucent75AlphaZReadWrite", v, s, true, true, true, 0); }
    { BlitTransLucent50AlphaZReadWrite<unsigned short> v(il,lv,hb);  SimdBlitTransLucent50AlphaZReadWrite<ISA> s(il,lv,hb);  total += Compare_Family_AW("TransLucent50AlphaZReadWrite", v, s, true, true, true, 0); }
    { BlitTransLucent25AlphaZReadWrite<unsigned short> v(il,lv,qb);  SimdBlitTransLucent25AlphaZReadWrite<ISA> s(il,lv,qb);  total += Compare_Family_AW("TransLucent25AlphaZReadWrite", v, s, true, true, true, 0); }

    /* Alpha + Z-read + Warp. */
    { BlitTransLucent75AlphaZReadWarp<unsigned short> v(il,lv,qb);  SimdBlitTransLucent75AlphaZReadWarp<ISA> s(il,lv,qb);  total += Compare_Family_AW("TransLucent75AlphaZReadWarp", v, s, true, false, true, -3); }
    { BlitTransLucent50AlphaZReadWarp<unsigned short> v(il,lv,hb);  SimdBlitTransLucent50AlphaZReadWarp<ISA> s(il,lv,hb);  total += Compare_Family_AW("TransLucent50AlphaZReadWarp", v, s, true, false, true, -3); }
    { BlitTransLucent25AlphaZReadWarp<unsigned short> v(il,lv,qb);  SimdBlitTransLucent25AlphaZReadWarp<ISA> s(il,lv,qb);  total += Compare_Family_AW("TransLucent25AlphaZReadWarp", v, s, true, false, true, -3); }

    /* Z-read + Warp (no alpha). */
    { BlitTransLucent75ZReadWarp<unsigned short> v(xl,qb);  SimdBlitTransLucent75ZReadWarp<ISA> s(xl,qb);  total += Compare_Family_AW("TransLucent75ZReadWarp", v, s, true, false, false, -3); }
    { BlitTransLucent50ZReadWarp<unsigned short> v(xl,hb);  SimdBlitTransLucent50ZReadWarp<ISA> s(xl,hb);  total += Compare_Family_AW("TransLucent50ZReadWarp", v, s, true, false, false, -3); }
    { BlitTransLucent25ZReadWarp<unsigned short> v(xl,qb);  SimdBlitTransLucent25ZReadWarp<ISA> s(xl,qb);  total += Compare_Family_AW("TransLucent25ZReadWarp", v, s, true, false, false, -3); }

    /* Alpha-buffer writers / compositor. */
    { BlitTransXlatWriteAlpha<unsigned short> v;      SimdBlitTransXlatWriteAlpha<ISA> s;          total += Compare_Family_WriteAlpha("TransXlatWriteAlpha", v, s, false); }
    { BlitTransXlatMultWriteAlpha<unsigned short> v;  SimdBlitTransXlatMultWriteAlpha<ISA> s;      total += Compare_Family_WriteAlpha("TransXlatMultWriteAlpha", v, s, false); }
    { BlitTranslucentWriteAlpha<unsigned short> v(il);SimdBlitTranslucentWriteAlpha<ISA> s(il);    total += Compare_Family_WriteAlpha("TranslucentWriteAlpha", v, s, true); }

    /* Alpha-gated translucency. */
    { BlitTranslucent50NonzeroAlpha<unsigned short> v(xl,hb); SimdBlitTranslucent50NonzeroAlpha<ISA> s(xl,hb); total += Compare_Family_Gate("Translucent50NonzeroAlpha", v, s); }
    { BlitTranslucent50ZeroAlpha<unsigned short> v(xl,hb);    SimdBlitTranslucent50ZeroAlpha<ISA> s(xl,hb);    total += Compare_Family_Gate("Translucent50ZeroAlpha", v, s); }
    { BlitTranslucent75NonzeroAlpha<unsigned short> v(xl,qb); SimdBlitTranslucent75NonzeroAlpha<ISA> s(xl,qb); total += Compare_Family_Gate("Translucent75NonzeroAlpha", v, s); }
    { BlitTranslucent75ZeroAlpha<unsigned short> v(xl,qb);    SimdBlitTranslucent75ZeroAlpha<ISA> s(xl,qb);    total += Compare_Family_Gate("Translucent75ZeroAlpha", v, s); }

    /* RLE (non-Z wave). */
    { RLEBlitTransXlat<unsigned short> v(xl);          SimdRLEBlitTransXlat<ISA> s(xl);          total += Compare_RLE_Family("RLE TransXlat", v, s); }
    { RLEBlitTransZRemapXlat<unsigned short> v(rm, xl);SimdRLEBlitTransZRemapXlat<ISA> s(rm, xl); total += Compare_RLE_Family("RLE ZRemapXlat", v, s); }
    { RLEBlitTransDarken<unsigned short> v(hb);        SimdRLEBlitTransDarken<ISA> s(hb);        total += Compare_RLE_Family("RLE Darken", v, s); }
    { RLEBlitTransLucent75<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent75<ISA> s(xl, qb);   total += Compare_RLE_Family("RLE Lucent75", v, s); }
    { RLEBlitTransLucent50<unsigned short> v(xl, hb);  SimdRLEBlitTransLucent50<ISA> s(xl, hb);   total += Compare_RLE_Family("RLE Lucent50", v, s); }
    { RLEBlitTransLucent25<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent25<ISA> s(xl, qb);   total += Compare_RLE_Family("RLE Lucent25", v, s); }

    /* RLE Z-read. */
    { RLEBlitTransXlatZRead<unsigned short> v(xl);          SimdRLEBlitTransXlatZRead<ISA> s(xl);          total += Compare_RLE_Family_Z("RLE TransXlatZRead", v, s, false); }
    { RLEBlitTransZRemapXlatZRead<unsigned short> v(rm, xl);SimdRLEBlitTransZRemapXlatZRead<ISA> s(rm, xl); total += Compare_RLE_Family_Z("RLE ZRemapXlatZRead", v, s, false); }
    { RLEBlitTransDarkenZRead<unsigned short> v(hb);        SimdRLEBlitTransDarkenZRead<ISA> s(hb);        total += Compare_RLE_Family_Z("RLE DarkenZRead", v, s, false); }
    { RLEBlitTransLucent75ZRead<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent75ZRead<ISA> s(xl, qb);   total += Compare_RLE_Family_Z("RLE Lucent75ZRead", v, s, false); }
    { RLEBlitTransLucent50ZRead<unsigned short> v(xl, hb);  SimdRLEBlitTransLucent50ZRead<ISA> s(xl, hb);   total += Compare_RLE_Family_Z("RLE Lucent50ZRead", v, s, false); }
    { RLEBlitTransLucent25ZRead<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent25ZRead<ISA> s(xl, qb);   total += Compare_RLE_Family_Z("RLE Lucent25ZRead", v, s, false); }

    /* RLE Z-read/write. */
    { RLEBlitTransXlatZReadWrite<unsigned short> v(xl);          SimdRLEBlitTransXlatZReadWrite<ISA> s(xl);          total += Compare_RLE_Family_Z("RLE TransXlatZReadWrite", v, s, true); }
    { RLEBlitTransZRemapXlatZReadWrite<unsigned short> v(rm, xl);SimdRLEBlitTransZRemapXlatZReadWrite<ISA> s(rm, xl); total += Compare_RLE_Family_Z("RLE ZRemapXlatZReadWrite", v, s, true); }
    { RLEBlitTransDarkenZReadWrite<unsigned short> v(hb);        SimdRLEBlitTransDarkenZReadWrite<ISA> s(hb);        total += Compare_RLE_Family_Z("RLE DarkenZReadWrite", v, s, true); }
    { RLEBlitTransLucent75ZReadWrite<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent75ZReadWrite<ISA> s(xl, qb);   total += Compare_RLE_Family_Z("RLE Lucent75ZReadWrite", v, s, true); }
    { RLEBlitTransLucent50ZReadWrite<unsigned short> v(xl, hb);  SimdRLEBlitTransLucent50ZReadWrite<ISA> s(xl, hb);   total += Compare_RLE_Family_Z("RLE Lucent50ZReadWrite", v, s, true); }
    { RLEBlitTransLucent25ZReadWrite<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent25ZReadWrite<ISA> s(xl, qb);   total += Compare_RLE_Family_Z("RLE Lucent25ZReadWrite", v, s, true); }

    /* RLE Alpha (no z). */
    { RLEBlitTransXlatAlpha<unsigned short> v(il, lv);          SimdRLEBlitTransXlatAlpha<ISA> s(il, lv);          total += Compare_RLE_Family_AW("RLE TransXlatAlpha", v, s, false, false, true, 0); }
    { RLEBlitTransZRemapXlatAlpha<unsigned short> v(rm, il, lv);SimdRLEBlitTransZRemapXlatAlpha<ISA> s(rm, il, lv); total += Compare_RLE_Family_AW("RLE ZRemapXlatAlpha", v, s, false, false, true, 0); }
    { RLEBlitTransLucent75Alpha<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent75Alpha<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent75Alpha", v, s, false, false, true, 0); }
    { RLEBlitTransLucent50Alpha<unsigned short> v(il, lv, hb);  SimdRLEBlitTransLucent50Alpha<ISA> s(il, lv, hb);   total += Compare_RLE_Family_AW("RLE Lucent50Alpha", v, s, false, false, true, 0); }
    { RLEBlitTransLucent25Alpha<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent25Alpha<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent25Alpha", v, s, false, false, true, 0); }

    /* RLE Alpha + Z-read. */
    { RLEBlitTransXlatAlphaZRead<unsigned short> v(il, lv);          SimdRLEBlitTransXlatAlphaZRead<ISA> s(il, lv);          total += Compare_RLE_Family_AW("RLE TransXlatAlphaZRead", v, s, true, false, true, 0); }
    { RLEBlitTransZRemapXlatAlphaZRead<unsigned short> v(rm, il, lv);SimdRLEBlitTransZRemapXlatAlphaZRead<ISA> s(rm, il, lv); total += Compare_RLE_Family_AW("RLE ZRemapXlatAlphaZRead", v, s, true, false, true, 0); }
    { RLEBlitTransLucent75AlphaZRead<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent75AlphaZRead<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent75AlphaZRead", v, s, true, false, true, 0); }
    { RLEBlitTransLucent50AlphaZRead<unsigned short> v(il, lv, hb);  SimdRLEBlitTransLucent50AlphaZRead<ISA> s(il, lv, hb);   total += Compare_RLE_Family_AW("RLE Lucent50AlphaZRead", v, s, true, false, true, 0); }
    { RLEBlitTransLucent25AlphaZRead<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent25AlphaZRead<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent25AlphaZRead", v, s, true, false, true, 0); }

    /* RLE Alpha + Z-read/write. */
    { RLEBlitTransXlatAlphaZReadWrite<unsigned short> v(il, lv);          SimdRLEBlitTransXlatAlphaZReadWrite<ISA> s(il, lv);          total += Compare_RLE_Family_AW("RLE TransXlatAlphaZReadWrite", v, s, true, true, true, 0); }
    { RLEBlitTransZRemapXlatAlphaZReadWrite<unsigned short> v(rm, il, lv);SimdRLEBlitTransZRemapXlatAlphaZReadWrite<ISA> s(rm, il, lv); total += Compare_RLE_Family_AW("RLE ZRemapXlatAlphaZReadWrite", v, s, true, true, true, 0); }
    { RLEBlitTransLucent75AlphaZReadWrite<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent75AlphaZReadWrite<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent75AlphaZReadWrite", v, s, true, true, true, 0); }
    { RLEBlitTransLucent50AlphaZReadWrite<unsigned short> v(il, lv, hb);  SimdRLEBlitTransLucent50AlphaZReadWrite<ISA> s(il, lv, hb);   total += Compare_RLE_Family_AW("RLE Lucent50AlphaZReadWrite", v, s, true, true, true, 0); }
    { RLEBlitTransLucent25AlphaZReadWrite<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent25AlphaZReadWrite<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent25AlphaZReadWrite", v, s, true, true, true, 0); }

    /* RLE Z-read + Warp (no alpha). */
    { RLEBlitTransLucent75ZReadWarp<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent75ZReadWarp<ISA> s(xl, qb);   total += Compare_RLE_Family_AW("RLE Lucent75ZReadWarp", v, s, true, false, false, -3); }
    { RLEBlitTransLucent50ZReadWarp<unsigned short> v(xl, hb);  SimdRLEBlitTransLucent50ZReadWarp<ISA> s(xl, hb);   total += Compare_RLE_Family_AW("RLE Lucent50ZReadWarp", v, s, true, false, false, -3); }
    { RLEBlitTransLucent25ZReadWarp<unsigned short> v(xl, qb);  SimdRLEBlitTransLucent25ZReadWarp<ISA> s(xl, qb);   total += Compare_RLE_Family_AW("RLE Lucent25ZReadWarp", v, s, true, false, false, -3); }

    /* RLE Alpha + Z-read + Warp. */
    { RLEBlitTransLucent75AlphaZReadWarp<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent75AlphaZReadWarp<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent75AlphaZReadWarp", v, s, true, false, true, -3); }
    { RLEBlitTransLucent50AlphaZReadWarp<unsigned short> v(il, lv, hb);  SimdRLEBlitTransLucent50AlphaZReadWarp<ISA> s(il, lv, hb);   total += Compare_RLE_Family_AW("RLE Lucent50AlphaZReadWarp", v, s, true, false, true, -3); }
    { RLEBlitTransLucent25AlphaZReadWarp<unsigned short> v(il, lv, qb);  SimdRLEBlitTransLucent25AlphaZReadWarp<ISA> s(il, lv, qb);   total += Compare_RLE_Family_AW("RLE Lucent25AlphaZReadWarp", v, s, true, false, true, -3); }

    return total;
}

} // namespace


/**
 *  Run the full bit-exactness matrix using the live drawer's tables.
 */
void Blitter_SIMD_SelfTest(ConvertClass* drawer)
{
    /**
     *  The translate/remap/mask tables are public members of ConvertClass.
     */
    struct Tables { unsigned short const* xl; unsigned short const* il; int lv; unsigned short hb; unsigned short qb; };
    Tables t;
    t.xl = (unsigned short const*)drawer->Translator;
    t.il = (unsigned short const*)drawer->IntensityTranslator;
    t.lv = drawer->IntensityLevels;
    t.hb = (unsigned short)drawer->HalfbrightMask;
    t.qb = (unsigned short)drawer->QuarterbrightMask;

    /**
     *  The drawer's RemapTable is only populated per-object at draw time, so it is not a valid
     *  pointer during init. Supply our own identity remap so the ZRemap test reads live memory
     *  (the test only needs valid tables, not specific contents; in-game the engine sets it).
     */
    static unsigned char remap_data[256];
    for (int i = 0; i < 256; ++i) remap_data[i] = (unsigned char)i;
    unsigned char const* remap_ptr = remap_data;
    unsigned char const* const* rm = &remap_ptr;

    DEBUG_INFO("[SIMD blit] Running bit-exactness self-test...\n");
    int total = 0;

    total += Run_Tier<SimdTier::SSE2>("SSE2", t.xl, t.il, t.lv, t.hb, t.qb, rm);
    if (CPUDetectClass::Has_AVX2_Instruction_Set()) {
        total += Run_Tier<SimdTier::AVX2>("AVX2", t.xl, t.il, t.lv, t.hb, t.qb, rm);
    }

    if (total == 0) {
        DEBUG_INFO("[SIMD blit] Self-test PASSED (all families bit-exact).\n");
    } else {
        DEBUG_WARNING("[SIMD blit] Self-test FAILED: {} total mismatching pixels.\n", total);
    }
}


#ifdef BLITTER_BENCH

namespace
{

static volatile unsigned g_bench_sink = 0;

/**
 *  Time one blit closure: warm up, then take the minimum cycle count over several batches
 *  (min rejects scheduler/interrupt noise). Returns cycles per call (TSC ticks; only the
 *  vanilla-vs-SIMD ratio is meaningful, not the absolute).
 */
template<class Fn>
static unsigned long long Time_Call(Fn fn, int iters, int batches)
{
    for (int i = 0; i < 200; ++i) fn();
    unsigned long long best = ~0ull;
    for (int b = 0; b < batches; ++b) {
        unsigned long long t0 = __rdtsc();
        for (int i = 0; i < iters; ++i) fn();
        unsigned long long t1 = __rdtsc();
        if (t1 - t0 < best) best = t1 - t0;
    }
    return best / (unsigned long long)iters;
}

static void Bench_Std(const char* name, bool avx, const Blitter& v, const Blitter& s2, const Blitter& sa,
                      unsigned short* dst, unsigned char* src, int w, int z_min, unsigned short* zbuf)
{
    const int IT = 1000, BA = 24;
    unsigned long long cv = Time_Call([&] { v.BlitForward(dst, src, w, z_min, zbuf, nullptr, 1000, 0); }, IT, BA);
    g_bench_sink += dst[w >> 1];
    unsigned long long c2 = Time_Call([&] { s2.BlitForward(dst, src, w, z_min, zbuf, nullptr, 1000, 0); }, IT, BA);
    g_bench_sink += dst[w >> 1];
    if (avx) {
        unsigned long long ca = Time_Call([&] { sa.BlitForward(dst, src, w, z_min, zbuf, nullptr, 1000, 0); }, IT, BA);
        g_bench_sink += dst[w >> 1];
        DEBUG_INFO("[SIMD bench] {:<26} w={:>3}: van={:>6}  sse2={:>6} ({:.2f}x)  avx2={:>6} ({:.2f}x)\n",
                   name, w, cv, c2, (double)cv / (double)c2, ca, (double)cv / (double)ca);
    } else {
        DEBUG_INFO("[SIMD bench] {:<26} w={:>3}: van={:>6}  sse2={:>6} ({:.2f}x)\n",
                   name, w, cv, c2, (double)cv / (double)c2);
    }
}

static void Bench_RLE(const char* name, bool avx, const RLEBlitter& v, const RLEBlitter& s2, const RLEBlitter& sa,
                      unsigned short* dst, unsigned char* stream, int w)
{
    const int IT = 1000, BA = 24;
    unsigned long long cv = Time_Call([&] { v.Blit(dst, stream, w, 0, 0, 0, 0, 1000, 0, 0); }, IT, BA);
    g_bench_sink += dst[w >> 1];
    unsigned long long c2 = Time_Call([&] { s2.Blit(dst, stream, w, 0, 0, 0, 0, 1000, 0, 0); }, IT, BA);
    g_bench_sink += dst[w >> 1];
    if (avx) {
        unsigned long long ca = Time_Call([&] { sa.Blit(dst, stream, w, 0, 0, 0, 0, 1000, 0, 0); }, IT, BA);
        g_bench_sink += dst[w >> 1];
        DEBUG_INFO("[SIMD bench] {:<26} w={:>3}: van={:>6}  sse2={:>6} ({:.2f}x)  avx2={:>6} ({:.2f}x)\n",
                   name, w, cv, c2, (double)cv / (double)c2, ca, (double)cv / (double)ca);
    } else {
        DEBUG_INFO("[SIMD bench] {:<26} w={:>3}: van={:>6}  sse2={:>6} ({:.2f}x)\n",
                   name, w, cv, c2, (double)cv / (double)c2);
    }
}

} // namespace

/**
 *  Time the SIMD tiers against the bound vanilla blitters across representative families and row
 *  widths. The gather-bound families (Xlat/ZRemap) should show the AVX2 win; the arithmetic ones
 *  (Darken/Lucent) the SSE2 win; the width sweep shows how the short-row scalar tail erodes wide
 *  vectorisation. Source is ~12% transparent (terrain/unit-like).
 */
void Blitter_SIMD_Benchmark(ConvertClass* drawer)
{
    unsigned short const* xl = (unsigned short const*)drawer->Translator;
    unsigned short hb = (unsigned short)drawer->HalfbrightMask;

    static unsigned char remap_data[256];
    for (int i = 0; i < 256; ++i) remap_data[i] = (unsigned char)i;
    unsigned char const* rp = remap_data;
    unsigned char const* const* rm = &rp;

    const bool avx = CPUDetectClass::Has_AVX2_Instruction_Set();

    static unsigned short dst[1024];
    static unsigned char  src[512];
    static unsigned char  stream[1200];
    static unsigned short zbuf[1024];

    for (int i = 0; i < 512; ++i)  src[i] = (Rng() % 100u < 12u) ? 0 : (unsigned char)(1 + (Rng() % 255));
    for (int i = 0; i < 1024; ++i) dst[i] = (unsigned short)(Rng() & 0xFFFF);
    for (int i = 0; i < 1024; ++i) zbuf[i] = (unsigned short)(Rng() & 0x7FFF);

    alignas(8) static unsigned char fakez[sizeof(ZBuffer)];
    ZBuffer* fz = reinterpret_cast<ZBuffer*>(fakez);
    ZBuffer* savedz = DepthBuffer;
    fz->BufferStart = (unsigned int)zbuf; fz->BufferEnd = (unsigned int)(zbuf + 1024); fz->BufferSize = 1024 * 2;
    DepthBuffer = fz;
    const int z_min = 0x4000;

    static const int W[] = { 32, 64, 256 };

    DEBUG_INFO("[SIMD bench] min cycles/call (lower=faster); source ~12%% transparent; {}\n",
               avx ? "AVX2 present" : "no AVX2 on this CPU");

    for (int wi = 0; wi < 3; ++wi) {
        int w = W[wi];
        { BlitTransXlat<unsigned short> v(xl); SimdBlitTransXlat<SimdTier::SSE2> s2(xl); SimdBlitTransXlat<SimdTier::AVX2> sa(xl);
          Bench_Std("TransXlat (gather)", avx, v, s2, sa, dst, src, w, 0, nullptr); }
        { BlitTransZRemapXlat<unsigned short> v(rm, xl); SimdBlitTransZRemapXlat<SimdTier::SSE2> s2(rm, xl); SimdBlitTransZRemapXlat<SimdTier::AVX2> sa(rm, xl);
          Bench_Std("TransZRemapXlat (gather)", avx, v, s2, sa, dst, src, w, 0, nullptr); }
        { BlitTransDarken<unsigned short> v(hb); SimdBlitTransDarken<SimdTier::SSE2> s2(hb); SimdBlitTransDarken<SimdTier::AVX2> sa(hb);
          Bench_Std("TransDarken (no gather)", avx, v, s2, sa, dst, src, w, 0, nullptr); }
        { BlitTransLucent50<unsigned short> v(xl, hb); SimdBlitTransLucent50<SimdTier::SSE2> s2(xl, hb); SimdBlitTransLucent50<SimdTier::AVX2> sa(xl, hb);
          Bench_Std("TransLucent50 (arith)", avx, v, s2, sa, dst, src, w, 0, nullptr); }
        { BlitTransXlatZRead<unsigned short> v(xl); SimdBlitTransXlatZRead<SimdTier::SSE2> s2(xl); SimdBlitTransXlatZRead<SimdTier::AVX2> sa(xl);
          Bench_Std("TransXlatZRead (gather+z)", avx, v, s2, sa, dst, src, w, z_min, zbuf); }

        int n = Encode_RLE(src, w, stream); (void)n;
        { RLEBlitTransXlat<unsigned short> v(xl); SimdRLEBlitTransXlat<SimdTier::SSE2> s2(xl); SimdRLEBlitTransXlat<SimdTier::AVX2> sa(xl);
          Bench_RLE("RLE TransXlat (gather)", avx, v, s2, sa, dst, stream, w); }
        { RLEBlitTransLucent50<unsigned short> v(xl, hb); SimdRLEBlitTransLucent50<SimdTier::SSE2> s2(xl, hb); SimdRLEBlitTransLucent50<SimdTier::AVX2> sa(xl, hb);
          Bench_RLE("RLE TransLucent50 (arith)", avx, v, s2, sa, dst, stream, w); }
    }

    DepthBuffer = savedz;
    DEBUG_INFO("[SIMD bench] done (sink={}).\n", (unsigned)g_bench_sink);
}

#endif // BLITTER_BENCH

#endif // BLITTER_TESTS || BLITTER_BENCH
