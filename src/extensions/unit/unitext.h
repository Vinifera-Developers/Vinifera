/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended UnitClass class.
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
#include "unit.h"
#include "building.h"


class DECLSPEC_UUID(UUID_UNIT_EXTENSION)
UnitClassExtension final : public FootClassExtension
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
        UnitClassExtension(const UnitClass *this_ptr = nullptr);
        UnitClassExtension(const NoInitClass &noinit);
        virtual ~UnitClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual UnitClass *This() const override { return reinterpret_cast<UnitClass *>(FootClassExtension::This()); }
        virtual const UnitClass *This_Const() const override { return reinterpret_cast<const UnitClass *>(FootClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_UNIT; }

    public:
        /**
        *  #issue-203
        *
        *  The building that this unit last docked with.
        *  Used by harvesters for considering the distance to their last refinery
        *  when picking a tiberium cell to harvest from.
        */
        BuildingClass *LastDockedBuilding;
};
