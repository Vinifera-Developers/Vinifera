/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TechnoClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "detach_listener.h"
#include "radioext.h"
#include "techno.h"


class SpawnManagerClass;
class EBoltClass;
class TechnoTypeClass;
class TechnoTypeClassExtension;
class AnimClass;


class TechnoClassExtension : public RadioClassExtension,
                             public Vinifera::Detach::Listener<TechnoClass>,
                             public Vinifera::Detach::Listener<AnimClass>
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        TechnoClassExtension(const TechnoClass *this_ptr);
        TechnoClassExtension(const NoInitClass &noinit);
        virtual ~TechnoClassExtension();

        virtual void Object_CRC(CRCEngine &crc) const override;

        void On_Detach(TechnoClass *target, bool all) override;
        void On_Detach(AnimClass *target, bool all) override;

        virtual TechnoClass *This() const override { return reinterpret_cast<TechnoClass *>(RadioClassExtension::This()); }
        virtual const TechnoClass *This_Const() const override { return reinterpret_cast<const TechnoClass *>(RadioClassExtension::This_Const()); }

        virtual EBoltClass *Electric_Zap(AbstractClass * target, int which, const WeaponTypeClass *weapontype, Coord &source_coord);
        virtual EBoltClass *Electric_Bolt(AbstractClass * target);
        virtual void Response_Capture();
        virtual void Response_Enter();
        virtual void Response_Deploy();
        virtual void Response_Harvest();
        virtual bool Can_Passive_Acquire() const;
        virtual Coord Fire_Coord(WeaponSlotType which, TPoint3D<int> offset = TPoint3D<int>(0, 0, 0)) const;

        void Put_Storage_Pointers();

        int Time_To_Build() const;
        bool Can_Opportunity_Fire() const;
        bool Opportunity_Fire();

        bool Iron_Curtain_Me(bool forced);
        int Get_Sight_Range() const;

    private:
        const TechnoTypeClass *Techno_Type_Class() const;
        const TechnoTypeClassExtension *Techno_Type_Class_Ext() const;

    public:
        /**
         *  The current electric bolt instance fired by this object.
         */
        EBoltClass *ElectricBolt;

        /**
         *  Replacement Tiberium storage.
         */
        VectorClass<int> Storage;

        /**
         *  The spawn manager of this unit.
         */
        SpawnManagerClass* SpawnManager;

        /**
         *  The object that spawned this object.
         */
        TechnoClass* SpawnOwner;

        /**
         *  Is this object's current target an opportunity fire target?
         */
        bool HasOpportunityFireTarget;

        /**
         *  When has this unit last received a target? (not comprehensive)
         */
        int LastTargetFrame;

        /**
         *  Should we reset burst once the countdown reaches 0?
         */
        bool IsToResetBurst;

        /**
         *  The countdown until burst gets reset if unit has lost the target.
         */
        CDTimerClass<FrameTimerClass> BurstResetTimer;

        /**
         *  The veternacy rank of this unit last time it performed its AI() function.
         *  Used to determine when a unit has ranked up.
         */
        VeterancyRankType LastVeterancy;

        /**
         *  The idle wake animation attached to this object.
         */
        AnimClass* IdleWakeAnim;

        /**
         *  The countdown until the object's Iron Curtain effect fades away.
         */
        CDTimerClass<FrameTimerClass> IronCurtainTimer;
};
