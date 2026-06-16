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

    template<SimdTier ISA>
    void Install_Blitters_16bit()
    {
        ConvertClassExt* cc = this;
        const unsigned short* xl = (const unsigned short*)cc->Translator;
        const unsigned short* il = (const unsigned short*)cc->IntensityTranslator;
        unsigned char const* const* rm = reinterpret_cast<unsigned char const* const*>(&cc->RemapTable);
        unsigned short hb = (unsigned short)cc->HalfbrightMask;
        unsigned short qb = (unsigned short)cc->QuarterbrightMask;
        int lv = cc->IntensityLevels;

    cc->PlainBlitter = new SimdBlitPlainXlat<ISA>(xl);
    cc->TransBlitter = new SimdBlitTransXlat<ISA>(xl);
    cc->RemapBlitter = new SimdBlitTransZRemapXlat<ISA>(rm, xl);
    cc->ShadowBlitter = new SimdBlitTransDarken<ISA>(hb);
    cc->Translucent1Blitter = new SimdBlitTransLucent75<ISA>(xl, qb);
    cc->Translucent2Blitter = new SimdBlitTransLucent50<ISA>(xl, hb);
    cc->Translucent3Blitter = new SimdBlitTransLucent25<ISA>(xl, qb);

    cc->BlitPlainXlatZReadPtr = new SimdBlitPlainXlatZRead<ISA>(xl);
    cc->BlitTransXlatZReadPtr = new SimdBlitTransXlatZRead<ISA>(xl);
    cc->BlitTransZRemapXlatZReadPtr = new SimdBlitTransZRemapXlatZRead<ISA>(rm, xl);
    cc->BlitTransDarkenZReadPtr = new SimdBlitTransDarkenZRead<ISA>(hb);
    cc->BlitTransLucent75ZReadPtr = new SimdBlitTransLucent75ZRead<ISA>(xl, qb);
    cc->BlitTransLucent50ZReadPtr = new SimdBlitTransLucent50ZRead<ISA>(xl, hb);
    cc->BlitTransLucent25ZReadPtr = new SimdBlitTransLucent25ZRead<ISA>(xl, qb);
    cc->BlitTransLucent75ZReadWarpPtr = new SimdBlitTransLucent75ZReadWarp<ISA>(xl, qb);
    cc->BlitTransLucent50ZReadWarpPtr = new SimdBlitTransLucent50ZReadWarp<ISA>(xl, hb);
    cc->BlitTransLucent25ZReadWarpPtr = new SimdBlitTransLucent25ZReadWarp<ISA>(xl, qb);

    cc->BlitPlainXlatZReadWritePtr = new SimdBlitPlainXlatZReadWrite<ISA>(xl);
    cc->BlitTransXlatZReadWritePtr = new SimdBlitTransXlatZReadWrite<ISA>(xl);
    cc->BlitTransZRemapXlatZReadWritePtr = new SimdBlitTransZRemapXlatZReadWrite<ISA>(rm, xl);
    cc->BlitTransDarkenZReadWritePtr = new SimdBlitTransDarkenZReadWrite<ISA>(hb);
    cc->BlitTransLucent75ZReadWritePtr = new SimdBlitTransLucent75ZReadWrite<ISA>(xl, qb);
    cc->BlitTransLucent50ZReadWritePtr = new SimdBlitTransLucent50ZReadWrite<ISA>(xl, hb);
    cc->BlitTransLucent25ZReadWritePtr = new SimdBlitTransLucent25ZReadWrite<ISA>(xl, qb);

    cc->BlitPlainXlatAlphaPtr = new SimdBlitPlainXlatAlpha<ISA>(il, lv);
    cc->BlitTransXlatAlphaPtr = new SimdBlitTransXlatAlpha<ISA>(il, lv);
    cc->BlitTransZRemapXlatAlphaPtr = new SimdBlitTransZRemapXlatAlpha<ISA>(rm, il, lv);
    cc->BlitTransLucent75AlphaPtr = new SimdBlitTransLucent75Alpha<ISA>(il, lv, qb);
    cc->BlitTransLucent50AlphaPtr = new SimdBlitTransLucent50Alpha<ISA>(il, lv, hb);
    cc->BlitTransLucent25AlphaPtr = new SimdBlitTransLucent25Alpha<ISA>(il, lv, qb);

    cc->BlitTransXlatWriteAlphaPtr = new BlitTransXlatWriteAlpha<unsigned short>();
    cc->BlitTransXlatMultWriteAlphaPtr = new BlitTransXlatMultWriteAlpha<unsigned short>();
    cc->BlitTranslucentWriteAlphaPtr = new BlitTranslucentWriteAlpha<unsigned short>(il);
    cc->BlitTranslucent50NonzeroAlphaPtr = new BlitTranslucent50NonzeroAlpha<unsigned short>(xl, hb);
    cc->BlitTranslucent50ZeroAlphaPtr = new BlitTranslucent50ZeroAlpha<unsigned short>(xl, hb);
    cc->BlitTranslucent75NonzeroAlphaPtr = new BlitTranslucent75NonzeroAlpha<unsigned short>(xl, qb);
    cc->BlitTranslucent75ZeroAlphaPtr = new BlitTranslucent75ZeroAlpha<unsigned short>(xl, qb);

    cc->BlitPlainXlatAlpha_2Ptr = new SimdBlitPlainXlatAlpha<ISA>(il, lv);
    cc->BlitTransXlatAlphaZReadPtr = new SimdBlitTransXlatAlphaZRead<ISA>(il, lv);
    cc->BlitTransZRemapXlatAlphaZReadPtr = new SimdBlitTransZRemapXlatAlphaZRead<ISA>(rm, il, lv);
    cc->BlitTransLucent75AlphaZReadPtr = new SimdBlitTransLucent75AlphaZRead<ISA>(il, lv, qb);
    cc->BlitTransLucent50AlphaZReadPtr = new SimdBlitTransLucent50AlphaZRead<ISA>(il, lv, hb);
    cc->BlitTransLucent25AlphaZReadPtr = new SimdBlitTransLucent25AlphaZRead<ISA>(il, lv, qb);
    cc->BlitTransLucent75AlphaZReadWarpPtr = new SimdBlitTransLucent75AlphaZReadWarp<ISA>(il, lv, qb);
    cc->BlitTransLucent50AlphaZReadWarpPtr = new SimdBlitTransLucent50AlphaZReadWarp<ISA>(il, lv, hb);
    cc->BlitTransLucent25AlphaZReadWarpPtr = new SimdBlitTransLucent25AlphaZReadWarp<ISA>(il, lv, qb);

    cc->BlitPlainXlatAlpha_3Ptr = new SimdBlitPlainXlatAlpha<ISA>(il, lv);
    cc->BlitTransXlatAlphaZReadWritePtr = new SimdBlitTransXlatAlphaZReadWrite<ISA>(il, lv);
    cc->BlitTransZRemapXlatAlphaZReadWritePtr = new SimdBlitTransZRemapXlatAlphaZReadWrite<ISA>(rm, il, lv);
    cc->BlitTransLucent75AlphaZReadWritePtr = new SimdBlitTransLucent75AlphaZReadWrite<ISA>(il, lv, qb);
    cc->BlitTransLucent50AlphaZReadWritePtr = new SimdBlitTransLucent50AlphaZReadWrite<ISA>(il, lv, hb);
    cc->BlitTransLucent25AlphaZReadWritePtr = new SimdBlitTransLucent25AlphaZReadWrite<ISA>(il, lv, qb);

    		/*
    		**	Create the RLE aware blitter objects.
    		*/
    cc->RLEBlitTransXlatPtr = new SimdRLEBlitTransXlat<ISA>(xl);
    cc->RLEBlitTransZRemapXlatPtr = new SimdRLEBlitTransZRemapXlat<ISA>(rm, xl);
    cc->RLEBlitTransDarkenPtr = new SimdRLEBlitTransDarken<ISA>(hb);
    cc->RLEBlitTransLucent75Ptr = new SimdRLEBlitTransLucent75<ISA>(xl, qb);
    cc->RLEBlitTransLucent50Ptr = new SimdRLEBlitTransLucent50<ISA>(xl, hb);
    cc->RLEBlitTransLucent25Ptr = new SimdRLEBlitTransLucent25<ISA>(xl, qb);

    cc->RLEBlitTransXlatZReadPtr = new SimdRLEBlitTransXlatZRead<ISA>(xl);
    cc->RLEBlitTransZRemapXlatZReadPtr = new SimdRLEBlitTransZRemapXlatZRead<ISA>(rm, xl);
    cc->RLEBlitTransDarkenZReadPtr = new SimdRLEBlitTransDarkenZRead<ISA>(hb);
    cc->RLEBlitTransLucent75ZReadPtr = new SimdRLEBlitTransLucent75ZRead<ISA>(xl, qb);
    cc->RLEBlitTransLucent50ZReadPtr = new SimdRLEBlitTransLucent50ZRead<ISA>(xl, hb);
    cc->RLEBlitTransLucent25ZReadPtr = new SimdRLEBlitTransLucent25ZRead<ISA>(xl, qb);
    cc->RLEBlitTransLucent75ZReadWarpPtr = new SimdRLEBlitTransLucent75ZReadWarp<ISA>(xl, qb);
    cc->RLEBlitTransLucent50ZReadWarpPtr = new SimdRLEBlitTransLucent50ZReadWarp<ISA>(xl, hb);
    cc->RLEBlitTransLucent25ZReadWarpPtr = new SimdRLEBlitTransLucent25ZReadWarp<ISA>(xl, qb);

    cc->RLEBlitTransXlatZReadWritePtr = new SimdRLEBlitTransXlatZReadWrite<ISA>(xl);
    cc->RLEBlitTransZRemapXlatZReadWritePtr = new SimdRLEBlitTransZRemapXlatZReadWrite<ISA>(rm, xl);
    cc->RLEBlitTransDarkenZReadWritePtr = new SimdRLEBlitTransDarkenZReadWrite<ISA>(hb);
    cc->RLEBlitTransLucent75ZReadWritePtr = new SimdRLEBlitTransLucent75ZReadWrite<ISA>(xl, qb);
    cc->RLEBlitTransLucent50ZReadWritePtr = new SimdRLEBlitTransLucent50ZReadWrite<ISA>(xl, hb);
    cc->RLEBlitTransLucent25ZReadWritePtr = new SimdRLEBlitTransLucent25ZReadWrite<ISA>(xl, qb);

    cc->RLEBlitTransXlatAlphaPtr = new SimdRLEBlitTransXlatAlpha<ISA>(il, lv);
    cc->RLEBlitTransZRemapXlatAlphaPtr = new SimdRLEBlitTransZRemapXlatAlpha<ISA>(rm, il, lv);
    cc->RLEBlitTransLucent75AlphaPtr = new SimdRLEBlitTransLucent75Alpha<ISA>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaPtr = new SimdRLEBlitTransLucent50Alpha<ISA>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaPtr = new SimdRLEBlitTransLucent25Alpha<ISA>(il, lv, qb);

    cc->RLEBlitTransXlatAlphaZReadPtr = new SimdRLEBlitTransXlatAlphaZRead<ISA>(il, lv);
    cc->RLEBlitTransZRemapXlatAlphaZReadPtr = new SimdRLEBlitTransZRemapXlatAlphaZRead<ISA>(rm, il, lv);
    cc->RLEBlitTransLucent75AlphaZReadPtr = new SimdRLEBlitTransLucent75AlphaZRead<ISA>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaZReadPtr = new SimdRLEBlitTransLucent50AlphaZRead<ISA>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaZReadPtr = new SimdRLEBlitTransLucent25AlphaZRead<ISA>(il, lv, qb);
    cc->RLEBlitTransLucent75AlphaZReadWarpPtr = new SimdRLEBlitTransLucent75AlphaZReadWarp<ISA>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaZReadWarpPtr = new SimdRLEBlitTransLucent50AlphaZReadWarp<ISA>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaZReadWarpPtr = new SimdRLEBlitTransLucent25AlphaZReadWarp<ISA>(il, lv, qb);

    cc->RLEBlitTransXlatAlphaZReadWritePtr = new SimdRLEBlitTransXlatAlphaZReadWrite<ISA>(il, lv);
    cc->RLEBlitTransZRemapXlatAlphaZReadWritePtr = new SimdRLEBlitTransZRemapXlatAlphaZReadWrite<ISA>(rm, il, lv);
    cc->RLEBlitTransLucent75AlphaZReadWritePtr = new SimdRLEBlitTransLucent75AlphaZReadWrite<ISA>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaZReadWritePtr = new SimdRLEBlitTransLucent50AlphaZReadWrite<ISA>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaZReadWritePtr = new SimdRLEBlitTransLucent25AlphaZReadWrite<ISA>(il, lv, qb);
    }
};


/**
 *  ConvertClass::Create_Blitters @ 0x00464100.
 */
DEFINE_HOOK(0x00464100, _ConvertClass_Create_Blitters_SIMD, 6)
{
    GET(ConvertClassExt*, cc, ECX);

    const SimdTier tier = Blitter_SIMD_Tier();

    /**
     *  8-bit drawers and CPUs without SSE2 keep the vanilla blitters entirely:
     *  let the original Create_Blitters run.
     */
    if (tier == SimdTier::Scalar || cc->Get_BBP() == 1) {
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
        Blitter_SIMD_SelfTest(cc);
    }
#endif

    switch (tier) {
    case SimdTier::AVX2: cc->Install_Blitters_16bit<SimdTier::AVX2>(); break;
    default:             cc->Install_Blitters_16bit<SimdTier::SSE2>(); break;
    }

    /**
     *  Skip the original body. 0x00465958 is a bare `retn` inside Create_Blitters;
     *  at hook entry ESP still points at the caller's return address (the prologue
     *  has not executed and Syringe preserves registers around the hook), so this
     *  returns cleanly to the caller.
     */
    return 0x00465958;
}
