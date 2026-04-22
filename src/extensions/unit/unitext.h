/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended UnitClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "building.h"
#include "footext.h"
#include "unit.h"


class DECLSPEC_UUID(UUID_UNIT_EXTENSION)
UnitClassExtension final : public FootClassExtension
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
        UnitClassExtension(const UnitClass *this_ptr = nullptr);
        UnitClassExtension(const NoInitClass &noinit);
        virtual ~UnitClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual UnitClass *This() const override { return reinterpret_cast<UnitClass *>(FootClassExtension::This()); }
        virtual const UnitClass *This_Const() const override { return reinterpret_cast<const UnitClass *>(FootClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_UNIT; }

    public:
        /**
        *  #issue-203
        *
        *  The building that this unit last docked with.
        *  Used by harvesters for considering the distance to their last refinery
        *  when picking a tiberium cell to harvest from.
        */
        BuildingClass *LastDockedBuilding;
};
