/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Simple stdio wrappers for miniaudio's read/seek/tell procs that are
 *          required for the custom audio decoder.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "miniaudio.h"

/**
 *  Backend table that declares all the custom VFS callbacks.
 */
extern ma_vfs_callbacks ma_custom_vfs_callbacks;
