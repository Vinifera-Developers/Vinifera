/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Utility functions for saving and loading.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "debughandler.h"
#include "newswizzle.h"
#include "tibsun_globals.h"

#include <optional>
#include <string_view>
#include <vector>


struct IStream;
class ViniferaSaveVersionInfo;


/**
 *  Wrappers for the new swizzle manager for providing debug information.
 */
#define VINIFERA_SWIZZLE_RESET(func) \
    { \
        ViniferaSwizzleManager.Reset(); \
    }

#define VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(pointer, variable) \
    { \
        ViniferaSwizzleManager.Swizzle_Dbg((void**)&pointer, __FILE__, __LINE__, __FUNCTION__ "()", variable); \
    }

#define VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(vector, variable) \
    { \
        for (int __i = 0; __i < vector.Count(); ++__i) { \
            ViniferaSwizzleManager.Swizzle_Dbg((void**)&vector[__i], __FILE__, __LINE__, __FUNCTION__ "()", variable); \
        } \
    }

#define VINIFERA_SWIZZLE_FETCH_SWIZZLE_ID(pointer, id, variable) \
    { \
        ViniferaSwizzleManager.Fetch_Swizzle_ID_Dbg((void*)pointer, (LONG*)&id, __FILE__, __LINE__, __FUNCTION__ "()", variable); \
    }

#define VINIFERA_SWIZZLE_REGISTER_POINTER(id, pointer, variable) \
    { \
        ViniferaSwizzleManager.Here_I_Am_Dbg(id, pointer, __FILE__, __LINE__, __FUNCTION__ "()", variable); \
    }


extern unsigned ViniferaGameVersion;

bool Vinifera_Put_All(IStream *pStm, bool save_net = false);
bool Vinifera_Get_All(IStream *pStm, bool load_net = false);
bool Vinifera_Remap_Extension_Pointers();
void Vinifera_Post_Load_Game();
bool Vinifera_Save_Game(const char* file_name, const char* descr, bool);
bool Vinifera_Load_Game(const char* file_name);
bool Vinifera_Is_Save_Loadable(std::string_view path, ViniferaSaveVersionInfo* info_out = nullptr);
void SaveGame_Hooks();


template<class T>
HRESULT Save_Primitive_Vector(LPSTREAM& pStm, VectorClass<T>& list)
{
    static_assert(std::is_trivially_copyable_v<T>, "Save_Primitive_Vector requires T to be trivially copyable.");

    int count = list.Length();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    if (count <= 0) {
        return hr;
    }

    for (int index = 0; index < count; ++index) {

        hr = pStm->Write(&list[index], sizeof(list[index]), nullptr);
        if (FAILED(hr)) {
            return hr;
        }

    }

    return hr;
}


template<class T>
HRESULT Load_Primitive_Vector(LPSTREAM& pStm, VectorClass<T>& list)
{
    static_assert(std::is_trivially_copyable_v<T>, "Load_Primitive_Vector requires T to be trivially copyable.");

    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    new (&list) VectorClass<T>(count);

    if (count <= 0) {
        return hr;
    }

    for (int index = 0; index < count; ++index) {

        T obj;
        hr = pStm->Read(&obj, sizeof(obj), nullptr);
        if (FAILED(hr)) {
            return hr;
        }
        list[index] = obj;

    }

    return hr;
}


template<class T>
HRESULT Save_Primitive_Vector(LPSTREAM& pStm, DynamicVectorClass<T>& list)
{
    static_assert(std::is_trivially_copyable_v<T>, "Save_Primitive_Vector requires T to be trivially copyable.");

    int count = list.Count();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    if (count <= 0) {
        return hr;
    }

    for (int index = 0; index < count; ++index) {

        hr = pStm->Write(&list[index], sizeof(list[index]), nullptr);
        if (FAILED(hr)) {
            return hr;
        }

    }

    return hr;
}


template<class T>
HRESULT Load_Primitive_Vector(LPSTREAM& pStm, DynamicVectorClass<T>& list)
{
    static_assert(std::is_trivially_copyable_v<T>, "Load_Primitive_Vector requires T to be trivially copyable.");

    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    new (&list) DynamicVectorClass<T>(count);

    if (count <= 0) {
        return hr;
    }

    for (int index = 0; index < count; ++index) {

        T obj;
        hr = pStm->Read(&obj, sizeof(obj), nullptr);
        if (FAILED(hr)) {
            return hr;
        }
        list.Add(obj);

    }

    return hr;
}


template<class T>
HRESULT Save_Primitive_Vector(LPSTREAM& pStm, std::vector<T>& list)
{
    static_assert(std::is_trivially_copyable_v<T>, "Save_Primitive_Vector requires T to be trivially copyable.");

    int count = list.size();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    if (count <= 0) {
        return hr;
    }

    for (int index = 0; index < count; ++index) {

        hr = pStm->Write(&list[index], sizeof(list[index]), nullptr);
        if (FAILED(hr)) {
            return hr;
        }

    }

    return hr;
}


template<class T>
HRESULT Load_Primitive_Vector(LPSTREAM& pStm, std::vector<T>& list)
{
    static_assert(std::is_trivially_copyable_v<T>, "Load_Primitive_Vector requires T to be trivially copyable.");

    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    new (&list) std::vector<T>(count);

    if (count <= 0) {
        return hr;
    }

    for (int index = 0; index < count; ++index) {

        T obj;
        hr = pStm->Read(&obj, sizeof(obj), nullptr);
        if (FAILED(hr)) {
            return hr;
        }
        list[index] = obj;

    }

    return hr;
}


template<typename T>
HRESULT Put_Optional(IStream* stream, const std::optional<T>& value)
{
    static_assert(std::is_trivially_copyable_v<T>, "Put_Optional requires T to be trivially copyable.");

    if (stream == nullptr) {
        return E_POINTER;
    }

    const std::uint8_t has_value = value.has_value() ? 1 : 0;

    ULONG written = 0;
    HRESULT hr = stream->Write(&has_value, sizeof(has_value), &written);
    if (FAILED(hr)) {
        return hr;
    }

    if (written != sizeof(has_value)) {
        return STG_E_WRITEFAULT;
    }

    if (!value.has_value()) {
        return S_OK;
    }

    written = 0;
    hr = stream->Write(&*value, sizeof(T), &written);
    if (FAILED(hr)) {
        return hr;
    }

    if (written != sizeof(T)) {
        return STG_E_WRITEFAULT;
    }

    return S_OK;
}

template<typename T>
HRESULT Read_Optional(IStream* stream, std::optional<T>& value)
{
    static_assert(std::is_trivially_copyable_v<T>, "Read_Optional requires T to be trivially copyable.");

    if (stream == nullptr) {
        return E_POINTER;
    }

    std::uint8_t has_value = 0;

    ULONG read = 0;
    HRESULT hr = stream->Read(&has_value, sizeof(has_value), &read);
    if (FAILED(hr)) {
        return hr;
    }

    if (read != sizeof(has_value)) {
        value.reset();
        return STG_E_READFAULT;
    }

    if (has_value == 0) {
        value.reset();
        return S_OK;
    }

    T temp {};

    read = 0;
    hr = stream->Read(&temp, sizeof(T), &read);
    if (FAILED(hr)) {
        value.reset();
        return hr;
    }

    if (read != sizeof(T)) {
        value.reset();
        return STG_E_READFAULT;
    }

    value = temp;
    return S_OK;
}

