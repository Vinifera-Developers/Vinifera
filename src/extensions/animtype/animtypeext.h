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

#include "objecttypeext.h"
#include "animtype.h"
#include "typelist.h"


class AnimTypeClass;
class CCINIClass;
class ParticleTypeClass;


class DECLSPEC_UUID(UUID_ANIMTYPE_EXTENSION)
AnimTypeClassExtension final : public ObjectTypeClassExtension
{
    public:
        /**
         *  IPersist
         */
        IFACEMETHOD(GetClassID)(CLSID *pClassID);

        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        AnimTypeClassExtension(const AnimTypeClass *this_ptr = nullptr);
        AnimTypeClassExtension(const NoInitClass &noinit);
        virtual ~AnimTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual AnimTypeClass *This() const override { return reinterpret_cast<AnimTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const AnimTypeClass *This_Const() const override { return reinterpret_cast<const AnimTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_ANIMTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

        int Stage_Count() const;

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
        int NumberOfParticles;

        /**
         *  The coordinate offset of the spawned particle.
         */
        Point3D ParticleSpawnOffset;

        /**
         *  List of animations to spawn at the logical start of this animation.
         */
        TypeList<AnimTypeClass *> StartAnims;
        TypeList<int> StartAnimsCount;
        TypeList<int> StartAnimsMinimum;
        TypeList<int> StartAnimsMaximum;
        TypeList<int> StartAnimsDelay;

        /**
         *  List of animations to spawn at the logical middle of this animation.
         */
        TypeList<AnimTypeClass *> MiddleAnims;
        TypeList<int> MiddleAnimsCount;
        TypeList<int> MiddleAnimsMinimum;
        TypeList<int> MiddleAnimsMaximum;
        TypeList<int> MiddleAnimsDelay;

        /**
         *  List of animations to spawn at the logical end of this animation.
         */
        TypeList<AnimTypeClass *> EndAnims;
        TypeList<int> EndAnimsCount;
        TypeList<int> EndAnimsMinimum;
        TypeList<int> EndAnimsMaximum;
        TypeList<int> EndAnimsDelay;

        /**
         *  The middle (biggest) frames.
         */
        TypeList<int> MiddleFrames;

        /**
         *  If positive, the animation will spawn an explosion during its biggest frame dealing this much damage.
         */
        int ExplosionDamage;

        /**
         *  Does this animation have a shadow?
         */
        bool IsShadow;

        /**
         *  A separate rate at which the anim deals damage.
         */
        int DamageRate;

        /**
         *  The sound effect to play when this anim has finished.
         */
        VocType StopSound;
};
