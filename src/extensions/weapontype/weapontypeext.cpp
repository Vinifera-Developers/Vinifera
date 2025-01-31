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
#include "ebolt.h"
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
WeaponTypeClassExtension::WeaponTypeClassExtension(const WeaponTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    IsSuicide(false),
    IsDeleteOnSuicide(false),
    IsElectricBolt(false),
    ElectricBoltColor1(EBOLT_DEFAULT_COLOR_1),
    ElectricBoltColor2(EBOLT_DEFAULT_COLOR_2),
    ElectricBoltColor3(EBOLT_DEFAULT_COLOR_3),
    ElectricBoltSegmentCount(EBOLT_DEFAULT_LINE_SEGEMENTS),
    ElectricBoltLifetime(EBOLT_DEFAULT_LIFETIME),
    ElectricBoltIterationCount(EBOLT_DEFAULT_INTERATIONS),
    ElectricBoltDeviation(EBOLT_DEFAULT_DEVIATION),
    IsSpawner(false),
    IsRevealOnFire(false),
    CursorAttack(ACTION_ATTACK),
    CursorStayAttack(ACTION_ATTACK)
{
    //if (this_ptr) EXT_DEBUG_TRACE("WeaponTypeClassExtension::WeaponTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    WeaponTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::WeaponTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::WeaponTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::~WeaponTypeClassExtension()
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::~WeaponTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    WeaponTypeExtensions.Delete(this);
}

/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT WeaponTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT WeaponTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
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
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int WeaponTypeClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void WeaponTypeClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WeaponTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(IsElectricBolt);
    crc(IsSpawner);
    crc(IsRevealOnFire);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool WeaponTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();
    
    IsSuicide = ini.Get_Bool(ini_name, "Suicide", IsSuicide);
    IsDeleteOnSuicide = ini.Get_Bool(ini_name, "DeleteOnSuicide", IsDeleteOnSuicide);

    IsElectricBolt = ini.Get_Bool(ini_name, "IsElectricBolt", IsElectricBolt);
    ElectricBoltColor1 = ini.Get_RGB(ini_name, "EBoltColor1", ElectricBoltColor1);
    ElectricBoltColor2 = ini.Get_RGB(ini_name, "EBoltColor2", ElectricBoltColor2);
    ElectricBoltColor3 = ini.Get_RGB(ini_name, "EBoltColor3", ElectricBoltColor3);
    ElectricBoltSegmentCount = ini.Get_Int(ini_name, "EBoltSegmentCount", ElectricBoltSegmentCount);
    ElectricBoltLifetime = ini.Get_Int(ini_name, "EBoltLifetime", ElectricBoltLifetime);
    ElectricBoltIterationCount = ini.Get_Int(ini_name, "EBoltIterations", ElectricBoltIterationCount);
    ElectricBoltDeviation = ini.Get_Float(ini_name, "EBoltDeviation", ElectricBoltDeviation);
    IsSpawner = ini.Get_Bool(ini_name, "Spawner", IsSpawner);
    IsRevealOnFire = ini.Get_Bool(ini_name, "RevealOnFire", IsRevealOnFire);
    CursorAttack = ini.Get_ActionType(ini_name, "CursorAttack", CursorAttack);
    CursorStayAttack = ini.Get_ActionType(ini_name, "CursorStayAttack", CursorStayAttack);
    //ElectricBoltSourceBoltParticleSys = ini.Get_ParticleSys(ini_name, "EBoltSourceParticleSys", ElectricBoltSourceBoltParticleSys);
    //ElectricBoltTargetBoltParticleSys = ini.Get_ParticleSys(ini_name, "EBoltTargetBoltParticleSys", ElectricBoltTargetBoltParticleSys);

    IsInitialized = true;

    return true;
}
