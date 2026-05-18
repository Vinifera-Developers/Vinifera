/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended InfantryClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "footext.h"
#include "infantry.h"


class InfantryClass;
class HouseClass;


class DECLSPEC_UUID(UUID_INFANTRY_EXTENSION)
InfantryClassExtension final : public FootClassExtension
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
        InfantryClassExtension(const InfantryClass *this_ptr = nullptr);
        InfantryClassExtension(const NoInitClass &noinit);
        virtual ~InfantryClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual InfantryClass *This() const override { return reinterpret_cast<InfantryClass *>(FootClassExtension::This()); }
        virtual const InfantryClass *This_Const() const override { return reinterpret_cast<const InfantryClass *>(FootClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_INFANTRY; }

    public:
};
