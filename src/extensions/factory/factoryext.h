/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FACTORYEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended FactoryClass class.
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
#include "factory.h"


class DECLSPEC_UUID(UUID_FACTORY_EXTENSION)
FactoryClassExtension final : public AbstractClassExtension
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
        FactoryClassExtension(const FactoryClass *this_ptr = nullptr);
        FactoryClassExtension(const NoInitClass &noinit);
        virtual ~FactoryClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual FactoryClass *This() const override { return reinterpret_cast<FactoryClass *>(AbstractClassExtension::This()); }
        virtual const FactoryClass *This_Const() const override { return reinterpret_cast<const FactoryClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_FACTORY; }

        virtual const char *Name() const { return "Factory"; }
        virtual const char *Full_Name() const { return "Factory"; }

    public:
};
