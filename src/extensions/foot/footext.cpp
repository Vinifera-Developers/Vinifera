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
#include "technotypeext.h"
#include "debughandler.h"
#include "extension.h"

 /*
 * The Foot object initializes the passenger.
 *
 * Injection objective function: FootClass::Unlimbo
 *
 * @author: RossinCarlinx
 */
void __stdcall FootClassExtension::InitialPassenger() {
    FootClass* const pFoot = this->This();
    TechnoTypeClassExtension* technotypeext = Extension::Fetch<TechnoTypeClassExtension>(pFoot->Techno_Type_Class());
    int const Count = technotypeext->InitPassengers.Count();
    // Check if the initial number of passenger types is greater than 0.
    if (Count > 0) {

        HouseClass* const house = pFoot->House;
        // Iterates through objects in InitPassenger, adding them if they are infantry or units.
        for (int i = 0;
            i < Count;
            i++) {
            TechnoTypeClass* const technotype = technotypeext->InitPassengers[i];
            RTTIType const rtti = technotype->Kind_Of();

            if (rtti != RTTI_INFANTRYTYPE &&
                rtti != RTTI_UNITTYPE) {
                continue;
            }

            int num = (technotypeext->InitPassengerNums.Count() > i) ?
                technotypeext->InitPassengerNums[i] :
                1;

            // A loop that creates objects and puts them into the position of passengers.
            do {
                ObjectClass* const object = technotype->Create_One_Of(house);
                FootClass* const passenger = object->As_Foot();
                if (!passenger) {
                    Fatal("*Error* Object creation failed.\n\tWhere the error occurred: \n\FootClass_Initialize_Passengers\n");
                }

                // Prevent the generation of initial occupants indefinitely.
                if (passenger->Class_Of() == pFoot->Class_Of()) {
                    Extension::Fetch<TechnoClassExtension>(passenger)->IsInitialized = true;
                }

                passenger->Set_Coord(pFoot->Get_Coord());
                passenger->Limbo();
                pFoot->Cargo.Attach(passenger);
#ifndef NDEBUG
                DEBUG_INFO("this_ptr->Cargo.Attach();");
#endif
            } while (--num > 0);
        }
    }
}

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
void FootClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("FootClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void FootClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("FootClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoClassExtension::Compute_CRC(crc);
}
