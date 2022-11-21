/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CAMPAIGNEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended CampaignClass class.
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
#include "campaignext.h"
#include "campaign.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
CampaignClassExtension::CampaignClassExtension(const CampaignClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    IsDebugOnly(false),
    IntroMovie()
{
    //if (this_ptr) EXT_DEBUG_TRACE("CampaignClassExtension::CampaignClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("CampaignClassExtension::CampaignClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
CampaignClassExtension::~CampaignClassExtension()
{
    //EXT_DEBUG_TRACE("CampaignClassExtension::~CampaignClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    CampaignExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT CampaignClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("CampaignClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("CampaignClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("CampaignClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int CampaignClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("CampaignClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void CampaignClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("CampaignClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void CampaignClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("CampaignClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool CampaignClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("CampaignClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    
    ini.Get_String(ini_name, "IntroMovie", IntroMovie, sizeof(IntroMovie));

    return true;
}
