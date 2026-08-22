/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended HouseTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "housetype.h"
#include "wstring.h"


class DECLSPEC_UUID(UUID_HOUSETYPE_EXTENSION)
HouseTypeClassExtension final : public AbstractTypeClassExtension
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
        HouseTypeClassExtension(const HouseTypeClass *this_ptr = nullptr);
        HouseTypeClassExtension(const NoInitClass &noinit);
        virtual ~HouseTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;
        
        virtual HouseTypeClass *This() const override { return reinterpret_cast<HouseTypeClass *>(AbstractTypeClassExtension::This()); }
        virtual const HouseTypeClass *This_Const() const override { return reinterpret_cast<const HouseTypeClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_HOUSETYPE; }

        virtual bool Read_INI(CCINIClass& ini) override;

        static HousesType House_From_Name(char const* name);

    public:
};
