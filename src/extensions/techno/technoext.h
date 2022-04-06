/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TechnoClass class.
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
#include "techno.h"


class EBoltClass;
class ParticleSystemClass;


class TechnoClassExtension final : public Extension<TechnoClass>
{
    private:
        enum {
            ATTACHED_PARTICLE_NATURAL2,
            ATTACHED_PARTICLE_NATURAL3,

            EXT_ATTACHED_PARTICLE_COUNT
        };

    public:
        TechnoClassExtension(TechnoClass *this_ptr);
        TechnoClassExtension(const NoInitClass &noinit);
        ~TechnoClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        /**
         *  Extended class functions.
         */
        void Response_Capture();
        void Response_Enter();
        void Response_Deploy();
        void Response_Harvest();
        bool Can_Passive_Acquire() const;
        void Spawn_Natural_Particle_System();

    public:
        /**
         *  The current electric bolt instance fired by this object.
         */
        EBoltClass *ElectricBolt;

        /**
         *  Additional attached particle system trackers for this object. 
         */
        ParticleSystemClass *ParticleSystems[EXT_ATTACHED_PARTICLE_COUNT];
};


extern ExtensionMap<TechnoClass, TechnoClassExtension> TechnoClassExtensions;
