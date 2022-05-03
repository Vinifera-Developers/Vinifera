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
#include "typelist.h"
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

        bool General(CCINIClass &ini);
        bool MPlayer(CCINIClass &ini);
        bool AudioVisual(CCINIClass &ini);
        bool Weapons(CCINIClass &ini);

        static bool Read_UI_INI();
        static bool Init_UI_Controls();

    private:
        void Check();

    public:
        typedef struct UIControlsStruct
        {
            /**
             *  Health bar draw positions.
             */
            TPoint2D<int> UnitHealthBarDrawPos;
            TPoint2D<int> InfantryHealthBarDrawPos;

            /**
             *  Should the text label be drawn with an outline?
             */
            bool IsTextLabelOutline;

            /**
             *  Transparency of the text background.
             */
            unsigned TextLabelBackgroundTransparency;

        } UIControlsStruct;

        static UIControlsStruct UIControls;

    public:
        /**
         *  Should the MCV unit auto deploy on game start?
         */
        bool IsMPAutoDeployMCV;

        /**
         *  Are construction yards pre-placed on the map rather than a MCV given to the player?
         */
        bool IsMPPrePlacedConYards;

        /**
         *  Can players build their own structures adjacent to structures owned by their allies?
         */
        bool IsBuildOffAlly;

        /**
         *  Should active super weapons show their recharge timer display
         *  on the tactical view?
         */
        bool IsShowSuperWeaponTimers;

        /**
         *  A list of buildings considered Naval Yard's for the computer to choose
         *  from when building its base.
         */
        TypeList<BuildingTypeClass *> BuildNavalYard;
};


/**
 *  Global instance of the extended class.
 */
extern RulesClassExtension *RulesExtension;
