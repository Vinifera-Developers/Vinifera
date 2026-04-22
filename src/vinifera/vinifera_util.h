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
#include <algorithm>
#include <cctype>
#include <string>
#include <stdexcept>


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

// Converts all characters in 'str' to uppercase in-place.
// Uses unsigned char cast to avoid UB with std::toupper on signed char values.
inline void string_to_upper(std::string& str)
{
    if (str.empty()) return;

    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
}

// Trims leading characters up to (not including) 'start_index' in-place.
// Erases str[0] through str[start_index - 1].
inline void string_ltrim(std::string& str, std::size_t start_index)
{
    if (str.empty()) return;

    // Validate index is within bounds
    if (start_index > str.size()) {
        throw std::out_of_range("start_index exceeds string length");
    }

    // Erase everything before start_index
    str.erase(0, start_index);
}

// Trims trailing characters from (not including) 'end_index' in-place.
// Erases str[end_index + 1] through str[size - 1].
inline void string_rtrim(std::string& str, std::size_t end_index)
{
    if (str.empty()) return;

    // Validate index is within bounds
    if (end_index >= str.size()) {
        throw std::out_of_range("end_index exceeds string length");
    }

    // Erase everything after end_index
    str.erase(end_index + 1);
}

// Trims both ends, keeping only str[start_index] through str[end_index] in-place.
// Erase back-to-front to avoid index shifting after front erase.
inline void string_trim(std::string& str, std::size_t start_index, std::size_t end_index)
{
    if (str.empty()) return;

    // Validate indices are within bounds
    if (start_index > end_index) {
        throw std::invalid_argument("start_index must be <= end_index");
    }
    if (end_index >= str.size()) {
        throw std::out_of_range("end_index exceeds string length");
    }

    // Erase trailing portion first to avoid invalidating start_index
    str.erase(end_index + 1);

    // Now erase leading portion
    str.erase(0, start_index);
}
