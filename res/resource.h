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
#define IDD_DESYNC_HOST                   4002
#define IDD_DESYNC_WAIT                   4003


/**
 *  Replaceable dialogs from here.
 */
#define IDD_VINIFERA_REPLACEABLE_START    4020        // Replaceable dialogs from here.


/**
 *  Dialog controls.
 */
#define IDC_RULE_SELECT                   1188
#define IDC_RULE_LISTBOX                  1187

#define IDC_EXCEPTION_DEBUG               1150
#define IDC_EXCEPTION_FILENAME            1151
#define IDC_EXCEPTION_QUIT                1153
#define IDC_EXCEPTION_LOG                 1156

#define IDC_DESYNC_HEADER                 1200
#define IDC_DESYNC_CHAT_LIST              1210
#define IDC_DESYNC_CHAT_EDIT              1211
#define IDC_DESYNC_PLAYER_LIST            1212
#define IDC_DESYNC_LOAD                   1220
#define IDC_DESYNC_CONTINUE               1221
#define IDC_DESYNC_QUIT                   1222
#define IDC_DESYNC_COUNTDOWN_BAR          1230
#define IDC_DESYNC_COUNTDOWN_TEXT         1231


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
