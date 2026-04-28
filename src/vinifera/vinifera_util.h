/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Various utility functions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "vector.h"


class Surface;
class BSurface;


const char *Vinifera_Version_String();
const char *Vinifera_Build_Type_String();
const char *TSpp_Version_String();

void Vinifera_Draw_Version_Text(Surface *surface, bool pre_init = false);

bool Vinifera_Generate_Mini_Dump();

int Vinifera_Do_WWMessageBox(const char *msg, const char *btn1, const char *btn2 = nullptr, const char *btn3 = nullptr);
void Vinifera_Log_And_Show_WWMessageBox(const char* msg, ...);
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
