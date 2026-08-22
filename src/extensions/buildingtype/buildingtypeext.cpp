/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended BuildingTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "buildingtypeext.h"

#include "buildingtype.h"
#include "ccini.h"
#include "extension.h"
#include "scenario.h"
#include "tibsun_defines.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
BuildingTypeClassExtension::BuildingTypeClassExtension(const BuildingTypeClass *this_ptr) :
    TechnoTypeClassExtension(this_ptr),
    GateUpSound(VOC_NONE),
    GateDownSound(VOC_NONE),
    ProduceCashStartup(0),
    ProduceCashAmount(0),
    ProduceCashDelay(0),
    ProduceCashBudget(0),
    IsStartupCashOneTime(false),
    IsResetBudgetOnCapture(false),
    IsEligibleForAllyBuilding(false),
    EngineerChance(0),
    IsHideDuringSpecialAnim(false),
    RoofDeployingAnim(nullptr),
    RoofDoorAnim(nullptr),
    UnderRoofDoorAnim(nullptr),
    IsExclusiveFactory(false),
    IsWallOwner(true),
    IsBarGate(false)
{
    BuildingTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
BuildingTypeClassExtension::BuildingTypeClassExtension(const NoInitClass &noinit) :
    TechnoTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
BuildingTypeClassExtension::~BuildingTypeClassExtension()
{
    BuildingTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT BuildingTypeClassExtension::GetClassID(CLSID *lpClassID)
{
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
HRESULT BuildingTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = TechnoTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) BuildingTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT BuildingTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
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
int BuildingTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void BuildingTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(IsEligibleForAllyBuilding);
    crc(IsExclusiveFactory);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool BuildingTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!IsInitialized) {
        IsEligibleForAllyBuilding = IsEligibleForAllyBuilding || This()->IsConstructionYard;
        EngineerChance = This()->ToBuild == RTTI_BUILDINGTYPE ? 25 : 0;
    }

    if (!TechnoTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    GateUpSound = ini.Get_VocType(ini_name, "GateUpSound", GateUpSound);
    GateDownSound = ini.Get_VocType(ini_name, "GateDownSound", GateDownSound);

    ProduceCashStartup = ini.Get_Int(ini_name, "ProduceCashStartup", ProduceCashStartup);
    ProduceCashAmount = ini.Get_Int(ini_name, "ProduceCashAmount", ProduceCashAmount);
    ProduceCashDelay = ini.Get_Int(ini_name, "ProduceCashDelay", ProduceCashDelay);
    ProduceCashBudget = ini.Get_Int(ini_name, "ProduceCashBudget", ProduceCashBudget);
    IsStartupCashOneTime = ini.Get_Int(ini_name, "ProduceCashStartupOneTime", IsStartupCashOneTime);
    IsResetBudgetOnCapture = ini.Get_Bool(ini_name, "ProduceCashResetOnCapture", IsResetBudgetOnCapture);

    IsEligibleForAllyBuilding = ini.Get_Bool(ini_name, "EligibleForAllyBuilding", IsEligibleForAllyBuilding);
    IsHideDuringSpecialAnim = ArtINI.Get_Bool(ini_name, "HideDuringSpecialAnim", IsHideDuringSpecialAnim);

    IsExclusiveFactory = ini.Get_Bool(ini_name, "ExclusiveFactory", IsExclusiveFactory);
    IsWallOwner = ini.Get_Bool(ini_name, "WallOwner", IsWallOwner);
    IsBarGate = ini.Get_Bool(ini_name, "BarGate", IsBarGate);

    Fetch_Building_Normal_Image(Scen->Theater);

    /**
     *  ObjectTypeClass::Read_INI attempts to preload the image from a MIX file.
     *  If you mark an object type DemandLoad=yes, and then place its image in a cached MIX,
     *  the game will incorrectly attempt to delete that image later. To avoid this,
     *  null the MIX-fetched image out now.
     */
    if (This()->IsDemandLoad) {
        This()->Image = nullptr;
    }

    /**
     *  Don't try to free buildups if we don't demand load them.
     */
    if (!This()->IsDemandLoadBuildup) {
        This()->IsFreeBuildup = false;
    }

    IsInitialized = true;

    return true;
}


/**
 *  Fetches the extra building graphics.
 *
 *  @author: ZivDero
 */
void BuildingTypeClassExtension::Fetch_Building_Normal_Image(TheaterType theater)
{
    char fullname[MAX_PATH];
    char buffer[64];

    ArtINI.Get_String(This()->GraphicName.c_str(), "RoofDeployingAnim", "", buffer, sizeof(buffer));
    if (strlen(buffer) != 0) {
        _makepath(fullname, nullptr, nullptr, buffer, ".SHP");
        This()->Theater_Naming_Convention(fullname, theater);
        RoofDeployingAnim = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
    }

    ArtINI.Get_String(This()->GraphicName.c_str(), "RoofDoorAnim", "", buffer, sizeof(buffer));
    if (strlen(buffer) != 0) {
        _makepath(fullname, nullptr, nullptr, buffer, ".SHP");
        This()->Theater_Naming_Convention(fullname, theater);
        RoofDoorAnim = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
    }

    ArtINI.Get_String(This()->GraphicName.c_str(), "UnderRoofDoorAnim", "", buffer, sizeof(buffer));
    if (strlen(buffer) != 0) {
        _makepath(fullname, nullptr, nullptr, buffer, ".SHP");
        This()->Theater_Naming_Convention(fullname, theater);
        UnderRoofDoorAnim = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
    }
}
