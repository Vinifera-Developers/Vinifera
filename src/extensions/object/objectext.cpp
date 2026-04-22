/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ObjectClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "objectext.h"

#include "objecttype.h"
#include "audio_voc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
ObjectClassExtension::ObjectClassExtension(const ObjectClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    AmbientSound(nullptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("ObjectClassExtension::ObjectClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ObjectClassExtension::ObjectClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::ObjectClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
ObjectClassExtension::~ObjectClassExtension()
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::~ObjectClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (AmbientSound) {
        AmbientSound->Stop();
    }
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT ObjectClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }
    
    return hr;
}



/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT ObjectClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void ObjectClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void ObjectClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Returns the name of this object type.
 *  
 *  @author: CCHyper
 */
const char *ObjectClassExtension::Name() const
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Name - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return reinterpret_cast<const ObjectClass *>(This())->Class_Of()->Name();
}


/**
 *  Returns the full name of this object type.
 *  
 *  @author: CCHyper
 */
const char *ObjectClassExtension::Full_Name() const
{
    //EXT_DEBUG_TRACE("ObjectClassExtension::Full_Name - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return reinterpret_cast<const ObjectClass *>(This())->Class_Of()->Full_Name();
}
