/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PARTICLESYSTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended ParticleSystemTypeClass class.
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
#include "particlesystype.h"


class DECLSPEC_UUID(UUID_PARTICLESYSTEMTYPE_EXTENSION)
ParticleSystemTypeClassExtension final : public ObjectTypeClassExtension
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
        ParticleSystemTypeClassExtension(const ParticleSystemTypeClass *this_ptr = nullptr);
        ParticleSystemTypeClassExtension(const NoInitClass &noinit);
        virtual ~ParticleSystemTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual ParticleSystemTypeClass *This() const override { return reinterpret_cast<ParticleSystemTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const ParticleSystemTypeClass *This_Const() const override { return reinterpret_cast<const ParticleSystemTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_PARTICLESYSTEMTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
};
