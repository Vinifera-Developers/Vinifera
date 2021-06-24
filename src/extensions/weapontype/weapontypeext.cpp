/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WEAPONTYPEEXT.CPP
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
#include "weapontypeext.h"
#include "weapontype.h"
#include "ccini.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all WeaponTypeClass extension instances.
 */
ExtensionMap<WeaponTypeClass, WeaponTypeClassExtension> WeaponTypeClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::WeaponTypeClassExtension(WeaponTypeClass *this_ptr) :
    Extension(this_ptr),
    SonicBeamColor{0,0,0},
    SonicBeamAlpha(0.5),
    SonicBeamSineDuration(0.125),
    SonicBeamSineAmplitude(12.0),
    SonicBeamOffset(0.49),
    SonicBeamSurfacePattern(SURFACE_PATTERN_CIRCLE),
    SonicBeamStartPinLeft(-30.0, -100.0, 0.0),
    SonicBeamStartPinRight(-30.0, 100.0, 0.0),
    SonicBeamEndPinLeft(30.0, -100.0, 0.0),
    SonicBeamEndPinRight(30.0, 100.0, 0.0)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("WeaponTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::WeaponTypeClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::~WeaponTypeClassExtension()
{
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("WeaponTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT WeaponTypeClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) WeaponTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT WeaponTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

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
int WeaponTypeClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void WeaponTypeClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WeaponTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool WeaponTypeClassExtension::Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("WeaponTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    DEV_DEBUG_WARNING("WeaponTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    char buffer[32];

    const char *ini_name = ThisPtr->Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    /**
     *  Make sure the entry is found before attempting to overwrite the default.
     */
    if (ini.Is_Present(ini_name, "SonicBeamColor")) {
        SonicBeamColor = ini.Get_RGB(ini_name, "SonicBeamColor", SonicBeamColor);

        /**
         *  We use 0 as a indicator the color was unassigned, so bump
         *  up and solid black by 1.
         */
        SonicBeamColor.R = std::clamp<unsigned char>(SonicBeamColor.R, 1, 255);
        SonicBeamColor.G = std::clamp<unsigned char>(SonicBeamColor.G, 1, 255);
        SonicBeamColor.B = std::clamp<unsigned char>(SonicBeamColor.B, 1, 255);
    }

    SonicBeamIsClear = ini.Get_Bool(ini_name, "SonicBeamIsClear", SonicBeamIsClear);
    SonicBeamAlpha = ini.Get_Float_Clamp(ini_name, "SonicBeamAlpha", 0.0, 1.0, SonicBeamAlpha);
    SonicBeamSineDuration = ini.Get_Float(ini_name, "SonicBeamSineDuration", SonicBeamSineDuration);
    SonicBeamSineAmplitude = ini.Get_Float(ini_name, "SonicBeamSineAmplitude", SonicBeamSineAmplitude);
    SonicBeamOffset = ini.Get_Float(ini_name, "SonicBeamOffset", SonicBeamOffset);
    SonicBeamStartPinLeft = ini.Get_Vector3(ini_name, "SonicBeamStartPinLeft", SonicBeamStartPinLeft);
    SonicBeamStartPinRight = ini.Get_Vector3(ini_name, "SonicBeamStartPinRight", SonicBeamStartPinRight);
    SonicBeamEndPinLeft = ini.Get_Vector3(ini_name, "SonicBeamEndPinLeft", SonicBeamEndPinLeft);
    SonicBeamEndPinRight = ini.Get_Vector3(ini_name, "SonicBeamEndPinRight", SonicBeamEndPinRight);

    static const char *surface_pattern_names[SURFACE_PATTERN_COUNT] = {
        "circle",
        "ellipse",
        "rhombus",
        "square"
    };
    ini.Get_String(ini_name, "SonicBeamSurfacePattern", buffer, sizeof(buffer));
    for (int i = 0; i < SURFACE_PATTERN_COUNT; ++i) {
        if (strcmpi(surface_pattern_names[i], buffer) == 0) {
            SonicBeamSurfacePattern = SonicBeamSurfacePatternType(i);
        }
    }

    static const char *sine_pattern_names[SINE_PATTERN_COUNT] = {
        "circle",
        "square",
        "sawtooth",
        "triangle"
    };
    ini.Get_String(ini_name, "SonicBeamSinePattern", buffer, sizeof(buffer));
    for (int i = 0; i < SINE_PATTERN_COUNT; ++i) {
        if (strcmpi(sine_pattern_names[i], buffer) == 0) {
            SonicBeamSinePattern = SonicBeamSinePatternType(i);
        }
    }

    return true;
}
