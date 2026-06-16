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
#include "cpudetect.h"
#include "zbuffer.h"
#include "debughandler.h"


extern ZBuffer*& DepthBuffer;


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
 *  Run the full family matrix for one SIMD tier. Returns the total mismatch count.
 */
template<SimdTier ISA>
static int Run_Tier(const char* tier_name,
                    unsigned short const* xl, unsigned short hb, unsigned short qb,
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

    DEBUG_INFO("[SIMD blit] Running bit-exactness self-test...\n");
    int total = 0;

    total += Run_Tier<SimdTier::SSE2>("SSE2", t.xl, t.hb, t.qb, rm);
    if (CPUDetectClass::Has_AVX2_Instruction_Set()) {
        total += Run_Tier<SimdTier::AVX2>("AVX2", t.xl, t.hb, t.qb, rm);
    }

    if (total == 0) {
        DEBUG_INFO("[SIMD blit] Self-test PASSED (all families bit-exact).\n");
    } else {
        DEBUG_WARNING("[SIMD blit] Self-test FAILED: {} total mismatching pixels.\n", total);
    }
}
