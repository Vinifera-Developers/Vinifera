/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Mouse cursor controls and overrides.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "vinifera_defines.h"


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

        static ActionType From_Name(const char *name);
        static const char *Name_From(ActionType type);

    private:
        static ActionTypeClass *Find_Or_Make(const char *name);

    private:
        std::string Name;
        MouseType Mouse;
        MouseType ShadowMouse;
};
