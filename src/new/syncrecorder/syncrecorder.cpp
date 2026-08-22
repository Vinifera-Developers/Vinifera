/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Records recent game state changes for desync debugging.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "syncrecorder.h"

#include "abstract.h"
#include "animtype.h"
#include "foot.h"
#include "house.h"
#include "random.h"
#include "scenario.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"

#include <algorithm>


/**
 *  A fake class to allow access to the protected state of Random2Class.
 *
 *  @note: This must not contain a constructor or destructor.
 */
class Random2ClassFake : public Random2Class
{
public:
    int _Get_Index1() const { return Index1; }
    int _Get_Index2() const { return Index2; }
};


/**
 *  A fixed-size ring buffer of the most recent call records. Recording must
 *  stay cheap (the RNG is recorded on every draw), so entries are plain
 *  structs in a static array and insertion is a masked index increment.
 */
template<typename T, unsigned SIZE>
struct CallHistoryBuffer
{
    static_assert((SIZE & (SIZE - 1)) == 0, "CallHistoryBuffer size must be a power of two.");

    T& Next() { return History[Total++ & (SIZE - 1)]; }

    /**
     *  How many entries can actually be printed, and the entry `n` steps
     *  back from the most recent one.
     */
    unsigned Available(unsigned count) const { return std::min(std::min(count, Total), SIZE); }
    const T& Nth_Newest(unsigned n) const { return History[(Total - 1 - n) & (SIZE - 1)]; }

    unsigned Total = 0;
    T History[SIZE] = {};
};


struct RNGCallNode
{
    bool IsRanged;
    bool IsCritical;
    int Index1;
    int Index2;
    unsigned Caller;
    long Frame;
    int Min;
    int Max;
};

struct FacingCallNode
{
    unsigned DirValue;
    unsigned Caller;
    long Frame;
};

struct TarComCallNode
{
    RTTIType MyRTTI;
    int MyID;
    RTTIType TargetRTTI;
    int TargetID;
    unsigned Caller;
    long Frame;
};

struct OverrideMissionCallNode
{
    RTTIType RTTI;
    int HouseID;
    unsigned Caller;
    long Frame;
};

struct AnimConstructorCallNode
{
    int X;
    int Y;
    int Z;
    int AnimTypeHeapID;
    unsigned Caller;
    long Frame;
};


static CallHistoryBuffer<RNGCallNode, 4096> RNGCalls;
static CallHistoryBuffer<FacingCallNode, 1024> FacingChanges;
static CallHistoryBuffer<TarComCallNode, 1024> TarComChanges;
static CallHistoryBuffer<OverrideMissionCallNode, 512> OverrideMissionCalls;
static CallHistoryBuffer<AnimConstructorCallNode, 512> AnimConstructions;


/**
 *  Records an unranged random number draw.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Record_RNG_Call(Random2Class* rng, unsigned caller)
{
    RNGCallNode& node = RNGCalls.Next();
    node.IsRanged = false;
    node.IsCritical = Scen != nullptr && rng == &Scen->RandomNumber;
    node.Index1 = static_cast<Random2ClassFake*>(rng)->_Get_Index1();
    node.Index2 = static_cast<Random2ClassFake*>(rng)->_Get_Index2();
    node.Caller = caller;
    node.Frame = Frame;
    node.Min = 0;
    node.Max = 0;
}


/**
 *  Records a ranged random number draw.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Record_RNG_Call(Random2Class* rng, unsigned caller, int minval, int maxval)
{
    RNGCallNode& node = RNGCalls.Next();
    node.IsRanged = true;
    node.IsCritical = Scen != nullptr && rng == &Scen->RandomNumber;
    node.Index1 = static_cast<Random2ClassFake*>(rng)->_Get_Index1();
    node.Index2 = static_cast<Random2ClassFake*>(rng)->_Get_Index2();
    node.Caller = caller;
    node.Frame = Frame;
    node.Min = minval;
    node.Max = maxval;
}


/**
 *  Records a facing change.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Record_Facing_Change(unsigned dir, unsigned caller)
{
    FacingCallNode& node = FacingChanges.Next();
    node.DirValue = dir;
    node.Caller = caller;
    node.Frame = Frame;
}


/**
 *  Records a target assignment.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Record_TarCom_Change(const AbstractClass* object, const AbstractClass* target, unsigned caller)
{
    TarComCallNode& node = TarComChanges.Next();
    node.MyRTTI = object->Fetch_RTTI();
    node.MyID = object->Fetch_ID();
    node.TargetRTTI = target != nullptr ? target->Fetch_RTTI() : RTTI_NONE;
    node.TargetID = target != nullptr ? target->Fetch_ID() : 0;
    node.Caller = caller;
    node.Frame = Frame;
}


/**
 *  Records a mission override.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Record_Override_Mission(const FootClass* foot, unsigned caller)
{
    OverrideMissionCallNode& node = OverrideMissionCalls.Next();
    node.RTTI = foot->Fetch_RTTI();
    node.HouseID = foot->House != nullptr ? foot->House->HeapID : -1;
    node.Caller = caller;
    node.Frame = Frame;
}


/**
 *  Records an animation creation.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Record_Anim_Construction(const AnimTypeClass* animtype, const Coord& coord, unsigned caller)
{
    AnimConstructorCallNode& node = AnimConstructions.Next();
    node.X = coord.X;
    node.Y = coord.Y;
    node.Z = coord.Z;
    node.AnimTypeHeapID = animtype != nullptr ? animtype->Fetch_Heap_ID() : -1;
    node.Caller = caller;
    node.Frame = Frame;
}


/**
 *  Prints the most recent random number draws.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Print_RNG_Calls(FILE* fp, unsigned count)
{
    std::fprintf(fp, "--- BEGIN RANDOM ---\n");

    for (unsigned n = 0; n < RNGCalls.Available(count); n++) {
        const RNGCallNode& node = RNGCalls.Nth_Newest(n);
        if (node.IsRanged) {
            std::fprintf(fp, "RNG: %s RANGED   CALLER: %08x  FRAME: %-10ld MIX1: %3d MIX2: %3d MIN: %d  MAX: %d\n",
                node.IsCritical ? "CRITICAL" : "NONCRIT ",
                node.Caller, node.Frame, node.Index1, node.Index2, node.Min, node.Max);
        } else {
            std::fprintf(fp, "RNG: %s UNRANGED CALLER: %08x  FRAME: %-10ld MIX1: %3d MIX2: %3d\n",
                node.IsCritical ? "CRITICAL" : "NONCRIT ",
                node.Caller, node.Frame, node.Index1, node.Index2);
        }
    }
    std::fprintf(fp, "\n");
}


/**
 *  Prints the most recent facing changes.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Print_Facing_Changes(FILE* fp, unsigned count)
{
    std::fprintf(fp, "--- BEGIN FACINGS ---\n");

    for (unsigned n = 0; n < FacingChanges.Available(count); n++) {
        const FacingCallNode& node = FacingChanges.Nth_Newest(n);

        /**
         *  The recorded facing value is deliberately not printed - it has
         *  been observed to differ between machines that are still in sync.
         */
        std::fprintf(fp, "FACING CHANGE, CALLER: %08x  FRAME: %-10ld\n", node.Caller, node.Frame);
    }
    std::fprintf(fp, "\n");
}


/**
 *  Prints the most recent target assignments.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Print_TarCom_Changes(FILE* fp, unsigned count)
{
    std::fprintf(fp, "--- BEGIN TARCOM CHANGES ---\n");

    for (unsigned n = 0; n < TarComChanges.Available(count); n++) {
        const TarComCallNode& node = TarComChanges.Nth_Newest(n);
        std::fprintf(fp, "TARCOM: MyRTTI: %02d MyID: %08d TargetRTTI: %02d TargetID: %08d Caller: %08x  Frame: %-10ld\n",
            node.MyRTTI, node.MyID, node.TargetRTTI, node.TargetID, node.Caller, node.Frame);
    }
    std::fprintf(fp, "\n");
}


/**
 *  Prints the most recent mission overrides.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Print_Override_Mission_Calls(FILE* fp, unsigned count)
{
    std::fprintf(fp, "--- BEGIN OVERRIDE_MISSION CALLS ---\n");

    for (unsigned n = 0; n < OverrideMissionCalls.Available(count); n++) {
        const OverrideMissionCallNode& node = OverrideMissionCalls.Nth_Newest(n);
        std::fprintf(fp, "Mission Override: RTTI: %02d House: %02d Caller: %08x Frame: %-10ld\n",
            node.RTTI, node.HouseID, node.Caller, node.Frame);
    }
    std::fprintf(fp, "\n");
}


/**
 *  Prints the most recent animation creations.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Print_Anim_Constructor_Calls(FILE* fp, unsigned count)
{
    std::fprintf(fp, "--- BEGIN ANIMATION CONSTRUCTOR CALLS ---\n");

    for (unsigned n = 0; n < AnimConstructions.Available(count); n++) {
        const AnimConstructorCallNode& node = AnimConstructions.Nth_Newest(n);
        std::fprintf(fp, "Anim creation: Coord: %d,%d,%d  Type: %d  Caller: %08x  Frame: %-10ld\n",
            node.X, node.Y, node.Z, node.AnimTypeHeapID, node.Caller, node.Frame);
    }
    std::fprintf(fp, "\n");
}


/**
 *  Prints all recorded call histories into the desync log.
 *
 *  @author: ZivDero
 */
void SyncRecorder::Print_All(FILE* fp)
{
    Print_RNG_Calls(fp, 4096);
    Print_Facing_Changes(fp, 1024);
    Print_TarCom_Changes(fp, 1024);
    Print_Override_Mission_Calls(fp, 512);
    Print_Anim_Constructor_Calls(fp, 512);
}
