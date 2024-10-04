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
#include "session.h"

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


/**
 *  Hide the player names when the IPX manager is asked for it.
 *
 *  @author: ZivDero
 */
char* IPXManagerClassExt::_Connection_Name(int id)
{
    if (Spawner::Active && Spawner::Get_Config()->QuickMatch)
    {
        return const_cast<char*>(PLAYER);
    }
    else
    {
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
    if (Spawner::Active && Spawner::Get_Config()->QuickMatch)
    {
        return std::sprintf(buffer, "%s", PLAYER);
    }
    else
    {
        return std::sprintf(buffer, format, str);
    }
}


/**
 *  Hide the player names in the on the progress screen.
 *
 *  @author: ZivDero
 */
static Point2D Fancy_Text_Print_ProgressScreenClass_Draw_Graphics_Wrapper(const char* text, XSurface* surface, Rect* rect, Point2D* xy, ColorScheme* fore, unsigned back, TextPrintType flag)
{
    if (Spawner::Active && Spawner::Get_Config()->QuickMatch)
    {
        return Fancy_Text_Print(PLAYER, surface, rect, xy, fore, back, flag);
    }
    else
    {
        return Fancy_Text_Print(text, surface, rect, xy, fore, back, flag);
    }
}


/**
 *  Hide the player anmes in the Kick Player dialog.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Kick_Player_Dialog_SendMessage_Hide_Name)
{
    GET_REGISTER_STATIC(HWND, hWnd, ebp);
    GET_REGISTER_STATIC(int, index, esi);

    _asm pushad

    if (Spawner::Active && Spawner::Get_Config()->QuickMatch)
    {
        SendMessageA(hWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(PLAYER));
    }
    else
    {
        SendMessageA(hWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(Session.Players[index]->Name));
    }

    _asm popad

    JMP(0x005B4038);
}


/**
 *  Main function for patching the hooks.
 */
void QuickMatch_Hooks()
{
    Patch_Call(0x005B980E, &sprintf_RadarClass_Draw_Names_Wrapper);
    Patch_Call(0x005ADC8F, &Fancy_Text_Print_ProgressScreenClass_Draw_Graphics_Wrapper);
    Patch_Jump(0x005B4024, &_Kick_Player_Dialog_SendMessage_Hide_Name);
    Patch_Call(0x00648EAE, &IPXManagerClassExt::_Connection_Name);
}
