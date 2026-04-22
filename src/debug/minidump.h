/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Creates a mini dump for analysis.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "exceptionhandler.h"


/**
 *  Dump full memory of the process?
 */
extern bool GenerateFullCrashDump;

/**
 *  Are we currently writing a minidump by request, such as by an assert?
 */
extern bool NonFatalMinidump;


/**
 *  Should we produce a minidump with the current time and date?
 */
extern bool MinidumpUseCurrentTime;

extern char MinidumpFilename[PATH_MAX];


bool Create_Mini_Dump(struct _EXCEPTION_POINTERS *e_info, const char *app_name = nullptr, const char *path = nullptr);
