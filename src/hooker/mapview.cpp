/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Map view helper to extract information from DOS executable.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "mapview.h"

#include <iterator>


MapViewOfFileClass::MapViewOfFileClass(const wchar_t *fileName) :
    File(INVALID_HANDLE_VALUE),
    FileMapping(NULL),
    FileBase(NULL),
    DosHeader(NULL),
    NTHeader(NULL),
    OptionalHeader(NULL),
    SectionHeaders(NULL)
{
    File = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (File != INVALID_HANDLE_VALUE) {
        FileMapping = CreateFileMapping(File, NULL, PAGE_READONLY, 0, 0, NULL);

        if (FileMapping != NULL) {
            FileBase = MapViewOfFile(FileMapping, FILE_MAP_READ, 0, 0, 0);

            if (FileBase != NULL) {
                DosHeader = (PIMAGE_DOS_HEADER)FileBase;

                if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
                    NTHeader = (PIMAGE_NT_HEADERS)((uint8_t *)DosHeader + DosHeader->e_lfanew);

                    if (NTHeader->Signature == IMAGE_NT_SIGNATURE) {
                        OptionalHeader = (PIMAGE_OPTIONAL_HEADER)&NTHeader->OptionalHeader;
                        SectionHeaders = IMAGE_FIRST_SECTION(NTHeader);
                    }
                }
            }
        }
    }
}


MapViewOfFileClass::~MapViewOfFileClass()
{
    if (FileBase != NULL)
        UnmapViewOfFile(FileBase);
    if (FileMapping != NULL)
        CloseHandle(FileMapping);
    if (File != INVALID_HANDLE_VALUE)
        CloseHandle(File);
}


bool GetModuleSectionInfo(ImageSectionInfo &info)
{
    wchar_t fileName[MAX_PATH] = { 0 };

    if (GetModuleFileNameW(NULL, fileName, std::size(fileName)) != 0) {
        MapViewOfFileClass mapView(fileName);
        PIMAGE_OPTIONAL_HEADER OptionalHeader = mapView.GetOptionalHeader();

        if (OptionalHeader != NULL) {
            info.BaseOfCode = LPVOID(OptionalHeader->ImageBase + OptionalHeader->BaseOfCode);
            info.BaseOfData = LPVOID(OptionalHeader->ImageBase + OptionalHeader->BaseOfData);
            info.SizeOfCode = SIZE_T(OptionalHeader->SizeOfCode);
            info.SizeOfData = SIZE_T(OptionalHeader->SizeOfInitializedData + OptionalHeader->SizeOfUninitializedData);

            return true;
        }
    }
    return false;
}
