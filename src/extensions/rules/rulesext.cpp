/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RULESEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended RulesClass class.
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
#include "rulesext.h"
#include "ccini.h"
#include "rules.h"
#include "asserthandler.h"
#include "debughandler.h"


RulesClassExtension *RulesExtension = nullptr;
RulesClassExtension::UIControlsStruct RulesClassExtension::UIControls;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::RulesClassExtension(RulesClass *this_ptr) :
    Extension(this_ptr),
    IsMPAutoDeployMCV(false),
    IsMPPrePlacedConYards(false)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("RulesClassExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::RulesClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::~RulesClassExtension()
{
    //DEV_DEBUG_TRACE("RulesClassExtension deconstructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("RulesClassExtension deconstructor - 0x%08X\n", (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT RulesClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension::Load - 0x%08X\n", (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) RulesClassExtension(NoInitClass());

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT RulesClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension::Save - 0x%08X\n", (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
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
int RulesClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

    crc(IsMPAutoDeployMCV);
    crc(IsMPPrePlacedConYards);
}


/**
 *  Fetch the bulk of the rule data from the control file.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Process(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

    General(ini);
    MPlayer(ini);
}


/**
 *  Process and initialise rule data from the control file.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Initialize(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("RulesClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

}


/**
 *  Process the general main game rules.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::General(CCINIClass &ini)
{
    static char const * const GENERAL = "General";

    if (!ini.Is_Present(GENERAL)) {
        return false;
    }

    return true;
}


/**
 *  Process the general main game rules.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::MPlayer(CCINIClass &ini)
{
    static char const * const MPLAYER = "MultiplayerDefaults";

    if (!ini.Is_Present(MPLAYER)) {
        return false;
    }

    IsMPAutoDeployMCV = ini.Get_Bool(MPLAYER, "AutoDeployMCV", IsMPAutoDeployMCV);
    IsMPPrePlacedConYards = ini.Get_Bool(MPLAYER, "PrePlacedConYards", IsMPPrePlacedConYards);

    return true;
}


/**
 *  Process the UI controls and overrides.
 * 
 *  This function is here for simplicity, and is loaded from its own INI, not Rules.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::Read_UI_INI()
{
    static char const * const GENERAL = "General";
    static char const * const INGAME = "Ingame";

    CCFileClass file("UI.INI");
    CCINIClass ini(file);

    //if (!ini.Is_Present(GENERAL)) {
    //    return false;
    //}

    UIControls.UnitHealthBarDrawPos = ini.Get_Point(INGAME, "UnitHealthBarPos", UIControls.UnitHealthBarDrawPos);
    UIControls.InfantryHealthBarDrawPos = ini.Get_Point(INGAME, "InfantryHealthBarPos", UIControls.InfantryHealthBarDrawPos);

    return true;
}


/**
 *  Initialises the UI controls defaults.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::Init_UI_Controls()
{
    /**
     *  #issue-541
     * 
     *  The health bar graphics "Y" position on selection boxes is off by 1 pixel.
     * 
     *  @author: CCHyper
     */
    UIControls.UnitHealthBarDrawPos.X = -25;
    UIControls.UnitHealthBarDrawPos.Y = -16; // was -15

    UIControls.InfantryHealthBarDrawPos.X = -24;
    UIControls.InfantryHealthBarDrawPos.Y = -5;

    return false;
}
