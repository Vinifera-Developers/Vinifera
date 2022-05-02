/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MAPVIEW.CPP
 *
 *  @author        xezon
 *
 *  @brief         Map view helper to extract information from DOS executable.
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
 *  @note          This file contains modified code from the source code of the
 *                 Thyme project released under the GPL3 license. Source:
 *                 https://github.com/TheAssemblyArmada/Thyme/
 *
 ******************************************************************************/
#pragma once

#include "always.h"


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
