/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Windows resources include file.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#ifndef _WINUSER_
#include <winres.h>
#endif


/**
 *  Icon with lowest ID value placed first to ensure application icon
 *  remains consistent on all systems.
 */
#define IDI_VINIFERA     100


/**
 *  Use these defines in code to allow easy updating in all areas used.
 */
#define VINIFERA_MAINICON		 IDI_VINIFERA
#define VINIFERA_MAINCURSOR		 IDC_ARROW


/**
 *  Dialogs.
 */
#define IDD_VINIFERA_START                4000        // Make sure our numbers are well above Tiberian Sun's

#define IDD_RULES                         4000
#define IDD_EXCEPTION                     4001


/**
 *  Replaceable dialogs from here.
 */
#define IDD_VINIFERA_REPLACEABLE_START    4020        // Replaceable dialogs from here.


/**
 *  Dialog controls.
 */
#define IDC_RULE_SELECT                   1188
#define IDC_RULE_LISTBOX                  1187

#define IDC_EXCEPTION_SAVE                1149
#define IDC_EXCEPTION_DEBUG               1150
#define IDC_EXCEPTION_FILENAME            1151
#define IDC_EXCEPTION_QUIT                1153
#define IDC_EXCEPTION_MAINMENU            1154
#define IDC_EXCEPTION_LOG                 1156
#define IDC_EXCEPTION_CONTINUE            1157


/**
 *  Version resources.
 */
#ifndef RELEASE
    #if defined(NIGHTLY)
        #define VER_SPECIALBUILD_STR "Nightly Build\0"
    #elif defined(PREVIEW)
        #define VER_SPECIALBUILD_STR "Preview Build\0"
    #else
        #define VER_SPECIALBUILD_STR "Local Unofficial Build\0"
    #endif
#else
    #define VER_SPECIALBUILD_STR "\0"
#endif
