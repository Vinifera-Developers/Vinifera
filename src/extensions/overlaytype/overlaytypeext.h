/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended OverlayTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objecttypeext.h"
#include "overlaytype.h"


class DECLSPEC_UUID(UUID_OVERLAYTYPE_EXTENSION)
OverlayTypeClassExtension final : public ObjectTypeClassExtension
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
        OverlayTypeClassExtension(const OverlayTypeClass *this_ptr = nullptr);
        OverlayTypeClassExtension(const NoInitClass &noinit);
        virtual ~OverlayTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual bool Read_INI(CCINIClass &ini) override;

        virtual OverlayTypeClass *This() const override { return reinterpret_cast<OverlayTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const OverlayTypeClass *This_Const() const override { return reinterpret_cast<const OverlayTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_INFANTRYTYPE; }

    public:
        /**
         *  Is this a water tunnel entrance?
         */
        bool IsWaterTunnel;
};
