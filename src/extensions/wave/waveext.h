/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended WaveClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objectext.h"
#include "wave.h"


class DECLSPEC_UUID(UUID_WAVE_EXTENSION)
WaveClassExtension final : public ObjectClassExtension
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
        WaveClassExtension(const WaveClass *this_ptr = nullptr);
        WaveClassExtension(const NoInitClass &noinit);
        virtual ~WaveClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual WaveClass *This() const override { return reinterpret_cast<WaveClass *>(ObjectClassExtension::This()); }
        virtual const WaveClass *This_Const() const override { return reinterpret_cast<const WaveClass *>(ObjectClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_WAVE; }

    public:
};
