/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TechnoClass class.
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

#include "objectext.h"
#include "techno.h"


class EBoltClass;
class TechnoTypeClass;
class TechnoTypeClassExtension;


class TechnoClassExtension : public ObjectClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        TechnoClassExtension(const TechnoClass *this_ptr);
        TechnoClassExtension(const NoInitClass &noinit);
        virtual ~TechnoClassExtension();

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual TechnoClass *This() const override { return reinterpret_cast<TechnoClass *>(ObjectClassExtension::This()); }
        virtual const TechnoClass *This_Const() const override { return reinterpret_cast<const TechnoClass *>(ObjectClassExtension::This_Const()); }

        virtual void Response_Capture();
        virtual void Response_Enter();
        virtual void Response_Deploy();
        virtual void Response_Harvest();
        virtual bool Can_Passive_Acquire() const;

    private:
        const TechnoTypeClass *Techno_Type_Class() const;
        const TechnoTypeClassExtension *Techno_Type_Class_Ext() const;

    public:
        /**
         *  The current electric bolt instance fired by this object.
         */
        EBoltClass *ElectricBolt;
};
