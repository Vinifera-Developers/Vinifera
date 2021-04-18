/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WINUTIL.H
 *
 *  @author        CCHyper
 *
 *  @brief         Utility functions for interacting with the Windows API.
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

#include "always.h"


const char *Last_System_Error_As_String();
void Convert_System_Error_To_String(int id, char *buffer, int buf_len);

const char *Get_Module_File_Name();
const char *Get_Module_File_Name_Ext();

int Load_String_Ex(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, INT nBufferMax, WORD wLanguage);
int Load_String_Ex(HINSTANCE hInstance, UINT uID, LPCSTR lpBuffer, INT nBufferMax, WORD wLanguage);

DWORD Find_Process_Id(const char *process_name);
HANDLE Get_Process_By_Name(const char *process_name);

DWORD Get_Process_Main_Thread_Id(DWORD pId);
HANDLE Get_Thread_Handle(DWORD pId, DWORD dwDesiredAccess);
HMODULE Get_Module_From_Address(LPVOID address);

HICON LoadIconFromFile(const char *filename, int width = 48, int height = 48);
