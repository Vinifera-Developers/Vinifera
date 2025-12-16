/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TActionClass.
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
#include "tactionext_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "trigger.h"
#include "triggertype.h"
#include "taction.h"
#include "scenario.h"
#include "scenarioext.h"
#include "voc.h"
#include "tactionext.h"
#include "taction.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "house.h"
#include "housetype.h"
#include "object.h"
#include "objecttype.h"
#include "trigger.h"
#include "triggertype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "house.h"
#include "housetype.h"
#include "session.h"

#include "hooker.h"
#include "mouse.h"
#include "rules.h"
#include "tagtype.h"
#include "teamtype.h"
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
 *  @author: ZivDero
 */
bool TActionClassExt::_Operator_Parens_Intercept(HouseClass* house, ObjectClass* object, TriggerClass* trigger, Cell const& cell)
{
    bool success = true;

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
    NeedTeamAndTime = 4
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
        if (Action == TACTION_TEXT_TRIGGER) {
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
