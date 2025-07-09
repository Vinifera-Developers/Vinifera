/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RADIOEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended Radio class.
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
#pragma once

#include "radioext.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
RadioClassExtension::RadioClassExtension(const RadioClass *this_ptr) :
    MissionClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("RadioClassExtension::RadioClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
RadioClassExtension::RadioClassExtension(const NoInitClass &noinit) :
    MissionClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("RadioClassExtension::RadioClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
RadioClassExtension::~RadioClassExtension()
{
    //EXT_DEBUG_TRACE("RadioClassExtension::~RadioClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT RadioClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("RadioClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = MissionClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT RadioClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("RadioClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = MissionClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void RadioClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("RadioClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    MissionClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void RadioClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("RadioClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    MissionClassExtension::Object_CRC(crc);
}
