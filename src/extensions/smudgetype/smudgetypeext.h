/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SmudgeTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objecttypeext.h"
#include "smudgetype.h"


class DECLSPEC_UUID(UUID_SMUDGETYPE_EXTENSION)
SmudgeTypeClassExtension final : public ObjectTypeClassExtension
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
        SmudgeTypeClassExtension(const SmudgeTypeClass *this_ptr = nullptr);
        SmudgeTypeClassExtension(const NoInitClass &noinit);
        virtual ~SmudgeTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual SmudgeTypeClass *This() const override { return reinterpret_cast<SmudgeTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const SmudgeTypeClass *This_Const() const override { return reinterpret_cast<const SmudgeTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_SMUDGETYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
};
