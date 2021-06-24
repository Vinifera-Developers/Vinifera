/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WARHEADTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended WarheadTypeClass class.
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

#include "extension.h"
#include "container.h"


class WarheadTypeClass;
class CCINIClass;


class WarheadTypeClassExtension final : public Extension<WarheadTypeClass>
{
    public:
        WarheadTypeClassExtension(WarheadTypeClass *this_ptr);
        WarheadTypeClassExtension(const NoInitClass &noinit);
        ~WarheadTypeClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  Does this warhead instantly destroy walls regardless of the warhead damage value?
         */
        bool IsWallAbsoluteDestroyer;

        /**
         *  Can this warhead damage friendly units?
         */
        bool IsAffectsAllies;

        /**
         *  This is used to override the size of the combat light flash at the point of impact.
         */
        float CombatLightSize;

        /**
         *  These values are used to shake the screen when the projectile impacts.
         */
        unsigned int ShakePixelYHi;
        unsigned int ShakePixelYLo;
        unsigned int ShakePixelXHi;
        unsigned int ShakePixelXLo;
};


extern ExtensionMap<WarheadTypeClass, WarheadTypeClassExtension> WarheadTypeClassExtensions;
