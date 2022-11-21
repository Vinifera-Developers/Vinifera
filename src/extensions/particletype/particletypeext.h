/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PARTICLETYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended ParticleTypeClass class.
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

#include "objecttypeext.h"
#include "particletype.h"


class DECLSPEC_UUID(UUID_PARTICLETYPE_EXTENSION)
ParticleTypeClassExtension final : public ObjectTypeClassExtension
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
        ParticleTypeClassExtension(const ParticleTypeClass *this_ptr = nullptr);
        ParticleTypeClassExtension(const NoInitClass &noinit);
        virtual ~ParticleTypeClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual ParticleTypeClass *This() const override { return reinterpret_cast<ParticleTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const ParticleTypeClass *This_Const() const override { return reinterpret_cast<const ParticleTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_PARTICLETYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
};
