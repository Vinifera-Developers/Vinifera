/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT.CPP
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
#include "technoext.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_inline.h"
#include "tibsun_globals.h"
#include "particlesys.h"
#include "particlesystype.h"
#include "house.h"
#include "voc.h"
#include "ebolt.h"
#include "tibsun_inline.h"
#include "iomap.h"
#include "wwcrc.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all TechnoClass extension instances.
 */
ExtensionMap<TechnoClass, TechnoClassExtension> TechnoClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::TechnoClassExtension(TechnoClass *this_ptr) :
    Extension(this_ptr),
    ElectricBolt(nullptr)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("TechnoClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    for (int index = 0; index < EXT_ATTACHED_PARTICLE_COUNT; ++index) {
        ParticleSystems[index] = nullptr;
    }

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::TechnoClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::~TechnoClassExtension()
{
    //EXT_DEBUG_TRACE("TechnoClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("TechnoClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TechnoClassExtension(NoInitClass());

    ElectricBolt = nullptr;
   
	for (int index = 0; index < EXT_ATTACHED_PARTICLE_COUNT; ++index) {
        SWIZZLE_REQUEST_POINTER_REMAP(ParticleSystems[index]);
    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    delete ElectricBolt;
    ElectricBolt = nullptr;

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int TechnoClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TechnoClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Handles the voice response when given capture order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Capture()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Capture - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    if (!AllowVoice) {
        return;
    }

    //if (!ThisPtr->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    TechnoTypeClass *technotype = ThisPtr->Techno_Type_Class();
    TechnoTypeClassExtension *technotypeext = TechnoTypeClassExtensions.find(technotype);
    if (technotypeext && technotypeext->VoiceCapture.Count() > 0) {

        response = technotypeext->VoiceCapture[Sim_Random_Pick(0, technotypeext->VoiceCapture.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Handles the voice response when given enter order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Enter()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Enter - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    if (!AllowVoice) {
        return;
    }

    //if (!ThisPtr->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    TechnoTypeClass *technotype = ThisPtr->Techno_Type_Class();
    TechnoTypeClassExtension *technotypeext = TechnoTypeClassExtensions.find(technotype);
    if (technotypeext && technotypeext->VoiceEnter.Count() > 0) {

        response = technotypeext->VoiceEnter[Sim_Random_Pick(0, technotypeext->VoiceEnter.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Handles the voice response when given deploy order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Deploy()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Deploy - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    if (!AllowVoice) {
        return;
    }

    //if (!ThisPtr->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    TechnoTypeClass *technotype = ThisPtr->Techno_Type_Class();
    TechnoTypeClassExtension *technotypeext = TechnoTypeClassExtensions.find(technotype);
    if (technotypeext && technotypeext->VoiceDeploy.Count() > 0) {

        response = technotypeext->VoiceDeploy[Sim_Random_Pick(0, technotypeext->VoiceDeploy.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Handles the voice response when given harvest order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Harvest()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Harvest - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    if (!AllowVoice) {
        return;
    }

    //if (!ThisPtr->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    TechnoTypeClass *technotype = ThisPtr->Techno_Type_Class();
    TechnoTypeClassExtension *technotypeext = TechnoTypeClassExtensions.find(technotype);
    if (technotypeext && technotypeext->VoiceHarvest.Count() > 0) {

        response = technotypeext->VoiceHarvest[Sim_Random_Pick(0, technotypeext->VoiceHarvest.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Returns if this object can acquire targets that are within range and attack them automatically.
 * 
 *  @author: CCHyper
 */
bool TechnoClassExtension::Can_Passive_Acquire() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Can_Passive_Acquire - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    
    TechnoTypeClass *technotype = ThisPtr->Techno_Type_Class();
    TechnoTypeClassExtension *technotypeext = TechnoTypeClassExtensions.find(technotype);

    if (technotypeext) {
        return technotypeext->IsCanPassiveAcquire;
    }

    /**
     *  Original behaviour, all units can passive acquire.
     */
    return true;
}


/**
 *  Handles the voice response when given harvest order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Spawn_Natural_Particle_System()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoClassExtension::Spawn_Natural_Particle_System - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    
    Coordinate where;

    TechnoTypeClass *technotype = ThisPtr->Techno_Type_Class();
    TechnoTypeClassExtension *technotypeext = TechnoTypeClassExtensions.find(technotype);

    /**
     *  Spawn NaturalParticleSystem.
     */
    if (!ThisPtr->ParticleSystems[ATTACHED_PARTICLE_NATURAL] && technotype->NaturalParticleSystem) {

        where = ThisPtr->Get_Coord();

        where.X += technotype->NaturalParticleSystemLocation.X;
        where.Y += technotype->NaturalParticleSystemLocation.Y;

        /**
         *  #BUGFIX: The original code did not take into account the Z coord
         *           of the NaturalParticleSystem location.
         */
        where.Z += technotype->NaturalParticleSystemLocation.Z;

        ThisPtr->ParticleSystems[ATTACHED_PARTICLE_NATURAL] = new ParticleSystemClass(
            technotype->NaturalParticleSystem, where, &Map[ThisPtr->Get_Coord()]
        );
    }

    /**
     *  Spawn NaturalParticleSystem2.
     */
    if (!ParticleSystems[ATTACHED_PARTICLE_NATURAL2] && technotypeext->NaturalParticleSystem2) {

        where = ThisPtr->Get_Coord();

        where.X += technotypeext->NaturalParticleSystemLocation2.X;
        where.Y += technotypeext->NaturalParticleSystemLocation2.Y;
        where.Z += technotypeext->NaturalParticleSystemLocation2.Z;

        ParticleSystems[ATTACHED_PARTICLE_NATURAL2] = new ParticleSystemClass(
            technotypeext->NaturalParticleSystem2, where, &Map[ThisPtr->Get_Coord()]
        );
    }

    /**
     *  Spawn NaturalParticleSystem3.
     */
    if (!ParticleSystems[ATTACHED_PARTICLE_NATURAL3] && technotypeext->NaturalParticleSystem3) {

        where = ThisPtr->Get_Coord();

        where.X += technotypeext->NaturalParticleSystemLocation3.X;
        where.Y += technotypeext->NaturalParticleSystemLocation3.Y;
        where.Z += technotypeext->NaturalParticleSystemLocation3.Z;

        ParticleSystems[ATTACHED_PARTICLE_NATURAL3] = new ParticleSystemClass(
            technotypeext->NaturalParticleSystem3, where, &Map[ThisPtr->Get_Coord()]
        );
    }
}
