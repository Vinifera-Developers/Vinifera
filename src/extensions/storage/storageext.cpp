/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          STORAGEEXT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Extended StorageClass class.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
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
        total += ((*Types)[i] * Tiberiums[i]->Value);
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
