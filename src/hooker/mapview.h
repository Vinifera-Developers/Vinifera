/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Map view helper to extract information from DOS executable.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


class MapViewOfFileClass
{
    public:
        explicit MapViewOfFileClass(const wchar_t *fileName);
        ~MapViewOfFileClass();

        LPVOID GetMapViewOfFile() const { return FileBase; }
        PIMAGE_DOS_HEADER GetDosHeader() const { return DosHeader; }
        PIMAGE_NT_HEADERS GetNtHeader() const { return NTHeader; }
        PIMAGE_OPTIONAL_HEADER GetOptionalHeader() const { return OptionalHeader; }
        PIMAGE_SECTION_HEADER GetSectionHeaders() const { return SectionHeaders; }
        WORD GetSectionHeaderCount() const { return NTHeader ? NTHeader->FileHeader.NumberOfSections : 0; }

    private:
        HANDLE File;
        HANDLE FileMapping;
        LPVOID FileBase;
        PIMAGE_DOS_HEADER DosHeader;
        PIMAGE_NT_HEADERS NTHeader;
        PIMAGE_OPTIONAL_HEADER OptionalHeader;
        PIMAGE_SECTION_HEADER SectionHeaders;
};


struct ImageSectionInfo
{
    LPVOID BaseOfCode;
    LPVOID BaseOfData;
    SIZE_T SizeOfCode;
    SIZE_T SizeOfData;
};

bool GetModuleSectionInfo(ImageSectionInfo &info);
