/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended AircraftTypeClass class.
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
#include "aircrafttype.h"


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

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;
        
        virtual AircraftTypeClass *This() const override { return reinterpret_cast<AircraftTypeClass *>(TechnoTypeClassExtension::This()); }
        virtual const AircraftTypeClass *This_Const() const override { return reinterpret_cast<const AircraftTypeClass *>(TechnoTypeClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_AIRCRAFTTYPE; }

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
