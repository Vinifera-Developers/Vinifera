/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SAVEDGAMESDIR_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for "Saved Games" directory customization.
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

#include "savedgamesdir_hooks.h"

#include <filesystem>

#include "debughandler.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "loadoptions.h"
#include "saveload.h"
#include "spawner.h"


namespace SavedGames
{
	static char Buffer[PATH_MAX];

	bool Ensure_Folder_Exists(const char* path)
    {
		const DWORD attributes = GetFileAttributes(path);

		// If path doesn't exist or isn't a directory, try creating it
		if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// Directory created or already exists
			if (CreateDirectory(path, nullptr) || GetLastError() == ERROR_ALREADY_EXISTS)
				return true; 

		    DEBUG_ERROR("Error: Could not create directory \"%s\". Error code: %d\n", path, GetLastError());
		    return false;
		}

		// Directory already exists
		return true; 
	}

	inline void Format_Path(char* buffer, size_t buffer_size, const char* filename)
	{
		std::snprintf(buffer, buffer_size, "%s\\%s", Spawner::Get_Config()->SavedGamesDir, filename);
	}

	void Check_And_Format_Path(char* buffer, size_t buffer_size, const char* filename)
    {
		std::strstr(filename, Spawner::Get_Config()->SavedGamesDir) ? std::snprintf(buffer, buffer_size, "%s", filename) : Format_Path(buffer, buffer_size, filename);
    }
}


 /**
   *  A fake class for implementing new member functions which allow
   *  access to the "this" pointer of the intended class.
   *
   *  @note: This must not contain a constructor or destructor.
   *
   *  @note: All functions must not be virtual and must also be prefixed
   *         with "_" to prevent accidental virtualization.
   */
class LoadOptionsClassExt : public LoadOptionsClass
{
public:
    bool _Delete_File(const char* filename);
};


bool LoadOptionsClassExt::_Delete_File(const char* filename)
{
	SavedGames::Check_And_Format_Path(SavedGames::Buffer, std::size(SavedGames::Buffer), filename);
	return DeleteFileA(SavedGames::Buffer);
}


int __cdecl sprintf_LoadOptionsClass_Wrapper1(char* buffer, const char*, int number, char* str)
{
	// First create the format string itself, using our custom folder, e. g. "Saved Games\SAVE%04lX.%3s"
	SavedGames::Format_Path(SavedGames::Buffer, std::size(SavedGames::Buffer), "SAVE%04lX.%3s");

	// Now actually format the path
    return std::sprintf(buffer, SavedGames::Buffer, number, str);
}


int __cdecl sprintf_LoadOptionsClass_Wrapper2(char* buffer, const char*, char* str)
{
	// First create the format string itself, using our custom folder, e. g. "Saved Games\*.%3s"
	SavedGames::Format_Path(SavedGames::Buffer, std::size(SavedGames::Buffer), "*.%3s");

	// Now actually format the path
	return std::sprintf(buffer, SavedGames::Buffer, str);
}


void __cdecl DebugString_Save_Game_Wrapper(const char* format, char* file_name, const char* descr)
{
	// Print the string it was going to print
	DEBUG_INFO(format, file_name, descr);

	// Format the path
	SavedGames::Check_And_Format_Path(SavedGames::Buffer, std::size(SavedGames::Buffer), file_name);

	// Print it back to the original buffer
	std::sprintf(file_name, "%s", SavedGames::Buffer);

	// Make sure the subfolder exists
	SavedGames::Ensure_Folder_Exists(Spawner::Get_Config()->SavedGamesDir);
}


void __cdecl DebugString_Load_Game_Wrapper(const char* format, char* file_name)
{
	// Print the string it was going to print
	DEBUG_INFO(format, file_name);

	// Format the path
	SavedGames::Check_And_Format_Path(SavedGames::Buffer, std::size(SavedGames::Buffer), file_name);

	// Print it back to the original buffer
	std::sprintf(file_name, "%s", SavedGames::Buffer);
}


bool Get_Savefile_Info_Wrapper(char* file_name, void* wwsaveload)
{
	SavedGames::Check_And_Format_Path(SavedGames::Buffer, std::size(SavedGames::Buffer), file_name);

	// Print it back to the original buffer
	std::sprintf(file_name, "%s", SavedGames::Buffer);

	return Get_Savefile_Info(file_name, wwsaveload);
}


void SavedGamesDir_Hooks()
{
	Patch_Call(0x00505001, &sprintf_LoadOptionsClass_Wrapper1);
	Patch_Call(0x00505294, &sprintf_LoadOptionsClass_Wrapper1);
	Patch_Call(0x00505509, &sprintf_LoadOptionsClass_Wrapper2);
	Patch_Call(0x00505863, &sprintf_LoadOptionsClass_Wrapper2);
	Patch_Call(0x005D4FF5, &DebugString_Save_Game_Wrapper);
	Patch_Call(0x005D6922, &DebugString_Load_Game_Wrapper);
	Patch_Jump(0x00505A20, &LoadOptionsClassExt::_Delete_File);
	Patch_Call(0x00505A8A, &Get_Savefile_Info_Wrapper);
}
