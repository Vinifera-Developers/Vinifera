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

#include "always.h"
#include "tibsun_defines.h"
#include "rules.h"
#include "extension.h"
#include "tpoint.h"


class CCINIClass;


class RulesClassExtension final : public GlobalExtensionClass<RulesClass>
{
    public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        RulesClassExtension(const RulesClass *this_ptr);
        RulesClassExtension(const NoInitClass &noinit);
        virtual ~RulesClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual const char *Name() const override { return "Rule"; }
        virtual const char *Full_Name() const override { return "Rule"; }

        void Process(CCINIClass &ini);
        void Initialize(CCINIClass &ini);

        bool Objects(CCINIClass &ini);

        bool General(CCINIClass &ini);
        bool MPlayer(CCINIClass &ini);
        bool AudioVisual(CCINIClass &ini);
        bool CombatDamage(CCINIClass &ini);
        bool Weapons(CCINIClass &ini);
        bool Armors(CCINIClass &ini);
        bool Tiberiums(CCINIClass &ini);

    private:
        void Check();
        void Fixups(CCINIClass &ini);

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

            /**
             *  Should the tactical rubber band box be drawn with a drop shadow?
             */
            bool IsBandBoxDropShadow;

            /**
             *  
             */
            bool IsBandBoxThick;

            /**
             *  
             */
            RGBStruct BandBoxColor;

            /**
             *  
             */
            RGBStruct BandBoxDropShadowColor;

            /**
             *  Transparency of the tactical rubber band.
             */
            unsigned BandBoxTintTransparency;

            /**
             *  
             */
            TypeList<RGBStruct> BandBoxTintColors;

            /**
             *  
             */
            bool IsMovementLineDashed;

            /**
             *  
             */
            bool IsMovementLineDropShadow;

            /**
             *  
             */
            bool IsMovementLineThick;

            /**
             *  
             */
            RGBStruct MovementLineColor;

            /**
             *  
             */
            RGBStruct MovementLineDropShadowColor;

            /**
             *  
             */
            bool IsTargetLineDashed;

            /**
             *  
             */
            bool IsTargetLineDropShadow;

            /**
             *  
             */
            bool IsTargetLineThick;

            /**
             *  
             */
            RGBStruct TargetLineColor;

            /**
             *  
             */
            RGBStruct TargetLineDropShadowColor;

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
         *  Defines the strength of ice. Higher values make ice less likely
         *  to break from a shot.
         */
        int IceStrength;

        /**
         *  Storage pip used for weeds.
         */
        int WeedPipIndex;

        /**
         *  Customizable maximum counts for drawing different pips.
         */
        TypeList<int> MaxPips;
};
