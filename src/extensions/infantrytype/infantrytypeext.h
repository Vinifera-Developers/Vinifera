/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INFANTRYTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended InfantryTypeClass class.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

#include "technotypeext.h"
#include "infantrytype.h"


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

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;
        
        virtual InfantryTypeClass *This() const override { return reinterpret_cast<InfantryTypeClass *>(TechnoTypeClassExtension::This()); }
        virtual const InfantryTypeClass *This_Const() const override { return reinterpret_cast<const InfantryTypeClass *>(TechnoTypeClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_INFANTRYTYPE; }

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
