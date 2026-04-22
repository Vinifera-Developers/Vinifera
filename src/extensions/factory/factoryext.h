/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended FactoryClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstractext.h"
#include "factory.h"


class DECLSPEC_UUID(UUID_FACTORY_EXTENSION)
FactoryClassExtension final : public AbstractClassExtension
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
        FactoryClassExtension(const FactoryClass *this_ptr = nullptr);
        FactoryClassExtension(const NoInitClass &noinit);
        virtual ~FactoryClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual FactoryClass *This() const override { return reinterpret_cast<FactoryClass *>(AbstractClassExtension::This()); }
        virtual const FactoryClass *This_Const() const override { return reinterpret_cast<const FactoryClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_FACTORY; }

        virtual const char *Name() const { return "Factory"; }
        virtual const char *Full_Name() const { return "Factory"; }

    public:
        /**
         *  Is this factory holding a unit that wants to exist, but wasn't able to?
         */
        bool IsHoldingExit;

        /**
         *  Has it been announced that this factory can finished construction?
         */
        bool HasSpoken;
};
