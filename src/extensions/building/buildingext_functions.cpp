/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGEXT_FUNCTIONS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the supporting functions for the extended BuildingClass.
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
#include "buildingext_functions.h"
#include "buildingext.h"
#include "buildingtypeext.h"
#include "building.h"
#include "house.h"
#include "housetype.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  #issue-26
 * 
 *  The produce cash per-frame update function.
 * 
 *  @author: CCHyper
 */
void BuildingClassExtension_Produce_Cash_AI(BuildingClass *this_ptr)
{
    /**
     *  Sanity check for the building class pointer.
     */
    if (!this_ptr) {
        return;
    }

    /**
     *  Find the extension instances.
     */
    BuildingClassExtension *ext_ptr = BuildingClassExtensions.find(this_ptr);
    BuildingTypeClassExtension *exttype_ptr = BuildingTypeClassExtensions.find(this_ptr->Class);
    if (!ext_ptr || !exttype_ptr) {
        return;
    }

#if 0
    /**
     *  Debugging code.
     * 
     *  Only updates player owned buildings.
     */
    if (this_ptr->House != PlayerPtr) {
        return;
    }
#endif

    /**
     *  Is this building able to produce cash?
     */
    if (!ext_ptr->IsBudgetDepleted && exttype_ptr->ProduceCashAmount != 0) {

        /**
         *  Check if this building requires power to produce cash.
         */
        if (reinterpret_cast<BuildingTypeClass const *>(this_ptr->Class_Of())->IsPowered) {

            /**
             *  Stop the timer if the building is offline or has low power.
             */
            if (ext_ptr->ProduceCashTimer.Is_Active() && !this_ptr->Is_Powered_On()) {
                ext_ptr->ProduceCashTimer.Stop();
            }

            /**
             *  Restart the timer is if it previously stopped due to low power or is offline.
             */
            if (!ext_ptr->ProduceCashTimer.Is_Active() && this_ptr->Is_Powered_On()) {
                ext_ptr->ProduceCashTimer.Start();
            }

        }

        /**
         *  Are we ready to hand out some cash?
         */
        if (ext_ptr->ProduceCashTimer.Is_Active() && ext_ptr->ProduceCashTimer.Expired()) {

            /**
             *  Is the owner a passive house? If so, they should not be receiving cash.
             */
            if (!this_ptr->House->Class->IsMultiplayPassive) {

                int amount = exttype_ptr->ProduceCashAmount;

                /**
                 *  Check we have not depleted our budget first.
                 */
                if (ext_ptr->CurrentProduceCashBudget > 0) {

                    /**
                     *  Adjust the budget tracker (if one as been set).
                     */
                    if (ext_ptr->CurrentProduceCashBudget != -1) {
                        ext_ptr->CurrentProduceCashBudget -= std::max<unsigned>(0, std::abs(amount));
                    }

                    /**
                     *  Has the budget been spent?
                     */
                    if (ext_ptr->CurrentProduceCashBudget <= 0) {
                        ext_ptr->IsBudgetDepleted = true;
                        ext_ptr->CurrentProduceCashBudget = -1;
                    }

                }

                /**
                 *  Check if we need to drain cash from the house or provide a bonus.
                 */
                if (!ext_ptr->IsBudgetDepleted && amount != 0) {
                    if (amount < 0) {
                        this_ptr->House->Spend_Money(std::abs(amount));
                    } else {
                        this_ptr->House->Refund_Money(amount);
                    }
                }

            }

            /**
             *  Reset the delay timer.
             */
            ext_ptr->ProduceCashTimer = exttype_ptr->ProduceCashDelay;
        }
    }
}
