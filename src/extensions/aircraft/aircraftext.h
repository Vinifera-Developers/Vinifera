/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended AircraftClass class.
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
#include "aircraft.h"


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

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual AircraftClass *This() const override { return reinterpret_cast<AircraftClass *>(FootClassExtension::This()); }
        virtual const AircraftClass *This_Const() const override { return reinterpret_cast<const AircraftClass *>(FootClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_AIRCRAFT; }

    public:

};
