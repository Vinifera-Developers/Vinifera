/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDETYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SideClass class.
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

#include "abstracttypeext.h"
#include "side.h"
#include "house.h"
#include "housetype.h"
#include "tibsun_globals.h"


class InfantryTypeClass;

class DECLSPEC_UUID(UUID_SIDE_EXTENSION)
SideClassExtension final : public AbstractTypeClassExtension
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
        SideClassExtension(const SideClass *this_ptr = nullptr);
        SideClassExtension(const NoInitClass &noinit);
        virtual ~SideClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual SideClass *This() const override { return reinterpret_cast<SideClass *>(AbstractTypeClassExtension::This()); }
        virtual const SideClass *This_Const() const override { return reinterpret_cast<const SideClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_SIDE; }

        virtual bool Read_INI(CCINIClass &ini) override;

        static const InfantryTypeClass* Get_Crew(SideType side);
        static const InfantryTypeClass* Get_Engineer(SideType side);
        static const InfantryTypeClass* Get_Technician(SideType side);
        static const InfantryTypeClass* Get_Disguise(SideType side);
        static int Get_Survivor_Divisor(SideType side);

        inline static const InfantryTypeClass* Get_Crew(const HouseClass* house) { return Get_Crew(house->Class->Side); }
        inline static const InfantryTypeClass* Get_Engineer(const HouseClass* house) { return Get_Engineer(house->Class->Side); }
        inline static const InfantryTypeClass* Get_Technician(const HouseClass* house) { return Get_Technician(house->Class->Side); }
        inline static const InfantryTypeClass* Get_Disguise(const HouseClass* house) { return Get_Disguise(house->Class->Side); }
        inline static int Get_Survivor_Divisor(const HouseClass* house) { return Get_Survivor_Divisor(house->Class->Side); }

    public:

        /**
         *  Color scheme to be used in the UI of this side.
         */
        ColorSchemeType UIColor;

        /**
         *  Color scheme to be used for the tooltips of this side.
         */
        ColorSchemeType ToolTipColor;

        /**
         *  InfantryType used as this Side's crew.
         */
        const InfantryTypeClass* Crew;

        /**
         *  InfantryType used as this Side's engineer.
         */
        const InfantryTypeClass* Engineer;

        /**
         *  InfantryType used as this Side's technician.
         */
        const InfantryTypeClass* Technician;

        /**
         *  InfantryType used as this Side's disguise.
         */
        const InfantryTypeClass* Disguise;

        /**
         *  The number of survivors is divided by this much when calculating a building's number of survivors.
         */
        int SurvivorDivisor;

        /**
         *  UnitType used as this Side's Hunter-Seeker.
         */
        const UnitTypeClass* HunterSeeker;
};
