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

#include <cstring>
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
    info = {};

    HMODULE module = GetModuleHandleW(NULL);
    if (module != NULL) {
        auto module_base = reinterpret_cast<uintptr_t>(module);
        auto DosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);

        if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
            auto NTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + DosHeader->e_lfanew);

            if (NTHeader->Signature == IMAGE_NT_SIGNATURE) {
                PIMAGE_SECTION_HEADER section_headers = IMAGE_FIRST_SECTION(NTHeader);

                for (WORD index = 0; index < NTHeader->FileHeader.NumberOfSections; ++index) {
                    const IMAGE_SECTION_HEADER &section = section_headers[index];
                    const DWORD section_size = section.Misc.VirtualSize != 0 ? section.Misc.VirtualSize : section.SizeOfRawData;
                    const DWORD section_content_flags = IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA;
                    const DWORD section_memory_flags = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

                    if (section_size == 0) {
                        continue;
                    }

                    if ((section.Characteristics & section_content_flags) == 0 || (section.Characteristics & section_memory_flags) == 0) {
                        continue;
                    }

                    if (info.SectionCount >= MAX_MODULE_SECTIONS) {
                        return false;
                    }

                    ImageSectionRange &range = info.Sections[info.SectionCount++];
                    range.Base = reinterpret_cast<LPVOID>(module_base + section.VirtualAddress);
                    range.Size = SIZE_T(section_size);
                    range.Characteristics = section.Characteristics;
                    std::memcpy(range.Name, section.Name, IMAGE_SIZEOF_SHORT_NAME);
                    range.Name[IMAGE_SIZEOF_SHORT_NAME] = '\0';
                }

                return info.SectionCount > 0;
            }
        }
    }

    return false;
}
