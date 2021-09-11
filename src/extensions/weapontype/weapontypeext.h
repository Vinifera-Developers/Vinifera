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
#include "rgb.h"
#include "vector3.h"


class WeaponTypeClass;
class CCINIClass;


typedef enum SonicBeamSurfacePatternType
{
    SURFACE_PATTERN_CIRCLE,
    SURFACE_PATTERN_ELLIPSE,
    SURFACE_PATTERN_RHOMBUS,
    SURFACE_PATTERN_SQUARE,

    SURFACE_PATTERN_COUNT
} SonicBeamSurfacePatternType;


typedef enum SonicBeamSinePatternType
{
    SINE_PATTERN_CIRCLE,
    SINE_PATTERN_SQUARE,
    SINE_PATTERN_SAWTOOTH,
    SINE_PATTERN_TRIANGLE,

    SINE_PATTERN_COUNT
} SonicBeamSinePatternType;


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
         *  Color of the sonic beam, in 24-bit RGB.
         */
        RGBStruct SonicBeamColor;

        /**
         *  Is the wave clear (no color)?
         */
        bool SonicBeamIsClear;

        /**
         *  The alpha blending of the sonic beam.
         */
        double SonicBeamAlpha;
        
        /**
         *  The duration of one wave cycle.
         */
        double SonicBeamSineDuration;
        
        /**
         *  The amplitude of the wave.
         */
        double SonicBeamSineAmplitude;
        
        /**
         *  The amount to offset the pixel data.
         */
        double SonicBeamOffset;

        /**
         *  Start and end pins for the shape of the sonic beam.
         */
        Vector3 SonicBeamStartPinLeft;
        Vector3 SonicBeamStartPinRight;
        Vector3 SonicBeamEndPinLeft;
        Vector3 SonicBeamEndPinRight;

        /**
         *  The pattern type for the sonic beam effect.
         */
        SonicBeamSurfacePatternType SonicBeamSurfacePattern;
        SonicBeamSinePatternType SonicBeamSinePattern;
};


extern ExtensionMap<WeaponTypeClass, WeaponTypeClassExtension> WeaponTypeClassExtensions;
