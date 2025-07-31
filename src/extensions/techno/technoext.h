/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TechnoClass class.
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
#include "techno.h"


class SpawnManagerClass;
class EBoltClass;
class TechnoTypeClass;
class TechnoTypeClassExtension;


class TechnoClassExtension : public RadioClassExtension
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

        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual TechnoClass *This() const override { return reinterpret_cast<TechnoClass *>(RadioClassExtension::This()); }
        virtual const TechnoClass *This_Const() const override { return reinterpret_cast<const TechnoClass *>(RadioClassExtension::This_Const()); }

        virtual EBoltClass *Electric_Zap(AbstractClass * target, int which, const WeaponTypeClass *weapontype, Coord &source_coord);
        virtual EBoltClass *Electric_Bolt(AbstractClass * target);
        virtual void Response_Capture();
        virtual void Response_Enter();
        virtual void Response_Deploy();
        virtual void Response_Harvest();
        virtual bool Can_Passive_Acquire() const;
        virtual Coord Fire_Coord(WeaponSlotType which, TPoint3D<int> offset = TPoint3D<int>()) const;

        void Put_Storage_Pointers();

        int Time_To_Build() const;
        bool Can_Opportunity_Fire() const;
        bool Opportunity_Fire();

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
};
