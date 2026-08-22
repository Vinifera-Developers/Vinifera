/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended ThemeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "themeext_hooks.h"

#include "audio_theme.h"
#include "audio_util.h"
#include "hooker.h"
#include "theme.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class ThemeClassExt : public ThemeClass
{
public:
    ThemeType _From_Name(char const* name) const { return AudioTheme.From_Name(name); }
    ThemeType _Next_Song(ThemeType index) const { return AudioTheme.Next_Song(index); }
    bool _Is_Allowed(ThemeType index) const { return AudioTheme.Is_Allowed(index); }
    char const* _Base_Name(ThemeType index) const { return AudioTheme.Base_Name(index); }
    char const* _Full_Name(ThemeType index) const { return AudioTheme.Full_Name(index); }
    int _Play_Song(ThemeType index) { return AudioTheme.Play_Song(index); }
    bool _Still_Playing() const { return AudioTheme.Still_Playing(); }
    int _Track_Length(ThemeType index) const { return AudioTheme.Track_Length(index); }
    void _Scan() { AudioTheme.Scan(); }
    void _AI() { AudioTheme.AI(); }
    void _Queue_Song(ThemeType index) { AudioTheme.Queue_Song(index); }
    void _Set_Theme_Data(ThemeType theme, int scenario, int owners) { AudioTheme.Set_Theme_Data(theme, scenario, (SideType)owners); }
    void _Stop(bool fade = false) { AudioTheme.Stop(fade); }
    void _Suspend() { AudioTheme.Suspend(); }

    void _Set_Volume(int volume) { AudioTheme.Set_Volume(volume); }

    void _Init_Themes(CCINIClass& ini) { AudioTheme.Init_Themes(ini); }
    void _Free_Themes() { AudioTheme.Free_Themes(); }
};


/**
 *  Main function for patching the hooks.
 */
void ThemeClassExtension_Hooks()
{
    Patch_Jump(0x00644390, &ThemeClassExt::_From_Name);
    Patch_Jump(0x00643E80, &ThemeClassExt::_Next_Song);
    Patch_Jump(0x00644300, &ThemeClassExt::_Is_Allowed);
    Patch_Jump(0x00643D40, &ThemeClassExt::_Base_Name);
    Patch_Jump(0x00643DA0, &ThemeClassExt::_Full_Name);
    Patch_Jump(0x00643FE0, &ThemeClassExt::_Play_Song);
    Patch_Jump(0x006442B0, &ThemeClassExt::_Still_Playing);
    Patch_Jump(0x00644160, &ThemeClassExt::_Track_Length);
    Patch_Jump(0x00643C70, &ThemeClassExt::_Scan);
    Patch_Jump(0x00643DC0, &ThemeClassExt::_AI);
    Patch_Jump(0x00643F20, &ThemeClassExt::_Queue_Song);
    Patch_Jump(0x00644190, &ThemeClassExt::_Stop);
    Patch_Jump(0x00644250, &ThemeClassExt::_Suspend);
    Patch_Jump(0x00644410, &ThemeClassExt::_Set_Volume);
    Patch_Jump(0x00643AC0, &ThemeClassExt::_Init_Themes);
    Patch_Jump(0x00643C20, &ThemeClassExt::_Free_Themes);
}
