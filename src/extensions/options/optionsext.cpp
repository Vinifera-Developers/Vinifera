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
#include "uicontrol.h"
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
int OptionsClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Get_Object_Size - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Detach - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("OptionsClassExtension::Object_CRC - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Load_Settings()
{
    // EXT_DEBUG_TRACE("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(This()));

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
    CCFileClass keyboard_file("Keyboard.ini");
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
    // EXT_DEBUG_TRACE("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(This()));

    RawFileClass file("SUN.INI");
    CCINIClass sun_ini;
    if (file.Is_Available()) {

        sun_ini.Load(file, false);

        char buffer[512];

        char old_letter[2] = {SidebarControls.MixLetter, 0};
        if (sun_ini.Get_String("Sidebar", "MixLetter", old_letter, buffer, sizeof(buffer)) > 0) {
            if (strlen(buffer) > 0) {
                SidebarControls.MixLetter = buffer[0];
            }
        }

        sun_ini.Get_String("Sidebar", "Preset", "", buffer, sizeof(buffer));

        UIControlsClass::SidebarPresetType preset = UIControlsClass::PRESET_VANILLA;

        if (strnicmp(buffer, "Vanilla", sizeof(buffer)) == 0) {
            preset = UIControlsClass::PRESET_VANILLA;
        } else if (strnicmp(buffer, "4Tabs", sizeof(buffer)) == 0) {
            preset = UIControlsClass::PRESET_4TABS;
        } else if (strnicmp(buffer, "4TabsWide", sizeof(buffer)) == 0) {
            preset = UIControlsClass::PRESET_4TABSWIDE;
        } else if (strnicmp(buffer, "6Tabs", sizeof(buffer)) == 0) {
            preset = UIControlsClass::PRESET_6TABS;
        }

        switch (preset) {
        case UIControlsClass::PRESET_4TABS:
            SidebarControls.IsTabs = true;
            SidebarControls.Tabs = 4;
            SidebarControls.Columns = 2;
            SidebarControls.BuildingsTab = 0;
            SidebarControls.DefensesTab = 0;
            SidebarControls.SpecialTab = 3;
            SidebarControls.InfantryTab = 1;
            SidebarControls.UnitsTab = 2;
            SidebarControls.NavalTab = 3;
            SidebarControls.AircraftTab = 3;
            SidebarControls.TabName.resize(SidebarControls.Tabs, "");
            SidebarControls.TabName[0] = "Buildings";
            SidebarControls.TabName[1] = "Infantry";
            SidebarControls.TabName[2] = "Vehicles";
            SidebarControls.TabName[3] = "Special";
            break;
        case UIControlsClass::PRESET_4TABSWIDE:
            SidebarControls.IsTabs = true;
            SidebarControls.Tabs = 4;
            SidebarControls.Columns = 2;
            SidebarControls.SidebarWidth = 200; // TODO
            SidebarControls.BuildingsTab = 0;
            SidebarControls.DefensesTab = 0;
            SidebarControls.SpecialTab = 3;
            SidebarControls.InfantryTab = 1;
            SidebarControls.UnitsTab = 2;
            SidebarControls.NavalTab = 3;
            SidebarControls.AircraftTab = 3;
            SidebarControls.TabName.resize(SidebarControls.Tabs, "");
            SidebarControls.TabName[0] = "Buildings";
            SidebarControls.TabName[1] = "Infantry";
            SidebarControls.TabName[2] = "Vehicles";
            SidebarControls.TabName[3] = "Special";
            break;
        case UIControlsClass::PRESET_6TABS:
            SidebarControls.IsTabs = true;
            SidebarControls.Tabs = 4;
            SidebarControls.Columns = 3;
            SidebarControls.SidebarWidth = 200; // TODO
            SidebarControls.BuildingsTab = 0;
            SidebarControls.DefensesTab = 1;
            SidebarControls.SpecialTab = 5;
            SidebarControls.InfantryTab = 2;
            SidebarControls.UnitsTab = 3;
            SidebarControls.NavalTab = 4;
            SidebarControls.AircraftTab = 4;
            SidebarControls.TabName.resize(SidebarControls.Tabs, "");
            SidebarControls.TabName[0] = "Buildings";
            SidebarControls.TabName[1] = "Defenses";
            SidebarControls.TabName[2] = "Infantry";
            SidebarControls.TabName[3] = "Units";
            SidebarControls.TabName[4] = "Air";
            SidebarControls.TabName[5] = "Special";
            break;
        }

        SidebarControls.IsTabs = sun_ini.Get_Bool("Sidebar", "IsTabs", SidebarControls.IsTabs);
        if (SidebarControls.IsTabs) {
            SidebarControls.Tabs = sun_ini.Get_Int_Clamp("Sidebar", "Tabs", 4, 6, SidebarControls.Tabs);
        } else {
            SidebarControls.Tabs = 0;
        }

        SidebarControls.Columns = sun_ini.Get_Int_Clamp("Sidebar", "Columns", 2, 5, SidebarControls.Columns);

        SidebarControls.SidebarWidth = sun_ini.Get_Int("Sidebar", "SidebarWidth", SidebarControls.SidebarWidth);
        SidebarControls.IsTopBar = sun_ini.Get_Bool("Sidebar", "IsTopBar", SidebarControls.IsTopBar);
        SidebarControls.TabHeight = sun_ini.Get_Int("Sidebar", "TabHeight", SidebarControls.TabHeight);

        SidebarControls.BuildingsTab = sun_ini.Get_Int_Clamp("Sidebar", "BuildingsTab", 0, SidebarControls.Tabs - 1, SidebarControls.BuildingsTab);
        SidebarControls.DefensesTab = sun_ini.Get_Int_Clamp("Sidebar", "DefensesTab", 0, SidebarControls.Tabs - 1, SidebarControls.DefensesTab);
        SidebarControls.SpecialTab = sun_ini.Get_Int_Clamp("Sidebar", "SpecialTab", 0, SidebarControls.Tabs - 1, SidebarControls.SpecialTab);
        SidebarControls.InfantryTab = sun_ini.Get_Int_Clamp("Sidebar", "InfantryTab", 0, SidebarControls.Tabs - 1, SidebarControls.InfantryTab);
        SidebarControls.UnitsTab = sun_ini.Get_Int_Clamp("Sidebar", "UnitsTab", 0, SidebarControls.Tabs - 1, SidebarControls.UnitsTab);
        SidebarControls.NavalTab = sun_ini.Get_Int_Clamp("Sidebar", "NavalTab", 0, SidebarControls.Tabs - 1, SidebarControls.NavalTab);
        SidebarControls.AircraftTab = sun_ini.Get_Int_Clamp("Sidebar", "AircraftTab", 0, SidebarControls.Tabs - 1, SidebarControls.AircraftTab);

        SidebarControls.TabName.resize(SidebarControls.Tabs, "");
        for (int i = 0; i < SidebarControls.Tabs; i++) {
            char key[32];
            std::snprintf(key, sizeof(key), "Tab%dName", i);
            if (sun_ini.Get_String("Sidebar", key, SidebarControls.TabName[i].c_str(), buffer, sizeof(buffer)) > 0) {
                SidebarControls.TabName[i] = buffer;
            }
        }
    }
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
