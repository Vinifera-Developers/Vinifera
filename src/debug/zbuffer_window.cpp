/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Temporary ImGui window that renders the engine's Z buffer.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "zbuffer_window.h"

#ifndef NDEBUG

#include "SDL3/SDL_render.h"
#include "surface.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "zbuffer.h"

#include <imgui.h>

#include <cstdint>
#include <vector>


namespace
{
    SDL_Texture* Texture = nullptr;
    int LastWidth = 0;
    int LastHeight = 0;
    std::vector<uint32_t> Scratch;


    bool Ensure_Texture(int width, int height)
    {
        if (SDLWindowRenderer == nullptr || width <= 0 || height <= 0) {
            return false;
        }

        if (Texture != nullptr && LastWidth == width && LastHeight == height) {
            return true;
        }

        if (Texture != nullptr) {
            SDL_DestroyTexture(Texture);
            Texture = nullptr;
        }

        Texture = SDL_CreateTexture(SDLWindowRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (Texture == nullptr) {
            LastWidth = 0;
            LastHeight = 0;
            return false;
        }

        LastWidth = width;
        LastHeight = height;
        return true;
    }
}


void ZBufferDebugWindow::Draw()
{
    if (DepthBuffer == nullptr) {
        return;
    }

    Surface* surface = DepthBuffer->Get_Surface();
    if (surface == nullptr) {
        return;
    }

    const int width = surface->Get_Width();
    const int height = surface->Get_Height();
    if (width <= 0 || height <= 0) {
        return;
    }

    if (!Ensure_Texture(width, height)) {
        return;
    }

    /**
     *  The Z buffer is a ring buffer. SurfaceOffset moves the view origin
     *  as the camera pans (horizontal pans shift by pixel-bytes, vertical
     *  pans by row-bytes), so we walk pixels linearly from
     *  BufferStart + SurfaceOffset and wrap at BufferEnd. This mirrors
     *  ZBuffer::Copy_To in the vanilla engine.
     */
    if (surface->Lock() == nullptr) {
        return;
    }

    const unsigned int buffer_start = DepthBuffer->BufferStart;
    const unsigned int buffer_end = DepthBuffer->BufferEnd;
    const unsigned int buffer_size = DepthBuffer->BufferSize;
    const unsigned int surface_offset = static_cast<unsigned int>(DepthBuffer->SurfaceOffset);

    if (buffer_start == 0 || buffer_end <= buffer_start || buffer_size == 0) {
        surface->Unlock();
        return;
    }

    const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);

    unsigned int pos = buffer_start + surface_offset;
    if (pos >= buffer_end) pos -= buffer_size;

    unsigned short min_value = 0xFFFF;
    unsigned short max_value = 0;
    {
        unsigned int p = pos;
        for (size_t i = 0; i < count; ++i) {
            const unsigned short v = *reinterpret_cast<const unsigned short*>(p);
            if (v < min_value) min_value = v;
            if (v > max_value) max_value = v;
            p += sizeof(unsigned short);
            if (p >= buffer_end) p -= buffer_size;
        }
    }

    Scratch.resize(count);
    const unsigned range = (max_value > min_value) ? static_cast<unsigned>(max_value - min_value) : 1u;
    {
        unsigned int p = pos;
        for (size_t i = 0; i < count; ++i) {
            const unsigned short v = *reinterpret_cast<const unsigned short*>(p);
            const unsigned norm = (static_cast<unsigned>(v - min_value) * 255u) / range;
            const uint32_t g = static_cast<uint32_t>((255u - norm) & 0xFFu);
            Scratch[i] = 0xFF000000u | (g << 16) | (g << 8) | g;
            p += sizeof(unsigned short);
            if (p >= buffer_end) p -= buffer_size;
        }
    }

    surface->Unlock();

    SDL_UpdateTexture(Texture, nullptr, Scratch.data(), width * static_cast<int>(sizeof(uint32_t)));

    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width) + 16.0f, static_cast<float>(height) + 36.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Z-Buffer", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<intptr_t>(Texture)), ImVec2(static_cast<float>(width), static_cast<float>(height)));
    }
    ImGui::End();
}


void ZBufferDebugWindow::Shutdown()
{
    if (Texture != nullptr) {
        SDL_DestroyTexture(Texture);
        Texture = nullptr;
    }
    LastWidth = 0;
    LastHeight = 0;
    Scratch.clear();
    Scratch.shrink_to_fit();
}

#else

void ZBufferDebugWindow::Draw() {}
void ZBufferDebugWindow::Shutdown() {}

#endif
