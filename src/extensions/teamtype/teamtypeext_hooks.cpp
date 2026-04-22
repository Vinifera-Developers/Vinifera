/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TeamTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "teamtypeext_hooks.h"

#include "ccini.h"
#include "extension.h"
#include "hooker.h"
#include "teamtype.h"
#include "teamtypeext.h"
#include "teamtypeext_init.h"


/**
 *  Reimplementation of TeamTypeClass::Read_All. Reads the INI data
 *  for TeamTypes from an INI file. Modified to read in extension data as well.
 *
 *  Author: tomsons26, modified by Rampastring
 */
void _TeamTypeClass_Read_All(CCINIClass& ini, INIScopeType scope)
{
	char buffer[32];

	int len = ini.Entry_Count("TeamTypes");
	for (int index = 0; index < len; index++) {
		char const* entry = ini.Get_Entry("TeamTypes", index);
		assert(entry != NULL);
		ini.Get_String("TeamTypes", entry, "", buffer, sizeof(buffer));
		TeamTypeClass* ttptr = TeamTypeClass::Find_Or_Make(buffer);
		assert(ttptr != NULL);
		ttptr->Read_INI(ini);
		ttptr->Scope = scope;

		auto extension = Extension::Fetch(ttptr);
		extension->Read_INI(ini);
	}
}


/**
 *  Main function for patching the hooks.
 */
void TeamTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TeamTypeClassExtension_Init();

	Patch_Jump(0x00628CD0, &_TeamTypeClass_Read_All);
}
