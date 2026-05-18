/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended WeaponTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "weapontypeext.h"

#include "ccini.h"
#include "ebolt.h"
#include "extension.h"
#include "weapontype.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
WeaponTypeClassExtension::WeaponTypeClassExtension(const WeaponTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    IsSuicide(false),
    IsDeleteOnSuicide(false),
    IsOmniFire(false),
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
int WeaponTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WeaponTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(IsOmniFire);
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
    IsOmniFire = ini.Get_Bool(ini_name, "OmniFire", IsOmniFire);

    IsElectricBolt = ini.Get_Bool(ini_name, "IsElectricBolt", IsElectricBolt);
    ElectricBoltColor1 = ini.Get_RGBColor(ini_name, "EBoltColor1", ElectricBoltColor1);
    ElectricBoltColor2 = ini.Get_RGBColor(ini_name, "EBoltColor2", ElectricBoltColor2);
    ElectricBoltColor3 = ini.Get_RGBColor(ini_name, "EBoltColor3", ElectricBoltColor3);
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
