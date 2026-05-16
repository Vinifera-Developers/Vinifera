/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Constant values and strings.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


/**
 *  Strings.
 */

#ifndef RELEASE
#if defined(NIGHTLY)
extern const char TXT_VINIFERA_NIGHTLY_BUILD[];
#elif defined(PREVIEW)
extern const char TXT_VINIFERA_PREVIEW_BUILD[];
#else
extern const char TXT_VINIFERA_LOCAL_BUILD[];
extern const char TXT_VINIFERA_UNOFFICIAL_BUILD[];
#endif
#endif
