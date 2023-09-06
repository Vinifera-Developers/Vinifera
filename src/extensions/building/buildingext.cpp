/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended BuildingClass class.
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
#include "buildingext.h"
#include "building.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "house.h"
#include "housetype.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
BuildingClassExtension::BuildingClassExtension(const BuildingClass *this_ptr) :
    TechnoClassExtension(this_ptr),
    ProduceCashTimer(),
    CurrentProduceCashBudget(-1),
    IsCaptureOneTimeCashGiven(false),
    IsBudgetDepleted(false),
    LastFlameSpawnFrame(0),
    AssignedExpansionPoint(0, 0)
{
    //if (this_ptr) EXT_DEBUG_TRACE("BuildingClassExtension::BuildingClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    BuildingExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
BuildingClassExtension::BuildingClassExtension(const NoInitClass &noinit) :
    TechnoClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::BuildingClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
BuildingClassExtension::~BuildingClassExtension()
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::~BuildingClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    BuildingExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT BuildingClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT BuildingClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) BuildingClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT BuildingClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoClassExtension::Save(pStm, fClearDirty);
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
int BuildingClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void BuildingClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void BuildingClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    /**
     *  #issue-26
     * 
     *  Members for the Produce Cash logic.
     */
    crc(ProduceCashTimer());
    crc(LastFlameSpawnFrame);
    crc(AssignedExpansionPoint.As_Cell_Number());
}

/**
 *  #issue-26
 * 
 *  The produce cash per-frame update function.
 *  
 *  @author: CCHyper
 */
void BuildingClassExtension::Produce_Cash_AI()
{
    //EXT_DEBUG_TRACE("BuildingClassExtension::Produce_Cash_AI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const BuildingClass *this_building = reinterpret_cast<const BuildingClass *>(This());
    const BuildingTypeClass *this_buildingtype = reinterpret_cast<const BuildingTypeClass *>(This()->Class_Of());

    /**
     *  Fetch the extension instance.
     */
    BuildingTypeClassExtension *buildingtypeext = Extension::Fetch<BuildingTypeClassExtension>(this_buildingtype);

#if 0
    /**
     *  Debugging code.
     * 
     *  Only updates player owned buildings.
     */
    if (This()->House != PlayerPtr) {
        return;
    }
#endif

    /**
     *  Is this building able to produce cash?
     */
    if (!IsBudgetDepleted && buildingtypeext->ProduceCashAmount != 0) {

        /**
         *  Check if this building requires power to produce cash.
         */
        if (this_buildingtype->IsPowered) {

            /**
             *  Stop the timer if the building is offline or has low power.
             */
            if (ProduceCashTimer.Is_Active() && !This()->Is_Powered_On()) {
                ProduceCashTimer.Stop();
            }

            /**
             *  Restart the timer is if it previously stopped due to low power or is offline.
             */
            if (!ProduceCashTimer.Is_Active() && This()->Is_Powered_On()) {
                ProduceCashTimer.Start();
            }

        }

        /**
         *  Are we ready to hand out some cash?
         */
        if (ProduceCashTimer.Is_Active() && ProduceCashTimer.Expired()) {

            /**
             *  Is the owner a passive house? If so, they should not be receiving cash.
             */
            if (!This()->House->Class->IsMultiplayPassive) {

                int amount = buildingtypeext->ProduceCashAmount;

                /**
                 *  Check we have not depleted our budget first.
                 */
                if (CurrentProduceCashBudget > 0) {

                    /**
                     *  Adjust the budget tracker (if one as been set).
                     */
                    if (CurrentProduceCashBudget != -1) {
                        CurrentProduceCashBudget -= std::max<unsigned>(0, std::abs(amount));
                    }

                    /**
                     *  Has the budget been spent?
                     */
                    if (CurrentProduceCashBudget <= 0) {
                        IsBudgetDepleted = true;
                        CurrentProduceCashBudget = -1;
                    }

                }

                /**
                 *  Check if we need to drain cash from the house or provide a bonus.
                 */
                if (!IsBudgetDepleted && amount != 0) {
                    if (amount < 0) {
                        This()->House->Spend_Money(std::abs(amount));
                    } else {
                        This()->House->Refund_Money(amount);
                    }
                }

            }

            /**
             *  Reset the delay timer.
             */
            ProduceCashTimer = buildingtypeext->ProduceCashDelay;

        }

    }

}
