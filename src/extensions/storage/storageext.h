/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       ZivDero
 *
 *  @file          STORAGEEXT.H
 *
 *  @author        CCHyper
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
#pragma once
#include "vector.h"

/**
 *  This class does not extend the vanilla StorageClass like AbstractClass extensions do.
 *  Instead, it is constructed in its place by the owner class's extension.
 */
class StorageClassExt
{
public:
    StorageClassExt(VectorClass<int>* vector) :
        Types(vector)
    {
    }

public:
    /**
     *  Pointer to the vector located in the extension for the class that contains the StorageClass.
     */
    VectorClass<int>* Types;

public:
    int Get_Total_Value() const;
    int Get_Total_Amount() const;
    int Get_Amount(int index) const;
    int Increase_Amount(int amount, int index);
    int Decrease_Amount(int amount, int index);
    int First_Used_Slot() const;

    StorageClassExt operator+=(StorageClassExt& that);
    StorageClassExt operator-=(StorageClassExt& that);
};
