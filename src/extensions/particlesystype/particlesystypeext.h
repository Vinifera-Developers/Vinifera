/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ParticleSystemTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objecttypeext.h"
#include "particlesystype.h"


class DECLSPEC_UUID(UUID_PARTICLESYSTEMTYPE_EXTENSION)
ParticleSystemTypeClassExtension final : public ObjectTypeClassExtension
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
        ParticleSystemTypeClassExtension(const ParticleSystemTypeClass *this_ptr = nullptr);
        ParticleSystemTypeClassExtension(const NoInitClass &noinit);
        virtual ~ParticleSystemTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual ParticleSystemTypeClass *This() const override { return reinterpret_cast<ParticleSystemTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const ParticleSystemTypeClass *This_Const() const override { return reinterpret_cast<const ParticleSystemTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_PARTICLESYSTEMTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
};
