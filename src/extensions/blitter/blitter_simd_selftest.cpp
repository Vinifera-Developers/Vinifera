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
#include "convert.h"
#include "debughandler.h"


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

} // namespace


/**
 *  Run the full bit-exactness matrix using the live drawer's tables.
 */
void Blitter_SIMD_SelfTest(ConvertClass* drawer)
{
    /**
     *  The translate/remap/mask tables are public members of ConvertClass.
     */
    struct Tables { unsigned short const* xl; unsigned short hb; unsigned short qb; };
    Tables t;
    t.xl = (unsigned short const*)drawer->Translator;
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

    DEBUG_INFO("[SIMD blit] Running SSE2 bit-exactness self-test...\n");
    int total = 0;

    {
        BlitPlainXlat<unsigned short> v(t.xl);
        SimdBlitPlainXlat<SimdTier::SSE2> s(t.xl);
        total += Compare_Family("PlainXlat", v, s);
    }
    {
        BlitTransXlat<unsigned short> v(t.xl);
        SimdBlitTransXlat<SimdTier::SSE2> s(t.xl);
        total += Compare_Family("TransXlat", v, s);
    }
    {
        BlitTransZRemapXlat<unsigned short> v(rm, t.xl);
        SimdBlitTransZRemapXlat<SimdTier::SSE2> s(rm, t.xl);
        total += Compare_Family("TransZRemapXlat", v, s);
    }
    {
        BlitTransDarken<unsigned short> v(t.hb);
        SimdBlitTransDarken<SimdTier::SSE2> s(t.hb);
        total += Compare_Family("TransDarken", v, s);
    }
    {
        BlitTransLucent75<unsigned short> v(t.xl, t.qb);
        SimdBlitTransLucent75<SimdTier::SSE2> s(t.xl, t.qb);
        total += Compare_Family("TransLucent75", v, s);
    }
    {
        BlitTransLucent50<unsigned short> v(t.xl, t.hb);
        SimdBlitTransLucent50<SimdTier::SSE2> s(t.xl, t.hb);
        total += Compare_Family("TransLucent50", v, s);
    }
    {
        BlitTransLucent25<unsigned short> v(t.xl, t.qb);
        SimdBlitTransLucent25<SimdTier::SSE2> s(t.xl, t.qb);
        total += Compare_Family("TransLucent25", v, s);
    }

    if (total == 0) {
        DEBUG_INFO("[SIMD blit] Self-test PASSED (all families bit-exact).\n");
    } else {
        DEBUG_WARNING("[SIMD blit] Self-test FAILED: {} total mismatching pixels.\n", total);
    }
}
