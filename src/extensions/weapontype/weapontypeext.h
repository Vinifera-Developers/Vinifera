/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WEAPONTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended WeaponTypeClass class.
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


class WeaponTypeClass;
class CCINIClass;


class WeaponTypeClassExtension final : public Extension<WeaponTypeClass>
{
    public:
        WeaponTypeClassExtension(WeaponTypeClass *this_ptr);
        WeaponTypeClassExtension(const NoInitClass &noinit);
        ~WeaponTypeClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  Does the firing unit destroy itself when this weapon is fired?
         */
        bool IsSuicide;

        /**
         *  Does the firing unit get explictly deleted when this weapon is fired?
         */
        bool IsDeleteOnSuicide;
};


extern ExtensionMap<WeaponTypeClass, WeaponTypeClassExtension> WeaponTypeClassExtensions;
