/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended InfantryTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "infantrytype.h"
#include "technotypeext.h"


class DECLSPEC_UUID(UUID_INFANTRYTYPE_EXTENSION)
InfantryTypeClassExtension final : public TechnoTypeClassExtension
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
        InfantryTypeClassExtension(const InfantryTypeClass *this_ptr = nullptr);
        InfantryTypeClassExtension(const NoInitClass &noinit);
        virtual ~InfantryTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;
        
        virtual InfantryTypeClass *This() const override { return reinterpret_cast<InfantryTypeClass *>(TechnoTypeClassExtension::This()); }
        virtual const InfantryTypeClass *This_Const() const override { return reinterpret_cast<const InfantryTypeClass *>(TechnoTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_INFANTRYTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
        /**
         *  If this infantry has a weapon with negative damage, does it target
         *  units and aircraft rather than other infantry (e.g., like a medic?)?
         */
        bool IsMechanic;

        /**
         *  If this infantry has a weapon with negative damage, does it target
         *  units, aircraft and other infantry (e.g., medic and mechanic combined)?
         */
        bool IsOmniHealer;
};
