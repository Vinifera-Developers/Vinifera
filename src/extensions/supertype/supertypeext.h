/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SUPERTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SuperWeaponTypeClass class.
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
#include "supertype.h"


class BSurface;


class DECLSPEC_UUID(UUID_SUPERWEAPONTYPE_EXTENSION)
SuperWeaponTypeClassExtension final : public AbstractTypeClassExtension
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
        SuperWeaponTypeClassExtension(const SuperWeaponTypeClass *this_ptr = nullptr);
        SuperWeaponTypeClassExtension(const NoInitClass &noinit);
        virtual ~SuperWeaponTypeClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual bool Read_INI(CCINIClass &ini) override;

        virtual SuperWeaponTypeClass *This() const override { return reinterpret_cast<SuperWeaponTypeClass *>(AbstractTypeClassExtension::This()); }
        virtual const SuperWeaponTypeClass *This_Const() const override { return reinterpret_cast<const SuperWeaponTypeClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_SUPERWEAPONTYPE; }

    protected:
        /**
         *  These are only to be accessed for save and load operations!
         */
        char SidebarImage[24 + 1];

    public:
        /**
         *  When this super weapon is active, does its recharge timer display
         *  on the tactical view?
         */
        bool IsShowTimer;

        /**
         *  Pointer to the cameo image surface.
         */
        BSurface *CameoImageSurface;

        /**
         *  Action type used for the cursor when the SW is out of range to fire.
         */
        ActionType ActionRange;
};
