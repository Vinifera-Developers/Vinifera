/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended UnitTypeClass class.
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


class UnitTypeClass;
class CCINIClass;


class UnitTypeClassExtension final : public Extension<UnitTypeClass>
{
    public:
        UnitTypeClassExtension(UnitTypeClass *this_ptr);
        UnitTypeClassExtension(const NoInitClass &noinit);
        ~UnitTypeClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  Can this unit be picked up (toted) by the carryall aircraft?
         */
        bool IsTotable;

        /**
         *  The starting frame for the turret graphics in the units shape file.
         */
        int StartTurretFrame;

        /**
         *  The facing count for the turret graphics in the units shape file.
         */
        int TurretFacings;

        /**
         *  The starting frame for the idle animation in the units shape file.
         */
        unsigned StartIdleFrame;

        /**
         *  The number of image frames for each of the idle animation sequences.
         */
        unsigned IdleFrames;
};


extern ExtensionMap<UnitTypeClass, UnitTypeClassExtension> UnitTypeClassExtensions;
