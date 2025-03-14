/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended UnitTypeClass class.
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

#include "technotypeext.h"
#include "unittype.h"


class DECLSPEC_UUID(UUID_UNITTYPE_EXTENSION)
UnitTypeClassExtension final : public TechnoTypeClassExtension
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
        UnitTypeClassExtension(const UnitTypeClass *this_ptr = nullptr);
        UnitTypeClassExtension(const NoInitClass &noinit);
        virtual ~UnitTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual UnitTypeClass *This() const override { return reinterpret_cast<UnitTypeClass *>(TechnoTypeClassExtension::This()); }
        virtual const UnitTypeClass *This_Const() const override { return reinterpret_cast<const UnitTypeClass *>(TechnoTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_UNITTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
        /**
         *  Can this unit be picked up (toted) by the carryall aircraft?
         */
        bool IsTotable;

        /**
         *  The starting frame for the turret graphics in the units shape file.
         */
        int StartTurretFrame;

        /**
         *  The facing count for the turret graphics in the units shape file.
         */
        int TurretFacings;

        /**
         *  The starting frame for the idle animation in the units shape file.
         */
        unsigned StartIdleFrame;

        /**
         *  The number of image frames for each of the idle animation sequences.
         */
        unsigned IdleFrames;

        /**
         *  The unit type that this unit type transforms into upon deploying, if any.
         */
        const UnitTypeClass* TransformsInto;

        /**
         *  If set, transforming to another unit will require this unit to have full charge.
         */
        bool IsTransformRequiresFullCharge;
};
