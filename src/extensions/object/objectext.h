/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ObjectClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstractext.h"
#include "object.h"


class AircraftClass;
class HouseClass;
class AudioAmbientClass;


class ObjectClassExtension : public AbstractClassExtension
{
public:
    /**
     *  IPersistStream
     */
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

public:
    ObjectClassExtension(const ObjectClass* this_ptr);
    ObjectClassExtension(const NoInitClass& noinit);
    virtual ~ObjectClassExtension();

    virtual void Detach(AbstractClass* target, bool all = true) override;
    virtual void Object_CRC(CRCEngine& crc) const override;

    virtual const char* Name() const override;
    virtual const char* Full_Name() const override;

    virtual ObjectClass* This() const override { return reinterpret_cast<ObjectClass*>(AbstractClassExtension::This()); }
    virtual const ObjectClass* This_Const() const override { return reinterpret_cast<const ObjectClass*>(AbstractClassExtension::This_Const()); }

    void Ambient_AI();
    void Stop_Ambient();
    void Attach_Ambient(VocType voc);

public:
    /**
     *  The object's own ambient sound.
     */
    AudioAmbientClass* AmbientSound;

    /**
     *  The ambient sound attached to this object by something else (usually a trigger).
     */
    VocType AttachedAmbientSoundType;
    AudioAmbientClass* AttachedAmbientSound;
};
