/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Simple stdio wrappers for Miniaudio�s read/seek/tell procs that are
 *          required for the custom audio decoder.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_io.h"

#include "audio_debug.h"
#include "ccfile.h"

#include <codecvt>
#include <locale>


/**
 *  Helper function to convert wchar to char.
 */
static std::string wchar_to_utf8(const std::wstring & wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}


/**
 *  MiniAudio callback functions that use the C&C engine file io.
 * 
 *  Read:
 *    Function used to read data from memory.
 *    
 *    ptr = Pointer to the buffer that the vorbis files need.
 *    size = How big a byte is.
 *    count = How much we should read.
 *    datasource = This is a pointer to the data we passed into ov_open_callbacks.
 * 
 *  Seek:
 *    Function used to seek to a specific part of the file in memory.
 * 
 *    datasource = This is a pointer to the data we passed into ov_open_callbacks.
 *    offset = The offset from the point we wish to seek to.
 *    origin = Where we want to seek to.
 * 
 *  Close:
 *    Function used to close the file in memory.
 * 
 *    datasource = This is a pointer to the data we passed into ov_open_callbacks.
 * 
 *  Tell:
 *    Function used to tell how much we have read so far.
 * 
 *    datasource = This is a pointer to the data we passed into ov_open_callbacks.
 */

static ma_result Audio_CCFile_onOpen(ma_vfs *pVFS, const char *pFilePath, ma_uint32 openMode, ma_vfs_file *pFile)
{
    (void)pVFS;

    if (pFile == nullptr) {
        return MA_INVALID_ARGS;
    }

    CCFileClass * hFile = new CCFileClass(pFilePath);
    if (hFile == nullptr) {
        return MA_OUT_OF_MEMORY;
    }

    if (!hFile->Is_Available()) {
        return MA_DOES_NOT_EXIST;
    }

    bool opened = false;

    if ((openMode & MA_OPEN_MODE_READ) != 0) {
        opened = hFile->Open(FILE_ACCESS_READ);

    } else if ((openMode & MA_OPEN_MODE_WRITE) != 0) {
        opened = hFile->Open(FILE_ACCESS_WRITE);
    }

    if (!opened || !hFile->Is_Open()) {
        return MA_DOES_NOT_EXIST;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_IO, "AudioIO: onOpen -> %s\n", hFile->File_Name());

    if (pFile != nullptr) {
        *pFile = hFile;
    }

    return MA_SUCCESS;
}

static ma_result Audio_CCFile_onOpenW(ma_vfs *pVFS, const wchar_t *pFilePath, ma_uint32 openMode, ma_vfs_file *pFile)
{
    (void)pVFS;

    if (pFile == nullptr) {
        return MA_INVALID_ARGS;
    }

    // Convert the input path to UTF8 as CCFileClass does not support wide strings.
    std::string _filePath = wchar_to_utf8(std::wstring(pFilePath));

    CCFileClass * hFile = new CCFileClass(_filePath.c_str());
    if (hFile == nullptr) {
        return MA_OUT_OF_MEMORY;
    }

    if (!hFile->Is_Available()) {
        return MA_DOES_NOT_EXIST;
    }

    bool opened = false;

    if ((openMode & MA_OPEN_MODE_READ) != 0) {
        opened = hFile->Open(FILE_ACCESS_READ);

    } else if ((openMode & MA_OPEN_MODE_WRITE) != 0) {
        opened = hFile->Open(FILE_ACCESS_WRITE);
    }

    if (!opened || !hFile->Is_Open()) {
        return MA_DOES_NOT_EXIST;
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_IO, "AudioIO: onOpenW -> %s\n", hFile->File_Name());

    if (pFile != nullptr) {
        *pFile = hFile;
    }

    return MA_SUCCESS;
}

static ma_result Audio_CCFile_onClose(ma_vfs *pVFS, ma_vfs_file file)
{
    (void)pVFS;

    if (file == nullptr) {
        return MA_INVALID_ARGS;
    }

    CCFileClass * hFile = reinterpret_cast<CCFileClass *>(file);

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_IO, "AudioIO: onClose -> %s\n", hFile->File_Name());

    if (hFile != nullptr) {
        if (hFile->Is_Open()) {
            hFile->Close();
        }
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_IO, "AudioIO: Deleting file object at %p (%s)\n", hFile, hFile->File_Name());
        delete hFile;
    }

    return MA_SUCCESS;
}

static ma_result Audio_CCFile_onRead(ma_vfs *pVFS, ma_vfs_file file, void *pDst, size_t sizeInBytes, size_t *pBytesRead)
{
    (void)pVFS;

    if (file == nullptr || pDst == nullptr || sizeInBytes <= 0 || pBytesRead == nullptr) {
        return MA_INVALID_ARGS;
    }

    CCFileClass * hFile = reinterpret_cast<CCFileClass *>(file);

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_IO, "AudioIO: onRead -> %s, Read %zu bytes\n", hFile->File_Name(), sizeInBytes);

    long totalBytesRead = hFile->Read(pDst, sizeInBytes);
    if (totalBytesRead == 0) {
        *pBytesRead = 0; // #BUGFIX: This will be fixed upstream in Miniaudio soon?
        return MA_AT_END;
    }

    if (pBytesRead != nullptr) {
        *pBytesRead = totalBytesRead;
    }

    return MA_SUCCESS;
}

static ma_result Audio_CCFile_onWrite(ma_vfs *pVFS, ma_vfs_file file, const void *pSrc, size_t sizeInBytes, size_t *pBytesWritten)
{
    // We don't need to support writing files out, so just return MA_NOT_IMPLEMENTED to flag an error.
    return MA_NOT_IMPLEMENTED;

#if 0
    (void)pVFS;

    if (file == nullptr || pSrc == nullptr || sizeInBytes <= 0 || pBytesWritten == nullptr) {
        return MA_INVALID_ARGS;
    }

    CCFileClass * hFile = reinterpret_cast<CCFileClass *>(file);

    long totalBytesWritten = hFile->Write(pSrc, sizeInBytes);
    if (totalBytesWritten == 0) {
        *pBytesWritten = 0; // #BUGFIX: This will be fixed upstream in Miniaudio soon?
        return MA_AT_END;
    }

    if (pBytesWritten != nullptr) {
        *pBytesWritten = totalBytesWritten;
    }

    return MA_SUCCESS;
#endif
}

static ma_result Audio_CCFile_onSeek(ma_vfs *pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin)
{
    (void)pVFS;

    if (file == nullptr) {
        return MA_INVALID_ARGS;
    }

    CCFileClass * hFile = reinterpret_cast<CCFileClass *>(file);

    off_t off = (uint32_t)(offset);
    off_t retoff = 0;

    switch (origin) {
        default:
        case ma_seek_origin_start:
            retoff = hFile->Seek(off, FILE_SEEK_START);
            break;

        case ma_seek_origin_current:
            retoff = hFile->Seek(off, FILE_SEEK_CURRENT);
            break;

        case ma_seek_origin_end:
            retoff = hFile->Seek(off, FILE_SEEK_END);
            break;
    };

    if (retoff < 0) {
        return MA_BAD_SEEK;
    }

    return MA_SUCCESS;
}

static ma_result Audio_CCFile_onTell(ma_vfs *pVFS, ma_vfs_file file, ma_int64 *pCursor)
{
    (void)pVFS;

    if (file == nullptr) {
        return MA_INVALID_ARGS;
    }

    CCFileClass * hFile = reinterpret_cast<CCFileClass *>(file);

    if (pCursor != nullptr) {
        *pCursor = hFile->Tell();
    }

    return MA_SUCCESS;
}

static ma_result Audio_CCFile_onInfo(ma_vfs *pVFS, ma_vfs_file file, ma_file_info *pInfo)
{
    (void)pVFS;

    if (file == nullptr) {
        return MA_INVALID_ARGS;
    }

    CCFileClass * hFile = reinterpret_cast<CCFileClass *>(file);

    if (pInfo != nullptr) {
        pInfo->sizeInBytes = hFile->Size();
    }

    return MA_SUCCESS;
}


/**
 *  Vtable for the custom VFS that uses the CCFileClass IO.
 * 
 *  NOTE: This must be the last in the file as the functions are local to this source file!
 */
/*static*/ ma_vfs_callbacks ma_custom_vfs_callbacks = {
    Audio_CCFile_onOpen,
    Audio_CCFile_onOpenW,
    Audio_CCFile_onClose,
    Audio_CCFile_onRead,
    Audio_CCFile_onWrite,    // Not required, we don't needs to write audio files.
    Audio_CCFile_onSeek,
    Audio_CCFile_onTell,
    Audio_CCFile_onInfo
};
