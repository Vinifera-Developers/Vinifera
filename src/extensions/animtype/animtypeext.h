/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ANIMTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended AnimTypeClass class.
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


class AnimTypeClass;
class CCINIClass;
class ParticleTypeClass;


class AnimTypeClassExtension final : public Extension<AnimTypeClass>
{
    public:
        AnimTypeClassExtension(AnimTypeClass *this_ptr);
        AnimTypeClassExtension(const NoInitClass &noinit);
        ~AnimTypeClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  If the cell in which this animation is placed does not contain
         *  tiberium overlay, then this anim will not be drawn.
         */
        bool IsHideIfNotTiberium;

        /**
         *  Are the craters spawned by this anim when it ends much larger than normal?
         */
        bool IsForceBigCraters;

        /**
         *  Fudge to this anims Z-axis (depth). Positive values move the
         *  animation "away from the screen" or "closer to the ground". Negative
         *  values do the opposite. 
         */
        int ZAdjust;

        /**
         *	The map layer this animation is in when attached to an object.
         */
        LayerType AttachLayer;

        /**
         *  The particle to spawn at the mid-point of this animation.
         */
        ParticleType ParticleToSpawn;

        /**
         *  The number of the particle to spawn.
         */
        unsigned NumberOfParticles;
};


extern ExtensionMap<AnimTypeClass, AnimTypeClassExtension> AnimTypeClassExtensions;
