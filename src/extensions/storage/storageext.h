/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended StorageClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
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
