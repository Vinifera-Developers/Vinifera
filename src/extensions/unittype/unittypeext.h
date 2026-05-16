/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended UnitTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
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
