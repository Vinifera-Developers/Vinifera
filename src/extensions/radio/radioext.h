/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RADIOEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended Radio class.
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

#include "missionext.h"
#include "radio.h"


class RadioClassExtension : public MissionClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        RadioClassExtension(const RadioClass *this_ptr);
        RadioClassExtension(const NoInitClass &noinit);
        virtual ~RadioClassExtension();

        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual RadioClass *This() const override { return reinterpret_cast<RadioClass *>(ObjectClassExtension::This()); }
        virtual const RadioClass *This_Const() const override { return reinterpret_cast<const RadioClass *>(ObjectClassExtension::This_Const()); }

    public:
};
