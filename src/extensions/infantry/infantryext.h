/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INFANTRYEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended InfantryClass class.
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

#include "footext.h"
#include "infantry.h"


class InfantryClass;
class HouseClass;


class DECLSPEC_UUID(UUID_INFANTRY_EXTENSION)
InfantryClassExtension final : public FootClassExtension
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
        InfantryClassExtension(const InfantryClass *this_ptr = nullptr);
        InfantryClassExtension(const NoInitClass &noinit);
        virtual ~InfantryClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual InfantryClass *This() const override { return reinterpret_cast<InfantryClass *>(FootClassExtension::This()); }
        virtual const InfantryClass *This_Const() const override { return reinterpret_cast<const InfantryClass *>(FootClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_INFANTRY; }

    public:
};
