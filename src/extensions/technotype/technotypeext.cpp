/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOTYPEEXT.CPP
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
#include "technotypeext.h"
#include "technotype.h"
#include "ccini.h"
#include "filepng.h"
#include "swizzle.h"
#include "bsurface.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "spritecollection.h"
#include "vinifera_saveload.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(const TechnoTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    CloakSound(VOC_NONE),
    UncloakSound(VOC_NONE),
    IsShakeScreen(false),
    IsImmuneToEMP(false),
    IsCanPassiveAcquire(true),
    IsCanRetaliate(true),
    IsLegalTargetComputer(true),
    ShakePixelYHi(0),
    ShakePixelYLo(0),
    ShakePixelXHi(0),
    ShakePixelXLo(0),
    UnloadingClass(nullptr),
    SoylentValue(0),
    EnterTransportSound(VOC_NONE),
    LeaveTransportSound(VOC_NONE),
    VoiceCapture(),
    VoiceEnter(),
    VoiceDeploy(),
    VoiceHarvest(),
    SpecialPipIndex(-1),
    PipWrap(0),
    IdleRate(0),
    CameoImageSurface(nullptr),
    IsSortCameoAsBaseDefense(false),
    Description(""),
    IsFilterFromBandBoxSelection(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TechnoTypeClassExtension::TechnoTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::TechnoTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::~TechnoTypeClassExtension()
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::~TechnoTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    delete CameoImageSurface;
    CameoImageSurface = nullptr;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(UnloadingClass, "UnloadingClass");

    /**
     *  We need to reload the "Cameo" key because TechnoTypeClass does
     *  not store the entry value. 
     */
    const char *ini_name = IniName;
    const char *graphic_name = GraphicName;

    char cameo_buffer[32];
    
    ArtINI.Get_String(ini_name, "Cameo", "XXICON", cameo_buffer, sizeof(cameo_buffer));
    if (Wstring(cameo_buffer) != "XXICON") {

        ArtINI.Get_String(graphic_name, "Cameo", "XXICON", cameo_buffer, sizeof(cameo_buffer));

        /**
         *  Fetch the cameo image surface if it exists.
         */
        BSurface *imagesurface = Vinifera_Get_Image_Surface(cameo_buffer);
        if (imagesurface) {
            CameoImageSurface = imagesurface;
        }

    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Retrieves the size of the stream needed to save the object.
 * 
 *  @author: CCHyper, tomsons26
 */
LONG TechnoTypeClassExtension::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::GetSizeMax - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart += VoiceCapture.Count() * sizeof(uint32_t);
    pcbSize->LowPart += VoiceEnter.Count() * sizeof(uint32_t);
    pcbSize->LowPart += VoiceDeploy.Count() * sizeof(uint32_t);
    pcbSize->LowPart += VoiceHarvest.Count() * sizeof(uint32_t);

    return S_OK;
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TechnoTypeClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(IsShakeScreen);
    crc(IsImmuneToEMP);
    crc(ShakePixelYHi);
    crc(ShakePixelYLo);
    crc(ShakePixelXHi);
    crc(ShakePixelXLo);
    crc(SoylentValue);
    crc(IsLegalTargetComputer);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TechnoTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!ObjectTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();
    const char *graphic_name = Graphic_Name();

    //if (!ArtINI.Is_Present(graphic_name)) {
    //    return false;
    //}

    /**
     *  #issue-407
     * 
     *  Allow WalkRate to be optionally loaded from ART.INI image entries. This
     *  will also override any value set on the RULES.INI section.
     * 
     *  @author: CCHyper
     */
    This()->WalkRate = ArtINI.Get_Int(graphic_name, "WalkRate", This()->WalkRate);
    
    CloakSound = ini.Get_VocType(ini_name, "CloakSound", CloakSound);
    UncloakSound = ini.Get_VocType(ini_name, "UncloakSound", UncloakSound);
    IsShakeScreen = ini.Get_Bool(ini_name, "CanShakeScreen", IsShakeScreen);
    IsImmuneToEMP = ini.Get_Bool(ini_name, "ImmuneToEMP", IsImmuneToEMP);
    IsCanPassiveAcquire = ini.Get_Bool(ini_name, "CanPassiveAcquire", IsCanPassiveAcquire);
    IsCanRetaliate = ini.Get_Bool(ini_name, "CanRetaliate", IsCanRetaliate);
    IsLegalTargetComputer = ini.Get_Bool(ini_name, "AILegalTarget", IsLegalTargetComputer);
    ShakePixelYHi = ini.Get_Int(ini_name, "ShakeYhi", ShakePixelYHi);
    ShakePixelYLo = ini.Get_Int(ini_name, "ShakeYlo", ShakePixelYLo);
    ShakePixelXHi = ini.Get_Int(ini_name, "ShakeXhi", ShakePixelXHi);
    ShakePixelXLo = ini.Get_Int(ini_name, "ShakeXlo", ShakePixelXLo);
    UnloadingClass = ini.Get_Techno(ini_name, "UnloadingClass", UnloadingClass);
    SoylentValue = ini.Get_Int(ini_name, "Soylent", SoylentValue);
    EnterTransportSound = ini.Get_VocType(ini_name, "EnterTransportSound", EnterTransportSound);
    LeaveTransportSound = ini.Get_VocType(ini_name, "LeaveTransportSound", LeaveTransportSound);
    VoiceCapture = ini.Get_VocTypes(ini_name, "VoiceCapture", VoiceCapture);
    VoiceEnter = ini.Get_VocTypes(ini_name, "VoiceEnter", VoiceEnter);
    VoiceDeploy = ini.Get_VocTypes(ini_name, "VoiceDeploy", VoiceDeploy);
    VoiceHarvest = ini.Get_VocTypes(ini_name, "VoiceHarvest", VoiceHarvest);
    SpecialPipIndex = ini.Get_Int(ini_name, "SpecialPipIndex", SpecialPipIndex);
    PipWrap = ini.Get_Int(ini_name, "PipWrap", PipWrap);

    if (ini.Is_Present(ini_name, "Description"))
        ini.Get_String(ini_name, "Description", Description, std::size(Description));

    IdleRate = ini.Get_Int(ini_name, "IdleRate", IdleRate);
    IdleRate = ArtINI.Get_Int(graphic_name, "IdleRate", IdleRate);

    /**
     *  Fetch the cameo image surface if it exists.
     */
    BSurface *imagesurface = Vinifera_Get_Image_Surface(This()->CameoFilename);
    if (imagesurface) {
        CameoImageSurface = imagesurface;
    }

    IsSortCameoAsBaseDefense = ini.Get_Bool(ini_name, "SortCameoAsBaseDefense", IsSortCameoAsBaseDefense);
    IsFilterFromBandBoxSelection = ini.Get_Bool(ini_name, "FilterFromBandBoxSelection", IsFilterFromBandBoxSelection);

    return true;
}
