/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BULLETTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended BulletTypeClass class.
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
#include "bullettype.h"


class DECLSPEC_UUID(UUID_BULLETTYPE_EXTENSION)
BulletTypeClassExtension final : public ObjectTypeClassExtension
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
        BulletTypeClassExtension(const BulletTypeClass *this_ptr = nullptr);
        BulletTypeClassExtension(const NoInitClass &noinit);
        virtual ~BulletTypeClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;
        
        virtual BulletTypeClass *This() const override { return reinterpret_cast<BulletTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const BulletTypeClass *This_Const() const override { return reinterpret_cast<const BulletTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_BULLETTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
        /**
         *  The number of frames between trailer anim spawns.
         */
        unsigned SpawnDelay;

        /**
         *  If set, this projectile can only be used against targets on water.
         */
        bool IsTorpedo;
};
