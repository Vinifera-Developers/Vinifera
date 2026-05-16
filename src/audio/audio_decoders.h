/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Custom decoders for Miniaudio
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "miniaudio.h"

/**
 *  Backend table that declares all the custom decoders.
 */
extern const ma_decoding_backend_vtable * ma_custom_backend_vtable[1];
