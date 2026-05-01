/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TeamClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "teamext.h"

#include "ccini.h"
#include "extension.h"
#include "team.h"
#include "teamtype.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: Rampastring
 */
TeamClassExtension::TeamClassExtension(const TeamClass *this_ptr) :
    AbstractClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TeamClassExtension::TeamClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TeamExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *
 *  @author: Rampastring
 */
TeamClassExtension::TeamClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TeamClassExtension::TeamClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *
 *  @author: Rampastring
 */
TeamClassExtension::~TeamClassExtension()
{
    //EXT_DEBUG_TRACE("TeamClassExtension::~TeamClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TeamExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *
 *  @author: Rampastring
 */
HRESULT TeamClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TeamClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *
 *  @author: Rampastring
 */
HRESULT TeamClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TeamClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TeamClassExtension(NoInitClass());

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *
 *  @author: Rampastring
 */
HRESULT TeamClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TeamClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *
 *  @author: Rampastring
 */
int TeamClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("TeamClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *
 *  @author: Rampastring
 */
void TeamClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("TeamClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *
 *  @author: Rampastring
 */
void TeamClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TeamClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}

/**
 *  Returns the name of this object type.
 *
 *  @author: Rampastring
 */
const char* TeamClassExtension::Name() const
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Name - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return This()->Class->Name();
}


/**
 *  Returns the full name of this object type.
 *
 *  @author: CCHyper
 */
const char* TeamClassExtension::Full_Name() const
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Full_Name - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return This()->Class->Full_Name();
}
