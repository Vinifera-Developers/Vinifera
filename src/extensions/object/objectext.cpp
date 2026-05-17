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

#include "audio_voc_handle.h"
#include "extension.h"
#include "objecttype.h"
#include "objecttypeext.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
ObjectClassExtension::ObjectClassExtension(const ObjectClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    AmbientSound(nullptr),
    AttachedAmbientSoundType(VOC_NONE),
    AttachedAmbientSound(nullptr)
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

    Stop_Ambient();
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

    AmbientSound = nullptr;
    AttachedAmbientSound = nullptr;
    
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


void ObjectClassExtension::Ambient_AI()
{
    if (This()->IsInLimbo) return;

    // Not all objects have a type class
    auto classof = This()->Class_Of();
    if (classof == nullptr) {
        return;
    }

    auto classext = Extension::Fetch(classof);

    if (classext->AmbientSound != VOC_NONE) {
        if (AmbientSound == nullptr) {
            AmbientSound = new AudioVocHandle(classext->AmbientSound);
            AmbientSound->Start(This()->PositionCoord);
        }
        AmbientSound->Update_Position(This()->PositionCoord);
    }

    if (AttachedAmbientSoundType != VOC_NONE) {
        if (AttachedAmbientSound == nullptr) {
            AttachedAmbientSound = new AudioVocHandle(AttachedAmbientSoundType);
            AttachedAmbientSound->Start(This()->PositionCoord);
        }
        AttachedAmbientSound->Update_Position(This()->PositionCoord);
    }
}


void ObjectClassExtension::Stop_Ambient()
{
    if (AmbientSound) {
        delete AmbientSound;
        AmbientSound = nullptr;
    }

    if (AttachedAmbientSound) {
        delete AttachedAmbientSound;
        AttachedAmbientSound = nullptr;
    }
}


void ObjectClassExtension::Attach_Ambient(VocType voc)
{
    VocType old = AttachedAmbientSoundType;
    AttachedAmbientSoundType = voc;

    if (old != voc && old != VOC_NONE) {
        delete AttachedAmbientSound;
        AttachedAmbientSound = nullptr;
    }
}
