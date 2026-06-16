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
 *  Best SIMD tier available on this CPU. (SSE4.1/AVX2 are wired in a later wave.)
 */
static SimdTier Blitter_SIMD_Tier()
{
    if (!Vinifera_SIMDBlitters) return SimdTier::Scalar;
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
    cc->BlitTransLucent75ZReadWarpPtr = new BlitTransLucent75ZReadWarp<unsigned short>(xl, qb);
    cc->BlitTransLucent50ZReadWarpPtr = new BlitTransLucent50ZReadWarp<unsigned short>(xl, hb);
    cc->BlitTransLucent25ZReadWarpPtr = new BlitTransLucent25ZReadWarp<unsigned short>(xl, qb);

    cc->BlitPlainXlatZReadWritePtr = new SimdBlitPlainXlatZReadWrite<ISA>(xl);
    cc->BlitTransXlatZReadWritePtr = new SimdBlitTransXlatZReadWrite<ISA>(xl);
    cc->BlitTransZRemapXlatZReadWritePtr = new SimdBlitTransZRemapXlatZReadWrite<ISA>(rm, xl);
    cc->BlitTransDarkenZReadWritePtr = new SimdBlitTransDarkenZReadWrite<ISA>(hb);
    cc->BlitTransLucent75ZReadWritePtr = new SimdBlitTransLucent75ZReadWrite<ISA>(xl, qb);
    cc->BlitTransLucent50ZReadWritePtr = new SimdBlitTransLucent50ZReadWrite<ISA>(xl, hb);
    cc->BlitTransLucent25ZReadWritePtr = new SimdBlitTransLucent25ZReadWrite<ISA>(xl, qb);

    cc->BlitPlainXlatAlphaPtr = new BlitPlainXlatAlpha<unsigned short>(il, lv);
    cc->BlitTransXlatAlphaPtr = new BlitTransXlatAlpha<unsigned short>(il, lv);
    cc->BlitTransZRemapXlatAlphaPtr = new BlitTransZRemapXlatAlpha<unsigned short>(rm, il, lv);
    cc->BlitTransLucent75AlphaPtr = new BlitTransLucent75Alpha<unsigned short>(il, lv, qb);
    cc->BlitTransLucent50AlphaPtr = new BlitTransLucent50Alpha<unsigned short>(il, lv, hb);
    cc->BlitTransLucent25AlphaPtr = new BlitTransLucent25Alpha<unsigned short>(il, lv, qb);

    cc->BlitTransXlatWriteAlphaPtr = new BlitTransXlatWriteAlpha<unsigned short>();
    cc->BlitTransXlatMultWriteAlphaPtr = new BlitTransXlatMultWriteAlpha<unsigned short>();
    cc->BlitTranslucentWriteAlphaPtr = new BlitTranslucentWriteAlpha<unsigned short>(il);
    cc->BlitTranslucent50NonzeroAlphaPtr = new BlitTranslucent50NonzeroAlpha<unsigned short>(xl, hb);
    cc->BlitTranslucent50ZeroAlphaPtr = new BlitTranslucent50ZeroAlpha<unsigned short>(xl, hb);
    cc->BlitTranslucent75NonzeroAlphaPtr = new BlitTranslucent75NonzeroAlpha<unsigned short>(xl, qb);
    cc->BlitTranslucent75ZeroAlphaPtr = new BlitTranslucent75ZeroAlpha<unsigned short>(xl, qb);

    cc->BlitPlainXlatAlpha_2Ptr = new BlitPlainXlatAlpha<unsigned short>(il, lv);
    cc->BlitTransXlatAlphaZReadPtr = new BlitTransXlatAlphaZRead<unsigned short>(il, lv);
    cc->BlitTransZRemapXlatAlphaZReadPtr = new BlitTransZRemapXlatAlphaZRead<unsigned short>(rm, il, lv);
    cc->BlitTransLucent75AlphaZReadPtr = new BlitTransLucent75AlphaZRead<unsigned short>(il, lv, qb);
    cc->BlitTransLucent50AlphaZReadPtr = new BlitTransLucent50AlphaZRead<unsigned short>(il, lv, hb);
    cc->BlitTransLucent25AlphaZReadPtr = new BlitTransLucent25AlphaZRead<unsigned short>(il, lv, qb);
    cc->BlitTransLucent75AlphaZReadWarpPtr = new BlitTransLucent75AlphaZReadWarp<unsigned short>(il, lv, qb);
    cc->BlitTransLucent50AlphaZReadWarpPtr = new BlitTransLucent50AlphaZReadWarp<unsigned short>(il, lv, hb);
    cc->BlitTransLucent25AlphaZReadWarpPtr = new BlitTransLucent25AlphaZReadWarp<unsigned short>(il, lv, qb);

    cc->BlitPlainXlatAlpha_3Ptr = new BlitPlainXlatAlpha<unsigned short>(il, lv);
    cc->BlitTransXlatAlphaZReadWritePtr = new BlitTransXlatAlphaZReadWrite<unsigned short>(il, lv);
    cc->BlitTransZRemapXlatAlphaZReadWritePtr = new BlitTransZRemapXlatAlphaZReadWrite<unsigned short>(rm, il, lv);
    cc->BlitTransLucent75AlphaZReadWritePtr = new BlitTransLucent75AlphaZReadWrite<unsigned short>(il, lv, qb);
    cc->BlitTransLucent50AlphaZReadWritePtr = new BlitTransLucent50AlphaZReadWrite<unsigned short>(il, lv, hb);
    cc->BlitTransLucent25AlphaZReadWritePtr = new BlitTransLucent25AlphaZReadWrite<unsigned short>(il, lv, qb);

    		/*
    		**	Create the RLE aware blitter objects.
    		*/
    cc->RLEBlitTransXlatPtr = new RLEBlitTransXlat<unsigned short>(xl);
    cc->RLEBlitTransZRemapXlatPtr = new RLEBlitTransZRemapXlat<unsigned short>(rm, xl);
    cc->RLEBlitTransDarkenPtr = new RLEBlitTransDarken<unsigned short>(hb);
    cc->RLEBlitTransLucent75Ptr = new RLEBlitTransLucent75<unsigned short>(xl, qb);
    cc->RLEBlitTransLucent50Ptr = new RLEBlitTransLucent50<unsigned short>(xl, hb);
    cc->RLEBlitTransLucent25Ptr = new RLEBlitTransLucent25<unsigned short>(xl, qb);

    cc->RLEBlitTransXlatZReadPtr = new RLEBlitTransXlatZRead<unsigned short>(xl);
    cc->RLEBlitTransZRemapXlatZReadPtr = new RLEBlitTransZRemapXlatZRead<unsigned short>(rm, xl);
    cc->RLEBlitTransDarkenZReadPtr = new RLEBlitTransDarkenZRead<unsigned short>(hb);
    cc->RLEBlitTransLucent75ZReadPtr = new RLEBlitTransLucent75ZRead<unsigned short>(xl, qb);
    cc->RLEBlitTransLucent50ZReadPtr = new RLEBlitTransLucent50ZRead<unsigned short>(xl, hb);
    cc->RLEBlitTransLucent25ZReadPtr = new RLEBlitTransLucent25ZRead<unsigned short>(xl, qb);
    cc->RLEBlitTransLucent75ZReadWarpPtr = new RLEBlitTransLucent75ZReadWarp<unsigned short>(xl, qb);
    cc->RLEBlitTransLucent50ZReadWarpPtr = new RLEBlitTransLucent50ZReadWarp<unsigned short>(xl, hb);
    cc->RLEBlitTransLucent25ZReadWarpPtr = new RLEBlitTransLucent25ZReadWarp<unsigned short>(xl, qb);

    cc->RLEBlitTransXlatZReadWritePtr = new RLEBlitTransXlatZReadWrite<unsigned short>(xl);
    cc->RLEBlitTransZRemapXlatZReadWritePtr = new RLEBlitTransZRemapXlatZReadWrite<unsigned short>(rm, xl);
    cc->RLEBlitTransDarkenZReadWritePtr = new RLEBlitTransDarkenZReadWrite<unsigned short>(hb);
    cc->RLEBlitTransLucent75ZReadWritePtr = new RLEBlitTransLucent75ZReadWrite<unsigned short>(xl, qb);
    cc->RLEBlitTransLucent50ZReadWritePtr = new RLEBlitTransLucent50ZReadWrite<unsigned short>(xl, hb);
    cc->RLEBlitTransLucent25ZReadWritePtr = new RLEBlitTransLucent25ZReadWrite<unsigned short>(xl, qb);

    cc->RLEBlitTransXlatAlphaPtr = new RLEBlitTransXlatAlpha<unsigned short>(il, lv);
    cc->RLEBlitTransZRemapXlatAlphaPtr = new RLEBlitTransZRemapXlatAlpha<unsigned short>(rm, il, lv);
    cc->RLEBlitTransLucent75AlphaPtr = new RLEBlitTransLucent75Alpha<unsigned short>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaPtr = new RLEBlitTransLucent50Alpha<unsigned short>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaPtr = new RLEBlitTransLucent25Alpha<unsigned short>(il, lv, qb);

    cc->RLEBlitTransXlatAlphaZReadPtr = new RLEBlitTransXlatAlphaZRead<unsigned short>(il, lv);
    cc->RLEBlitTransZRemapXlatAlphaZReadPtr = new RLEBlitTransZRemapXlatAlphaZRead<unsigned short>(rm, il, lv);
    cc->RLEBlitTransLucent75AlphaZReadPtr = new RLEBlitTransLucent75AlphaZRead<unsigned short>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaZReadPtr = new RLEBlitTransLucent50AlphaZRead<unsigned short>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaZReadPtr = new RLEBlitTransLucent25AlphaZRead<unsigned short>(il, lv, qb);
    cc->RLEBlitTransLucent75AlphaZReadWarpPtr = new RLEBlitTransLucent75AlphaZReadWarp<unsigned short>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaZReadWarpPtr = new RLEBlitTransLucent50AlphaZReadWarp<unsigned short>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaZReadWarpPtr = new RLEBlitTransLucent25AlphaZReadWarp<unsigned short>(il, lv, qb);

    cc->RLEBlitTransXlatAlphaZReadWritePtr = new RLEBlitTransXlatAlphaZReadWrite<unsigned short>(il, lv);
    cc->RLEBlitTransZRemapXlatAlphaZReadWritePtr = new RLEBlitTransZRemapXlatAlphaZReadWrite<unsigned short>(rm, il, lv);
    cc->RLEBlitTransLucent75AlphaZReadWritePtr = new RLEBlitTransLucent75AlphaZReadWrite<unsigned short>(il, lv, qb);
    cc->RLEBlitTransLucent50AlphaZReadWritePtr = new RLEBlitTransLucent50AlphaZReadWrite<unsigned short>(il, lv, hb);
    cc->RLEBlitTransLucent25AlphaZReadWritePtr = new RLEBlitTransLucent25AlphaZReadWrite<unsigned short>(il, lv, qb);
    }
};


/**
 *  ConvertClass::Create_Blitters @ 0x00464100.
 */
DEFINE_HOOK(0x00464100, _ConvertClass_Create_Blitters_SIMD, 6)
{
    GET(ConvertClassExt*, cc, ECX);

    /**
     *  8-bit drawers and CPUs without SSE2 keep the vanilla blitters entirely:
     *  let the original Create_Blitters run.
     */
    if (Blitter_SIMD_Tier() == SimdTier::Scalar || cc->Get_BBP() == 1) {
        return 0;
    }

#ifndef NDEBUG
    /**
     *  Debug builds verify bit-exactness against the bound vanilla blitters once,
     *  using this drawer's live tables, before the SIMD blitters go into service.
     */
    static bool tested = false;
    if (!tested) {
        tested = true;
        Blitter_SIMD_SelfTest(cc);
    }
#endif

    cc->Install_Blitters_16bit<SimdTier::SSE2>();

    /**
     *  Skip the original body. 0x00465958 is a bare `retn` inside Create_Blitters;
     *  at hook entry ESP still points at the caller's return address (the prologue
     *  has not executed and Syringe preserves registers around the hook), so this
     *  returns cleanly to the caller.
     */
    return 0x00465958;
}
