/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended HouseTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "housetypeext.h"

#include "ccini.h"
#include "extension.h"
#include "housetype.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
HouseTypeClassExtension::HouseTypeClassExtension(const HouseTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr)
{
    HouseTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
HouseTypeClassExtension::HouseTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
HouseTypeClassExtension::~HouseTypeClassExtension()
{
    HouseTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT HouseTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT HouseTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) HouseTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT HouseTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int HouseTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void HouseTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool HouseTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    IsInitialized = true;
    
    return true;
}


/**
 *  Fetch house pointer from its name.
 *  Also takes care of ts-patches spawn houses.
 *
 *  @warning: Do not use the raw output if you expect spawn houses!
 *
 *  @author: ZivDero
 */
HousesType HouseTypeClassExtension::House_From_Name(char const* name)
{
    if (Session.Type != GAME_NORMAL) {
        int spawn_number;

        /**
         *  Try to read the house name as a spawn house name and extract its number.
         */
        if (std::sscanf(name, "Spawn%d", &spawn_number) == 1) {
            spawn_number--;
            if (spawn_number >= 0 && spawn_number < MAX_PLAYERS) {
                return static_cast<HousesType>(spawn_number + 50);
            }
        }
    }

    if (name != nullptr) {
        for (int house = HOUSE_FIRST; house < HouseTypes.Count(); house++) {
            HouseTypeClass* ptr = HouseTypes[house];
            if (ptr->GivenName == name || ptr->IniName == name) {
                return ptr->House;
            }
        }
    }

    return HOUSE_NONE;
}
