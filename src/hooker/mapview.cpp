/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MAPVIEW.H
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
#include "mapview.h"


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

    if (GetModuleFileNameW(NULL, fileName, ARRAY_SIZE(fileName)) != 0) {
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
