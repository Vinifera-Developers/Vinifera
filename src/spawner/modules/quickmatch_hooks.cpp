/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          QUICKMATCH_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the quick match mode.
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

#include "quickmatch_hooks.h"

#include "hooker.h"
#include "spawner.h"
#include "house.h"
#include "textprint.h"
#include "ipxmgr.h"

#include "hooker_macros.h"

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


char* IPXManagerClassExt::_Connection_Name(int id)
{
    if (Spawner::Active && Spawner::GetConfig()->QuickMatch)
    {
        return (char*)PLAYER;
    }
    else
    {
        return IPXManagerClass::Connection_Name(id);
    }
}


static int sprintf_RadarClass_Draw_Names_Wrapper(char* buffer, const char* format, char* str)
{
    if (Spawner::Active && Spawner::GetConfig()->QuickMatch)
    {
        return std::sprintf(buffer, "%s", PLAYER);
    }
    else
    {
        return std::sprintf(buffer, format, str);
    }
}


static Point2D Fancy_Text_Print_ProgressScreenClass_Draw_Graphics_Wrapper(const char* text, XSurface* surface, Rect* rect, Point2D* xy, ColorScheme* fore, unsigned back = COLOR_TBLACK, TextPrintType flag = TPF_8POINT | TPF_DROPSHADOW)
{
    if (Spawner::Active && Spawner::GetConfig()->QuickMatch)
    {
        return Fancy_Text_Print(PLAYER, surface, rect, xy, fore, back, flag);
    }
    else
    {
        return Fancy_Text_Print(text, surface, rect, xy, fore, back, flag);
    }
}


static LRESULT SendMessageA_Kick_Player_Dialog_Wrapper(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Spawner::Active && Spawner::GetConfig()->QuickMatch)
    {
        return SendMessageA(hWnd, Msg, wParam, (LPARAM)PLAYER);
    }
    else
    {
        return SendMessageA(hWnd, Msg, wParam, lParam);
    }
}


void QuickMatch_Hooks()
{
    Patch_Call(0x005B980E, &sprintf_RadarClass_Draw_Names_Wrapper);
    Patch_Call(0x005ADC8F, &Fancy_Text_Print_ProgressScreenClass_Draw_Graphics_Wrapper);
    Patch_Call(0x005B4032, &SendMessageA_Kick_Player_Dialog_Wrapper);
    Patch_Call(0x00648EAE, &IPXManagerClassExt::_Connection_Name);
}
