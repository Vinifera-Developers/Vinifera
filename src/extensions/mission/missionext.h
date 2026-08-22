/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended Mission class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "mission.h"
#include "objectext.h"


class MissionClassExtension : public ObjectClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        MissionClassExtension(const MissionClass *this_ptr);
        MissionClassExtension(const NoInitClass &noinit);
        virtual ~MissionClassExtension();

        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual MissionClass *This() const override { return reinterpret_cast<MissionClass *>(ObjectClassExtension::This()); }
        virtual const MissionClass *This_Const() const override { return reinterpret_cast<const MissionClass *>(ObjectClassExtension::This_Const()); }

    public:
};
