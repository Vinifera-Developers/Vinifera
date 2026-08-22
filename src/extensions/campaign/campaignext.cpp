/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended CampaignClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "campaignext.h"

#include "campaign.h"
#include "ccini.h"
#include "extension.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
CampaignClassExtension::CampaignClassExtension(const CampaignClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    IsDebugOnly(false),
    IntroMovie(),
    _House(HOUSE_NONE)
{
    CampaignExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
CampaignClassExtension::CampaignClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
CampaignClassExtension::~CampaignClassExtension()
{
    CampaignExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT CampaignClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT CampaignClassExtension::Load(IStream *pStm)
{
    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) CampaignClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT CampaignClassExtension::Save(IStream *pStm, BOOL fClearDirty)
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
int CampaignClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void CampaignClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool CampaignClassExtension::Read_INI(CCINIClass &ini)
{
    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    IsDebugOnly = ini.Get_Bool(ini_name, "DebugOnly", IsDebugOnly);

    /**
     *  Reload the campaign description so we can prepend the debug string.
     */
    if (IsDebugOnly) {
        char buffer[128];
        std::strncpy(buffer, This()->Description, sizeof(buffer));
        std::snprintf(This()->Description, sizeof(This()->Description), "[Debug] - %s", buffer);
    }
    
    ini.Get_String(ini_name, "IntroMovie", "", IntroMovie, sizeof(IntroMovie));

    _House = static_cast<HousesType>(ini.Get_Int(ini_name, "Side", _House));

    IsInitialized = true;

    return true;
}


/**
 *  Fetches this campaign's house.
 *
 *  @author: ZivDero
 */
HousesType CampaignClassExtension::Get_House() const
{
    if (_House == HOUSE_NONE) {
        if (std::strstr(This()->Scenario, "GDI")) {
            return HOUSE_GDI;
        } else if (std::strstr(This()->Scenario, "NOD")) {
            return HOUSE_NOD;
        }
        return HOUSE_GDI;
    }

    return _House;
}


/**
 *  Sets this campaign's house.
 *
 *  @author: ZivDero
 */
void CampaignClassExtension::Set_House(HousesType house)
{
    _House = house;
}
