/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TechnoTypeClass class.
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
#include "tibsun_defines.h"


class TechnoTypeClass;
class CCINIClass;
class BSurface;


class TechnoTypeClassExtension final : public Extension<TechnoTypeClass>
{
    public:
        TechnoTypeClassExtension(TechnoTypeClass *this_ptr);
        TechnoTypeClassExtension(const NoInitClass &noinit);
        ~TechnoTypeClassExtension();

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  This is the sound effect to play when the unit is cloaking.
         */
        VocType CloakSound;

        /**
         *  This is the sound effect to play when the unit is decloaking.
         */
        VocType UncloakSound;

        /**
         *  Can this object shake the screen when it is destroyed?
         *  (Must meet the rules as specified by Rule->ShakeScreen.
         */
        bool IsShakeScreen;

        /**
         *  Is this object immune to EMP (electromagnetic pulse) effects?
         *  Powered buildings, vehicles and cyborgs are typically disabled
         *  by EMP, unless this is set for them.
         */
        bool IsImmuneToEMP;

        /**
         *  Can this object acquire targets that are within its weapons range
         *  and attack them automatically?
         */
        bool IsCanPassiveAcquire;

        /**
         *  Can this object retaliate when hit by enemy fire?
         */
        bool IsCanRetaliate;

        /**
         *  These values are used to shake the screen when the object is destroyed.
         */
        unsigned ShakePixelYHi;
        unsigned ShakePixelYLo;
        unsigned ShakePixelXHi;
        unsigned ShakePixelXLo;

        /**
         *  The graphic class to switch to when this unit is unloading (e.g., at a refinery).
         */
        const TechnoTypeClass *UnloadingClass;

        /**
         *  The refund value for the unit when it is sold at a Service Depot.
         */
        unsigned SoylentValue;

        /**
         *  This is the sound effect to play when a passenger enters this unit.
         */
        VocType EnterTransportSound;

        /**
         *  This is the sound effect to play when a passenger leaves this unit.
         */
        VocType LeaveTransportSound;

        /**
         *  List of voices to use when giving this object a capture order.
         */
        TypeList<VocType> VoiceCapture;

        /**
         *  List of voices to use when giving this object an enter order (ie, transport, infiltrate building).
         */
        TypeList<VocType> VoiceEnter;

        /**
         *  List of voices to use when giving this object a unload order.
         */
        TypeList<VocType> VoiceDeploy;

        /**
         *  List of voices to use when giving this object a harvest order.
         */
        TypeList<VocType> VoiceHarvest;

        /**
         *  The rate at which this unit animates when it is standing idle (not moving).
         */
        unsigned IdleRate;

        /**
         *  Pointer to the cameo image surface.
         */
        BSurface *CameoImageSurface;

        /**
         *  Is this object considered a water based vehicle (i.e. ship, submarine, etc)?
         * 
         *  #NOTE: This is a temporary property that is used to enforce special rules to
         *         allow better naval and shipyard support until a full Vessel class is hooked
         *         up and implemented.
         */
        bool IsNaval;
};


extern ExtensionMap<TechnoTypeClass, TechnoTypeClassExtension> TechnoTypeClassExtensions;
