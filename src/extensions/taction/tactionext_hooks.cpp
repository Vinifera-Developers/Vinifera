/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TActionClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "tactionext_hooks.h"

#include "asserthandler.h"
#include "audio_theme.h"
#include "audio_util.h"
#include "audio_voc.h"
#include "audio_vox.h"
#include "debughandler.h"
#include "hooker.h"
#include "house.h"
#include "mouse.h"
#include "object.h"
#include "taction.h"
#include "tactionext.h"
#include "tagtype.h"
#include "teamtype.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "triggertype.h"
#include "waypoint.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TActionClass)
{
public:
    bool _Operator_Parens_Intercept(HouseClass* house, ObjectClass* object, TriggerClass* trigger, Cell const& cell);
    void _Read_INI();
};


/**
 *  Intercept for TActionClass::operator() to add the
 *  execution of our new TActions.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_Operator_Parens_Intercept(HouseClass* house, ObjectClass* object, TriggerClass* trigger, Cell const& cell)
{
    bool success = true;

    if (Vinifera_DeveloperMode) {
        DEBUG_INFO("Executing TAction %d %s. Trigger: \"%s\", Frame: %d\n", Action, TActionClassExtension::Action_Name(Action), trigger->Class->GivenName.c_str(), Frame);
    }

    /**
     *  If this is a Vinifera TAction, execute it.
     */
    if (TActionClassExtension::Is_Vinifera_TAction(Action)) {
        success = Extension::Fetch(this)->Execute(house, object, trigger, cell);
    }

    /**
     *  Otherwise, let the game handle it.
     */
    else {
        success = TActionClass::operator()(house, object, trigger, cell);
    }

    return success;
}


enum NeedCode {
    NeedOther = 0,
    NeedTeam = 1,
    NeedTrigger = 2,
    NeedTag = 3,
    NeedTeamAndTime = 4,
    NeedSpeech = 5,
    NeedSound = 6,
    NeedTheme = 7
};


/**
 *  Parses the INI text for this action's data.
 *
 *  @author: ZivDero, tomsons26
 */
void TActionClassExt::_Read_INI()
{
    auto& extension = *Extension::Fetch(this);

    Data.Value = 0;
    Action = static_cast<TActionType>(atoi(strtok(nullptr, ",")));
    NeedCode code = static_cast<NeedCode>(atoi(strtok(nullptr, ",")));
    char* text = strtok(nullptr, ",");
    int val = atoi(text);

    switch (code) {
    case NeedOther:

        /**
         *  Hack: for text triggers, we want text now, but we won't change the need code
         *  to preserve compatibility.
         */
        if (Action == TACTION_TEXT_TRIGGER || Action == EXT_TACTION_ENABLE_TEMPLATED_TEXT) {
            extension.Text = text;
        } else {
            Data.Value = val;
        }
        break;

    case NeedTeam:
    case NeedTeamAndTime:
        if (val == -1) {
            Team = nullptr;
        } else {
            if (strlen(text) < 3) {
                Team = TeamTypes[val];
            } else {
                Team = TeamTypeClass::Find_Or_Make(text);
            }
        }
        break;

    case NeedTrigger:
        if (val == -1) {
            Trigger = nullptr;
        } else {
            if (strlen(text) < 3) {
                Trigger = TriggerTypes[val];
            } else {
                Trigger = TriggerTypeClass::Find_Or_Make(text);
            }
        }
        break;

    case NeedTag:
        if (val == -1) {
            Tag = nullptr;
        } else {
            if (strlen(text) < 3) {
                Tag = TagTypes[val];
            } else {
                Tag = TagTypeClass::Find_Or_Make(text);
            }
        }
        break;

    case NeedSpeech:
        Data.Speech = AudioVoxClass::From_Name(text);
        break;

    case NeedSound:
        Data.Sound = AudioVocClass::From_Name(text);
        break;

    case NeedTheme:
        Data.Theme = AudioTheme.From_Name(text);
        break;
    }

    TriggerRect.X = atoi(strtok(nullptr, ","));
    TriggerRect.Y = atoi(strtok(nullptr, ","));
    TriggerRect.Width = atoi(strtok(nullptr, ","));
    TriggerRect.Height = atoi(strtok(nullptr, ","));
    char* temp = strtok(nullptr, ",");
    if (temp != nullptr) {
        if (code == NeedTeamAndTime) {
            Data.Value = atoi(temp);
        } else {
            EffectLocation = Waypoint_From_String(temp);
        }
    }
}


/**
 *  What can this action attach to?
 *
 *  @author: ZivDero
 */
AttachType _Attaches_To(TActionType event)
{
    AttachType attach = ATTACH_NONE;

    switch (event) {
    case TACTION_DESTROY_OBJECT:
    case TACTION_SELL_ATTACHED:
    case TACTION_TURN_OFF_ATTACHED:
    case TACTION_TURN_ON_ATTACHED:
    case TACTION_CHANGE_HOUSE:
    case TACTION_GO_BERZERK:
    case TACTION_SET_GROUP_ID:
    case EXT_TACTION_ATTACH_SOUND:
    case EXT_TACTION_DETACH_SOUND:
        attach |= ATTACH_OBJECT;
        break;

    default:
        break;
    }
    return attach;
}


/**
 *  Main function for patching the hooks.
 */
void TActionClassExtension_Hooks()
{
    Patch_Call(0x0064961C, &TActionClassExt::_Operator_Parens_Intercept);
    Patch_Jump(0x00618F70, &TActionClassExt::_Read_INI);
    Patch_Jump(0x0061D9C0, &_Attaches_To);

    /**
     *  #issue-674
     * 
     *  Fixes a bug where the game would crash when TACTION_WAKEUP_GROUP was
     *  executed but the game was not able to match the Group to the triggers
     *  group. This was because the game was searching the Foots vector with
     *  the count of the Technos vector, and in cases where the Group did
     *  not match, the game would crash trying to search out of bounds.
     * 
     *  @author: CCHyper
     */
    Patch_Dword(0x00619552+2, (0x007E4820+4)); // Foot vector to Technos vector.
}
