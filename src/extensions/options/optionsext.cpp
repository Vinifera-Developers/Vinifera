/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended OptionsClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "optionsext.h"

#include "ccini.h"
#include "debughandler.h"
#include "noinit.h"
#include "options.h"
#include "rawfile.h"
#include "tibsun_globals.h"
#include "uicontrol.h"
#include "vinifera_globals.h"


namespace
{
    struct RendererDriverInfo
    {
        const char* ConfigName;
        const char* SDLName;
        OptionsClassExtension::RendererDriverType Type;
    };

    const RendererDriverInfo RendererDrivers[] = {
        {"Direct3D", "direct3d", OptionsClassExtension::RENDERER_DRIVER_DIRECT3D},
        {"Direct3D11", "direct3d11", OptionsClassExtension::RENDERER_DRIVER_DIRECT3D11},
        {"Direct3D12", "direct3d12", OptionsClassExtension::RENDERER_DRIVER_DIRECT3D12},
        {"OpenGL", "opengl", OptionsClassExtension::RENDERER_DRIVER_OPENGL},
        {"Vulkan", "vulkan", OptionsClassExtension::RENDERER_DRIVER_VULKAN}
    };
}


/**
 *  Parses the configured renderer driver name into an internal enum value.
 *
 *  @author: ZivDero
 */
OptionsClassExtension::RendererDriverType OptionsClassExtension::Parse_Renderer_Driver(const char* name)
{
    if (name == nullptr || *name == '\0' || stricmp(name, "Auto") == 0) {
        return RENDERER_DRIVER_AUTO;
    }

    for (const RendererDriverInfo& driver : RendererDrivers) {
        if (stricmp(name, driver.ConfigName) == 0 || stricmp(name, driver.SDLName) == 0) {
            return driver.Type;
        }
    }

    return RENDERER_DRIVER_AUTO;
}


/**
 *  Returns the INI-facing renderer driver name for the given enum value.
 *
 *  @author: ZivDero
 */
const char* OptionsClassExtension::Get_Renderer_Driver_Config_Name(RendererDriverType driver)
{
    if (driver == RENDERER_DRIVER_AUTO) {
        return "Auto";
    }

    for (const RendererDriverInfo& renderer_driver : RendererDrivers) {
        if (renderer_driver.Type == driver) {
            return renderer_driver.ConfigName;
        }
    }

    return "Auto";
}


/**
 *  Returns the SDL renderer driver name for the given enum value.
 *
 *  @author: ZivDero
 */
const char* OptionsClassExtension::Get_Renderer_Driver_SDL_Name(RendererDriverType driver)
{
    for (const RendererDriverInfo& renderer_driver : RendererDrivers) {
        if (renderer_driver.Type == driver) {
            return renderer_driver.SDLName;
        }
    }

    return nullptr;
}


/**
 *  Parses a SubtitleMode INI string into the internal enum value.
 *
 *  @author: ZivDero
 */
OptionsClassExtension::SubtitleModeType OptionsClassExtension::Parse_Subtitle_Mode(const char* name)
{
    if (name == nullptr || *name == '\0') {
        return SUBTITLE_MODE_ALL;
    }
    if (stricmp(name, "None") == 0)       return SUBTITLE_MODE_NONE;
    if (stricmp(name, "All") == 0)        return SUBTITLE_MODE_ALL;
    if (stricmp(name, "Scenario") == 0)   return SUBTITLE_MODE_SCENARIO;
    if (stricmp(name, "System") == 0)     return SUBTITLE_MODE_SYSTEM;
    return SUBTITLE_MODE_ALL;
}


/**
 *  Returns the INI-facing string for a given SubtitleMode value.
 *
 *  @author: ZivDero
 */
const char* OptionsClassExtension::Subtitle_Mode_Config_Name(SubtitleModeType mode)
{
    switch (mode) {
    case SUBTITLE_MODE_NONE:      return "None";
    case SUBTITLE_MODE_ALL:       return "All";
    case SUBTITLE_MODE_SCENARIO:  return "Scenario";
    case SUBTITLE_MODE_SYSTEM:    return "System";
    }
    return "All";
}


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
OptionsClassExtension::OptionsClassExtension(const OptionsClass *this_ptr) :
    GlobalExtensionClass(this_ptr),
    SortDefensesAsLast(true),
    FilterBandBoxSelection(true),
    SidebarViewTypeOverride(SIDEBAR_COUNT),
    KeyChatToAll1(KN_RETURN),
    KeyChatToAll2(KN_F8),
    KeyChatToAllies(KN_BACKSPACE),
    WindowWidth(-1),
    WindowHeight(-1),
    ScaleMode(SDL_SCALEMODE_PIXELART),
    CursorScale(0),
    IsVSync(false),
    RendererDriver(RENDERER_DRIVER_AUTO),
    SubtitleMode(SUBTITLE_MODE_NONE),
    IsPauseRepairs(true),
    AutoSaveCount(5),
    AutoSaveInterval(7200),
    IsAutoSaveInSkirmish(false)
{
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::OptionsClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::~OptionsClassExtension()
{
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT OptionsClassExtension::Load(IStream *pStm)
{
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
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Load_Settings()
{
    SortDefensesAsLast = ConfigINI.Get_Bool("Options", "SortDefensesAsLast", SortDefensesAsLast);
    FilterBandBoxSelection = ConfigINI.Get_Bool("Options", "FilterBandBoxSelection", FilterBandBoxSelection);
    IsPauseRepairs = ConfigINI.Get_Bool("Options", "PauseRepairs", IsPauseRepairs);

    SidebarViewTypeOverride = SIDEBAR_COUNT;

    std::string sidebar_view = ConfigINI.Get_String("Options", "SidebarViewType", "");
    if (!sidebar_view.empty()) {
        SidebarViewTypeOverride = Sidebar_View_From_Name(sidebar_view.c_str(), SIDEBAR_COUNT);

        if (SidebarViewTypeOverride == SIDEBAR_COUNT) {
            DEBUG_WARNING("Unknown sidebar view type \"{}\", using UI.INI setting.\n", sidebar_view);
        }
    }

    char subtitle_mode_buf[32];
    if (ConfigINI.Get_String("Options", "SubtitleMode", "", subtitle_mode_buf, sizeof(subtitle_mode_buf)) > 0) {
        SubtitleMode = Parse_Subtitle_Mode(subtitle_mode_buf);
    }

    AutoSaveCount = ConfigINI.Get_Int("Options", "AutoSaveCount", AutoSaveCount);
    AutoSaveInterval = ConfigINI.Get_Int("Options", "AutoSaveInterval", AutoSaveInterval);
    IsAutoSaveInSkirmish = ConfigINI.Get_Bool("Options", "AutoSaveInSkirmish", IsAutoSaveInSkirmish);
    
    /**
     *  Read keys from Keyboard.ini.
     *
     *  @author: ZivDero
     */
    CCFileClass keyboard_file("Keyboard.ini");
    CCINIClass keyboard_ini;

    if (keyboard_file.Is_Available()) {

        keyboard_ini.Load(keyboard_file, false);

        Options.KeyForceMove1 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "ForceMove", VK_MENU));
        Options.KeyForceMove2 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "ForceMove", VK_MENU));
        Options.KeyForceAttack1 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "ForceAttack", VK_CONTROL));
        Options.KeyForceAttack2 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "ForceAttack", VK_CONTROL));
        Options.KeySelect1 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "Select", VK_SHIFT));
        Options.KeySelect2 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "Select", VK_SHIFT));
        Options.KeyQueueMove1 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "QueueMove", KN_Z));
        Options.KeyQueueMove2 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "QueueMove", KN_Z));

        KeyChatToAll1 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "ChatToAll", KeyChatToAll1));
        KeyChatToAll2 = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "ChatToAll2", KeyChatToAll2));
        KeyChatToAllies = static_cast<KeyNumType>(keyboard_ini.Get_Int("Hotkey", "ChatToAllies", KeyChatToAllies));
    }
}


/**
 *  Fetches the extension data from the INI database at game init.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Load_Init_Settings()
{
    WindowWidth = ConfigINI.Get_Int("Video", "WindowWidth", WindowWidth);
    WindowHeight = ConfigINI.Get_Int("Video", "WindowHeight", WindowHeight);

    char buffer[256];
    if (ConfigINI.Get_String("Video", "ScaleMode", "", buffer, std::size(buffer)) > 0) {
        if (stricmp(buffer, "Linear") == 0) {
            ScaleMode = SDL_SCALEMODE_LINEAR;
        } else if (stricmp(buffer, "Nearest") == 0) {
            ScaleMode = SDL_SCALEMODE_NEAREST;
        } else if (stricmp(buffer, "PixelArt") == 0) {
            ScaleMode = SDL_SCALEMODE_PIXELART;
        }
    }

    CursorScale = ConfigINI.Get_Int("Video", "CursorScale", CursorScale);
    WindowedMode = ConfigINI.Get_Bool("Video", "Windowed", WindowedMode);
    IsVSync = ConfigINI.Get_Bool("Video", "VSync", IsVSync);

    if (ConfigINI.Get_String("Video", "RendererDriver", "", buffer, std::size(buffer)) > 0) {
        RendererDriver = Parse_Renderer_Driver(buffer);

        if (RendererDriver == RENDERER_DRIVER_AUTO && stricmp(buffer, "Auto") != 0) {
            DEBUG_WARNING("Unknown renderer driver \"{}\", falling back to Auto.\n", buffer);
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
    RawFileClass file("SUN.INI");

    /**
     *  Save keys to Keyboard.ini.
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

        keyboard_ini.Put_Int("Hotkey", "ChatToAll", KeyChatToAll1);
        keyboard_ini.Put_Int("Hotkey", "ChatToAll2", KeyChatToAll2);
        keyboard_ini.Put_Int("Hotkey", "ChatToAllies", KeyChatToAllies);

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
}


/**
 *  Returns the effective sidebar view type, with user options overriding UI.INI.
 *
 *  @author: ZivDero
 */
SidebarViewType OptionsClassExtension::Get_Sidebar_View_Type() const
{
    if (SidebarViewTypeOverride != SIDEBAR_COUNT) {
        return SidebarViewTypeOverride;
    }

    if (UIControls != nullptr) {
        return UIControls->BattleSidebarViewType;
    }

    return SIDEBAR_CLASSIC;
}
