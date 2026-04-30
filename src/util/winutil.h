/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Utility functions for interacting with the Windows API.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


const char *Last_System_Error_As_String();
void Convert_System_Error_To_String(int id, char *buffer, int buf_len);

const char *Get_Module_File_Name();
const char *Get_Module_File_Name_Ext();

int Load_String_Ex(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, INT nBufferMax, WORD wLanguage);
int Load_String_Ex(HINSTANCE hInstance, UINT uID, LPCSTR lpBuffer, INT nBufferMax, WORD wLanguage);

DWORD Find_Process_Id(const char *process_name);
HANDLE Get_Process_by_Id(DWORD pId);
HANDLE Get_Process_By_Name(const char *process_name);

DWORD Get_Process_Main_Thread_Id(DWORD pId);
HANDLE Get_Thread_Handle(DWORD pId, DWORD dwDesiredAccess);
HMODULE Get_Module_From_Address(LPVOID address);

HICON LoadIconFromFile(const char *filename, int width = 48, int height = 48);

bool DeleteFilesOlderThan(unsigned days, const char *dir, const char *file);
