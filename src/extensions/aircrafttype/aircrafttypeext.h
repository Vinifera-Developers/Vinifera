/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended AircraftTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "aircrafttype.h"
#include "technotypeext.h"


class DECLSPEC_UUID(UUID_AIRCRAFTTYPE_EXTENSION)
AircraftTypeClassExtension final : public TechnoTypeClassExtension
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
        AircraftTypeClassExtension(const AircraftTypeClass *this_ptr = nullptr);
        AircraftTypeClassExtension(const NoInitClass &noinit);
        virtual ~AircraftTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;
        
        virtual AircraftTypeClass *This() const override { return reinterpret_cast<AircraftTypeClass *>(TechnoTypeClassExtension::This()); }
        virtual const AircraftTypeClass *This_Const() const override { return reinterpret_cast<const AircraftTypeClass *>(TechnoTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_AIRCRAFTTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
        /**
         *  Should this aircraft shuffle its position between firing at its target?
         */
        bool IsCurleyShuffle;

        /**
         *  This is the rate that this aircraft will reload its ammo when docked with a helipad.
         */
        double ReloadRate;
};
