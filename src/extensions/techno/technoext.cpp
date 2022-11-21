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
#include "house.h"
#include "voc.h"
#include "ebolt.h"
#include "tibsun_inline.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::TechnoClassExtension(const TechnoClass *this_ptr) :
    ObjectClassExtension(this_ptr),
    ElectricBolt(nullptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TechnoClassExtension::TechnoClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::TechnoClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::TechnoClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::~TechnoClassExtension()
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::~TechnoClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ElectricBolt = nullptr;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    ElectricBolt = nullptr;
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    delete ElectricBolt;
    ElectricBolt = nullptr;

    return hr;
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TechnoClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectClassExtension::Compute_CRC(crc);
}


/**
 *  Handles the voice response when given capture order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Capture()
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Capture - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);
    if (technotypeext->VoiceCapture.Count() > 0) {

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Enter - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);
    if (technotypeext->VoiceEnter.Count() > 0) {

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Deploy - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);
    if (technotypeext->VoiceDeploy.Count() > 0) {

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Harvest - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);
    if (technotypeext->VoiceHarvest.Count() > 0) {

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Can_Passive_Acquire - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
    
    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);

    /**
     *  IsCanPassiveAcquire defaults to true to copy original behaviour, so all units can passive acquire unless told otherwise.
     */
    return technotypeext->IsCanPassiveAcquire;
}


/**
 *  Provides access to the TechnoTypeClass instance for this extension. 
 * 
 *  @author: CCHyper
 */
const TechnoTypeClass *TechnoClassExtension::Techno_Type_Class() const
{
    return reinterpret_cast<TechnoClass *>(This())->Techno_Type_Class();
}
