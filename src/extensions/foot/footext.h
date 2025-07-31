/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FOOTEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended FootClass class.
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

#include "technoext.h"
#include "foot.h"


class FootClassExtension : public TechnoClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        FootClassExtension(const FootClass *this_ptr);
        FootClassExtension(const NoInitClass &noinit);
        virtual ~FootClassExtension();

        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual FootClass *This() const override { return reinterpret_cast<FootClass *>(TechnoClassExtension::This()); }
        virtual const FootClass *This_Const() const override { return reinterpret_cast<const FootClass *>(TechnoClassExtension::This_Const()); }

        virtual void Set_Last_Flight_Cell(Cell cell);
        virtual Cell Get_Last_Flight_Cell() const;

    public:
        /**
         *  The last known flight cell of this object, used by the AircraftTracker.
         */
        Cell LastFlightCell;
};
