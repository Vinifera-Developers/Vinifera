/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WARHEADTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended WarheadTypeClass class.
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
#include "warheadtypeext.h"
#include "warheadtype.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "armortype.h"
#include "rules.h"
#include "ccini.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
WarheadTypeClassExtension::WarheadTypeClassExtension(const WarheadTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    IsWallAbsoluteDestroyer(false),
    IsAffectsAllies(true),
    CombatLightSize(0.0f),
    ShakePixelYHi(0),
    ShakePixelYLo(0),
    ShakePixelXHi(0),
    ShakePixelXLo(0),
    Modifier()
{
    //if (this_ptr) EXT_DEBUG_TRACE("WarheadTypeClassExtension::WarheadTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    for (int armor = 0; armor < ArmorTypes.Count(); ++armor) {
        Modifier.Add(1.0f);
    }

    WarheadTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
WarheadTypeClassExtension::WarheadTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::WarheadTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
WarheadTypeClassExtension::~WarheadTypeClassExtension()
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::~WarheadTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    WarheadTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT WarheadTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT WarheadTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return hr;
    }

    new (this) WarheadTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT WarheadTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
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
int WarheadTypeClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void WarheadTypeClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WarheadTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(IsWallAbsoluteDestroyer);
    crc(IsAffectsAllies);
    crc(CombatLightSize);
    crc(ShakePixelYHi);
    crc(ShakePixelYLo);
    crc(ShakePixelXHi);
    crc(ShakePixelXLo);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool WarheadTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    char buffer[256];

    const char *ini_name = Name();

    IsWallAbsoluteDestroyer = ini.Get_Bool(ini_name, "WallAbsoluteDestroyer", IsWallAbsoluteDestroyer);
    IsAffectsAllies = ini.Get_Bool(ini_name, "AffectsAllies", IsAffectsAllies);
    CombatLightSize = ini.Get_Float_Clamp(ini_name, "CombatLightSize", 0.0f, 1.0f, CombatLightSize);
    ShakePixelYHi = ini.Get_Int(ini_name, "ShakeYhi", ShakePixelYHi);
    ShakePixelYLo = ini.Get_Int(ini_name, "ShakeYlo", ShakePixelYLo);
    ShakePixelXHi = ini.Get_Int(ini_name, "ShakeXhi", ShakePixelXHi);
    ShakePixelXLo = ini.Get_Int(ini_name, "ShakeXlo", ShakePixelXLo);

    /**
     *  Reload the Verses entry into the new Modifier array.
     */
    if (ini.Get_String(ini_name, "Verses", ArmorTypeClass::Get_Modifier_Default_String(), buffer, sizeof(buffer)) > 0) {
        char *aval = std::strtok(buffer, ",");
        for (int armor = 0; armor < ArmorTypes.Count(); ++armor) {

            if (std::strchr(aval, '%')) {
                Modifier[armor] = std::atoi(aval) * 0.01;
            } else {
                Modifier[armor] = std::atof(aval);
            }

            //Modifier[armor] = Percent_String_To_Float(aval);

            aval = std::strtok(nullptr, ",");
        }
    }

    return true;
}
