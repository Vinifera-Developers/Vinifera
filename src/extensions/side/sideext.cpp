/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SideClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sideext.h"

#include "ccini.h"
#include "colorscheme.h"
#include "debughandler.h"
#include "extension.h"
#include "findmake.h"
#include "rules.h"
#include "side.h"
#include "tibsun_globals.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SideClassExtension::SideClassExtension(const SideClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    UIColor(COLORSCHEME_FIRST),
    HoverHighlightColor(COLORSCHEME_FIRST),
    ToolTipColor(COLORSCHEME_FIRST),
    Crew(nullptr),
    Engineer(nullptr),
    Technician(nullptr),
    Disguise(nullptr),
    SurvivorDivisor(100),
    RegularPowerPlant(nullptr),
    AdvancedPowerPlant(nullptr),
    PowerTurbine(nullptr),
    HunterSeeker(nullptr),
    OptionsMenuTextColor(OPTIONS_MENU_TEXT_DEFAULT_COLOR),
    ScreenTextColor(SCREEN_TEXT_DEFAULT_COLOR)
{
    SideExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SideClassExtension::SideClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit),
    OptionsMenuTextColor(noinit),
    ScreenTextColor(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SideClassExtension::~SideClassExtension()
{
    SideExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT SideClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT SideClassExtension::Load(IStream *pStm)
{
    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SideClassExtension(NoInitClass());

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Crew, "Crew");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Engineer, "Engineer");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Technician, "Technician");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Disguise, "Disguise");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(RegularPowerPlant, "RegularPowerPlant");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(AdvancedPowerPlant, "AdvancedPowerPlant");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(PowerTurbine, "PowerTurbine");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(HunterSeeker, "HunterSeeker");
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SideClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
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
int SideClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SideClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool SideClassExtension::Read_INI(CCINIClass &ini)
{
    //DEV_DEBUG_WARNING("SideClassExtension::Read_INI - Name: {} (0x{:08X})\n", Name(), (uintptr_t)(This()));

    const char* ini_name = Name();

    if (!IsInitialized) {

        UIColor = Fetch_Scheme_Index_By_Name("LightGold");
        HoverHighlightColor = Fetch_Scheme_Index_By_Name("LightGold");
        ToolTipColor = Fetch_Scheme_Index_By_Name("Green");

        Crew = Rule->Crew;
        Engineer = Rule->Engineer;
        Technician = Rule->Technician;
        Disguise = Rule->Disguise;
        SurvivorDivisor = Rule->SurvivorDivisor;

        if (std::strstr(ini_name, "GDI")) {
            RegularPowerPlant = Rule->GDIPowerPlant;
            AdvancedPowerPlant = nullptr;
            PowerTurbine = Rule->GDIPowerTurbine;
            HunterSeeker = Rule->GDIHunterSeeker;

        }
        else {
            RegularPowerPlant = Rule->NodRegularPower;
            AdvancedPowerPlant = Rule->NodAdvancedPower;
            PowerTurbine = nullptr;
            HunterSeeker = Rule->NodHunterSeeker;
        }
    }

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    UIColor = ini.Get_Scheme_Index(ini_name, "UIColor", UIColor);
    HoverHighlightColor = ini.Get_Scheme_Index(ini_name, "HoverHighlightColor", HoverHighlightColor);
    ToolTipColor = ini.Get_Scheme_Index(ini_name, "ToolTipColor", ToolTipColor);

    Crew = TGet_Class(ini, ini_name, "Crew", Crew);
    Engineer = TGet_Class(ini, ini_name, "Engineer", Engineer);
    Technician = TGet_Class(ini, ini_name, "Technician", Technician);
    Disguise = TGet_Class(ini, ini_name, "Disguise", Disguise);
    SurvivorDivisor = ini.Get_Int(ini_name, "SurvivorDivisor", SurvivorDivisor);

    RegularPowerPlant = TGet_Class(ini, ini_name, "RegularPowerPlant", RegularPowerPlant);
    AdvancedPowerPlant = TGet_Class(ini, ini_name, "AdvancedPowerPlant", AdvancedPowerPlant);
    PowerTurbine = TGet_Class(ini, ini_name, "PowerTurbine", PowerTurbine);

    HunterSeeker = TGet_Class(ini, ini_name, "HunterSeeker", HunterSeeker);

    OptionsMenuTextColor = ini.Get_RGBColor(ini_name, "OptionsMenuTextColor", OptionsMenuTextColor);
    ScreenTextColor = ini.Get_RGBColor(ini_name, "ScreenTextColor", ScreenTextColor);

    IsInitialized = true;

    return true;
}


/**
 *  Returns the crew type for the given side.
 *
 *  @author: ZivDero
 */
const InfantryTypeClass* SideClassExtension::Get_Crew(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->Crew;

    return Extension::Fetch(Sides[side])->Crew;
}


/**
 *  Returns the engineer type for the given side.
 *
 *  @author: ZivDero
 */
const InfantryTypeClass* SideClassExtension::Get_Engineer(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->Engineer;

    return Extension::Fetch(Sides[side])->Engineer;
}


/**
 *  Returns the technician type for the given side.
 *
 *  @author: ZivDero
 */
const InfantryTypeClass* SideClassExtension::Get_Technician(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->Technician;

    return Extension::Fetch(Sides[side])->Technician;
}


/**
 *  Returns the disguise type for the given side.
 *
 *  @author: ZivDero
 */
const InfantryTypeClass* SideClassExtension::Get_Disguise(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->Disguise;

    return Extension::Fetch(Sides[side])->Disguise;
}


/**
 *  Returns the survivor divisor for the given side.
 *
 *  @author: ZivDero
 */
int SideClassExtension::Get_Survivor_Divisor(SideType side)
{
    if (side == SIDE_NONE)
        return Rule->SurvivorDivisor;

    return Extension::Fetch(Sides[side])->SurvivorDivisor;
}
