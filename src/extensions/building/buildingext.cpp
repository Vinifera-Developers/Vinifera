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
#include "technoext.h"
#include "technotypeext.h"
#include "weapontype.h"
#include "house.h"
#include "wwcrc.h"
#include "vinifera_defines.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all BuildingClass extension instances.
 */
ExtensionMap<BuildingClass, BuildingClassExtension> BuildingClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
BuildingClassExtension::BuildingClassExtension(BuildingClass *this_ptr) :
    Extension(this_ptr),
    ProduceCashTimer(),
    CurrentProduceCashBudget(-1),
    IsCaptureOneTimeCashGiven(false),
    IsBudgetDepleted(false)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("BuildingClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
BuildingClassExtension::BuildingClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
BuildingClassExtension::~BuildingClassExtension()
{
    //EXT_DEBUG_TRACE("BuildingClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("BuildingClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT BuildingClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
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
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

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
int BuildingClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void BuildingClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void BuildingClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    /**
     *  #issue-26
     * 
     *  Members for the Produce Cash logic.
     */
    crc(ProduceCashTimer());
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
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Produce_Cash_AI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    /**
     *  Find the type class extension instance.
     */
    BuildingTypeClassExtension *buildingtypeext = BuildingTypeClassExtensions.find(ThisPtr->Class);
    if (!buildingtypeext) {
        return;
    }

#if 0
    /**
     *  Debugging code.
     * 
     *  Only updates player owned buildings.
     */
    if (ThisPtr->House != PlayerPtr) {
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
        if (reinterpret_cast<BuildingTypeClass const *>(ThisPtr->Class_Of())->IsPowered) {

            /**
             *  Stop the timer if the building is offline or has low power.
             */
            if (ProduceCashTimer.Is_Active() && !ThisPtr->Is_Powered_On()) {
                ProduceCashTimer.Stop();
            }

            /**
             *  Restart the timer is if it previously stopped due to low power or is offline.
             */
            if (!ProduceCashTimer.Is_Active() && ThisPtr->Is_Powered_On()) {
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
            if (!ThisPtr->House->Class->IsMultiplayPassive) {

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
                        ThisPtr->House->Spend_Money(std::abs(amount));
                    } else {
                        ThisPtr->House->Refund_Money(amount);
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


/**
 *  Returns the control struct for the desired weapon type.
 * 
 *  @author: CCHyper
 */
const WeaponInfoStruct * BuildingClassExtension::Get_Weapon(WeaponSlotType weapon) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Get_Weapon - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    TechnoClassExtension *technoext;
    technoext = TechnoClassExtensions.find(ThisPtr);
    if (!technoext) {
        return nullptr;
    }

    TechnoTypeClassExtension *technotypeext;

    const WeaponInfoStruct * weaponptr = nullptr;

    /**
     *  Fetch the desired weapon from the upgrade.
     */
    for (int upgrade = 0; upgrade < BUILDING_UPGRADE_MAX; ++upgrade) {
        if (ThisPtr->Upgrades[upgrade]) {
            technotypeext = TechnoTypeClassExtensions.find(ThisPtr->Upgrades[upgrade]);
            ASSERT(technotypeext != nullptr);
            if (technotypeext) {
                weaponptr = &technotypeext->Fetch_Weapon_Info(weapon);
                if (technotypeext->Fetch_Weapon_Info(weapon).Weapon) {
                    return weaponptr;
                }
            }
        }
        if (upgrade >= ThisPtr->UpgradeLevel) {
            break;
        }
    }
    
    return technoext->Get_Weapon(weapon);
}


/**
 *  Searches for target that building can fire upon.
 * 
 *  @author: CCHyper
 */
TARGET BuildingClassExtension::Greatest_Threat(ThreatType method, Coordinate &coord, bool a3) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("BuildingClassExtension::Greatest_Threat - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    TechnoTypeClassExtension *technotypeext;
    technotypeext = TechnoTypeClassExtensions.find(ThisPtr->Techno_Type_Class());
    if (!technotypeext) {
        return nullptr;
    }

    /**
     *  
     */
    for (WeaponSlotType slot = WEAPON_SLOT_FIRST; slot < EXT_WEAPON_SLOT_COUNT; ++slot) {
        const WeaponTypeClass *weaponptr = ThisPtr->Get_Weapon(slot)->Weapon;
        if (weaponptr) {
            method |= weaponptr->Allowed_Threats();
        }
    }

    if (ThisPtr->House->Is_Human_Control()) {
        method = method & ~THREAT_BUILDINGS;
    }

    method |= THREAT_RANGE;

    return ThisPtr->TechnoClass::Greatest_Threat(method, coord, a3);
}
