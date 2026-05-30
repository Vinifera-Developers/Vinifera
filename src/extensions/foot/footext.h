/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended FootClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "foot.h"
#include "technoext.h"


class FootClassExtension : public TechnoClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        FootClassExtension(const FootClass *this_ptr);
        FootClassExtension(const NoInitClass &noinit);
        virtual ~FootClassExtension();

        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual FootClass *This() const override { return reinterpret_cast<FootClass *>(TechnoClassExtension::This()); }
        virtual const FootClass *This_Const() const override { return reinterpret_cast<const FootClass *>(TechnoClassExtension::This_Const()); }

        virtual void Set_Last_Flight_Cell(Cell cell);
        virtual Cell Get_Last_Flight_Cell() const;

    public:
        /**
         *  The last known flight cell of this object, used by the AircraftTracker.
         */
        Cell LastFlightCell;
};
