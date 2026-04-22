/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended Radio class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "missionext.h"
#include "radio.h"


class RadioClassExtension : public MissionClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        RadioClassExtension(const RadioClass *this_ptr);
        RadioClassExtension(const NoInitClass &noinit);
        virtual ~RadioClassExtension();

        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual RadioClass *This() const override { return reinterpret_cast<RadioClass *>(ObjectClassExtension::This()); }
        virtual const RadioClass *This_Const() const override { return reinterpret_cast<const RadioClass *>(ObjectClassExtension::This_Const()); }

    public:
};
