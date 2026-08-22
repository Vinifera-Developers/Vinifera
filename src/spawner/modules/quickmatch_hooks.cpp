/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the quick match mode.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "quickmatch_hooks.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "ipxmgr.h"
#include "session.h"
#include "sessionext.h"
#include "spawner.h"
#include "syringe.h"
#include "textprint.h"


static const char* PLAYER = "Player";


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class IPXManagerClassExt : public IPXManagerClass
{
public:
    char* _Connection_Name(int id);
};


/**
 *  Hide the player names when the IPX manager is asked for it.
 *
 *  @author: ZivDero
 */
char* IPXManagerClassExt::_Connection_Name(int id)
{
    if (SessionExtension->ExtOptions.IsQuickMatch) {
        return const_cast<char*>(PLAYER);
    } else {
        return IPXManagerClass::Connection_Name(id);
    }
}


/**
 *  Hide the player names in the in the radar.
 *
 *  @author: ZivDero
 */
static int __cdecl sprintf_RadarClass_Draw_Names_Wrapper(char* buffer, const char* format, char* str)
{
    if (SessionExtension->ExtOptions.IsQuickMatch) {
        return std::sprintf(buffer, "%s", PLAYER);
    } else {
        return std::sprintf(buffer, format, str);
    }
}


/**
 *  Hide the player names in the on the progress screen.
 *
 *  @author: ZivDero
 */
static Point2D Fancy_Text_Print_ProgressScreenClass_Draw_Graphics_Wrapper(const char* text, Surface& surface, Rect& rect, Point2D& xy, ColorScheme* fore, unsigned back, TextPrintType flag)
{
    if (SessionExtension->ExtOptions.IsQuickMatch) {
        return Fancy_Text_Print(PLAYER, surface, rect, xy, fore, back, flag);
    } else {
        return Fancy_Text_Print(text, surface, rect, xy, fore, back, flag);
    }
}


/**
 *  Hide the player anmes in the Kick Player dialog.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005B4024, _Kick_Player_Dialog_SendMessage_Hide_Name, 0)
{
    GET(HWND, hWnd, EBP);
    GET(int, index, ESI);

    if (SessionExtension->ExtOptions.IsQuickMatch) {
        SendMessageA(hWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(PLAYER));
    } else {
        SendMessageA(hWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(Session.Players[index]->Name));
    }

    return 0x005B4038;
}


/**
 *  Main function for patching the hooks.
 */
void QuickMatch_Hooks()
{
    Patch_Call(0x005B980E, &sprintf_RadarClass_Draw_Names_Wrapper);
    Patch_Call(0x005ADC8F, &Fancy_Text_Print_ProgressScreenClass_Draw_Graphics_Wrapper);
    Patch_Call(0x00648EAE, &IPXManagerClassExt::_Connection_Name);
}
