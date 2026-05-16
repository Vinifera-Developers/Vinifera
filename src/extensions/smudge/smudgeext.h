/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SmudgeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objectext.h"
#include "smudge.h"


class DECLSPEC_UUID(UUID_SMUDGE_EXTENSION)
SmudgeClassExtension final : public ObjectClassExtension
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
        SmudgeClassExtension(const SmudgeClass *this_ptr = nullptr);
        SmudgeClassExtension(const NoInitClass &noinit);
        virtual ~SmudgeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual SmudgeClass *This() const override { return reinterpret_cast<SmudgeClass *>(ObjectClassExtension::This()); }
        virtual const SmudgeClass *This_Const() const override { return reinterpret_cast<const SmudgeClass *>(ObjectClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_OVERLAY; }

    public:
};
