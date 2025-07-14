/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended BuildingTypeClass class.
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
#include "buildingtypeext.h"
#include "buildingtype.h"
#include "tibsun_defines.h"
#include "ccini.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "scenario.h"


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
    IsVerticalGate(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("BuildingTypeClassExtension::BuildingTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::BuildingTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
BuildingTypeClassExtension::~BuildingTypeClassExtension()
{
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::~BuildingTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    BuildingTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT BuildingTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void BuildingTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoTypeClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void BuildingTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("BuildingTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!IsInitialized) {
        IsEligibleForAllyBuilding = This()->IsConstructionYard;
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

    Fetch_Building_Normal_Image(Scen->Theater);

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

    ArtINI.Get_String(This()->GraphicName, "RoofDeployingAnim", "", buffer, sizeof(buffer));
    if (strlen(buffer) != 0) {
        _makepath(fullname, nullptr, nullptr, buffer, ".SHP");
        This()->Theater_Naming_Convention(fullname, theater);
        RoofDeployingAnim = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
    }

    ArtINI.Get_String(This()->GraphicName, "RoofDoorAnim", "", buffer, sizeof(buffer));
    if (strlen(buffer) != 0) {
        _makepath(fullname, nullptr, nullptr, buffer, ".SHP");
        This()->Theater_Naming_Convention(fullname, theater);
        RoofDoorAnim = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
    }

    ArtINI.Get_String(This()->GraphicName, "UnderRoofDoorAnim", "", buffer, sizeof(buffer));
    if (strlen(buffer) != 0) {
        _makepath(fullname, nullptr, nullptr, buffer, ".SHP");
        This()->Theater_Naming_Convention(fullname, theater);
        UnderRoofDoorAnim = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
    }
}
