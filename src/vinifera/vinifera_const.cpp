/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Constant values and strings.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "vinifera_const.h"


#ifndef RELEASE
/**
 *  78 characters max for 640 width!
 */
#if defined(NIGHTLY)
const char TXT_VINIFERA_NIGHTLY_BUILD[78] = { "This is a nightly build of Vinifera, please use with caution!" };
#elif defined(PREVIEW)
const char TXT_VINIFERA_PREVIEW_BUILD[78] = { "This is a preview build of Vinifera, please use with caution!" };
#else
const char TXT_VINIFERA_LOCAL_BUILD[78] = { "This is a local unofficial build of Vinifera, please use with caution!" };
const char TXT_VINIFERA_UNOFFICIAL_BUILD[78] = { "This is an unofficial build of Vinifera, please use with caution!" };
#endif
#endif
