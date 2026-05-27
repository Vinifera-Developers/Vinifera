/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TActionClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "extension.h"
#include "stringid.h"
#include "taction.h"
#include "tibsun_defines.h"


class TActionClass;
class HouseClass;
class ObjectClass;
class TriggerClass;


class DECLSPEC_UUID(UUID_ACTION_EXTENSION)
TActionClassExtension final : public AbstractClassExtension
{
public:
    /**
     *  IPersist
     */
    IFACEMETHOD(GetClassID)(CLSID* pClassID);

    /**
     *  IPersistStream
     */
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

public:
    TActionClassExtension(const TActionClass* this_ptr = nullptr);
    TActionClassExtension(const NoInitClass& noinit);
    virtual ~TActionClassExtension();

    virtual int Get_Object_Size() const override;
    virtual void Object_CRC(CRCEngine& crc) const override;

    virtual TActionClass* This() const override { return reinterpret_cast<TActionClass*>(AbstractClassExtension::This()); }
    virtual const TActionClass* This_Const() const override { return reinterpret_cast<const TActionClass*>(AbstractClassExtension::This_Const()); }
    virtual RTTIType Fetch_RTTI() const override { return RTTI_ACTION; }

    /**
     *  Trigger actions don't have names.
     */
    virtual const char* Name() const { return ""; }
    virtual const char* Full_Name() const { return ""; }

public:
    bool Execute(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Is_Vinifera_TAction(TActionType type);

    static const char* Action_Name(int action);
    static const char* Action_Description(int action);

private:
    /**
     *  Vanilla TActions that we re-implement.
     */
    bool Do_PLAY_SPEECH(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell const& cell);
    bool Do_WIN(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_LOSE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_BEGIN_PRODUCTION(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ALL_HUNT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_REINFORCEMENTS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_FIRE_SALE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_TEXT_TRIGGER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DESTROY_TRIGGER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_AUTOCREATE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_CHANGE_HOUSE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ALL_CHANGE_HOUSE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MAKE_ALLY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MAKE_ENEMY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ENABLE_TRIGGER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DESTROY_TAG(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_BEGIN_AI_TRIGGERS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_STOP_AI_TRIGGERS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_PLAY_SOUND_RANDOM(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_CENTER_VIEWPOINT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_REVEAL_SOME(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_PLAY_SOUND_AT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_REINFORCEMENTS_SPECIAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);

    /**
     *  New TActions.
     */
    bool Do_GIVE_CREDITS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ENABLE_SHORT_GAME(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DISABLE_SHORT_GAME(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_CREATE_BUILDING_AT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_HOUSE_DESTROY_ALL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MAKE_ELITE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ENABLE_ALLYREVEAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DISABLE_ALLYREVEAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_CREATE_AUTOSAVE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DELETE_OBJECT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ALL_ASSIGN_MISSION(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MAKE_ALLY_ONE_WAY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MAKE_ENEMY_ONE_WAY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MODIFY_GLOBAL_CONSTANT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MODIFY_GLOBAL_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MODIFY_GLOBAL_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_INCREMENT_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DECREMENT_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MODIFY_LOCAL_CONSTANT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MODIFY_LOCAL_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_MODIFY_LOCAL_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_INCREMENT_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DECREMENT_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_RANDOM_NUMBER_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_RANDOM_NUMBER_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_PRINT_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_PRINT_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ENABLE_TEMPLATED_TEXT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DISABLE_TEMPLATED_TEXT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ADJUST_HOUSE_MODIFIER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_APPLY_IRON_CURTAIN(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_STOP_SOUNDS_AT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_ATTACH_SOUND(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    bool Do_DETACH_SOUND(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);

public:
    /**
     *  Buffer for any text parameter the action may use.
     */
    FixedString<128> Text;

private:
    static TActionClass::ActionDescriptionStruct ExtActionDescriptions[EXT_TACTION_COUNT - EXT_TACTION_FIRST];
};
