/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RULESEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended RulesClass class.
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

#include "noinit.h"
#include "tpoint.h"


class CCINIClass;
class RulesClass;


class RulesClassExtension final : public Extension<RulesClass>
{
    public:
        RulesClassExtension(RulesClass *this_ptr);
        RulesClassExtension(const NoInitClass &noinit);
        ~RulesClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        void Process(CCINIClass &ini);
        void Initialize(CCINIClass &ini);

        static bool Read_UI_INI();
        static bool Init_UI_Controls();

    public:
        typedef struct UIControlsStruct
        {
            /**
             *  Health bar draw positions.
             */
            TPoint2D<int> UnitHealthBarDrawPos;
            TPoint2D<int> InfantryHealthBarDrawPos;

            /**
             *  QoL Improvement: Does a right mouse click cancel player message input?
             */
            bool IsRightClickCancelMessage;

        } UIControlsStruct;

        static UIControlsStruct UIControls;

    private:
        bool General(CCINIClass &ini);

    public:

};


/**
 *  Global instance of the extended class.
 */
extern RulesClassExtension *RulesExtension;
