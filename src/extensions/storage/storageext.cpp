/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended StorageClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "storageext.h"

#include "tiberium.h"
#include "tibsun_globals.h"


/**
 *  Reimplements StorageClass::Get_Total_Value.
 *
 *  @author: ZivDero
 */
int StorageClassExt::Get_Total_Value() const
{
    int total = 0;

    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        total += ((*Types)[i] * Tiberiums[i]->CreditValue);
    }

    return total;
}


/**
 *  Reimplements StorageClass::Get_Total_Amount.
 *
 *  @author: ZivDero
 */
int StorageClassExt::Get_Total_Amount() const
{
    int total = 0;

    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        total += (*Types)[i];
    }

    return total;
}


/**
 *  Reimplements StorageClass::Get_Amount.
 *
 *  @author: ZivDero
 */
int StorageClassExt::Get_Amount(int index) const
{
    return (*Types)[index];
}


/**
 *  Reimplements StorageClass::Increase_Amount.
 *
 *  @author: ZivDero
 */
int StorageClassExt::Increase_Amount(int amount, int index)
{
    (*Types)[index] += amount;
    return (*Types)[index];
}


/**
 *  Reimplements StorageClass::Decrease_Amount.
 *
 *  @author: ZivDero
 */
int StorageClassExt::Decrease_Amount(int amount, int index)
{
    if (amount > (*Types)[index])
        amount = (*Types)[index];

    (*Types)[index] -= amount;
    return amount;
}


/**
 *  Reimplements StorageClass::First_Used_Slot.
 *
 *  @author: ZivDero
 */
int StorageClassExt::First_Used_Slot() const
{
    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        if ((*Types)[i] > 0.0)
            return i;
    }

    return -1;
}


/**
 *  Reimplements StorageClass::operator+=.
 *
 *  @author: ZivDero
 */
StorageClassExt StorageClassExt::operator+=(StorageClassExt& that)
{
    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        (*Types)[i] += (*that.Types)[i];
    }

    return *this;
}


/**
 *  Reimplements StorageClass::operator-=.
 *
 *  @author: ZivDero
 */
StorageClassExt StorageClassExt::operator-=(StorageClassExt& that)
{
    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        (*Types)[i] -= (*that.Types)[i];
    }

    return *this;
}
