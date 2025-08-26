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

        char const* const SIDEBAR = "Sidebar";

        {
            char buffer[512];
            sun_ini.Get_String(SIDEBAR, "Preset", "", buffer, sizeof(buffer));

            enum {
                PRESET_VANILLA,
                PRESET_4TABS,
                PRESET_4TABSWIDE,
                PREST_6TABS
            } preset = PREST_6TABS; // PRESET_VANILLA;

            if (strnicmp(buffer, "Vanilla", sizeof(buffer)) == 0) {
                preset = PRESET_VANILLA;
            } else if (strnicmp(buffer, "4Tabs", sizeof(buffer)) == 0) {
                preset = PRESET_4TABS;
            } else if (strnicmp(buffer, "4TabsWide", sizeof(buffer)) == 0) {
                preset = PRESET_4TABSWIDE;
            } else if (strnicmp(buffer, "6Tabs", sizeof(buffer)) == 0) {
                preset = PREST_6TABS;
            }

            switch (preset) {
            case PRESET_VANILLA:
                break;

            case PRESET_4TABS: {

                SidebarControls.IsTabs = true;
                SidebarControls.Tabs = 4;
                SidebarControls.Columns = 2;
                SidebarControls.MixLetter = 'T';

                SidebarControls.TabAction[0] = TAB_ACTION_BUILDINGS;

                SidebarControls.BuildingsTab = 0;
                SidebarControls.DefensesTab = 0;
                SidebarControls.SpecialTab = 3;
                SidebarControls.InfantryTab = 1;
                SidebarControls.UnitsTab = 2;
                SidebarControls.NavalTab = 3;
                SidebarControls.AircraftTab = 3;

                SidebarControls.StripYOffset = 54;

                SidebarControls.RepairButtonPosition = Point2D(31, -9);
                SidebarControls.SellButtonPosition = Point2D(58, -9);
                SidebarControls.PowerButtonPosition = Point2D(85, -9);
                SidebarControls.WaypointButtonPosition = Point2D(112, -9);

                SidebarControls.TabButtonOffset[0] = Point2D(20, 27);
                SidebarControls.TabButtonOffset[1] = Point2D(55, 27);
                SidebarControls.TabButtonOffset[2] = Point2D(90, 27);
                SidebarControls.TabButtonOffset[3] = Point2D(125, 27);

                SidebarControls.UpButtonOffset = Point2D(2, -1);
                SidebarControls.DownButtonOffset = Point2D(68, -1);

                SidebarControls.PowerPosition = Point2D(8, 53);

                SidebarControls.TabName[0] = "Buildings";
                SidebarControls.TabName[1] = "Infantry";
                SidebarControls.TabName[2] = "Vehicles";
                SidebarControls.TabName[3] = "Special";
                break;
            }

            case PRESET_4TABSWIDE: {

                SidebarControls.IsTabs = true;
                SidebarControls.Tabs = 4;
                SidebarControls.Columns = 3;
                SidebarControls.MixLetter = 'T';

                SidebarControls.TabAction[0] = TAB_ACTION_BUILDINGS;

                SidebarControls.BuildingsTab = 0;
                SidebarControls.DefensesTab = 0;
                SidebarControls.SpecialTab = 3;
                SidebarControls.InfantryTab = 1;
                SidebarControls.UnitsTab = 2;
                SidebarControls.NavalTab = 3;
                SidebarControls.AircraftTab = 3;

                SidebarControls.StripYOffset = 54;

                SidebarControls.RepairButtonPosition = Point2D(31, -9);
                SidebarControls.SellButtonPosition = Point2D(58, -9);
                SidebarControls.PowerButtonPosition = Point2D(85, -9);
                SidebarControls.WaypointButtonPosition = Point2D(112, -9);

                SidebarControls.TabButtonOffset[0] = Point2D(20, 27);
                SidebarControls.TabButtonOffset[1] = Point2D(55, 27);
                SidebarControls.TabButtonOffset[2] = Point2D(90, 27);
                SidebarControls.TabButtonOffset[3] = Point2D(125, 27);

                SidebarControls.UpButtonOffset = Point2D(2, -1);
                SidebarControls.DownButtonOffset = Point2D(68, -1);

                SidebarControls.PowerPosition = Point2D(8, 53);

                SidebarControls.TabName[0] = "Buildings";
                SidebarControls.TabName[1] = "Infantry";
                SidebarControls.TabName[2] = "Units";
                SidebarControls.TabName[3] = "Special";
                break;
            }

            case PREST_6TABS: {

                SidebarControls.IsTabs = true;
                SidebarControls.Tabs = 6;
                SidebarControls.Columns = 3;
                SidebarControls.MixLetter = 'T';

                SidebarControls.TabAction[0] = TAB_ACTION_BUILDINGS;
                SidebarControls.TabAction[1] = TAB_ACTION_DEFENSES;

                SidebarControls.BuildingsTab = 0;
                SidebarControls.DefensesTab = 1;
                SidebarControls.SpecialTab = 5;
                SidebarControls.InfantryTab = 2;
                SidebarControls.UnitsTab = 3;
                SidebarControls.NavalTab = 4;
                SidebarControls.AircraftTab = 4;

                SidebarControls.StripYOffset = 54;

                SidebarControls.RepairButtonPosition = Point2D(31, -9);
                SidebarControls.SellButtonPosition = Point2D(58, -9);
                SidebarControls.PowerButtonPosition = Point2D(85, -9);
                SidebarControls.WaypointButtonPosition = Point2D(112, -9);

                SidebarControls.TabButtonOffset[0] = Point2D(20, 27);
                SidebarControls.TabButtonOffset[1] = Point2D(55, 27);
                SidebarControls.TabButtonOffset[2] = Point2D(90, 27);
                SidebarControls.TabButtonOffset[3] = Point2D(125, 27);
                SidebarControls.TabButtonOffset[4] = Point2D(160, 27);
                SidebarControls.TabButtonOffset[5] = Point2D(195, 27);

                SidebarControls.UpButtonOffset = Point2D(2, -1);
                SidebarControls.DownButtonOffset = Point2D(68, -1);

                SidebarControls.PowerPosition = Point2D(8, 53);
                SidebarControls.RadarHeight = 188;
                SidebarControls.RadarMapRect = Rect(15, 12, 196, 151);

                SidebarControls.TabName[0] = "Buildings";
                SidebarControls.TabName[1] = "Defenses";
                SidebarControls.TabName[2] = "Infantry";
                SidebarControls.TabName[3] = "Units";
                SidebarControls.TabName[4] = "Air";
                SidebarControls.TabName[5] = "Special";
                break;
            }
            }

            char old_letter[2] = {SidebarControls.MixLetter, 0};
            if (sun_ini.Get_String(SIDEBAR, "MixLetter", old_letter, buffer, sizeof(buffer)) > 0) {
                if (strlen(buffer) > 0) {
                    SidebarControls.MixLetter = buffer[0];
                }
            }

            SidebarControls.TabHeight = sun_ini.Get_Int(SIDEBAR, "TabHeight", SidebarControls.TabHeight);
            SidebarControls.CameoWidth = sun_ini.Get_Int(SIDEBAR, "CameoWidth", SidebarControls.CameoWidth);
            SidebarControls.CameoHeight = sun_ini.Get_Int(SIDEBAR, "CameoHeight", SidebarControls.CameoHeight);
            SidebarControls.CameoXSpacing = sun_ini.Get_Int(SIDEBAR, "CameoXSpacing", SidebarControls.CameoXSpacing);
            SidebarControls.CameoYSpacing = sun_ini.Get_Int(SIDEBAR, "CameoYSpacing", SidebarControls.CameoYSpacing);
            SidebarControls.CameoNameOffset = sun_ini.Get_Point(SIDEBAR, "CameoNameOffset", SidebarControls.CameoNameOffset);
            SidebarControls.CameoQueueCountOffset = sun_ini.Get_Point(SIDEBAR, "CameoQueueCountOffset", SidebarControls.CameoQueueCountOffset);
            SidebarControls.CameoStateOffset = sun_ini.Get_Point(SIDEBAR, "CameoStateOffset", SidebarControls.CameoStateOffset);
            SidebarControls.CameoQueueStateOffset = sun_ini.Get_Point(SIDEBAR, "CameoQueueStateOffset", SidebarControls.CameoQueueStateOffset);
            SidebarControls.StripXLeftSpace = sun_ini.Get_Int(SIDEBAR, "StripXLeftSpace", SidebarControls.StripXLeftSpace);
            SidebarControls.StripXRightSpace = sun_ini.Get_Int(SIDEBAR, "StripXRightSpace", SidebarControls.StripXRightSpace);
            SidebarControls.StripYOffset = sun_ini.Get_Int(SIDEBAR, "StripYOffset", SidebarControls.StripYOffset);
            SidebarControls.ScrollRate = sun_ini.Get_Int(SIDEBAR, "ScrollRate", SidebarControls.ScrollRate);

            SidebarControls.PowerPosition = sun_ini.Get_Point(SIDEBAR, "PowerPosition", SidebarControls.PowerPosition);
            SidebarControls.PowerWidth = sun_ini.Get_Int(SIDEBAR, "PowerWidth", SidebarControls.PowerWidth);
            SidebarControls.PowerHeightFudge = sun_ini.Get_Int(SIDEBAR, "PowerHeightFudge", SidebarControls.PowerHeightFudge);
            SidebarControls.PowerPipHeight = sun_ini.Get_Int(SIDEBAR, "PowerPipHeight", SidebarControls.PowerPipHeight);

            SidebarControls.RadarHeight = sun_ini.Get_Int(SIDEBAR, "RadarHeight", SidebarControls.RadarHeight);
            SidebarControls.RadarMapRect = sun_ini.Get_Rect(SIDEBAR, "RadarMapRect", SidebarControls.RadarMapRect);

            SidebarControls.RepairButtonPosition = sun_ini.Get_Point(SIDEBAR, "RepairButtonPosition", SidebarControls.RepairButtonPosition);
            SidebarControls.SellButtonPosition = sun_ini.Get_Point(SIDEBAR, "SellButtonPosition", SidebarControls.SellButtonPosition);
            SidebarControls.PowerButtonPosition = sun_ini.Get_Point(SIDEBAR, "PowerButtonPosition", SidebarControls.PowerButtonPosition);
            SidebarControls.WaypointButtonPosition = sun_ini.Get_Point(SIDEBAR, "WaypointButtonPosition", SidebarControls.WaypointButtonPosition);

            for (int i = 0; i < SidebarControls.Tabs; i++) {
                char key[32];
                std::snprintf(key, sizeof(key), "TabButton%dOffset", i);
                SidebarControls.TabButtonOffset[i] = sun_ini.Get_Point(SIDEBAR, "TabButtonOffset", SidebarControls.TabButtonOffset[i]);
            }

            SidebarControls.UpButtonOffset = sun_ini.Get_Point(SIDEBAR, "UpButtonOffset", SidebarControls.UpButtonOffset);
            SidebarControls.DownButtonOffset = sun_ini.Get_Point(SIDEBAR, "DownButtonOffset", SidebarControls.DownButtonOffset);

            if (sun_ini.Get_String(SIDEBAR, "StateColor", SidebarControls.StateColor.c_str(), buffer, sizeof(buffer)) > 0) {
                SidebarControls.StateColor = buffer;
            }

            if (sun_ini.Get_String(SIDEBAR, "OnHoldColor", SidebarControls.OnHoldColor.c_str(), buffer, sizeof(buffer)) > 0) {
                SidebarControls.OnHoldColor = buffer;
            }

            SidebarControls.BuildingsTab = sun_ini.Get_Int_Clamp(SIDEBAR, "BuildingsTab", 0, SidebarControls.Tabs - 1, SidebarControls.BuildingsTab);
            SidebarControls.DefensesTab = sun_ini.Get_Int_Clamp(SIDEBAR, "DefensesTab", 0, SidebarControls.Tabs - 1, SidebarControls.DefensesTab);
            SidebarControls.SpecialTab = sun_ini.Get_Int_Clamp(SIDEBAR, "SpecialTab", 0, SidebarControls.Tabs - 1, SidebarControls.SpecialTab);
            SidebarControls.InfantryTab = sun_ini.Get_Int_Clamp(SIDEBAR, "InfantryTab", 0, SidebarControls.Tabs - 1, SidebarControls.InfantryTab);
            SidebarControls.UnitsTab = sun_ini.Get_Int_Clamp(SIDEBAR, "UnitsTab", 0, SidebarControls.Tabs - 1, SidebarControls.UnitsTab);
            SidebarControls.NavalTab = sun_ini.Get_Int_Clamp(SIDEBAR, "NavalTab", 0, SidebarControls.Tabs - 1, SidebarControls.NavalTab);
            SidebarControls.AircraftTab = sun_ini.Get_Int_Clamp(SIDEBAR, "AircraftTab", 0, SidebarControls.Tabs - 1, SidebarControls.AircraftTab);

            auto tab_action_from_string = [](const char* str) -> TabActionType {
                if (strnicmp(str, "Buildings", 9) == 0) {
                    return TAB_ACTION_BUILDINGS;
                }
                if (strnicmp(str, "Defenses", 8) == 0) {
                    return TAB_ACTION_DEFENSES;
                }
                return TAB_ACTION_NONE;
            };

            auto tab_action_to_string = [](TabActionType type) -> const char* {
                switch (type) {
                case TAB_ACTION_BUILDINGS:
                    return "Buildings";
                case TAB_ACTION_DEFENSES:
                    return "Defenses";
                default:
                    return "None";
                }
            };

            for (int i = 0; i < SidebarControls.Tabs; i++) {
                char key[32];
                std::snprintf(key, sizeof(key), "Tab%dAction", i);
                if (sun_ini.Get_String(SIDEBAR, key, tab_action_to_string(SidebarControls.TabAction[i]), buffer, sizeof(buffer)) > 0) {
                    SidebarControls.TabAction[i] = tab_action_from_string(buffer);
                }
            }

            for (int i = 0; i < SidebarControls.Tabs; i++) {
                char key[32];
                std::snprintf(key, sizeof(key), "Tab%dName", i);
                if (sun_ini.Get_String(SIDEBAR, key, SidebarControls.TabName[i].c_str(), buffer, sizeof(buffer)) > 0) {
                    SidebarControls.TabName[i] = buffer;
                }
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
