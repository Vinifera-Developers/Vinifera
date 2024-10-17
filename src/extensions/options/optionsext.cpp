/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OPTIONSEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended OptionsClass class.
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
#include "optionsext.h"
#include "options.h"
#include "tibsun_globals.h"
#include "noinit.h"
#include "options.h"
#include "ccini.h"
#include "rawfile.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "vinifera_globals.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::OptionsClassExtension(const OptionsClass *this_ptr) :
    GlobalExtensionClass(this_ptr),
    SortDefensesAsLast(true),
    FilterBandBoxSelection(true)
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::OptionsClassExtension - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::OptionsClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::OptionsClassExtension(NoInitClass) - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::~OptionsClassExtension()
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::~OptionsClassExtension - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT OptionsClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) OptionsClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT OptionsClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
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
int OptionsClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Detach - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Compute_CRC - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Load_Settings()
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(This()));
    
    RawFileClass file("SUN.INI");
    CCINIClass sun_ini;

    if (file.Is_Available()) {

        sun_ini.Load(file, false);

        SortDefensesAsLast = sun_ini.Get_Bool("Options", "SortDefensesAsLast", SortDefensesAsLast);
        FilterBandBoxSelection = sun_ini.Get_Bool("Options", "FilterBandBoxSelection", FilterBandBoxSelection);
    }

    /**
     *  Read hardcoded modifier keys from Keyboard.ini.
     *
     *  @author: ZivDero
     */
    RawFileClass keyboard_file("Keyboard.ini");
    CCINIClass keyboard_ini;

    if (keyboard_file.Is_Available()) {

        keyboard_ini.Load(keyboard_file, false);

        Options.KeyForceMove1 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "ForceMove", VK_MENU);
        Options.KeyForceMove2 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "ForceMove", VK_MENU);
        Options.KeyForceAttack1 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "ForceAttack", VK_CONTROL);
        Options.KeyForceAttack2 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "ForceAttack", VK_CONTROL);
        Options.KeySelect1 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "Select", VK_SHIFT);
        Options.KeySelect2 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "Select", VK_SHIFT);
        Options.KeyQueueMove1 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "QueueMove", Vinifera_NewSidebar ? KN_Z : KN_Q);
        Options.KeyQueueMove2 = (KeyNumType)keyboard_ini.Get_Int("Hotkey", "QueueMove", Vinifera_NewSidebar ? KN_Z : KN_Q);
    }
}


/**
 *  Fetches the extension data from the INI database at game init.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Load_Init_Settings()
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(This()));
    
    RawFileClass file("SUN.INI");
}


/**
 *  Saves the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Save_Settings()
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Save_Settings - 0x%08X\n", (uintptr_t)(This()));
    
    RawFileClass file("SUN.INI");

    /**
     *  Save hardcoded modifier keys to Keyboard.ini.
     *
     *  @author: ZivDero
     */
    RawFileClass keyboard_file("Keyboard.ini");
    CCINIClass keyboard_ini;

    if (keyboard_file.Is_Available()) {

        keyboard_ini.Load(keyboard_file, false);

        keyboard_ini.Put_Int("Hotkey", "ForceMove", Options.KeyForceMove1);
        keyboard_ini.Put_Int("Hotkey", "ForceAttack", Options.KeyForceAttack1);
        keyboard_ini.Put_Int("Hotkey", "Select", Options.KeySelect1);
        keyboard_ini.Put_Int("Hotkey", "QueueMove", Options.KeyQueueMove1);

        keyboard_ini.Save(keyboard_file, false);
    }
}


/**
 *  Sets any options based on current settings.
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Set()
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Set - 0x%08X\n", (uintptr_t)(This()));
}
