/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Shared sidebar tooltip formatting helpers.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

struct BuildItem;

/**
 *  Formats the tooltip text for a BuildItem (name, cost, description).
 *  Returns a pointer to a static buffer, or nullptr if invalid.
 */
const char* Format_Cameo_Tooltip(const BuildItem& item);
