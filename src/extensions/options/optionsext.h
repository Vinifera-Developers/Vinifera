/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OPTIONSEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended OptionsClass class.
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


class OptionsClass;
class CCINIClass;


class OptionsClassExtension final : public Extension<OptionsClass>
{
    public:
        OptionsClassExtension(OptionsClass *this_ptr);
        OptionsClassExtension(const NoInitClass &noinit);
        ~OptionsClassExtension();

        virtual HRESULT Load(IStream *pStm) override { return S_OK; }
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override { return S_OK; }
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override {}
        virtual void Compute_CRC(WWCRCEngine &crc) const override {}

        void Load_Settings();
        void Load_Init_Settings();
        void Save_Settings();

        void Set();

    public:
};


extern OptionsClassExtension *OptionsExtension;
