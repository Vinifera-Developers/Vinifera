/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_UTIL.H
 *
 *  @authors       CCHyper
 *
 *  @brief         Various utility functions.
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
#include "vector.h"


class XSurface;
class BSurface;


const char *Vinifera_Version_String();
const char *Vinifera_Build_Type_String();
const char *TSpp_Version_String();

void Vinifera_Draw_Version_Text(XSurface *surface, bool pre_init = false);

bool Vinifera_Generate_Mini_Dump();

int Vinifera_Do_WWMessageBox(const char *msg, const char *btn1, const char *btn2 = nullptr, const char *btn3 = nullptr);
void Vinifera_DeveloperMode_Warning_WWMessageBox(const char *msg, ...);

const char *Vinifera_Get_Window_Title(DWORD dwPid);

bool Vinifera_Create_Zip(const char *filename, DynamicVectorClass<const char *> &filelist, const char *path = nullptr);
bool Vinifera_Collect_Debug_Files();

/**
 *  Functions for fetching windows resources.
 */
#ifndef NDEBUG
const char *Vinifera_Fetch_String(HMODULE handle, ULONG id, const char *file = nullptr, int line = 0);
HGLOBAL Vinifera_Fetch_Resource(HMODULE handle, const char *id, const char *type, const char *file = nullptr, int line = 0);
#define FETCH_STRING(handle, id) Vinifera_Fetch_String(handle, id, __FILE__, __LINE__);
#define FETCH_RESOURCE(handle, id, type) Vinifera_Fetch_Resource(handle, id, type, __FILE__, __LINE__);
#else
const char *Vinifera_Fetch_String(HMODULE handle, ULONG id);
HGLOBAL Vinifera_Fetch_Resource(HMODULE handle, ULONG id, ULONG type);
HGLOBAL Vinifera_Fetch_Resource(HMODULE handle, const char *id, const char *type);
#define FETCH_STRING Vinifera_Fetch_String
#define FETCH_RESOURCE Vinifera_Fetch_Resource
#endif

BSurface *Vinifera_Get_Image_Surface(const char *filename);
