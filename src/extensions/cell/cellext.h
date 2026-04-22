/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended CellClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"


/**
 *  Extending CellClass comes with a number of complications, so
 *  until it is truly necessary, we're avoiding it.
 */
static class CellClassExtension
{
public:
    static bool Spread_Tiberium(CellClass* this_ptr, bool forced, int richness = 5);
};
