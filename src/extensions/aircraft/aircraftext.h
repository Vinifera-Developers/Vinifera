/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended AircraftClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "aircraft.h"
#include "footext.h"


class AircraftClass;
class HouseClass;


class DECLSPEC_UUID(UUID_AIRCRAFT_EXTENSION)
AircraftClassExtension final : public FootClassExtension
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
        AircraftClassExtension(const AircraftClass *this_ptr = nullptr);
        AircraftClassExtension(const NoInitClass &noinit);
        virtual ~AircraftClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual AircraftClass *This() const override { return reinterpret_cast<AircraftClass *>(FootClassExtension::This()); }
        virtual const AircraftClass *This_Const() const override { return reinterpret_cast<const AircraftClass *>(FootClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_AIRCRAFT; }

    public:

};
