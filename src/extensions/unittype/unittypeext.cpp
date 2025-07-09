/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended UnitTypeClass class.
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
#include "unittypeext.h"
#include "unittype.h"
#include "ccini.h"
#include "tibsun_globals.h"
#include "extension.h"
#include "vinifera_saveload.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
UnitTypeClassExtension::UnitTypeClassExtension(const UnitTypeClass *this_ptr) :
    TechnoTypeClassExtension(this_ptr),
    IsTotable(true),
    StartTurretFrame(-1),
    TurretFacings(32),		// Must default to 32 as all Tiberian Sun units have 32 facings for turrets.,
    StartIdleFrame(0),
    IdleFrames(0),
    TransformsInto(nullptr),
    IsTransformRequiresFullCharge(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("UnitTypeClassExtension::UnitTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    UnitTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
UnitTypeClassExtension::UnitTypeClassExtension(const NoInitClass &noinit) :
    TechnoTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::UnitTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
UnitTypeClassExtension::~UnitTypeClassExtension()
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::~UnitTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    UnitTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT UnitTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT UnitTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) UnitTypeClassExtension(NoInitClass());
    
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(TransformsInto, "TransformsInto");

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT UnitTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoTypeClassExtension::Save(pStm, fClearDirty);
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
int UnitTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void UnitTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoTypeClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void UnitTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool UnitTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!TechnoTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();
    const char *graphic_name = This()->Graphic_Name();
    
    //if (!ArtINI.Is_Present(graphic_name)) {
    //    return false;
    //}

    IsTotable = ini.Get_Bool(ini_name, "Totable", IsTotable);
    TransformsInto = ini.Get_Unit(ini_name, "TransformsInto", TransformsInto);
    IsTransformRequiresFullCharge = ini.Get_Bool(ini_name, "TransformRequiresFullCharge", IsTransformRequiresFullCharge);

    StartTurretFrame = ArtINI.Get_Int(graphic_name, "StartTurretFrame", StartTurretFrame);
    TurretFacings = ArtINI.Get_Int(graphic_name, "TurretFacings", TurretFacings);

    /**
     *  Set the defaults to walk frames (this ensures IdleRate by itself works as expected).
     */
    StartIdleFrame = This()->StartWalkFrame;
    IdleFrames = This()->WalkFrames;

    StartIdleFrame = ArtINI.Get_Int(graphic_name, "StartIdleFrame", StartIdleFrame);
    IdleFrames = ArtINI.Get_Int(graphic_name, "IdleFrames", IdleFrames);

    IsInitialized = true;

    return true;
}
