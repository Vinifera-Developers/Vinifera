/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended RulesClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "rulesext_hooks.h"

#include "addon.h"
#include "ccini.h"
#include "debughandler.h"
#include "extension_globals.h"
#include "hooker.h"
#include "rules.h"
#include "rulesext.h"
#include "rulesext_init.h"
#include "sessionext.h"
#include "syringe.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vector.h"
#include "windialog.h"
#include "wwmouse.h"

#include <resource.h>


extern HMODULE DLLInstance;


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class RulesClassExt : public RulesClass
{
public:
    void _Process(CCINIClass &ini);
    void _Initialize(CCINIClass& ini);
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
 *  Intercepts the rules initialization.
 *
 *  @author: ZivDero
 */
void RulesClassExt::_Initialize(CCINIClass& ini)
{
    RuleExtension->Initialize(ini);
    RulesClass::Initialize(ini);
}


/**
 *  Patch to only show the rules selection dialog when in Developer Mode.
 *  
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E12EB, _Init_Rules_Show_Rules_Select_Dialog_Patch, 0)
{
    if (!Vinifera_DeveloperMode) {
        goto use_rules_ini;
    }

    /**
     *  Stolen bytes/code.
     */
    MouseCursor->Release_Mouse();

show_rules_dialog:
    return 0x004E12F6;

use_rules_ini:
    return 0x004E12E3;
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
                (*vec)[i]->Get_String("General", "Name", "", buffer, sizeof(buffer));
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
            On_WM_MOVING(hWnd, wParam, lParam);
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
DEFINE_HOOK(0x004E138B, _Init_Rules_Extended_Class_Patch, 5)
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
     *  Store extended class values.
     */
    SessionExtension->ExtOptions.IsAutoDeployMCV = RuleExtension->IsMPAutoDeployMCV;
    SessionExtension->ExtOptions.IsPrePlacedConYards = RuleExtension->IsMPPrePlacedConYards;
    SessionExtension->ExtOptions.IsBuildOffAlly = RuleExtension->IsBuildOffAlly;

    return 0;
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
    Patch_Call(0x0053E408, &RulesClassExt::_Initialize);

    /**
     *  Patch the dialog init to use out rules dialog resource.
     */
    Patch_Dword(0x004E12FC+1, (uintptr_t)&DLLInstance);
    Patch_Dword(0x004E130C+1, IDD_RULES);
    Patch_Jump(0x004E17B0, &Rules_Dialog_Procedure);
}
