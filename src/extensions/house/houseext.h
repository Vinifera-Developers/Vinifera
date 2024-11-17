/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended HouseClass class.
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

#include "abstractext.h"
#include "house.h"
#include "housetype.h"


class DECLSPEC_UUID(UUID_HOUSE_EXTENSION)
HouseClassExtension final : public AbstractClassExtension
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
        HouseClassExtension(const HouseClass *this_ptr = nullptr);
        HouseClassExtension(const NoInitClass &noinit);
        virtual ~HouseClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual const char *Name() const override { return reinterpret_cast<const HouseClass *>(This())->Class->Name(); }
        virtual const char *Full_Name() const override { return reinterpret_cast<const HouseClass *>(This())->Class->Full_Name(); }
        
        virtual HouseClass *This() const override { return reinterpret_cast<HouseClass *>(AbstractClassExtension::This()); }
        virtual const HouseClass *This_Const() const override { return reinterpret_cast<const HouseClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_HOUSE; }

        void Put_Storage_Pointers();
        static void Load_Unit_Trackers(HouseClass* house, IStream* pStm);
        static void Save_Unit_Trackers(HouseClass* house, IStream* pStm);

    public:
        /**
         *  Replacement Tiberium storage.
         */
        VectorClass<int> TiberiumStorage;

        /**
         *  Replacement Weed storage.
         */
        VectorClass<int> WeedStorage;
};
