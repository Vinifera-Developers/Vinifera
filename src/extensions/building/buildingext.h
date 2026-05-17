/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended BuildingClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "building.h"
#include "ftimer.h"
#include "technoext.h"
#include "ttimer.h"


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

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual BuildingClass *This() const override { return reinterpret_cast<BuildingClass *>(TechnoClassExtension::This()); }
        virtual const BuildingClass *This_Const() const override { return reinterpret_cast<const BuildingClass *>(TechnoClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_BUILDING; }

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
