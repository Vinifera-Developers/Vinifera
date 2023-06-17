/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGEXT.H
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
#pragma once

#include "technoext.h"
#include "building.h"
#include "ttimer.h"
#include "ftimer.h"


class BuildingClass;
class HouseClass;


class DECLSPEC_UUID(UUID_BUILDING_EXTENSION)
BuildingClassExtension final : public TechnoClassExtension
{
    public:
        /**
         *  IPersist
         */
        IFACEMETHOD(GetClassID)(CLSID *pClassID);

        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        BuildingClassExtension(const BuildingClass *this_ptr = nullptr);
        BuildingClassExtension(const NoInitClass &noinit);
        virtual ~BuildingClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual BuildingClass *This() const override { return reinterpret_cast<BuildingClass *>(TechnoClassExtension::This()); }
        virtual const BuildingClass *This_Const() const override { return reinterpret_cast<const BuildingClass *>(TechnoClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_BUILDING; }

        void Produce_Cash_AI();

    public:
        /**
         *  The delay between each cash bonus.
         */
        CDTimerClass<FrameTimerClass> ProduceCashTimer;

        /**
         *  The remaining budget for the building.
         */
        int CurrentProduceCashBudget;

        /**
         *  Have we given our "one time" cash bonus on capture?
         */
        bool IsCaptureOneTimeCashGiven;

        /**
         *  Has the cash budget been depleted (stops producing cash)?
         */
        bool IsBudgetDepleted;

        /**
         *  Records the last frame when flames were created on the building
         *  as a result of it receiving damage. This is used to check to
         *  prevent the flames from accumulating if the building rapidly
         *  switches between damage stages (typically as a result of
         *  existing flames or another damage-over-time effect combined
         *  with building repair bringing it back to green health).
         */
        int LastFlameSpawnFrame;
};
