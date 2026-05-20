/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended AnimClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "animext.h"

#include "anim.h"
#include "animtype.h"
#include "animtypeext.h"
#include "asserthandler.h"
#include "extension.h"
#include "options.h"
#include "tibsun_inline.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
AnimClassExtension::AnimClassExtension(const AnimClass *this_ptr) :
    ObjectClassExtension(this_ptr),
    DamageStage()
{
    //if (this_ptr) EXT_DEBUG_TRACE("AnimClassExtension::AnimClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AnimExtensions.Add(this);

    if (this_ptr) {
        const auto animtypeext = Extension::Fetch(This()->Class);

        /**
         *  Reimplement part of the vanilla constructor below.
         *  Anim extensions are created earlier, so we move par of the code here
         *  so that we have access to the extension at this point.
         */
        if (This()->Class->Stages == -1) {
            This()->Class->Stages = animtypeext->Stage_Count();
        }

        if (This()->Class->LoopEnd == -1) {
            This()->Class->LoopEnd = This()->Class->Stages;
        }

        int delay = This()->Class->Delay;
        if (This()->Class->RandomRateMin != 0 || This()->Class->RandomRateMax != 0) {
            if (This()->Class->RandomRateMin <= This()->Class->RandomRateMax) {
                delay = Random_Pick(This()->Class->RandomRateMin, This()->Class->RandomRateMax);
            }
        }
        if (This()->Class->IsNormalized) {
            This()->Set_Rate(Options.Normalize_Delay(delay));
        }
        else {
            This()->Set_Rate(delay);
        }

        This()->Set_Stage(0);

        /**
         *  Initialize the delay stage counter.
         */
        int damagedelay = animtypeext->DamageRate == -1 ? This()->Fetch_Rate() : animtypeext->DamageRate;
        DamageStage.Set_Rate(damagedelay);
    }
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AnimClassExtension::AnimClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit),
    DamageStage(noinit)
{
    //EXT_DEBUG_TRACE("AnimClassExtension::AnimClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AnimClassExtension::~AnimClassExtension()
{
    //EXT_DEBUG_TRACE("AnimClassExtension::~AnimClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AnimExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT AnimClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("AnimClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT AnimClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("AnimClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) AnimClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT AnimClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("AnimClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int AnimClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("AnimClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void AnimClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("AnimClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Processes any start events.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::Start()
{
    AnimTypeClassExtension *animtypeext = Extension::Fetch(This()->Class);

    /**
     *  #issue-752
     * 
     *  Spawns the start animations.
     */
    Spawn_Animations(This()->Center_Coord(), animtypeext->StartAnims, animtypeext->StartAnimsCount, animtypeext->StartAnimsMinimum, animtypeext->StartAnimsMaximum, animtypeext->StartAnimsDelay);
    
    return true;
}


/**
 *  Processes any middle events.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::Middle()
{
    AnimTypeClassExtension *animtypeext = Extension::Fetch(This()->Class);

    /**
     *  #issue-752
     * 
     *  Spawns the middle animations.
     */
    Spawn_Animations(This()->Center_Coord(), animtypeext->MiddleAnims, animtypeext->MiddleAnimsCount, animtypeext->MiddleAnimsMinimum, animtypeext->MiddleAnimsMaximum, animtypeext->MiddleAnimsDelay);
    
    return true;
}


/**
 *  Processes any end events.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::End()
{
    AnimTypeClassExtension *animtypeext = Extension::Fetch(This()->Class);

    /**
     *  #issue-752
     * 
     *  Spawns the end animations.
     */
    Spawn_Animations(This()->Center_Coord(), animtypeext->EndAnims, animtypeext->EndAnimsCount, animtypeext->EndAnimsMinimum, animtypeext->EndAnimsMaximum, animtypeext->EndAnimsDelay);
    
    return true;
}


/**
 *  #issue-752
 * 
 *  Spawns the requested animation from the parsed type lists.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::Spawn_Animations(const Coord &coord, const TypeList<AnimTypeClass *> &animlist, const TypeList<int> &countlist, const TypeList<int> &minlist, const TypeList<int> &maxlist, const TypeList<int>& delaylist)
{
    if (!animlist.Count()) {
        return false;
    }

    /**
     *  Some checks to make sure values are within expected ranges.
     */
    if (!countlist.Count()) {
        ASSERT(animlist.Count() == minlist.Count());
        ASSERT(animlist.Count() == maxlist.Count());
    }

    /**
     *  Iterate over all animations set and spawn them.
     */
    for (int index = 0; index < animlist.Count(); ++index) {

        const AnimTypeClass *animtype = animlist[index];

        int count = 1;

        /**
         *  Pick a random count based on the minimum and maximum values
         *  defined and spawn the animations.
         */
        if (animlist.Count() == countlist.Count()) {
            count = countlist[index];

        } else if (minlist.Count() && maxlist.Count()) {

            int min = minlist[index];
            int max = maxlist[index];

            if (min != max) {
                count = Random_Pick(std::min(min, max), std::max(min, max));
            } else {
                count = std::min(min, max);
            }
        }

        /**
         *  Based on the count decided above, spawn the animation type.
         */
        for (int i = 0; i < count; ++i) {

            /**
             *  Adjust the delay for the animation's rate.
             */
            int delay = delaylist[index] * This()->Fetch_Rate();

            AnimClass *anim = new AnimClass(animtype, (Coord &)coord, delay);
            ASSERT(anim != nullptr);
        }
    }

    return true;
}
