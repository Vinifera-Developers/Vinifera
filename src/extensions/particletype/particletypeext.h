/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ParticleTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objecttypeext.h"
#include "particletype.h"


class DECLSPEC_UUID(UUID_PARTICLETYPE_EXTENSION)
ParticleTypeClassExtension final : public ObjectTypeClassExtension
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
        ParticleTypeClassExtension(const ParticleTypeClass *this_ptr = nullptr);
        ParticleTypeClassExtension(const NoInitClass &noinit);
        virtual ~ParticleTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual ParticleTypeClass *This() const override { return reinterpret_cast<ParticleTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const ParticleTypeClass *This_Const() const override { return reinterpret_cast<const ParticleTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_PARTICLETYPE; }

        virtual bool Read_INI(CCINIClass& ini) override;

    public:
};
