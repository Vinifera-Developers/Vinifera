/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Creation-time dispatch that installs the SIMD blitters.
 *
 *          Hooks ConvertClass::Create_Blitters: for a 16-bit drawer on an
 *          SSE2-capable CPU it constructs the full blitter set itself --
 *          SimdBlit<tier> for the families that have a SIMD kernel, and the
 *          (fully-modelled, bit-exact) vanilla classes for the rest -- then
 *          skips the original body. 8-bit drawers and non-SSE2 CPUs fall
 *          through to the vanilla routine unchanged.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "always.h"

#include "blitter_simd.h"
#include "convert.h"
#include "cpudetect.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "syringe.h"


/**
 *  Runtime on/off switch for the SIMD blitters (A/B testing).
 */
bool Vinifera_SIMDBlitters = true;


/**
 *  Best SIMD tier available on this CPU.
 */
static SimdTier Blitter_SIMD_Tier()
{
    if (!Vinifera_SIMDBlitters) return SimdTier::Scalar;
    if (CPUDetectClass::Has_AVX2_Instruction_Set()) return SimdTier::AVX2;
    if (CPUDetectClass::Has_SSE2_Instruction_Set()) return SimdTier::SSE2;
    return SimdTier::Scalar;
}


/**
 *  Layout-compatible accessor for ConvertClass (adds no data members, so a
 *  reinterpret_cast of a live ConvertClass* is valid). Builds the 16-bit
 *  blitter set into the protected member slots.
 */
class ConvertClassExt : public ConvertClass
{
public:
    int Get_BBP() const { return BBP; }

    /**
     *  Per-family tier selection:
     *    - TransXlat keeps the VANILLA blitter: it measured faster than every SIMD tier. Its
     *      vanilla loop is already tight, and a plain transparent translate has no per-pixel
     *      arithmetic for SIMD to amortise its setup against -- only a palette gather (which
     *      does not vectorise) and a store -- so the SIMD wrapper is pure overhead here.
     *    - The four non-Z arithmetic families (Darken, Lucent25/50/75) use ISA = the best tier
     *      (AVX2 when available, else SSE2); their 16-wide blend is where AVX2 wins.
     *    - Everything else is pinned to SSE2: for the gather / Z / alpha families AVX2 is no
     *      faster (vpgatherdd is slow, and the Z/alpha path is 128-bit on both tiers).
     *  `ISA` is therefore consulted only by the four AVX2-benefiting families.
     */
    template<SimdTier ISA>
    void Install_Blitters_16bit()
    {
        const unsigned short* xl = (const unsigned short*)Translator;
        const unsigned short* il = (const unsigned short*)IntensityTranslator;
        unsigned char const* const* rm = reinterpret_cast<unsigned char const* const*>(&RemapTable);
        unsigned short hb = (unsigned short)HalfbrightMask;
        unsigned short qb = (unsigned short)QuarterbrightMask;
        int lv = IntensityLevels;

        PlainBlitter = new SimdBlitPlainXlat<SimdTier::SSE2>(xl);
        TransBlitter = new BlitTransXlat<unsigned short>(xl); // vanilla: benchmarks faster (gather-bound)
        RemapBlitter = new SimdBlitTransZRemapXlat<SimdTier::SSE2>(rm, xl);
        ShadowBlitter = new SimdBlitTransDarken<ISA>(hb); // AVX2-benefiting (arithmetic)
        Translucent1Blitter = new SimdBlitTransLucent75<ISA>(xl, qb);
        Translucent2Blitter = new SimdBlitTransLucent50<ISA>(xl, hb);
        Translucent3Blitter = new SimdBlitTransLucent25<ISA>(xl, qb);

        BlitPlainXlatZReadPtr = new SimdBlitPlainXlatZRead<SimdTier::SSE2>(xl);
        BlitTransXlatZReadPtr = new SimdBlitTransXlatZRead<SimdTier::SSE2>(xl);
        BlitTransZRemapXlatZReadPtr = new SimdBlitTransZRemapXlatZRead<SimdTier::SSE2>(rm, xl);
        BlitTransDarkenZReadPtr = new SimdBlitTransDarkenZRead<SimdTier::SSE2>(hb);
        BlitTransLucent75ZReadPtr = new SimdBlitTransLucent75ZRead<SimdTier::SSE2>(xl, qb);
        BlitTransLucent50ZReadPtr = new SimdBlitTransLucent50ZRead<SimdTier::SSE2>(xl, hb);
        BlitTransLucent25ZReadPtr = new SimdBlitTransLucent25ZRead<SimdTier::SSE2>(xl, qb);
        BlitTransLucent75ZReadWarpPtr = new SimdBlitTransLucent75ZReadWarp<SimdTier::SSE2>(xl, qb);
        BlitTransLucent50ZReadWarpPtr = new SimdBlitTransLucent50ZReadWarp<SimdTier::SSE2>(xl, hb);
        BlitTransLucent25ZReadWarpPtr = new SimdBlitTransLucent25ZReadWarp<SimdTier::SSE2>(xl, qb);

        BlitPlainXlatZReadWritePtr = new SimdBlitPlainXlatZReadWrite<SimdTier::SSE2>(xl);
        BlitTransXlatZReadWritePtr = new SimdBlitTransXlatZReadWrite<SimdTier::SSE2>(xl);
        BlitTransZRemapXlatZReadWritePtr = new SimdBlitTransZRemapXlatZReadWrite<SimdTier::SSE2>(rm, xl);
        BlitTransDarkenZReadWritePtr = new SimdBlitTransDarkenZReadWrite<SimdTier::SSE2>(hb);
        BlitTransLucent75ZReadWritePtr = new SimdBlitTransLucent75ZReadWrite<SimdTier::SSE2>(xl, qb);
        BlitTransLucent50ZReadWritePtr = new SimdBlitTransLucent50ZReadWrite<SimdTier::SSE2>(xl, hb);
        BlitTransLucent25ZReadWritePtr = new SimdBlitTransLucent25ZReadWrite<SimdTier::SSE2>(xl, qb);

        BlitPlainXlatAlphaPtr = new SimdBlitPlainXlatAlpha<SimdTier::SSE2>(il, lv);
        BlitTransXlatAlphaPtr = new SimdBlitTransXlatAlpha<SimdTier::SSE2>(il, lv);
        BlitTransZRemapXlatAlphaPtr = new SimdBlitTransZRemapXlatAlpha<SimdTier::SSE2>(rm, il, lv);
        BlitTransLucent75AlphaPtr = new SimdBlitTransLucent75Alpha<SimdTier::SSE2>(il, lv, qb);
        BlitTransLucent50AlphaPtr = new SimdBlitTransLucent50Alpha<SimdTier::SSE2>(il, lv, hb);
        BlitTransLucent25AlphaPtr = new SimdBlitTransLucent25Alpha<SimdTier::SSE2>(il, lv, qb);

        BlitTransXlatWriteAlphaPtr = new SimdBlitTransXlatWriteAlpha<SimdTier::SSE2>();
        BlitTransXlatMultWriteAlphaPtr = new SimdBlitTransXlatMultWriteAlpha<SimdTier::SSE2>();
        BlitTranslucentWriteAlphaPtr = new SimdBlitTranslucentWriteAlpha<SimdTier::SSE2>(il);
        BlitTranslucent50NonzeroAlphaPtr = new SimdBlitTranslucent50NonzeroAlpha<SimdTier::SSE2>(xl, hb);
        BlitTranslucent50ZeroAlphaPtr = new SimdBlitTranslucent50ZeroAlpha<SimdTier::SSE2>(xl, hb);
        BlitTranslucent75NonzeroAlphaPtr = new SimdBlitTranslucent75NonzeroAlpha<SimdTier::SSE2>(xl, qb);
        BlitTranslucent75ZeroAlphaPtr = new SimdBlitTranslucent75ZeroAlpha<SimdTier::SSE2>(xl, qb);

        BlitPlainXlatAlpha_2Ptr = new SimdBlitPlainXlatAlpha<SimdTier::SSE2>(il, lv);
        BlitTransXlatAlphaZReadPtr = new SimdBlitTransXlatAlphaZRead<SimdTier::SSE2>(il, lv);
        BlitTransZRemapXlatAlphaZReadPtr = new SimdBlitTransZRemapXlatAlphaZRead<SimdTier::SSE2>(rm, il, lv);
        BlitTransLucent75AlphaZReadPtr = new SimdBlitTransLucent75AlphaZRead<SimdTier::SSE2>(il, lv, qb);
        BlitTransLucent50AlphaZReadPtr = new SimdBlitTransLucent50AlphaZRead<SimdTier::SSE2>(il, lv, hb);
        BlitTransLucent25AlphaZReadPtr = new SimdBlitTransLucent25AlphaZRead<SimdTier::SSE2>(il, lv, qb);
        BlitTransLucent75AlphaZReadWarpPtr = new SimdBlitTransLucent75AlphaZReadWarp<SimdTier::SSE2>(il, lv, qb);
        BlitTransLucent50AlphaZReadWarpPtr = new SimdBlitTransLucent50AlphaZReadWarp<SimdTier::SSE2>(il, lv, hb);
        BlitTransLucent25AlphaZReadWarpPtr = new SimdBlitTransLucent25AlphaZReadWarp<SimdTier::SSE2>(il, lv, qb);

        BlitPlainXlatAlpha_3Ptr = new SimdBlitPlainXlatAlpha<SimdTier::SSE2>(il, lv);
        BlitTransXlatAlphaZReadWritePtr = new SimdBlitTransXlatAlphaZReadWrite<SimdTier::SSE2>(il, lv);
        BlitTransZRemapXlatAlphaZReadWritePtr = new SimdBlitTransZRemapXlatAlphaZReadWrite<SimdTier::SSE2>(rm, il, lv);
        BlitTransLucent75AlphaZReadWritePtr = new SimdBlitTransLucent75AlphaZReadWrite<SimdTier::SSE2>(il, lv, qb);
        BlitTransLucent50AlphaZReadWritePtr = new SimdBlitTransLucent50AlphaZReadWrite<SimdTier::SSE2>(il, lv, hb);
        BlitTransLucent25AlphaZReadWritePtr = new SimdBlitTransLucent25AlphaZReadWrite<SimdTier::SSE2>(il, lv, qb);

        /*
        **	Create the RLE aware blitter objects.
        */
        RLEBlitTransXlatPtr = new SimdRLEBlitTransXlat<SimdTier::SSE2>(xl);
        RLEBlitTransZRemapXlatPtr = new SimdRLEBlitTransZRemapXlat<SimdTier::SSE2>(rm, xl);
        RLEBlitTransDarkenPtr = new SimdRLEBlitTransDarken<SimdTier::SSE2>(hb);
        RLEBlitTransLucent75Ptr = new SimdRLEBlitTransLucent75<SimdTier::SSE2>(xl, qb);
        RLEBlitTransLucent50Ptr = new SimdRLEBlitTransLucent50<SimdTier::SSE2>(xl, hb);
        RLEBlitTransLucent25Ptr = new SimdRLEBlitTransLucent25<SimdTier::SSE2>(xl, qb);

        RLEBlitTransXlatZReadPtr = new SimdRLEBlitTransXlatZRead<SimdTier::SSE2>(xl);
        RLEBlitTransZRemapXlatZReadPtr = new SimdRLEBlitTransZRemapXlatZRead<SimdTier::SSE2>(rm, xl);
        RLEBlitTransDarkenZReadPtr = new SimdRLEBlitTransDarkenZRead<SimdTier::SSE2>(hb);
        RLEBlitTransLucent75ZReadPtr = new SimdRLEBlitTransLucent75ZRead<SimdTier::SSE2>(xl, qb);
        RLEBlitTransLucent50ZReadPtr = new SimdRLEBlitTransLucent50ZRead<SimdTier::SSE2>(xl, hb);
        RLEBlitTransLucent25ZReadPtr = new SimdRLEBlitTransLucent25ZRead<SimdTier::SSE2>(xl, qb);
        RLEBlitTransLucent75ZReadWarpPtr = new SimdRLEBlitTransLucent75ZReadWarp<SimdTier::SSE2>(xl, qb);
        RLEBlitTransLucent50ZReadWarpPtr = new SimdRLEBlitTransLucent50ZReadWarp<SimdTier::SSE2>(xl, hb);
        RLEBlitTransLucent25ZReadWarpPtr = new SimdRLEBlitTransLucent25ZReadWarp<SimdTier::SSE2>(xl, qb);

        RLEBlitTransXlatZReadWritePtr = new SimdRLEBlitTransXlatZReadWrite<SimdTier::SSE2>(xl);
        RLEBlitTransZRemapXlatZReadWritePtr = new SimdRLEBlitTransZRemapXlatZReadWrite<SimdTier::SSE2>(rm, xl);
        RLEBlitTransDarkenZReadWritePtr = new SimdRLEBlitTransDarkenZReadWrite<SimdTier::SSE2>(hb);
        RLEBlitTransLucent75ZReadWritePtr = new SimdRLEBlitTransLucent75ZReadWrite<SimdTier::SSE2>(xl, qb);
        RLEBlitTransLucent50ZReadWritePtr = new SimdRLEBlitTransLucent50ZReadWrite<SimdTier::SSE2>(xl, hb);
        RLEBlitTransLucent25ZReadWritePtr = new SimdRLEBlitTransLucent25ZReadWrite<SimdTier::SSE2>(xl, qb);

        RLEBlitTransXlatAlphaPtr = new SimdRLEBlitTransXlatAlpha<SimdTier::SSE2>(il, lv);
        RLEBlitTransZRemapXlatAlphaPtr = new SimdRLEBlitTransZRemapXlatAlpha<SimdTier::SSE2>(rm, il, lv);
        RLEBlitTransLucent75AlphaPtr = new SimdRLEBlitTransLucent75Alpha<SimdTier::SSE2>(il, lv, qb);
        RLEBlitTransLucent50AlphaPtr = new SimdRLEBlitTransLucent50Alpha<SimdTier::SSE2>(il, lv, hb);
        RLEBlitTransLucent25AlphaPtr = new SimdRLEBlitTransLucent25Alpha<SimdTier::SSE2>(il, lv, qb);

        RLEBlitTransXlatAlphaZReadPtr = new SimdRLEBlitTransXlatAlphaZRead<SimdTier::SSE2>(il, lv);
        RLEBlitTransZRemapXlatAlphaZReadPtr = new SimdRLEBlitTransZRemapXlatAlphaZRead<SimdTier::SSE2>(rm, il, lv);
        RLEBlitTransLucent75AlphaZReadPtr = new SimdRLEBlitTransLucent75AlphaZRead<SimdTier::SSE2>(il, lv, qb);
        RLEBlitTransLucent50AlphaZReadPtr = new SimdRLEBlitTransLucent50AlphaZRead<SimdTier::SSE2>(il, lv, hb);
        RLEBlitTransLucent25AlphaZReadPtr = new SimdRLEBlitTransLucent25AlphaZRead<SimdTier::SSE2>(il, lv, qb);
        RLEBlitTransLucent75AlphaZReadWarpPtr = new SimdRLEBlitTransLucent75AlphaZReadWarp<SimdTier::SSE2>(il, lv, qb);
        RLEBlitTransLucent50AlphaZReadWarpPtr = new SimdRLEBlitTransLucent50AlphaZReadWarp<SimdTier::SSE2>(il, lv, hb);
        RLEBlitTransLucent25AlphaZReadWarpPtr = new SimdRLEBlitTransLucent25AlphaZReadWarp<SimdTier::SSE2>(il, lv, qb);

        RLEBlitTransXlatAlphaZReadWritePtr = new SimdRLEBlitTransXlatAlphaZReadWrite<SimdTier::SSE2>(il, lv);
        RLEBlitTransZRemapXlatAlphaZReadWritePtr = new SimdRLEBlitTransZRemapXlatAlphaZReadWrite<SimdTier::SSE2>(rm, il, lv);
        RLEBlitTransLucent75AlphaZReadWritePtr = new SimdRLEBlitTransLucent75AlphaZReadWrite<SimdTier::SSE2>(il, lv, qb);
        RLEBlitTransLucent50AlphaZReadWritePtr = new SimdRLEBlitTransLucent50AlphaZReadWrite<SimdTier::SSE2>(il, lv, hb);
        RLEBlitTransLucent25AlphaZReadWritePtr = new SimdRLEBlitTransLucent25AlphaZReadWrite<SimdTier::SSE2>(il, lv, qb);
    }
};


/**
 *  ConvertClass::Create_Blitters @ 0x00464100.
 */
DEFINE_HOOK(0x00464100, _ConvertClass_Create_Blitters_SIMD, 6)
{
    GET(ConvertClassExt*, this_ptr, ECX);

    const SimdTier tier = Blitter_SIMD_Tier();

    /**
     *  8-bit drawers and CPUs without SSE2 keep the vanilla blitters entirely:
     *  let the original Create_Blitters run.
     */
    if (tier == SimdTier::Scalar || this_ptr->Get_BBP() == 1) {
        return 0;
    }

#ifdef BLITTER_TESTS
    /**
     *  When built for testing, verify bit-exactness against the bound vanilla blitters once,
     *  using this drawer's live tables, before the SIMD blitters go into service. (This only
     *  passes because BLITTER_TESTS also builds the blitters bug-compatible with vanilla.)
     */
    static bool tested = false;
    if (!tested) {
        tested = true;
        Blitter_SIMD_SelfTest(this_ptr);
    }
#endif

#ifdef BLITTER_BENCH
    /**
     *  Time the SIMD tiers against the bound vanilla blitters once, using this drawer's tables.
     */
    static bool benched = false;
    if (!benched) {
        benched = true;
        Blitter_SIMD_Benchmark(this_ptr);
    }
#endif

    switch (tier) {
    case SimdTier::AVX2:
        this_ptr->Install_Blitters_16bit<SimdTier::AVX2>();
        break;
    default:
        this_ptr->Install_Blitters_16bit<SimdTier::SSE2>();
        break;
    }

    /**
     *  Skip the original body. 0x00465958 is a bare `retn` inside Create_Blitters;
     *  at hook entry ESP still points at the caller's return address (the prologue
     *  has not executed and Syringe preserves registers around the hook), so this
     *  returns cleanly to the caller.
     */
    return 0x00465958;
}
