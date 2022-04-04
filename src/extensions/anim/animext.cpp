/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ANIMEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended AnimClass class.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "animext.h"
#include "anim.h"
#include "animtype.h"
#include "animtypeext.h"
#include "wwcrc.h"
#include "tibsun_inline.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all AnimClass extension instances.
 */
ExtensionMap<AnimClass, AnimClassExtension> AnimClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
AnimClassExtension::AnimClassExtension(AnimClass *this_ptr) :
    Extension(this_ptr)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("AnimClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AnimClassExtension::AnimClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AnimClassExtension::~AnimClassExtension()
{
    //EXT_DEBUG_TRACE("AnimClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("AnimClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT AnimClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
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
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
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
int AnimClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void AnimClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void AnimClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Processes any start events.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::Start()
{
    AnimTypeClassExtension *animext = AnimTypeClassExtensions.find(ThisPtr->Class);
    if (!animext) {
        return false;
    }

    /**
     *  #issue-752
     * 
     *  Spawns the start animations.
     */
    Spawn_Animations(ThisPtr->Center_Coord(), animext->StartAnims, animext->StartAnimsCount, animext->StartAnimsMinimum, animext->StartAnimsMaximum);
    
    return true;
}


/**
 *  Processes any middle events.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::Middle()
{
    AnimTypeClassExtension *animext = AnimTypeClassExtensions.find(ThisPtr->Class);
    if (!animext) {
        return false;
    }

    /**
     *  #issue-752
     * 
     *  Spawns the middle animations.
     */
    Spawn_Animations(ThisPtr->Center_Coord(), animext->MiddleAnims, animext->MiddleAnimsCount, animext->MiddleAnimsMinimum, animext->MiddleAnimsMaximum);
    
    return true;
}


/**
 *  Processes any end events.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::End()
{
    AnimTypeClassExtension *animext = AnimTypeClassExtensions.find(ThisPtr->Class);
    if (!animext) {
        return false;
    }

    /**
     *  #issue-752
     * 
     *  Spawns the end animations.
     */
    Spawn_Animations(ThisPtr->Center_Coord(), animext->EndAnims, animext->EndAnimsCount, animext->EndAnimsMinimum, animext->EndAnimsMaximum);
    
    return true;
}


/**
 *  #issue-752
 * 
 *  Spawns the requested animation from the parsed type lists.
 *  
 *  @author: CCHyper
 */
bool AnimClassExtension::Spawn_Animations(const Coordinate &coord, const TypeList<AnimTypeClass *> &animlist, const TypeList<int> &countlist, const TypeList<int> &minlist, const TypeList<int> &maxlist)
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
            AnimClass *anim = new AnimClass(animtype, (Coordinate &)coord);
            ASSERT(anim != nullptr);
        }
    }

    return true;
}

