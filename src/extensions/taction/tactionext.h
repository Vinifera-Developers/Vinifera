/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TActionClass class.
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

#include "extension.h"
#include "taction.h"
#include "tibsun_defines.h"


class TActionClass;
class HouseClass;
class ObjectClass;
class TriggerClass;


class TActionClassExtension
{
public:
    static bool Execute(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Is_Vinifera_TAction(TActionType type);

    static const char* Action_Name(int action);
    static const char* Action_Description(int action);

private:

    /**
     *  Vanilla TActions that we re-implement.
     */
    static bool Do_WIN(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_LOSE(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_TEXT_TRIGGER(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DESTROY_TRIGGER(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_ENABLE_TRIGGER(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DESTROY_TAG(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_PLAY_SOUND_RANDOM(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);

    /**
     *  New TActions.
     */
    static bool Do_GIVE_CREDITS(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_ENABLE_SHORT_GAME(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DISABLE_SHORT_GAME(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_HOUSE_DESTROY_ALL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MAKE_ELITE(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_ENABLE_ALLYREVEAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DISABLE_ALLYREVEAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_CREATE_AUTOSAVE(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DELETE_OBJECT(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_ALL_ASSIGN_MISSION(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MAKE_ALLY_ONE_WAY(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MAKE_ENEMY_ONE_WAY(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MODIFY_GLOBAL_CONSTANT(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MODIFY_GLOBAL_GLOBAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MODIFY_GLOBAL_LOCAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_INCREMENT_GLOBAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DECREMENT_GLOBAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MODIFY_LOCAL_CONSTANT(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MODIFY_LOCAL_GLOBAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_MODIFY_LOCAL_LOCAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_INCREMENT_LOCAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DECREMENT_LOCAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_RANDOM_NUMBER_GLOBAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_RANDOM_NUMBER_LOCAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_PRINT_GLOBAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_PRINT_LOCAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_ENABLE_TEMPLATED_TEXT(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_DISABLE_TEMPLATED_TEXT(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);
    static bool Do_ADJUST_HOUSE_MODIFIER(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell);

private:
    static TActionClass::ActionDescriptionStruct ExtActionDescriptions[EXT_TACTION_COUNT - EXT_TACTION_FIRST];
};
