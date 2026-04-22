/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TeamTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "teamtype.h"


class DECLSPEC_UUID(UUID_TEAMTYPE_EXTENSION)
TeamTypeClassExtension final : public AbstractTypeClassExtension
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
        TeamTypeClassExtension(const TeamTypeClass *this_ptr = nullptr);
        TeamTypeClassExtension(const NoInitClass &noinit);
        virtual ~TeamTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual TeamTypeClass *This() const override { return reinterpret_cast<TeamTypeClass *>(AbstractTypeClassExtension::This()); }
        virtual const TeamTypeClass *This_Const() const override { return reinterpret_cast<const TeamTypeClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_TEAMTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
};
