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
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all TechnoTypeClass extension instances.
 */
ExtensionMap<TechnoTypeClass, TechnoTypeClassExtension> TechnoTypeClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(TechnoTypeClass *this_ptr) :
    Extension(this_ptr),
    CloakSound(VOC_NONE),
    UncloakSound(VOC_NONE),
    IsShakeScreen(false),
    IsImmuneToEMP(false),
    IsCanPassiveAcquire(true),
    IsCanRetaliate(true),
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
    IdleRate(0),
    CameoImageSurface(nullptr),
    Weapons()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("TechnoTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;

    for (int i = 0; i < ARRAY_SIZE(Weapons); ++i) {
        Weapons[i].Weapon = nullptr;
    }
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::~TechnoTypeClassExtension()
{
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("TechnoTypeClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    delete CameoImageSurface;
    CameoImageSurface = nullptr;

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoTypeClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TechnoTypeClassExtension(NoInitClass());

    SWIZZLE_REQUEST_POINTER_REMAP(UnloadingClass);

    /**
     *  We need to reload the "Cameo" key because TechnoTypeClass does
     *  not store the entry value. 
     */
    const char *ini_name = ThisPtr->Name();
    const char *graphic_name = ThisPtr->Graphic_Name();

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
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int TechnoTypeClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TechnoTypeClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    crc(IsShakeScreen);
    crc(IsImmuneToEMP);
    crc(ShakePixelYHi);
    crc(ShakePixelYLo);
    crc(ShakePixelXHi);
    crc(ShakePixelXLo);
    crc(SoylentValue);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TechnoTypeClassExtension::Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TechnoTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("TechnoTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    const char *ini_name = ThisPtr->Name();
    const char *graphic_name = ThisPtr->Graphic_Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

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
    ThisPtr->WalkRate = ArtINI.Get_Int(graphic_name, "WalkRate", ThisPtr->WalkRate);

    /**
     *  As we 
     */
    std::memcpy(&Weapons[WEAPON_SLOT_PRIMARY], &ThisPtr->Weapons[WEAPON_SLOT_PRIMARY], sizeof(WeaponInfoStruct));
    std::memcpy(&Weapons[WEAPON_SLOT_SECONDARY], &ThisPtr->Weapons[WEAPON_SLOT_SECONDARY], sizeof(WeaponInfoStruct));
    std::memcpy(&Weapons[WEAPON_SLOT_ELITE_PRIMARY], &ThisPtr->Weapons[WEAPON_SLOT_ELITE], sizeof(WeaponInfoStruct));
    std::memcpy(&Weapons[WEAPON_SLOT_ELITE_SECONDARY], &ThisPtr->Weapons[WEAPON_SLOT_SECONDARY], sizeof(WeaponInfoStruct));

    /**
     *  This is an alias for the "Elite" key, to fit the new naming format.
     */
    Weapons[WEAPON_SLOT_ELITE_PRIMARY].Weapon = ini.Get_Weapon(ini_name, "ElitePrimary", Weapons[WEAPON_SLOT_ELITE_PRIMARY].Weapon);
    Weapons[WEAPON_SLOT_ELITE_PRIMARY].FireFLH = ArtINI.Get_Point(ini_name, "ElitePrimaryFireFLH", Weapons[WEAPON_SLOT_ELITE_PRIMARY].FireFLH);
    Weapons[WEAPON_SLOT_ELITE_PRIMARY].BarrelThickness = ArtINI.Get_Int(ini_name, "ElitePBarrelThickness", Weapons[WEAPON_SLOT_ELITE_PRIMARY].BarrelThickness);
    Weapons[WEAPON_SLOT_ELITE_PRIMARY].BarrelLength = ArtINI.Get_Int(ini_name, "ElitePBarrelLength", Weapons[WEAPON_SLOT_ELITE_PRIMARY].BarrelLength);

    Weapons[WEAPON_SLOT_ELITE_SECONDARY].Weapon = ini.Get_Weapon(ini_name, "EliteSecondary", Weapons[WEAPON_SLOT_ELITE_SECONDARY].Weapon);
    Weapons[WEAPON_SLOT_ELITE_SECONDARY].FireFLH = ArtINI.Get_Point(ini_name, "EliteSecondaryFireFLH", Weapons[WEAPON_SLOT_ELITE_SECONDARY].FireFLH);
    Weapons[WEAPON_SLOT_ELITE_SECONDARY].BarrelThickness = ArtINI.Get_Int(ini_name, "EliteSBarrelThickness", Weapons[WEAPON_SLOT_ELITE_SECONDARY].BarrelThickness);
    Weapons[WEAPON_SLOT_ELITE_SECONDARY].BarrelLength = ArtINI.Get_Int(ini_name, "EliteSBarrelLength", Weapons[WEAPON_SLOT_ELITE_SECONDARY].BarrelLength);

    std::memcpy(&Weapons[WEAPON_SLOT_VETERAN_PRIMARY], &ThisPtr->Weapons[WEAPON_SLOT_PRIMARY], sizeof(WeaponInfoStruct));

    Weapons[WEAPON_SLOT_VETERAN_PRIMARY].Weapon = ini.Get_Weapon(ini_name, "VeteranPrimary", Weapons[WEAPON_SLOT_VETERAN_PRIMARY].Weapon);
    Weapons[WEAPON_SLOT_VETERAN_PRIMARY].FireFLH = ArtINI.Get_Point(ini_name, "VeteranPrimaryFireFLH", Weapons[WEAPON_SLOT_VETERAN_PRIMARY].FireFLH);
    Weapons[WEAPON_SLOT_VETERAN_PRIMARY].BarrelThickness = ArtINI.Get_Int(ini_name, "VeteranPBarrelThickness", Weapons[WEAPON_SLOT_VETERAN_PRIMARY].BarrelThickness);
    Weapons[WEAPON_SLOT_VETERAN_PRIMARY].BarrelLength = ArtINI.Get_Int(ini_name, "VeteranPBarrelLength", Weapons[WEAPON_SLOT_VETERAN_PRIMARY].BarrelLength);

    std::memcpy(&Weapons[WEAPON_SLOT_VETERAN_SECONDARY], &ThisPtr->Weapons[WEAPON_SLOT_SECONDARY], sizeof(WeaponInfoStruct));

    Weapons[WEAPON_SLOT_VETERAN_SECONDARY].Weapon = ini.Get_Weapon(ini_name, "VeteranSecondary", Weapons[WEAPON_SLOT_VETERAN_SECONDARY].Weapon);
    Weapons[WEAPON_SLOT_VETERAN_SECONDARY].FireFLH = ArtINI.Get_Point(ini_name, "VeteranSecondaryFireFLH", Weapons[WEAPON_SLOT_VETERAN_SECONDARY].FireFLH);
    Weapons[WEAPON_SLOT_VETERAN_SECONDARY].BarrelThickness = ArtINI.Get_Int(ini_name, "VeteranSBarrelThickness", Weapons[WEAPON_SLOT_VETERAN_SECONDARY].BarrelThickness);
    Weapons[WEAPON_SLOT_VETERAN_SECONDARY].BarrelLength = ArtINI.Get_Int(ini_name, "VeteranSBarrelLength", Weapons[WEAPON_SLOT_VETERAN_SECONDARY].BarrelLength);

    Weapons[WEAPON_SLOT_TERTIARY].Weapon = ini.Get_Weapon(ini_name, "Tertiary", Weapons[WEAPON_SLOT_TERTIARY].Weapon);
    Weapons[WEAPON_SLOT_TERTIARY].FireFLH = ArtINI.Get_Point(ini_name, "TertiaryFireFLH", Weapons[WEAPON_SLOT_TERTIARY].FireFLH);
    Weapons[WEAPON_SLOT_TERTIARY].BarrelThickness = ArtINI.Get_Int(ini_name, "TBarrelThickness", Weapons[WEAPON_SLOT_TERTIARY].BarrelThickness);
    Weapons[WEAPON_SLOT_TERTIARY].BarrelLength = ArtINI.Get_Int(ini_name, "TBarrelLength", Weapons[WEAPON_SLOT_TERTIARY].BarrelLength);

    std::memcpy(&Weapons[WEAPON_SLOT_VETERAN_TERTIARY], &Weapons[WEAPON_SLOT_TERTIARY], sizeof(WeaponInfoStruct));
    std::memcpy(&Weapons[WEAPON_SLOT_ELITE_TERTIARY], &Weapons[WEAPON_SLOT_TERTIARY], sizeof(WeaponInfoStruct));

    Weapons[WEAPON_SLOT_VETERAN_TERTIARY].Weapon = ini.Get_Weapon(ini_name, "VeteranTertiary", Weapons[WEAPON_SLOT_VETERAN_TERTIARY].Weapon);
    Weapons[WEAPON_SLOT_VETERAN_TERTIARY].FireFLH = ArtINI.Get_Point(ini_name, "VeteranTertiaryFireFLH", Weapons[WEAPON_SLOT_VETERAN_TERTIARY].FireFLH);
    Weapons[WEAPON_SLOT_VETERAN_TERTIARY].BarrelThickness = ArtINI.Get_Int(ini_name, "VeteranTBarrelThickness", Weapons[WEAPON_SLOT_VETERAN_TERTIARY].BarrelThickness);
    Weapons[WEAPON_SLOT_VETERAN_TERTIARY].BarrelLength = ArtINI.Get_Int(ini_name, "VeteranTBarrelLength", Weapons[WEAPON_SLOT_VETERAN_TERTIARY].BarrelLength);
    Weapons[WEAPON_SLOT_ELITE_TERTIARY].Weapon = ini.Get_Weapon(ini_name, "EliteTertiary", Weapons[WEAPON_SLOT_ELITE_TERTIARY].Weapon);
    Weapons[WEAPON_SLOT_ELITE_TERTIARY].FireFLH = ArtINI.Get_Point(ini_name, "EliteTertiaryFireFLH", Weapons[WEAPON_SLOT_ELITE_TERTIARY].FireFLH);
    Weapons[WEAPON_SLOT_ELITE_TERTIARY].BarrelThickness = ArtINI.Get_Int(ini_name, "EliteTBarrelThickness", Weapons[WEAPON_SLOT_ELITE_TERTIARY].BarrelThickness);
    Weapons[WEAPON_SLOT_ELITE_TERTIARY].BarrelLength = ArtINI.Get_Int(ini_name, "EliteTBarrelLength", Weapons[WEAPON_SLOT_ELITE_TERTIARY].BarrelLength);

    Weapons[WEAPON_SLOT_QUATERNARY].Weapon = ini.Get_Weapon(ini_name, "Quaternary", Weapons[WEAPON_SLOT_QUATERNARY].Weapon);
    Weapons[WEAPON_SLOT_QUATERNARY].FireFLH = ArtINI.Get_Point(ini_name, "QuaternaryFireFLH", Weapons[WEAPON_SLOT_QUATERNARY].FireFLH);
    Weapons[WEAPON_SLOT_QUATERNARY].BarrelThickness = ArtINI.Get_Int(ini_name, "QBarrelThickness", Weapons[WEAPON_SLOT_QUATERNARY].BarrelThickness);
    Weapons[WEAPON_SLOT_QUATERNARY].BarrelLength = ArtINI.Get_Int(ini_name, "QBarrelLength", Weapons[WEAPON_SLOT_QUATERNARY].BarrelLength);

    std::memcpy(&Weapons[WEAPON_SLOT_VETERAN_QUATERNARY], &Weapons[WEAPON_SLOT_QUATERNARY], sizeof(WeaponInfoStruct));
    std::memcpy(&Weapons[WEAPON_SLOT_ELITE_QUATERNARY], &Weapons[WEAPON_SLOT_QUATERNARY], sizeof(WeaponInfoStruct));

    Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].Weapon = ini.Get_Weapon(ini_name, "VeteranQuaternary", Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].Weapon);
    Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].FireFLH = ArtINI.Get_Point(ini_name, "VeteranQuaternaryFireFLH", Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].FireFLH);
    Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].BarrelThickness = ArtINI.Get_Int(ini_name, "VeteranQBarrelThickness", Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].BarrelThickness);
    Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].BarrelLength = ArtINI.Get_Int(ini_name, "VeteranQBarrelLength", Weapons[WEAPON_SLOT_VETERAN_QUATERNARY].BarrelLength);
    Weapons[WEAPON_SLOT_ELITE_QUATERNARY].Weapon = ini.Get_Weapon(ini_name, "EliteQuaternary", Weapons[WEAPON_SLOT_ELITE_QUATERNARY].Weapon);
    Weapons[WEAPON_SLOT_ELITE_QUATERNARY].FireFLH = ArtINI.Get_Point(ini_name, "EliteQuaternaryFireFLH", Weapons[WEAPON_SLOT_ELITE_QUATERNARY].FireFLH);
    Weapons[WEAPON_SLOT_ELITE_QUATERNARY].BarrelThickness = ArtINI.Get_Int(ini_name, "EliteQBarrelThickness", Weapons[WEAPON_SLOT_ELITE_QUATERNARY].BarrelThickness);
    Weapons[WEAPON_SLOT_ELITE_QUATERNARY].BarrelLength = ArtINI.Get_Int(ini_name, "EliteQBarrelLength", Weapons[WEAPON_SLOT_ELITE_QUATERNARY].BarrelLength);

    CloakSound = ini.Get_VocType(ini_name, "CloakSound", CloakSound);
    UncloakSound = ini.Get_VocType(ini_name, "UncloakSound", UncloakSound);
    IsShakeScreen = ini.Get_Bool(ini_name, "CanShakeScreen", IsShakeScreen);
    IsImmuneToEMP = ini.Get_Bool(ini_name, "ImmuneToEMP", IsImmuneToEMP);
    IsCanPassiveAcquire = ini.Get_Bool(ini_name, "CanPassiveAcquire", IsCanPassiveAcquire);
    IsCanRetaliate = ini.Get_Bool(ini_name, "CanRetaliate", IsCanRetaliate);
    ShakePixelYHi = ini.Get_Int(ini_name, "ShakeYhi", ShakePixelYHi);
    ShakePixelYLo = ini.Get_Int(ini_name, "ShakeYlo", ShakePixelYLo);
    ShakePixelXHi = ini.Get_Int(ini_name, "ShakeXhi", ShakePixelXHi);
    ShakePixelXLo = ini.Get_Int(ini_name, "ShakeXlo", ShakePixelXLo);
    UnloadingClass = ini.Get_Techno(ini_name, "UnloadingClass", UnloadingClass);
    SoylentValue = ini.Get_Int(ini_name, "Soylent", SoylentValue);
    EnterTransportSound = ini.Get_VocType(ini_name, "EnterTransportSound", EnterTransportSound);
    LeaveTransportSound = ini.Get_VocType(ini_name, "LeaveTransportSound", LeaveTransportSound);
    VoiceCapture = ini.Get_VocType_List(ini_name, "VoiceCapture", VoiceCapture);
    VoiceEnter = ini.Get_VocType_List(ini_name, "VoiceEnter", VoiceEnter);
    VoiceDeploy = ini.Get_VocType_List(ini_name, "VoiceDeploy", VoiceDeploy);
    VoiceHarvest = ini.Get_VocType_List(ini_name, "VoiceHarvest", VoiceHarvest);

    IdleRate = ini.Get_Int(ini_name, "IdleRate", IdleRate);
    IdleRate = ArtINI.Get_Int(graphic_name, "IdleRate", IdleRate);

    /**
     *  Fetch the cameo image surface if it exists.
     */
    BSurface *imagesurface = Vinifera_Get_Image_Surface(ThisPtr->CameoFilename);
    if (imagesurface) {
        CameoImageSurface = imagesurface;
    }

    return true;
}


/**
 *  
 *  
 *  @author: CCHyper
 */
const WeaponInfoStruct &TechnoTypeClassExtension::Fetch_Weapon_Info(WeaponSlotType slot) const
{
    if (slot == WEAPON_SLOT_SECONDARY && !Weapons[slot].Weapon) {
        slot = WEAPON_SLOT_PRIMARY;
    }
    if (slot == WEAPON_SLOT_ELITE_SECONDARY && !Weapons[slot].Weapon) {
        slot = WeaponSlotType(WEAPON_SLOT_ELITE_PRIMARY);
    }
    if (slot == WEAPON_SLOT_VETERAN_SECONDARY && !Weapons[slot].Weapon) {
        slot = WeaponSlotType(WEAPON_SLOT_VETERAN_PRIMARY);
    }

#ifndef NDEBUG
    if (Weapons[slot].Weapon == nullptr) {
        //DEBUG_WARNING("Fetching info for weapon \"%s\", but WeaponType pointer is null!\n", Name_From_WeaponSlot(slot));
    }
#endif

    return Weapons[slot];
}
