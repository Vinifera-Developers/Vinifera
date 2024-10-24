/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ACTIONTYPE.H
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         Mouse cursor controls and overrides.
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
#pragma once

#include "always.h"
#include "iomap.h"
#include "wstring.h"


class CCINIClass;
class MouseTypeClass;


class ActionTypeClass
{
    public:
        ActionTypeClass(const char* name, MouseType mouse = MOUSE_NORMAL, MouseType shadow_mouse = MOUSE_NORMAL);
        ActionTypeClass(const NoInitClass &noinit);
        ~ActionTypeClass();

        MouseType Get_Mouse() const { return Mouse; }
        MouseType Get_Shadow_Mouse() const { return ShadowMouse; }

        static void One_Time();

        static bool Read_INI(CCINIClass &ini);
#ifndef NDEBUG
        static bool Write_Default_INI(CCINIClass &ini);
#endif

        static const ActionTypeClass *As_Pointer(ActionType type);
        static const ActionTypeClass *As_Pointer(const char *name);
        static const ActionTypeClass &As_Reference(ActionType type);
        static const ActionTypeClass &As_Reference(const char *name);
        static ActionType From_Name(const char *name);
        static const char *Name_From(ActionType type);

    private:
        static ActionTypeClass *Find_Or_Make(const char *name);

    private:
        Wstring Name;
        MouseType Mouse;
        MouseType ShadowMouse;

    private:
        static ActionTypeClass ActionControl[ACTION_COUNT];
        static const char *ActionNames[ACTION_COUNT];
};
