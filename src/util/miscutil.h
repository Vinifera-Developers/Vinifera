/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MISCUTIL.H
 *
 *  @author        CCHyper
 *
 *  @brief         Misc utility functions for common tasks.
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
#include <windows.h>
#include <time.h>


typedef int clockid_t;


char const *Get_Text_Time();

void Seconds_To_Hms(float seconds, int & h, int & m, int & s);

void Get_Time(int &hour, int &min, int &sec);
void Get_Full_Time(int &day, int &month, int &year, int &hour, int &min, int &sec);
const char *Get_Date_Time_String(bool filename_safe = false);
void Get_File_Id_String(char const *filename, char &str);
bool Get_File_Creation_Time(char const *filename, time_t &time);
bool Get_EXE_File_Header(char const *filename, IMAGE_FILE_HEADER *header);
bool Get_EXE_File_Header_From_Instance(HINSTANCE h, IMAGE_FILE_HEADER *f_hdr);
int Compare_EXE_Version(HINSTANCE h, char const *filename);
bool Get_Version_Info(char const *filename, VS_FIXEDFILEINFO *file_info);

void HexPrint32(const uint32_t *data, size_t size);
void HexPrint64(const uint64_t *data, size_t size);

int Clock_Get_Time(clockid_t id, struct timespec *ts);

bool Create_Directory(char const *name);

void Set_Working_Directory();

int Get_Last_System_Error();

bool Delete_File(char const *filename);
bool Rename_File(char const *filename, char const *new_filename);
bool File_Exists(char const *filename);
bool WinAPI_File_Exists(const char *filename);

bool Is_Full_Path(const char *path);

const char *Get_User_Documents_Path();

const char *Filename_From_Path(const char *filename);
