/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FOOTEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended FootClass class.
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

#include "footext.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
FootClassExtension::FootClassExtension(const FootClass *this_ptr) :
    TechnoClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("FootClassExtension::FootClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
FootClassExtension::FootClassExtension(const NoInitClass &noinit) :
    TechnoClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("FootClassExtension::FootClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
FootClassExtension::~FootClassExtension()
{
    //EXT_DEBUG_TRACE("FootClassExtension::~FootClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT FootClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("FootClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoClassExtension::Load(pStm);
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
HRESULT FootClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("FootClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoClassExtension::Save(pStm, fClearDirty);
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
void FootClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("FootClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void FootClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("FootClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoClassExtension::Object_CRC(crc);
}


/**
 *  Sets the last known flight cell of this object.
 *
 *  @author: ZivDero
 */
void FootClassExtension::Set_Last_Flight_Cell(Cell cell)
{
    LastFlightCell = cell;
}


/**
 *  Gets the last known flight cell of this object.
 *
 *  @author: ZivDero
 */
Cell FootClassExtension::Get_Last_Flight_Cell() const
{
    return LastFlightCell;
}
