/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TeamClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "team.h"


class DECLSPEC_UUID(UUID_TEAM_EXTENSION)
TeamClassExtension final : public AbstractClassExtension
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
        TeamClassExtension(const TeamClass *this_ptr = nullptr);
        TeamClassExtension(const NoInitClass &noinit);
        virtual ~TeamClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char* Name() const override;
        virtual const char* Full_Name() const override;

        virtual TeamClass *This() const override { return reinterpret_cast<TeamClass *>(AbstractClassExtension::This()); }
        virtual const TeamClass *This_Const() const override { return reinterpret_cast<const TeamClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_TEAM; }

    public:

};
