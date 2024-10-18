/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDETYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SideClass class.
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
#include "sideext.h"
#include "side.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "colorscheme.h"
#include "rules.h"
#include "debughandler.h"
#include "tibsun_globals.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
SideClassExtension::SideClassExtension(const SideClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    UIColor(COLORSCHEME_NONE),
    ToolTipColor(COLORSCHEME_NONE)
{
    //if (this_ptr) EXT_DEBUG_TRACE("SideClassExtension::SideClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    SideExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SideClassExtension::SideClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("SideClassExtension::SideClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SideClassExtension::~SideClassExtension()
{
    //EXT_DEBUG_TRACE("SideClassExtension::~SideClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    SideExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT SideClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("SideClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT SideClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("SideClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SideClassExtension(NoInitClass());

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Crew, "Crew");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Engineer, "Engineer");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Technician, "Technician");
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SideClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("SideClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int SideClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("SideClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void SideClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("SideClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SideClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("SideClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool SideClassExtension::Read_INI(CCINIClass &ini)
{
    DEV_DEBUG_WARNING("SideClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    if (!IsInitialized) {

        UIColor = ColorScheme::From_Name("LightGold");
        ToolTipColor = ColorScheme::From_Name("Green");
        Crew = Rule->Crew;
        Engineer = Rule->Engineer;
        Technician = Rule->Technician;
        SurvivorDivisor = Rule->SurvivorDivisor;
    }

    UIColor = ini.Get_ColorSchemeType(ini_name, "UIColor", UIColor);
    ToolTipColor = ini.Get_ColorSchemeType(ini_name, "ToolTipColor", ToolTipColor);
    Crew = ini.Get_Infantry(ini_name, "Crew", Crew);
    Engineer = ini.Get_Infantry(ini_name, "Engineer", Engineer);
    Technician = ini.Get_Infantry(ini_name, "Technician", Technician);
    SurvivorDivisor = ini.Get_Int(ini_name, "SurvivorDivisor", SurvivorDivisor);

    IsInitialized = true;

    return true;
}

const InfantryTypeClass* SideClassExtension::Get_Crew(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->Crew;

    return Extension::Fetch<SideClassExtension>(Sides[side])->Crew;
}


const InfantryTypeClass* SideClassExtension::Get_Engineer(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->Engineer;

    return Extension::Fetch<SideClassExtension>(Sides[side])->Engineer;
}


const InfantryTypeClass* SideClassExtension::Get_Technician(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->Technician;

    return Extension::Fetch<SideClassExtension>(Sides[side])->Technician;
}


int SideClassExtension::Get_Survivor_Divisor(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->SurvivorDivisor;

    return Extension::Fetch<SideClassExtension>(Sides[side])->SurvivorDivisor;
}
