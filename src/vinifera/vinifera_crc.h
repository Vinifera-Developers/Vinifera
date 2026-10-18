/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Storage for native CRC byte feeds.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "wwcrc.h"

#include <cstddef>

/**
 *  Native TS CRC routines write a staging-count byte at offset 0x0C when
 *  completing a four-byte group. TSpp exposes only the first twelve bytes.
 */
struct NativeCRCStorage
{
    CRCEngine Engine;
    unsigned char StagingPadding[sizeof(long)] {};
};

static_assert(sizeof(CRCEngine) == 12);
static_assert(offsetof(NativeCRCStorage, StagingPadding) == 12);
static_assert(sizeof(NativeCRCStorage) == 16);

/**
 *  Feed new fields without requiring callers to provide the extra native byte.
 *  Copying the twelve-byte state back preserves partial staging and CRC values.
 */
template<class Feed>
void Feed_Native_CRC(CRCEngine& crc, Feed&& feed)
{
    NativeCRCStorage storage;
    storage.Engine = crc;
    feed(storage.Engine);
    crc = storage.Engine;
}
