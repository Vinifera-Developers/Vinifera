/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEAMTYPEEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended TeamTypeClass.
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
