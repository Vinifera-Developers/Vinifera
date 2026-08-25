/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Function for writing PNG files from a graphic surface.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "filepng.h"

#include "asserthandler.h"
#include "bsurface.h"
#include "buff.h"
#include "ccfile.h"
#include "debughandler.h"
#include "dsurface.h"
#include "stristr.h"
#include "surface.h"

#include <lodepng.h>


#include <algorithm>
#include <wincodec.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


/**
 *  Writes the contents of a graphic surface as PNG to a file instance.
 *
 *  The encoding is done through the Windows Imaging Component PNG encoder,
 *  feeding it scanlines in small bands. WIC compresses incrementally into
 *  the output stream, so peak memory use stays at a few megabytes no matter
 *  the resolution. This matters for screenshots at very high resolutions
 *  (8K and beyond), where a 32-bit process does not have a large enough
 *  contiguous stretch of free address space left to hold even one full
 *  RGB copy of the frame.
 *
 *  @author: CCHyper, ZivDero
 */
bool Write_PNG_File(FileClass *name, Surface &pic)
{
    const int width = pic.Get_Width();
    const int height = pic.Get_Height();
    if (width <= 0 || height <= 0) {
        return false;
    }

    /**
     *  WIC is a COM component; make sure COM is up on this thread. S_FALSE
     *  (already initialized) is fine, and RPC_E_CHANGED_MODE (initialized
     *  with a different threading model) is also fine for WIC - just don't
     *  pair it with CoUninitialize in that case.
     */
    const HRESULT cohr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool com_initialized = SUCCEEDED(cohr);

    bool success = false;
    unsigned char *band = nullptr;
    const unsigned char *base = nullptr;
    HRESULT hr = S_OK;

    /**
     *  Scope the COM pointers so they are released before CoUninitialize.
     */
    {
        ComPtr<IWICImagingFactory> factory;
        ComPtr<IWICStream> stream;
        ComPtr<IWICBitmapEncoder> encoder;
        ComPtr<IWICBitmapFrameEncode> frame;
        ComPtr<IPropertyBag2> props;

        wchar_t wide_path[PATH_MAX];

        do {
            hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
            if (FAILED(hr)) break;

            /**
             *  Open the output stream.
             */
            if (MultiByteToWideChar(CP_ACP, 0, name->File_Name(), -1, wide_path, std::size(wide_path)) == 0) {
                hr = E_FAIL;
                break;
            }

            hr = factory->CreateStream(&stream);
            if (FAILED(hr)) break;

            hr = stream->InitializeFromFilename(wide_path, GENERIC_WRITE);
            if (FAILED(hr)) break;

            /**
             *  Create the PNG encoder and its single frame. Use the "Up"
             *  filter unconditionally instead of letting the encoder try
             *  all five per scanline - it is a good fit for rendered
             *  content and several times faster.
             */
            hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
            if (FAILED(hr)) break;

            hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
            if (FAILED(hr)) break;

            hr = encoder->CreateNewFrame(&frame, &props);
            if (FAILED(hr)) break;

            PROPBAG2 option = {};
            wchar_t option_name[] = L"FilterOption";
            option.pstrName = option_name;
            VARIANT filter_value = {};
            filter_value.vt = VT_UI1;
            filter_value.bVal = WICPngFilterUp;
            props->Write(1, &option, &filter_value); // Non-fatal if unsupported.

            hr = frame->Initialize(props.Get());
            if (FAILED(hr)) break;

            hr = frame->SetSize(width, height);
            if (FAILED(hr)) break;

            WICPixelFormatGUID format = GUID_WICPixelFormat24bppBGR;
            hr = frame->SetPixelFormat(&format);
            if (FAILED(hr)) break;

            if (!IsEqualGUID(format, GUID_WICPixelFormat24bppBGR)) {
                hr = WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;
                break;
            }

            /**
             *  Convert the pixel data from 16bit to 24bit and feed it to the
             *  encoder one band at a time, honouring the surface's stride
             *  (rows may be padded; SDLSurface is backed by a DWORD-aligned
             *  GDI DIB).
             *
             *  All game surfaces are RGB565 - see SDLSurface's use of
             *  SDL_PIXELFORMAT_RGB565.
             */
            const size_t rowbytes = (size_t)width * 3;
            const int band_rows = std::clamp((int)((4 * 1024 * 1024) / rowbytes), 1, height);

            band = (unsigned char *)std::malloc((size_t)band_rows * rowbytes);
            if (band == nullptr) {
                hr = E_OUTOFMEMORY;
                break;
            }

            base = (const unsigned char *)pic.Lock();
            if (base == nullptr) {
                hr = E_FAIL;
                break;
            }

            const int stride = pic.Stride();
            for (int y = 0; y < height && SUCCEEDED(hr); y += band_rows) {

                const int rows = std::min(band_rows, height - y);
                for (int row = 0; row < rows; ++row) {
                    const unsigned short *src = (const unsigned short *)(base + (size_t)(y + row) * stride);
                    unsigned char *dst = band + (size_t)row * rowbytes;
                    for (int x = 0; x < width; ++x) {
                        unsigned short value = *src++;
                        *dst++ = (unsigned char)(((value & 0x001F) * 255) / 31);         // Extract the 5 B bits.
                        *dst++ = (unsigned char)((((value & 0x07E0) >> 5) * 255) / 63);  // Extract the 6 G bits.
                        *dst++ = (unsigned char)((((value & 0xF800) >> 11) * 255) / 31); // Extract the 5 R bits.
                    }
                }

                hr = frame->WritePixels(rows, (UINT)rowbytes, (UINT)((size_t)rows * rowbytes), band);
            }
            if (FAILED(hr)) break;

            hr = frame->Commit();
            if (FAILED(hr)) break;

            hr = encoder->Commit();
            if (FAILED(hr)) break;

            success = true;

        } while (false);
    }

    if (base != nullptr) {
        pic.Unlock();
    }
    std::free(band);

    if (com_initialized) {
        CoUninitialize();
    }

    if (!success) {
        DEBUG_ERROR("Write_PNG_File() - Failed to encode \"{}\"! HRESULT = {:#010x}\n", name->File_Name(), (unsigned)hr);

        /**
         *  Don't leave a partial file behind.
         */
        if (name->Is_Available()) {
            name->Delete();
        }
    }

    return success;
}

/** 
 *  Read the contents of a PNG file into a graphic surface.
 * 
 *  @author: CCHyper
 */
BSurface *Read_PNG_File(FileClass *name, unsigned char *palette, void *buff, long size)
{
    ASSERT(name != nullptr);

    LodePNGState state;
    BSurface *pic = nullptr;

    unsigned char *png_image = nullptr;     // Output png image.
    unsigned int png_width;
    unsigned int png_height;

    unsigned char *png_buffer = nullptr;    // Raw png loaded from file.
    size_t png_buffersize;

    if (!stristr(name->File_Name(), ".png")) {
        DEBUG_ERROR("Read_PNG_File() - Invalid filename!\n");
        return false;
    }

    if (!name->Is_Available()) return nullptr;

    bool file_opened = false;
    if (!name->Is_Open()) {
        name->Open(FILE_ACCESS_READ);
        file_opened = true;
    }

    png_buffersize = name->Size();

    png_buffer = (unsigned char *)std::malloc(png_buffersize);
    if (!png_buffer) {
        DEBUG_ERROR("Read_PNG_File() - Failed to allocate PNG buffer!\n");
        return nullptr;
    }

    if (!name->Read(png_buffer, png_buffersize)) {
        DEBUG_ERROR("Read_PNG_File() - Failed to read PNG file!\n");

        //delete [] png_buffer;
        std::free(png_buffer);

        return nullptr;
    }

    lodepng_state_init(&state);

    state.info_raw.colortype = LCT_RGB;
    state.info_raw.bitdepth = 8;
    state.decoder.color_convert = false;

    /**
     *  Decode the PNG data.
     */
    unsigned error = lodepng_decode(&png_image, &png_width, &png_height, &state, png_buffer, png_buffersize);
    if (!png_image || error) {
        DEBUG_ERROR("Read_PNG_File() - Failed to decode PNG data!\n");
    
        lodepng_state_cleanup(&state);
    
        //delete [] png_buffer;
        std::free(png_buffer);
        std::free(png_image);
    
        return nullptr;
    }

    /**
     *  We only support standard 8bit PNG RGB, report error otherwise.
     */
    if (state.info_raw.bitdepth == 16
     || state.info_raw.colortype == LCT_GREY
     || state.info_raw.colortype == LCT_PALETTE
     || state.info_raw.colortype == LCT_GREY_ALPHA
     || state.info_raw.colortype == LCT_RGBA) {

        DEBUG_ERROR("Read_PNG_File() - Unsupported PNG format type!\n");

        lodepng_state_cleanup(&state);
    
        //delete [] png_buffer;
        std::free(png_buffer);
        std::free(png_image);
    
        return nullptr;
    }


#ifndef NDEBUG
    DEBUG_INFO("Read_PNG_File() - bitdepth: {}, colortype: {}.\n",
        state.info_raw.bitdepth, (int)state.info_raw.colortype);
#endif

    if (buff) {
        Buffer b(buff, size);
        pic = new BSurface(png_width, png_height, 2, b);
    } else {
        pic = new BSurface(png_width, png_height, 2);
    }
    ASSERT(pic != nullptr);

    //size_t buffersize = lodepng_get_raw_size(png_width, png_height, &state.info_raw);
    //ASSERT(buffersize == (png_width * png_height));

    /**
     *  Copy the decoded PNG data into the image surface.
     */
    for (int y = 0; y < pic->Get_Height(); ++y) {
    
        unsigned short *buffptr = (unsigned short *)pic->Lock(Point2D(0, y));
        for (int x = 0; x < pic->Get_Width(); ++x) {
    
            int r = *png_image++; // & 0xFF;
            int g = *png_image++; // & 0xFF;
            int b = *png_image++; // & 0xFF;
    
            *buffptr++ = DSurface::Build_Hicolor_Pixel(r, g, b);
        }
    
        pic->Unlock();
    }

    std::free(png_buffer);

    lodepng_state_cleanup(&state);

    if (file_opened) {
        name->Close();
    }

    return pic;
}


/** 
 *  Read the contents of a PNG file into a graphic surface.
 * 
 *  @author: CCHyper
 */
BSurface *Read_PNG_File(FileClass *name, const Buffer &buff, PaletteClass *palette)
{
    return Read_PNG_File(name, (unsigned char *)palette, buff.Get_Buffer(), buff.Get_Size());
}
