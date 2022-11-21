/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RULESEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended RulesClass.
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
#include "rulesext_hooks.h"
#include "rulesext_init.h"
#include "rulesext.h"
#include "rules.h"
#include "tibsun_globals.h"
#include "session.h"
#include "sessionext.h"
#include "ccini.h"
#include "vector.h"
#include "addon.h"
#include "wwmouse.h"
#include "windialog.h"
#include "extension_globals.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include <resource.h>

#include "hooker.h"
#include "hooker_macros.h"


extern HMODULE DLLInstance;


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class RulesClassExt final : public RulesClass
{
    public:
        void _Process(CCINIClass &ini);
};


/**
 *  Intercepts the rules main rules processing.
 * 
 *  @author: CCHyper
 */
void RulesClassExt::_Process(CCINIClass &ini)
{
    /**
     *  Process the rules extension.
     * 
     *  #NOTE: This must be last!
     */
    RuleExtension->Process(ini);
}


/**
 *  Patch to only show the rules selection dialog when in Developer Mode.
 *  
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Rules_Show_Rules_Select_Dialog_Patch)
{
    if (!Vinifera_DeveloperMode) {
        goto use_rules_ini;
    }

    /**
     *  Stolen bytes/code.
     */
    WWMouse->Release_Mouse();

show_rules_dialog:
    JMP(0x004E12F6);

use_rules_ini:
    JMP(0x004E12E3);
}


/**
 *  The rules select dialog procedure.
 * 
 *  @author: CCHyper
 */
LRESULT CALLBACK Rules_Dialog_Procedure(HWND hWnd, UINT uMsg, UINT wParam, LONG lParam)
{
    char buffer[128];

    switch (uMsg) {
        case WM_INITDIALOG:
        {
            WinDialogClass::Center_Window(hWnd);

            HWND hDlgItem = GetDlgItem(hWnd, IDC_RULE_LISTBOX);
            DynamicVectorClass<CCINIClass *> *vec = reinterpret_cast<DynamicVectorClass<CCINIClass *> *>(lParam);
            for (int i = 0; i < vec->Count(); ++i) {
                (*vec)[i]->Get_String("General", "Name", buffer, sizeof(buffer));
                SendMessage(hDlgItem, LB_ADDSTRING, 0, (LPARAM)buffer);
            }
            SendMessage(hDlgItem, LB_SETCURSEL, 0, 0);
            break;
        }
        case WM_COMMAND:
        {
            if (/*wParam == IDCANCEL ||*/ wParam == IDC_RULE_SELECT) {
                HWND hDlgItem = GetDlgItem(hWnd, IDC_RULE_LISTBOX);
                LRESULT res = SendMessage(hDlgItem, LB_GETCURSEL, 0, 0);
                EndDialog(hWnd, res);
                //DestroyWindow(hWnd);  // Causes the return value to be lost.
            }
            break;
        }
        case WM_MOVING:
            WinDialogClass::Dialog_Move(hWnd, wParam, lParam, uMsg);
            break;
        case WM_HELP:
            //Show_Help_File(lparam);
            break;
        case WM_CONTEXTMENU:
            //Show_Description_From_Help_File(wParam);
            break;
    };

    return 0;
}


/**
 *  Patch to intercept the rules initialisation for setting extended values.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Rules_Extended_Class_Patch)
{
    /**
     *  #issue-583
     * 
     *  Allow Colors, AudioVisual and MPlayer sections to be read
     *  from FSRuleINI on rules init.
     * 
     *  @author: CCHyper
     */
    if (Addon_Enabled(ADDON_FIRESTORM)) {
        Rule->Colors(FSRuleINI);
        Rule->AudioVisual(FSRuleINI);
        Rule->MPlayer(FSRuleINI);
    }

    /**
     *  Original code.
     */
    Session.Options.UnitCount = Rule->MPUnitCount;
    BuildLevel = Rule->MPTechLevel;
    Session.Options.Credits = Rule->MPMaxMoney;
    Session.Options.FogOfWar = false;
    Session.Options.BridgeDestruction = Rule->IsMPBridgeDestruction;
    Session.Options.Goodies = Rule->IsMPCrates;
    Session.Options.Bases = Rule->IsMPBasesOn;
    Session.Options.CaptureTheFlag = Rule->IsMPCaptureTheFlag;
    Session.Options.AIPlayers = 0;
    Session.Options.AIDifficulty = DIFF_NORMAL;

    /**
     *  Store extended class values.
     */
    SessionExtension->ExtOptions.IsAutoDeployMCV = RuleExtension->IsMPAutoDeployMCV;
    SessionExtension->ExtOptions.IsPrePlacedConYards = RuleExtension->IsMPPrePlacedConYards;
    SessionExtension->ExtOptions.IsBuildOffAlly = RuleExtension->IsBuildOffAlly;

    /**
     *  Stolen bytes/code.
     */
    _asm { push 0x006FE02C } // "LANGRULE.INI"
    _asm { lea ecx, [esp+0x1AC] }

    JMP(0x004E1401);
}


/**
 *  Main function for patching the hooks.
 */
void RulesClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    RulesClassExtension_Init();

    Patch_Jump(0x005C6710, &RulesClassExt::_Process);

    Patch_Jump(0x004E138B, &_Init_Rules_Extended_Class_Patch);
    Patch_Jump(0x004E12EB, &_Init_Rules_Show_Rules_Select_Dialog_Patch);

    /**
     *  Patch the dialog init to use out rules dialog resource.
     */
    Patch_Dword(0x004E12FC+1, (uintptr_t)&DLLInstance);
    Patch_Dword(0x004E130C+1, IDD_RULES);
    Patch_Jump(0x004E17B0, &Rules_Dialog_Procedure);
}
