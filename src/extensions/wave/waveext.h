/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WAVEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended WaveClass class.
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
#include "wave.h"
#include "vector3.h"
#include "weapontypeext.h"


class WaveClassExtension final : public Extension<WaveClass>
{
    public:
        WaveClassExtension(WaveClass *this_ptr);
        WaveClassExtension(const NoInitClass &noinit);
        ~WaveClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        void Init();

        void Draw_Sonic_Beam_Pixel(int a1, int a2, int a3, unsigned short *buffer);

        bool Calculate_Sonic_Beam_Tables();

    public:
        /**
         *  Pointer to the weapon this is firing this wave.
         */
        WeaponTypeClass *WeaponTypePtr;

        /**
         *  The following are copied from WeaponTypeExtension on creation.
         */
        RGBStruct SonicBeamColor;
        bool SonicBeamIsClear;
        double SonicBeamAlpha;
        double SonicBeamSineDuration;
        double SonicBeamSineAmplitude;
        double SonicBeamOffset;
        SonicBeamSurfacePatternType SonicBeamSurfacePattern;
        SonicBeamSinePatternType SonicBeamSinePattern;
        Vector3 SonicBeamStartPinLeft;
        Vector3 SonicBeamStartPinRight;
        Vector3 SonicBeamEndPinLeft;
        Vector3 SonicBeamEndPinRight;

    private:
        /**
         *  Generated tables based on the custom values.
         */
        bool SonicBeamTablesCalculated;
        short SonicBeamSineTable[500];
        short SonicBeamSurfacePatternTable[300][300];
        int SonicBeamIntensityTable[14];
};


extern ExtensionMap<WaveClass, WaveClassExtension> WaveClassExtensions;

extern WeaponTypeClass *Wave_TempWeaponTypePtr;
