/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MISSIONEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended Mission class.
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

#include "missionext.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
MissionClassExtension::MissionClassExtension(const MissionClass *this_ptr) :
    ObjectClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("MissionClassExtension::MissionClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
MissionClassExtension::MissionClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("MissionClassExtension::MissionClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
MissionClassExtension::~MissionClassExtension()
{
    //EXT_DEBUG_TRACE("MissionClassExtension::~MissionClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT MissionClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("MissionClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Load(pStm);
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
HRESULT MissionClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("MissionClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Save(pStm, fClearDirty);
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
void MissionClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("MissionClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void MissionClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("MissionClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectClassExtension::Compute_CRC(crc);
}
